#pragma once

#include "Component.h"

// JSON 에 문자열로 저장된 타입 이름("SpriteRenderer" 등)을 보고
// 실제 컴포넌트 객체를 생성해주는 팩토리. 씬 로드 시 사용된다.
class ComponentFactory
{
public:
    using Creator = std::function<std::unique_ptr<Component>()>;

    static ComponentFactory& Instance();

    void Register(const std::string& typeName, Creator creator);
    std::unique_ptr<Component> Create(const std::string& typeName) const;

private:
    ComponentFactory() = default;
    std::unordered_map<std::string, Creator> m_creators;
};

template <typename T>
struct ComponentRegistrar
{
    explicit ComponentRegistrar(const std::string& name)
    {
        ComponentFactory::Instance().Register(name, []() { return std::make_unique<T>(); });
    }
};

#define REGISTER_COMPONENT(TypeClass) \
    static ComponentRegistrar<TypeClass> g_##TypeClass##Registrar(#TypeClass);
