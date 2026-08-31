#pragma once

#include "GameObject.h"

// 여러 GameObject 를 소유하고 일괄적으로 Update/Render 하는 씬.
class Scene
{
public:
    explicit Scene(const std::string& name = "Scene");

    GameObject* CreateGameObject(const std::string& name = "GameObject");

    // 즉시 제거되지 않고, 프레임이 끝난 뒤 ApplyPendingChanges() 에서 정리된다.
    // 자식들도 함께 파괴 대상이 된다.
    void DestroyGameObject(GameObject* gameObject);

    GameObject* FindGameObjectById(uint64_t id) const;
    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_gameObjects; }

    void Start();
    void Update(float deltaTime);
    void Render();
    void ApplyPendingChanges();

    bool SaveToFile(const std::string& path) const;
    bool LoadFromFile(const std::string& path);

    const std::string& GetName() const { return m_name; }

private:
    std::string m_name;
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    std::vector<GameObject*> m_pendingDestroy;
};
