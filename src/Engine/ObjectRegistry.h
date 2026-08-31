#pragma once

#include "Core/Common.h"
#include <atomic>

class GameObject;
class Component;

// 모든 GameObject / Component 에 고유 ID를 부여하고,
// ID만으로 어떤 객체든 즉시 찾아올 수 있도록 하는 전역 레지스트리.
class ObjectRegistry
{
public:
    static ObjectRegistry& Instance();

    uint64_t GenerateId();

    void RegisterGameObject(uint64_t id, GameObject* obj);
    void UnregisterGameObject(uint64_t id);
    GameObject* FindGameObject(uint64_t id) const;

    void RegisterComponent(uint64_t id, Component* comp);
    void UnregisterComponent(uint64_t id);
    Component* FindComponent(uint64_t id) const;

private:
    ObjectRegistry() = default;

    std::atomic<uint64_t> m_nextId{ 1 };
    std::unordered_map<uint64_t, GameObject*> m_gameObjects;
    std::unordered_map<uint64_t, Component*> m_components;
};
