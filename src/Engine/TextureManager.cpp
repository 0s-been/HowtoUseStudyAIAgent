#include "TextureManager.h"

TextureManager& TextureManager::Instance()
{
    static TextureManager instance;
    return instance;
}

void TextureManager::Init(ID3D11Device* device)
{
    m_device = device;
}

std::shared_ptr<Texture> TextureManager::LoadTexture(const std::wstring& path)
{
    auto it = m_cache.find(path);
    if (it != m_cache.end())
        return it->second;

    auto texture = std::make_shared<Texture>();
    if (!texture->LoadFromFile(m_device, path))
        return nullptr;

    m_cache[path] = texture;
    return texture;
}

std::shared_ptr<Texture> TextureManager::LoadTexture(const std::string& path)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
    std::wstring wide(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wide.data(), size);
    if (!wide.empty() && wide.back() == L'\0')
        wide.pop_back();
    return LoadTexture(wide);
}
