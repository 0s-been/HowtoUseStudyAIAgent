#include "Shader.h"
#include <fstream>

namespace
{
    std::string ToNarrow(const std::wstring& wide)
    {
        if (wide.empty()) return {};
        int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string result(size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, result.data(), size, nullptr, nullptr);
        if (!result.empty() && result.back() == '\0') result.pop_back();
        return result;
    }

    std::wstring ToWide(const std::string& narrow)
    {
        if (narrow.empty()) return {};
        int size = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, nullptr, 0);
        std::wstring result(size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, result.data(), size);
        if (!result.empty() && result.back() == L'\0') result.pop_back();
        return result;
    }
}

FILETIME Shader::GetHlslWriteTime() const
{
    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (GetFileAttributesExW(m_hlslPath.c_str(), GetFileExInfoStandard, &data))
        return data.ftLastWriteTime;
    return FILETIME{};
}

bool Shader::CompileToBlobDirect(ComPtr<ID3DBlob>& outBlob) const
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        m_hlslPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        m_entryPoint.c_str(), m_target.c_str(), flags, 0,
        outBlob.GetAddressOf(), errorBlob.GetAddressOf());

    if (FAILED(hr))
    {
        std::string msg = "셰이더 컴파일 실패: " + ToNarrow(m_hlslPath) + "\n";
        if (errorBlob)
            msg += static_cast<const char*>(errorBlob->GetBufferPointer());
        MessageBoxA(nullptr, msg.c_str(), "Shader Compile Error", MB_OK | MB_ICONERROR);
        return false;
    }
    return true;
}

bool Shader::CompileAndCache(ComPtr<ID3DBlob>& outBlob) const
{
    if (!CompileToBlobDirect(outBlob))
        return false;

    std::wstring csoPathW = ToWide(m_csoPath);
    D3DWriteBlobToFile(outBlob.Get(), csoPathW.c_str(), TRUE);
    return true;
}

bool Shader::LoadCompiledBlob(ComPtr<ID3DBlob>& outBlob) const
{
    std::wstring csoPathW = ToWide(m_csoPath);
    return SUCCEEDED(D3DReadFileToBlob(csoPathW.c_str(), outBlob.GetAddressOf()));
}

bool Shader::IsCsoUpToDate() const
{
    WIN32_FILE_ATTRIBUTE_DATA csoData{};
    if (!GetFileAttributesExW(ToWide(m_csoPath).c_str(), GetFileExInfoStandard, &csoData))
        return false; // .cso 없음

    FILETIME hlslTime = GetHlslWriteTime();
    return CompareFileTime(&csoData.ftLastWriteTime, &hlslTime) >= 0;
}

bool Shader::LoadVertexShader(
    ID3D11Device* device,
    const std::wstring& hlslPath,
    const std::string& csoPath,
    const std::string& entryPoint,
    const std::string& target,
    const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
    UINT layoutCount)
{
    m_device = device;
    m_hlslPath = hlslPath;
    m_csoPath = csoPath;
    m_entryPoint = entryPoint;
    m_target = target;
    m_isVertex = true;
    m_layoutDesc.assign(layoutDesc, layoutDesc + layoutCount);

    ComPtr<ID3DBlob> blob;

#ifdef HOT_RELOAD_ENABLED
    // 개발 모드: HLSL -> Compile -> Blob 을 즉시 CreateVertexShader 로 전달.
    if (!CompileToBlobDirect(blob))
        return false;
#else
    // 출시 모드: HLSL -> Compile -> .cso 저장 -> .cso 로드 -> CreateVertexShader.
    if (IsCsoUpToDate() && LoadCompiledBlob(blob))
    {
        // 캐시 그대로 사용
    }
    else if (!CompileAndCache(blob))
    {
        return false;
    }
#endif

    if (FAILED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_vs.ReleaseAndGetAddressOf())))
        return false;

    if (FAILED(device->CreateInputLayout(m_layoutDesc.data(), static_cast<UINT>(m_layoutDesc.size()),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_layout.ReleaseAndGetAddressOf())))
        return false;

    m_lastWriteTime = GetHlslWriteTime();
    return true;
}

bool Shader::LoadPixelShader(
    ID3D11Device* device,
    const std::wstring& hlslPath,
    const std::string& csoPath,
    const std::string& entryPoint,
    const std::string& target)
{
    m_device = device;
    m_hlslPath = hlslPath;
    m_csoPath = csoPath;
    m_entryPoint = entryPoint;
    m_target = target;
    m_isVertex = false;

    ComPtr<ID3DBlob> blob;

#ifdef HOT_RELOAD_ENABLED
    if (!CompileToBlobDirect(blob))
        return false;
#else
    if (IsCsoUpToDate() && LoadCompiledBlob(blob))
    {
        // 캐시 그대로 사용
    }
    else if (!CompileAndCache(blob))
    {
        return false;
    }
#endif

    if (FAILED(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_ps.ReleaseAndGetAddressOf())))
        return false;

    m_lastWriteTime = GetHlslWriteTime();
    return true;
}

bool Shader::ReloadIfChanged()
{
#ifdef HOT_RELOAD_ENABLED
    if (!m_device || m_hlslPath.empty())
        return false;

    FILETIME current = GetHlslWriteTime();
    if (CompareFileTime(&current, &m_lastWriteTime) <= 0)
        return false; // 변경 없음

    ComPtr<ID3DBlob> blob;
    if (!CompileToBlobDirect(blob))
        return false; // 컴파일 실패 시 기존 셰이더를 그대로 유지

    if (m_isVertex)
    {
        ComPtr<ID3D11VertexShader> newVs;
        ComPtr<ID3D11InputLayout> newLayout;
        if (FAILED(m_device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, newVs.GetAddressOf())))
            return false;
        if (FAILED(m_device->CreateInputLayout(m_layoutDesc.data(), static_cast<UINT>(m_layoutDesc.size()),
            blob->GetBufferPointer(), blob->GetBufferSize(), newLayout.GetAddressOf())))
            return false;
        m_vs = newVs;
        m_layout = newLayout;
    }
    else
    {
        ComPtr<ID3D11PixelShader> newPs;
        if (FAILED(m_device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, newPs.GetAddressOf())))
            return false;
        m_ps = newPs;
    }

    m_lastWriteTime = current;
    return true;
#else
    return false;
#endif
}
