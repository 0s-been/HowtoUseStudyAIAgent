#pragma once

#include "Component.h"

// 위치, 회전(오일러, degree), 스케일을 갖는 컴포넌트.
// GameObject 의 부모-자식 계층 구조를 따라 로컬 -> 월드 행렬을 계산하며,
// dirty flag 를 이용해 값이 바뀌었을 때만 월드 행렬을 재계산한다.
class Transform : public Component
{
public:
    Transform();

    void SetLocalPosition(const XMFLOAT3& position);
    void SetLocalRotation(const XMFLOAT3& eulerDegrees);
    void SetLocalScale(const XMFLOAT3& scale);

    const XMFLOAT3& GetLocalPosition() const { return m_localPosition; }
    const XMFLOAT3& GetLocalRotation() const { return m_localRotation; }
    const XMFLOAT3& GetLocalScale() const { return m_localScale; }

    XMFLOAT3 GetWorldPosition();
    XMMATRIX GetLocalMatrix() const;

    // dirty 상태라면 부모 체인을 따라 월드 행렬을 재계산하고 캐시한다.
    XMMATRIX GetWorldMatrix();

    // 자기 자신과 모든 자식 Transform 을 dirty 로 표시한다.
    // 이미 dirty 라면(부모 쪽에서 먼저 처리된 경우) 더 내려가지 않고 즉시 반환하여
    // 동일한 서브트리를 중복으로 순회하지 않도록 최적화한다.
    void MarkDirty();

    std::string GetTypeName() const override { return "Transform"; }

    nlohmann::json ToJson() const override;
    void FromJson(const nlohmann::json& json) override;

private:
    XMFLOAT3 m_localPosition{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_localRotation{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_localScale{ 1.0f, 1.0f, 1.0f };

    XMFLOAT4X4 m_worldMatrixCache;
    bool m_dirty = true;
};
