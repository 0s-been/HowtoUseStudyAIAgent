#pragma once

#include "Shader.h"

// HOT_RELOAD_ENABLED 가 켜져 있을 때, 등록된 Shader 들의 .hlsl 파일 변경을
// 주기적으로 감시하고 자동으로 다시 로드시키는 싱글톤.
class ShaderManager
{
public:
    static ShaderManager& Instance();

    void Register(Shader* shader);

    // Game 의 메인 루프에서 매 프레임 호출한다.
    // 매 프레임 파일 시스템을 조회하는 비용을 줄이기 위해 일정 주기마다만 검사한다.
    void Update(float deltaTime);

private:
    ShaderManager() = default;

    std::vector<Shader*> m_shaders;
    float m_checkTimer = 0.0f;
    static constexpr float kCheckInterval = 0.5f; // 초
};
