#include "SceneManager.h"

SceneManager& SceneManager::Instance()
{
    static SceneManager instance;
    return instance;
}

Scene* SceneManager::CreateScene(const std::string& name)
{
    m_activeScene = std::make_unique<Scene>(name);
    return m_activeScene.get();
}

bool SceneManager::LoadScene(const std::string& path)
{
    if (!m_activeScene)
        m_activeScene = std::make_unique<Scene>();

    if (!m_activeScene->LoadFromFile(path))
        return false;

    m_activeScene->Start();
    return true;
}

bool SceneManager::SaveActiveScene(const std::string& path) const
{
    if (!m_activeScene)
        return false;
    return m_activeScene->SaveToFile(path);
}

void SceneManager::Update(float deltaTime)
{
    if (m_activeScene)
        m_activeScene->Update(deltaTime);
}

void SceneManager::Render()
{
    if (m_activeScene)
        m_activeScene->Render();
}

void SceneManager::ApplyPendingChanges()
{
    if (m_activeScene)
        m_activeScene->ApplyPendingChanges();
}
