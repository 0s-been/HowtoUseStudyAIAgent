#include "SpriteRenderer.h"
#include "GameObject.h"
#include "TextureManager.h"
#include "ComponentFactory.h"
#include "Core/Graphics.h"

REGISTER_COMPONENT(SpriteRenderer)

void SpriteRenderer::SetTexturePath(const std::string& path)
{
    m_texturePath = path;
    m_texture = nullptr; // 다음 Start/Render 시점에 다시 로드
}

void SpriteRenderer::Start()
{
    if (!m_texturePath.empty() && !m_texture)
        m_texture = TextureManager::Instance().LoadTexture(m_texturePath);
}

void SpriteRenderer::Render()
{
    if (!m_texture && !m_texturePath.empty())
        m_texture = TextureManager::Instance().LoadTexture(m_texturePath);

    if (!m_texture || !m_owner)
        return;

    Graphics* graphics = Graphics::Get();
    if (!graphics)
        return;

    XMMATRIX world = m_owner->GetTransform()->GetWorldMatrix();
    graphics->DrawSprite(m_texture->GetSRV(), world,
        static_cast<float>(m_texture->GetWidth()), static_cast<float>(m_texture->GetHeight()), m_tint);
}

nlohmann::json SpriteRenderer::ToJson() const
{
    nlohmann::json j = Component::ToJson();
    j["texturePath"] = m_texturePath;
    j["tint"] = { m_tint.x, m_tint.y, m_tint.z, m_tint.w };
    return j;
}

void SpriteRenderer::FromJson(const nlohmann::json& json)
{
    Component::FromJson(json);
    if (json.contains("texturePath"))
        m_texturePath = json.at("texturePath").get<std::string>();
    if (json.contains("tint"))
    {
        const auto& t = json.at("tint");
        m_tint = XMFLOAT4(t[0].get<float>(), t[1].get<float>(), t[2].get<float>(), t[3].get<float>());
    }
}
