#include "Texture.h"
#include <wincodec.h>
#include <vector>

#pragma comment(lib, "windowscodecs.lib")

bool Texture::LoadFromFile(ID3D11Device* device, const std::wstring& path)
{
    ComPtr<IWICImagingFactory> wicFactory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(wicFactory.GetAddressOf()));
    if (FAILED(hr))
        return false;

    ComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromFilename(
        path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf());
    if (FAILED(hr))
        return false;

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, frame.GetAddressOf());
    if (FAILED(hr))
        return false;

    ComPtr<IWICFormatConverter> converter;
    wicFactory->CreateFormatConverter(converter.GetAddressOf());
    hr = converter->Initialize(
        frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone,
        nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr))
        return false;

    UINT width = 0, height = 0;
    converter->GetSize(&width, &height);

    const UINT rowPitch = width * 4;
    std::vector<uint8_t> pixels(static_cast<size_t>(rowPitch) * height);
    hr = converter->CopyPixels(nullptr, rowPitch, static_cast<UINT>(pixels.size()), pixels.data());
    if (FAILED(hr))
        return false;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = rowPitch;

    ComPtr<ID3D11Texture2D> texture;
    hr = device->CreateTexture2D(&desc, &initData, texture.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = device->CreateShaderResourceView(texture.Get(), nullptr, m_srv.GetAddressOf());
    if (FAILED(hr))
        return false;

    m_width = static_cast<int>(width);
    m_height = static_cast<int>(height);
    return true;
}
