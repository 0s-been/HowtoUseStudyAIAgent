#pragma once

#include "Core/Common.h"
#include <nlohmann/json.hpp>

class GameObject;

// 모든 컴포넌트의 기반 클래스.
// GameObject 가 소유하며, Start -> Update -> Render 순서의 생명주기를 가진다.
class Component
{
public:
    Component();
    virtual ~Component();

    virtual void Start() {}
    virtual void Update(float deltaTime) {}
    virtual void Render() {}
    virtual void OnDestroy() {}

    virtual std::string GetTypeName() const = 0;

    uint64_t GetId() const { return m_id; }
    GameObject* GetOwner() const { return m_owner; }
    void SetOwner(GameObject* owner) { m_owner = owner; }

    bool IsPendingDestroy() const { return m_pendingDestroy; }
    void MarkPendingDestroy() { m_pendingDestroy = true; }

    bool HasStarted() const { return m_started; }
    void SetStarted(bool started) { m_started = started; }

    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }

    // 역직렬화 시 저장되어 있던 ID를 그대로 복원하기 위해 사용한다.
    void OverrideId(uint64_t newId);

    virtual nlohmann::json ToJson() const;
    virtual void FromJson(const nlohmann::json& json);

protected:
    uint64_t m_id = 0;
    GameObject* m_owner = nullptr;
    bool m_pendingDestroy = false;
    bool m_started = false;
    bool m_enabled = true;
};
