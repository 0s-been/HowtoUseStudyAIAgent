#include "Component.h"
#include "ObjectRegistry.h"

Component::Component()
{
    m_id = ObjectRegistry::Instance().GenerateId();
    ObjectRegistry::Instance().RegisterComponent(m_id, this);
}

Component::~Component()
{
    ObjectRegistry::Instance().UnregisterComponent(m_id);
}

void Component::OverrideId(uint64_t newId)
{
    ObjectRegistry::Instance().UnregisterComponent(m_id);
    m_id = newId;
    ObjectRegistry::Instance().RegisterComponent(m_id, this);
}

nlohmann::json Component::ToJson() const
{
    return nlohmann::json{
        { "id", m_id },
        { "type", GetTypeName() },
        { "enabled", m_enabled }
    };
}

void Component::FromJson(const nlohmann::json& json)
{
    if (json.contains("enabled"))
        m_enabled = json.at("enabled").get<bool>();
}
