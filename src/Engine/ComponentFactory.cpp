#include "ComponentFactory.h"

ComponentFactory& ComponentFactory::Instance()
{
    static ComponentFactory instance;
    return instance;
}

void ComponentFactory::Register(const std::string& typeName, Creator creator)
{
    m_creators[typeName] = std::move(creator);
}

std::unique_ptr<Component> ComponentFactory::Create(const std::string& typeName) const
{
    auto it = m_creators.find(typeName);
    if (it == m_creators.end())
        return nullptr;
    return it->second();
}
