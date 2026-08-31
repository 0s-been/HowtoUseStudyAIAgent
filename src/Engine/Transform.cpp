#include "Transform.h"
#include "GameObject.h"

Transform::Transform()
{
    XMStoreFloat4x4(&m_worldMatrixCache, XMMatrixIdentity());
}

void Transform::SetLocalPosition(const XMFLOAT3& position)
{
    m_localPosition = position;
    MarkDirty();
}

void Transform::SetLocalRotation(const XMFLOAT3& eulerDegrees)
{
    m_localRotation = eulerDegrees;
    MarkDirty();
}

void Transform::SetLocalScale(const XMFLOAT3& scale)
{
    m_localScale = scale;
    MarkDirty();
}

XMMATRIX Transform::GetLocalMatrix() const
{
    const float toRad = XM_PI / 180.0f;
    XMMATRIX scale = XMMatrixScaling(m_localScale.x, m_localScale.y, m_localScale.z);
    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(
        m_localRotation.x * toRad, m_localRotation.y * toRad, m_localRotation.z * toRad);
    XMMATRIX translation = XMMatrixTranslation(m_localPosition.x, m_localPosition.y, m_localPosition.z);
    return scale * rotation * translation;
}

XMMATRIX Transform::GetWorldMatrix()
{
    if (m_dirty)
    {
        XMMATRIX world = GetLocalMatrix();

        GameObject* parent = m_owner ? m_owner->GetParent() : nullptr;
        if (parent && parent->GetTransform())
            world = world * parent->GetTransform()->GetWorldMatrix();

        XMStoreFloat4x4(&m_worldMatrixCache, world);
        m_dirty = false;
    }
    return XMLoadFloat4x4(&m_worldMatrixCache);
}

XMFLOAT3 Transform::GetWorldPosition()
{
    XMMATRIX world = GetWorldMatrix();
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, world.r[3]);
    return pos;
}

void Transform::MarkDirty()
{
    // 이미 dirty 라면 이 서브트리는 이전 호출에서 이미 전파가 끝난 상태이므로
    // 다시 자식들을 순회하지 않는다. (핵심 최적화 지점)
    if (m_dirty)
        return;

    m_dirty = true;

    if (m_owner)
    {
        for (GameObject* child : m_owner->GetChildren())
        {
            if (Transform* childTransform = child->GetTransform())
                childTransform->MarkDirty();
        }
    }
}

nlohmann::json Transform::ToJson() const
{
    nlohmann::json j = Component::ToJson();
    j["position"] = { m_localPosition.x, m_localPosition.y, m_localPosition.z };
    j["rotation"] = { m_localRotation.x, m_localRotation.y, m_localRotation.z };
    j["scale"] = { m_localScale.x, m_localScale.y, m_localScale.z };
    return j;
}

void Transform::FromJson(const nlohmann::json& json)
{
    Component::FromJson(json);

    auto readVec3 = [](const nlohmann::json& arr) {
        return XMFLOAT3(arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>());
    };

    if (json.contains("position")) SetLocalPosition(readVec3(json.at("position")));
    if (json.contains("rotation")) SetLocalRotation(readVec3(json.at("rotation")));
    if (json.contains("scale")) SetLocalScale(readVec3(json.at("scale")));
}
