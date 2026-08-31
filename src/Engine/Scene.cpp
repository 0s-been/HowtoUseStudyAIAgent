#include "Scene.h"
#include "ComponentFactory.h"
#include "ObjectRegistry.h"
#include <fstream>
#include <algorithm>

Scene::Scene(const std::string& name)
    : m_name(name)
{
}

GameObject* Scene::CreateGameObject(const std::string& name)
{
    auto go = std::make_unique<GameObject>(name);
    GameObject* raw = go.get();
    m_gameObjects.push_back(std::move(go));
    return raw;
}

void Scene::DestroyGameObject(GameObject* gameObject)
{
    if (!gameObject)
        return;

    // 자식들을 먼저 파괴 대상으로 등록 (부모가 사라지면 자식도 함께 사라진다)
    for (GameObject* child : gameObject->GetChildren())
        DestroyGameObject(child);

    gameObject->SetActive(false); // Update/Render 에서 즉시 제외
    m_pendingDestroy.push_back(gameObject);
}

GameObject* Scene::FindGameObjectById(uint64_t id) const
{
    for (auto& go : m_gameObjects)
        if (go->GetId() == id)
            return go.get();
    return nullptr;
}

void Scene::Start()
{
    for (auto& go : m_gameObjects)
        go->Start();
}

void Scene::Update(float deltaTime)
{
    for (auto& go : m_gameObjects)
    {
        if (go->IsActive())
            go->Update(deltaTime);
    }
}

void Scene::Render()
{
    for (auto& go : m_gameObjects)
    {
        if (go->IsActive())
            go->Render();
    }
}

void Scene::ApplyPendingChanges()
{
    for (auto& go : m_gameObjects)
        go->ApplyPendingChanges();

    for (GameObject* go : m_pendingDestroy)
    {
        go->SetParent(nullptr);

        auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
            [go](const std::unique_ptr<GameObject>& p) { return p.get() == go; });
        if (it != m_gameObjects.end())
            m_gameObjects.erase(it);
    }
    m_pendingDestroy.clear();
}

bool Scene::SaveToFile(const std::string& path) const
{
    nlohmann::json root;
    root["scene"] = m_name;

    nlohmann::json objects = nlohmann::json::array();
    for (auto& go : m_gameObjects)
        objects.push_back(go->ToJson());
    root["gameObjects"] = objects;

    std::ofstream file(path);
    if (!file.is_open())
        return false;

    file << root.dump(2);
    return true;
}

bool Scene::LoadFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    nlohmann::json root;
    try
    {
        file >> root;
    }
    catch (const nlohmann::json::parse_error&)
    {
        return false;
    }

    // 기존 씬 내용을 비운다.
    m_gameObjects.clear();
    m_pendingDestroy.clear();

    if (root.contains("scene"))
        m_name = root.at("scene").get<std::string>();

    if (!root.contains("gameObjects"))
        return true;

    // 1단계: 모든 GameObject 와 컴포넌트를 생성하고 자기 자신의 데이터를 복원한다.
    for (const auto& goJson : root.at("gameObjects"))
    {
        auto go = std::make_unique<GameObject>();
        go->OverrideId(goJson.at("id").get<uint64_t>());
        go->FromJson(goJson);

        if (goJson.contains("components"))
        {
            for (const auto& compJson : goJson.at("components"))
            {
                const std::string type = compJson.at("type").get<std::string>();
                if (type == "Transform")
                {
                    go->GetTransform()->OverrideId(compJson.at("id").get<uint64_t>());
                    go->GetTransform()->FromJson(compJson);
                    continue;
                }

                auto comp = ComponentFactory::Instance().Create(type);
                if (!comp)
                    continue; // 알 수 없는 타입은 건너뛴다.

                comp->OverrideId(compJson.at("id").get<uint64_t>());
                comp->FromJson(compJson);
                go->AttachComponentImmediate(std::move(comp));
            }
        }

        m_gameObjects.push_back(std::move(go));
    }

    // 2단계: 모든 객체가 존재하는 상태에서 부모-자식 관계를 복원한다.
    for (size_t i = 0; i < m_gameObjects.size(); ++i)
    {
        const auto& goJson = root.at("gameObjects")[i];
        uint64_t parentId = goJson.value("parentId", static_cast<uint64_t>(0));
        if (parentId != 0)
        {
            GameObject* parent = FindGameObjectById(parentId);
            if (parent)
                m_gameObjects[i]->SetParent(parent);
        }
    }

    return true;
}
