#include "Graphics.h"
#include "Engine/ShaderManager.h"
#include <cstring>

Graphics* Graphics::s_instance = nullptr;

namespace
{
    struct SpriteVertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    struct SpriteConstantBuffer
    {
        XMMATRIX worldViewProj;
        XMFLOAT4 tint;
    };
}

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
    s_instance = this;
    m_width = width;
    m_height = height;

    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        &featureLevel, 1, D3D11_SDK_VERSION, &scd,
        m_swapChain.GetAddressOf(), m_device.GetAddressOf(), nullptr, m_context.GetAddressOf());
    if (FAILED(hr))
        return false;

    ComPtr<ID3D11Texture2D> backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.GetAddressOf());

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &viewport);

    // 화면 중심이 원점(0,0)인 2D 정사영 카메라.
    m_viewMatrix = XMMatrixIdentity();
    m_projMatrix = XMMatrixOrthographicLH(static_cast<float>(width), static_cast<float>(height), 0.1f, 100.0f);

    CreateSpriteQuad();
    CreateSpritePipelineStates();
    LoadSpriteShaders();

    return true;
}

void Graphics::Shutdown()
{
    if (s_instance == this)
        s_instance = nullptr;
}

void Graphics::CreateSpriteQuad()
{
    // 중심이 원점인 단위 사각형. SpriteRenderer 가 텍스처 크기만큼 스케일링해서 사용한다.
    SpriteVertex vertices[] = {
        { { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } },
        { {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } },
    };
    uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

    D3D11_BUFFER_DESC vbDesc{};
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData{ vertices };
    m_device->CreateBuffer(&vbDesc, &vbData, m_quadVB.GetAddressOf());

    D3D11_BUFFER_DESC ibDesc{};
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA ibData{ indices };
    m_device->CreateBuffer(&ibDesc, &ibData, m_quadIB.GetAddressOf());

    D3D11_BUFFER_DESC cbDesc{};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(SpriteConstantBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_device->CreateBuffer(&cbDesc, nullptr, m_spriteCB.GetAddressOf());
}

void Graphics::CreateSpritePipelineStates()
{
    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    m_device->CreateSamplerState(&sampDesc, m_sampler.GetAddressOf());

    D3D11_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    m_device->CreateBlendState(&blendDesc, m_blendState.GetAddressOf());

    D3D11_RASTERIZER_DESC rastDesc{};
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_NONE;
    m_device->CreateRasterizerState(&rastDesc, m_rasterState.GetAddressOf());
}

void Graphics::LoadSpriteShaders()
{
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    m_spriteVertexShader.LoadVertexShader(
        m_device.Get(), L"Shaders/SpriteVS.hlsl", "Shaders/SpriteVS.cso",
        "main", "vs_5_0", layout, ARRAYSIZE(layout));

    m_spritePixelShader.LoadPixelShader(
        m_device.Get(), L"Shaders/SpritePS.hlsl", "Shaders/SpritePS.cso",
        "main", "ps_5_0");

    ShaderManager::Instance().Register(&m_spriteVertexShader);
    ShaderManager::Instance().Register(&m_spritePixelShader);
}

void Graphics::BeginFrame()
{
    const float blue[4] = { 0.0f, 0.35f, 0.75f, 1.0f };
    m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), nullptr);
    m_context->ClearRenderTargetView(m_rtv.Get(), blue);
    m_context->RSSetState(m_rasterState.Get());
}

void Graphics::EndFrame()
{
    m_swapChain->Present(1, 0);
}

void Graphics::DrawSprite(ID3D11ShaderResourceView* srv, const XMMATRIX& world, float texWidth, float texHeight, const XMFLOAT4& tint)
{
    if (!srv)
        return;

    XMMATRIX scaledWorld = XMMatrixScaling(texWidth, texHeight, 1.0f) * world;
    XMMATRIX wvp = scaledWorld * m_viewMatrix * m_projMatrix;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (SUCCEEDED(m_context->Map(m_spriteCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        SpriteConstantBuffer cb;
        cb.worldViewProj = XMMatrixTranspose(wvp);
        cb.tint = tint;
        std::memcpy(mapped.pData, &cb, sizeof(cb));
        m_context->Unmap(m_spriteCB.Get(), 0);
    }

    UINT stride = sizeof(SpriteVertex);
    UINT offset = 0;
    m_context->IASetInputLayout(m_spriteVertexShader.GetInputLayout());
    m_context->IASetVertexBuffers(0, 1, m_quadVB.GetAddressOf(), &stride, &offset);
    m_context->IASetIndexBuffer(m_quadIB.Get(), DXGI_FORMAT_R16_UINT, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_context->VSSetShader(m_spriteVertexShader.GetVertexShader(), nullptr, 0);
    m_context->VSSetConstantBuffers(0, 1, m_spriteCB.GetAddressOf());
    m_context->PSSetShader(m_spritePixelShader.GetPixelShader(), nullptr, 0);
    m_context->PSSetConstantBuffers(0, 1, m_spriteCB.GetAddressOf());
    m_context->PSSetShaderResources(0, 1, &srv);
    m_context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
    m_context->OMSetBlendState(m_blendState.Get(), nullptr, 0xFFFFFFFF);

    m_context->DrawIndexed(6, 0, 0);
}
