#pragma once

#include "Component.h"
#include "Transform.h"
#include <algorithm>
#include <type_traits>
#include <utility>
#include <typeinfo>

// Unity 스타일의 GameObject.
// - 생성 시 항상 Transform 컴포넌트를 하나 가진다.
// - 컴포넌트는 타입(type_index) 기준의 해시맵으로 관리되어 GetComponent 조회가 빠르다.
// - 같은 타입의 컴포넌트를 여러 개 등록할 수 있다 (GetComponents<T>()).
// - AddComponent / RemoveComponent 는 즉시 반영되지 않고 큐에 쌓였다가
//   ApplyPendingChanges() 호출 시점(한 프레임의 렌더가 끝난 뒤)에 일괄 반영된다.
//   단, 삭제 요청된 컴포넌트는 즉시 Update/Render 대상에서 제외된다.
class GameObject
{
public:
    explicit GameObject(const std::string& name = "GameObject");
    ~GameObject();

    uint64_t GetId() const { return m_id; }
    void OverrideId(uint64_t newId);

    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }

    Transform* GetTransform() const { return m_transform; }

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

    // ---- 컴포넌트 ----
    template <typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = comp.get();
        raw->SetOwner(this);
        m_pendingAdds.push_back(std::move(comp));
        return raw;
    }

    template <typename T>
    T* GetComponent() const
    {
        auto it = m_componentsByType.find(std::type_index(typeid(T)));
        if (it == m_componentsByType.end() || it->second.empty())
            return nullptr;
        return static_cast<T*>(it->second.front());
    }

    template <typename T>
    std::vector<T*> GetComponents() const
    {
        std::vector<T*> result;
        auto it = m_componentsByType.find(std::type_index(typeid(T)));
        if (it != m_componentsByType.end())
        {
            result.reserve(it->second.size());
            for (Component* c : it->second)
                result.push_back(static_cast<T*>(c));
        }
        return result;
    }

    Component* GetComponentById(uint64_t id) const;

    // Transform 은 제거할 수 없다.
    void RemoveComponent(Component* component);

    // 로더 전용: 지연 큐를 거치지 않고 즉시 컴포넌트 목록에 편입시킨다.
    void AttachComponentImmediate(std::unique_ptr<Component> component);

    // ---- 계층 구조 ----
    void SetParent(GameObject* parent);
    GameObject* GetParent() const { return m_parent; }
    const std::vector<GameObject*>& GetChildren() const { return m_children; }

    // ---- 생명주기 ----
    void Start();
    void Update(float deltaTime);
    void Render();

    // 프레임 렌더링이 끝난 뒤 호출: 대기 중이던 추가/삭제 요청을 일괄 처리한다.
    void ApplyPendingChanges();

    nlohmann::json ToJson() const;
    void FromJson(const nlohmann::json& json);

private:
    void RegisterInTypeMap(Component* raw);
    void UnregisterFromTypeMap(Component* raw);

    uint64_t m_id = 0;
    std::string m_name;
    bool m_active = true;

    Transform* m_transform = nullptr;
    std::vector<std::unique_ptr<Component>> m_components;
    std::unordered_map<std::type_index, std::vector<Component*>> m_componentsByType;
    std::vector<std::unique_ptr<Component>> m_pendingAdds;

    GameObject* m_parent = nullptr;
    std::vector<GameObject*> m_children;
};
