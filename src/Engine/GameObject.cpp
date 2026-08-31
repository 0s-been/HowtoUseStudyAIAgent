#include "GameObject.h"
#include "ObjectRegistry.h"

GameObject::GameObject(const std::string& name)
    : m_name(name)
{
    m_id = ObjectRegistry::Instance().GenerateId();
    ObjectRegistry::Instance().RegisterGameObject(m_id, this);

    auto transform = std::make_unique<Transform>();
    m_transform = transform.get();
    m_transform->SetOwner(this);
    RegisterInTypeMap(m_transform);
    m_components.push_back(std::move(transform));
}

GameObject::~GameObject()
{
    for (auto& c : m_components)
        c->OnDestroy();
    ObjectRegistry::Instance().UnregisterGameObject(m_id);
}

void GameObject::OverrideId(uint64_t newId)
{
    ObjectRegistry::Instance().UnregisterGameObject(m_id);
    m_id = newId;
    ObjectRegistry::Instance().RegisterGameObject(m_id, this);
}

void GameObject::RegisterInTypeMap(Component* raw)
{
    m_componentsByType[std::type_index(typeid(*raw))].push_back(raw);
}

void GameObject::UnregisterFromTypeMap(Component* raw)
{
    auto& vec = m_componentsByType[std::type_index(typeid(*raw))];
    vec.erase(std::remove(vec.begin(), vec.end(), raw), vec.end());
}

Component* GameObject::GetComponentById(uint64_t id) const
{
    for (auto& c : m_components)
        if (c->GetId() == id)
            return c.get();
    for (auto& c : m_pendingAdds)
        if (c->GetId() == id)
            return c.get();
    return nullptr;
}

void GameObject::RemoveComponent(Component* component)
{
    if (!component || component == m_transform)
        return;
    component->MarkPendingDestroy();
}

void GameObject::AttachComponentImmediate(std::unique_ptr<Component> component)
{
    Component* raw = component.get();
    raw->SetOwner(this);
    RegisterInTypeMap(raw);
    m_components.push_back(std::move(component));
}

void GameObject::SetParent(GameObject* parent)
{
    if (m_parent == parent)
        return;

    if (m_parent)
    {
        auto& siblings = m_parent->m_children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }

    m_parent = parent;

    if (m_parent)
        m_parent->m_children.push_back(this);

    if (m_transform)
        m_transform->MarkDirty();
}

void GameObject::Start()
{
    for (auto& c : m_components)
    {
        if (!c->HasStarted())
        {
            c->Start();
            c->SetStarted(true);
        }
    }
}

void GameObject::Update(float deltaTime)
{
    if (!m_active)
        return;

    for (auto& c : m_components)
    {
        if (c->IsPendingDestroy() || !c->IsEnabled())
            continue;
        c->Update(deltaTime);
    }
}

void GameObject::Render()
{
    if (!m_active)
        return;

    for (auto& c : m_components)
    {
        if (c->IsPendingDestroy() || !c->IsEnabled())
            continue;
        c->Render();
    }
}

void GameObject::ApplyPendingChanges()
{
    // 1) 대기 중이던 추가 요청을 실제 목록에 편입
    for (auto& comp : m_pendingAdds)
    {
        Component* raw = comp.get();
        RegisterInTypeMap(raw);
        m_components.push_back(std::move(comp));
        raw->Start();
        raw->SetStarted(true);
    }
    m_pendingAdds.clear();

    // 2) 삭제 요청된 컴포넌트를 실제로 제거
    for (auto it = m_components.begin(); it != m_components.end();)
    {
        if ((*it)->IsPendingDestroy())
        {
            Component* raw = it->get();
            raw->OnDestroy();
            UnregisterFromTypeMap(raw);
            it = m_components.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

nlohmann::json GameObject::ToJson() const
{
    nlohmann::json j;
    j["id"] = m_id;
    j["name"] = m_name;
    j["active"] = m_active;
    j["parentId"] = m_parent ? m_parent->GetId() : 0;

    nlohmann::json components = nlohmann::json::array();
    for (auto& c : m_components)
        components.push_back(c->ToJson());
    j["components"] = components;

    return j;
}

void GameObject::FromJson(const nlohmann::json& json)
{
    if (json.contains("name")) m_name = json.at("name").get<std::string>();
    if (json.contains("active")) m_active = json.at("active").get<bool>();
}
