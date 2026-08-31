#pragma once

#include "Common.h"

// 프레임 간 경과 시간(Delta Time)과 프로그램 시작 이후 총 경과 시간을 제공하는 싱글톤.
// QueryPerformanceCounter 기반 고해상도 타이머를 사용한다.
class TimeManager
{
public:
    static TimeManager& Instance();

    void Init();
    void Tick(); // 매 프레임 시작 시 호출

    float GetDeltaTime() const { return m_deltaTime; }
    float GetTotalTime() const { return m_totalTime; }
    uint64_t GetFrameCount() const { return m_frameCount; }

private:
    TimeManager() = default;

    LARGE_INTEGER m_frequency{};
    LARGE_INTEGER m_startTime{};
    LARGE_INTEGER m_lastTime{};

    float m_deltaTime = 0.0f;
    float m_totalTime = 0.0f;
    uint64_t m_frameCount = 0;
};
