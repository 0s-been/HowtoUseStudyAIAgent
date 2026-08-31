#pragma once

#include "Core/Common.h"

// WIC 로 로드한 이미지 데이터를 GPU 텍스처 + SRV 로 감싸는 클래스.
class Texture
{
public:
    bool LoadFromFile(ID3D11Device* device, const std::wstring& path);

    ID3D11ShaderResourceView* GetSRV() const { return m_srv.Get(); }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    ComPtr<ID3D11ShaderResourceView> m_srv;
    int m_width = 0;
    int m_height = 0;
};
