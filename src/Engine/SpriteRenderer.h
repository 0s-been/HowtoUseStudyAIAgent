#pragma once

#include "Component.h"
#include "Texture.h"

// GameObject 에 부착해서 2D 텍스처를 화면에 그리는 컴포넌트.
// 실제 정점/인덱스 버퍼는 Graphics 가 공유하는 단위 quad 를 사용하고,
// 이 컴포넌트는 어떤 텍스처를 어떤 Transform 으로 그릴지만 담당한다.
class SpriteRenderer : public Component
{
public:
    void SetTexturePath(const std::string& path);
    const std::string& GetTexturePath() const { return m_texturePath; }

    void SetTint(const XMFLOAT4& tint) { m_tint = tint; }
    const XMFLOAT4& GetTint() const { return m_tint; }

    void Start() override;
    void Render() override;

    std::string GetTypeName() const override { return "SpriteRenderer"; }

    nlohmann::json ToJson() const override;
    void FromJson(const nlohmann::json& json) override;

private:
    std::string m_texturePath;
    std::shared_ptr<Texture> m_texture;
    XMFLOAT4 m_tint{ 1.0f, 1.0f, 1.0f, 1.0f };
};
