#pragma once

#include "Common.h"
#include "Engine/Shader.h"

// DirectX 11 디바이스/스왑체인 초기화와 렌더 루프의 기본 로직,
// 그리고 스프라이트(2D 텍스처 사각형)를 그리기 위한 공용 파이프라인을 담당한다.
class Graphics
{
public:
    static Graphics* Get() { return s_instance; }

    bool Initialize(HWND hwnd, int width, int height);
    void Shutdown();

    void BeginFrame();       // 파란색으로 클리어
    void EndFrame();         // Present

    ID3D11Device* GetDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_context.Get(); }

    // world: 스프라이트의 월드 변환 행렬(텍스처 픽셀 크기 스케일 포함 전)
    // texWidth/texHeight: 텍스처 원본 크기(px) - 쿼드에 곱해져 실제 크기로 렌더링된다.
    void DrawSprite(ID3D11ShaderResourceView* srv, const XMMATRIX& world, float texWidth, float texHeight, const XMFLOAT4& tint);

private:
    void CreateSpriteQuad();
    void CreateSpritePipelineStates();
    void LoadSpriteShaders();

    static Graphics* s_instance;

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_rtv;

    int m_width = 0;
    int m_height = 0;

    ComPtr<ID3D11Buffer> m_quadVB;
    ComPtr<ID3D11Buffer> m_quadIB;
    ComPtr<ID3D11Buffer> m_spriteCB;
    ComPtr<ID3D11SamplerState> m_sampler;
    ComPtr<ID3D11BlendState> m_blendState;
    ComPtr<ID3D11RasterizerState> m_rasterState;

    Shader m_spriteVertexShader;
    Shader m_spritePixelShader;

    XMMATRIX m_viewMatrix = XMMatrixIdentity();
    XMMATRIX m_projMatrix = XMMatrixIdentity();
};
