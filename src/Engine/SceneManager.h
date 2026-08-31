#pragma once

#include "Scene.h"

// 현재 활성화된 씬을 관리하는 싱글톤.
class SceneManager
{
public:
    static SceneManager& Instance();

    Scene* CreateScene(const std::string& name);
    Scene* GetActiveScene() const { return m_activeScene.get(); }

    bool LoadScene(const std::string& path);
    bool SaveActiveScene(const std::string& path) const;

    void Update(float deltaTime);
    void Render();
    void ApplyPendingChanges();

private:
    SceneManager() = default;
    std::unique_ptr<Scene> m_activeScene;
};
