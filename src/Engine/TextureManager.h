#pragma once

#include "Texture.h"

// 텍스처 경로별로 로드 결과를 캐싱해서 재사용하는 싱글톤.
class TextureManager
{
public:
    static TextureManager& Instance();

    void Init(ID3D11Device* device);

    // 이미 로드된 적이 있으면 캐시를 반환하고, 없으면 새로 로드한다.
    std::shared_ptr<Texture> LoadTexture(const std::wstring& path);
    std::shared_ptr<Texture> LoadTexture(const std::string& path);

private:
    TextureManager() = default;

    ID3D11Device* m_device = nullptr;
    std::unordered_map<std::wstring, std::shared_ptr<Texture>> m_cache;
};
