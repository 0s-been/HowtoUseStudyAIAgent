#include "ObjectRegistry.h"

ObjectRegistry& ObjectRegistry::Instance()
{
    static ObjectRegistry instance;
    return instance;
}

uint64_t ObjectRegistry::GenerateId()
{
    return m_nextId.fetch_add(1);
}

void ObjectRegistry::RegisterGameObject(uint64_t id, GameObject* obj)
{
    m_gameObjects[id] = obj;
}

void ObjectRegistry::UnregisterGameObject(uint64_t id)
{
    m_gameObjects.erase(id);
}

GameObject* ObjectRegistry::FindGameObject(uint64_t id) const
{
    auto it = m_gameObjects.find(id);
    return it != m_gameObjects.end() ? it->second : nullptr;
}

void ObjectRegistry::RegisterComponent(uint64_t id, Component* comp)
{
    m_components[id] = comp;
}

void ObjectRegistry::UnregisterComponent(uint64_t id)
{
    m_components.erase(id);
}

Component* ObjectRegistry::FindComponent(uint64_t id) const
{
    auto it = m_components.find(id);
    return it != m_components.end() ? it->second : nullptr;
}
