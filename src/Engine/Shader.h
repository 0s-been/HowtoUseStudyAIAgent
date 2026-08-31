#pragma once

#include "Core/Common.h"
#include "Core/EngineConfig.h"
#include <d3dcompiler.h>

// 하나의 셰이더 스테이지(Vertex 또는 Pixel)를 표현한다.
//
// 로딩 경로는 두 가지다.
//  1) HOT_RELOAD_ENABLED 가 켜져 있으면: .hlsl 을 즉시 컴파일해서 나온 Blob 을
//     바로 CreateVertexShader/CreatePixelShader 에 넘긴다. (개발용, 파일 변경 감지 대상)
//  2) 꺼져 있으면: .cso 캐시가 있고 .hlsl 보다 최신이면 그대로 로드하고,
//     없거나 오래됐으면 컴파일 후 .cso 로 저장(캐시)한 뒤 사용한다. (출시용)
class Shader
{
public:
    bool LoadVertexShader(
        ID3D11Device* device,
        const std::wstring& hlslPath,
        const std::string& csoPath,
        const std::string& entryPoint,
        const std::string& target,
        const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
        UINT layoutCount);

    bool LoadPixelShader(
        ID3D11Device* device,
        const std::wstring& hlslPath,
        const std::string& csoPath,
        const std::string& entryPoint,
        const std::string& target);

    // .hlsl 파일의 수정 시각을 확인해서 바뀌었으면 다시 컴파일 후 교체한다.
    // HOT_RELOAD_ENABLED 가 꺼져 있으면 항상 false.
    bool ReloadIfChanged();

    ID3D11VertexShader* GetVertexShader() const { return m_vs.Get(); }
    ID3D11PixelShader* GetPixelShader() const { return m_ps.Get(); }
    ID3D11InputLayout* GetInputLayout() const { return m_layout.Get(); }

private:
    bool CompileToBlobDirect(ComPtr<ID3DBlob>& outBlob) const;
    bool CompileAndCache(ComPtr<ID3DBlob>& outBlob) const;
    bool LoadCompiledBlob(ComPtr<ID3DBlob>& outBlob) const;
    FILETIME GetHlslWriteTime() const;
    bool IsCsoUpToDate() const;

    ID3D11Device* m_device = nullptr;
    std::wstring m_hlslPath;
    std::string m_csoPath;
    std::string m_entryPoint;
    std::string m_target;
    bool m_isVertex = false;

    std::vector<D3D11_INPUT_ELEMENT_DESC> m_layoutDesc; // Reload 시 InputLayout 재생성용

    FILETIME m_lastWriteTime{};

    ComPtr<ID3D11VertexShader> m_vs;
    ComPtr<ID3D11PixelShader> m_ps;
    ComPtr<ID3D11InputLayout> m_layout;
};
