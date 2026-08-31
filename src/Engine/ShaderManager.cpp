#include "ShaderManager.h"

ShaderManager& ShaderManager::Instance()
{
    static ShaderManager instance;
    return instance;
}

void ShaderManager::Register(Shader* shader)
{
    m_shaders.push_back(shader);
}

void ShaderManager::Update(float deltaTime)
{
#ifdef HOT_RELOAD_ENABLED
    m_checkTimer += deltaTime;
    if (m_checkTimer < kCheckInterval)
        return;
    m_checkTimer = 0.0f;

    for (Shader* shader : m_shaders)
        shader->ReloadIfChanged();
#endif
}
