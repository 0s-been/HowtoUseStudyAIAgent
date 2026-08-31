#include "TimeManager.h"

TimeManager& TimeManager::Instance()
{
    static TimeManager instance;
    return instance;
}

void TimeManager::Init()
{
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_startTime);
    m_lastTime = m_startTime;
    m_deltaTime = 0.0f;
    m_totalTime = 0.0f;
    m_frameCount = 0;
}

void TimeManager::Tick()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    m_deltaTime = static_cast<float>(now.QuadPart - m_lastTime.QuadPart) / static_cast<float>(m_frequency.QuadPart);
    m_totalTime = static_cast<float>(now.QuadPart - m_startTime.QuadPart) / static_cast<float>(m_frequency.QuadPart);

    m_lastTime = now;
    ++m_frameCount;
}
