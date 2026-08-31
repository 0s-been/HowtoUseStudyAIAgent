#include "InputManager.h"
#include <cstring>

InputManager& InputManager::Instance()
{
    static InputManager instance;
    return instance;
}

void InputManager::BeginFrame()
{
    std::memcpy(m_previousKeys, m_currentKeys, sizeof(m_currentKeys));
    std::memcpy(m_previousMouseButtons, m_currentMouseButtons, sizeof(m_currentMouseButtons));
    m_wheelDelta = 0;
}

void InputManager::OnKeyDown(int virtualKey)
{
    if (virtualKey >= 0 && virtualKey < kKeyCount)
        m_currentKeys[virtualKey] = true;
}

void InputManager::OnKeyUp(int virtualKey)
{
    if (virtualKey >= 0 && virtualKey < kKeyCount)
        m_currentKeys[virtualKey] = false;
}

void InputManager::OnMouseMove(int x, int y)
{
    m_mouseX = x;
    m_mouseY = y;
}

void InputManager::OnMouseButtonDown(int button)
{
    if (button >= 0 && button < kMouseButtonCount)
        m_currentMouseButtons[button] = true;
}

void InputManager::OnMouseButtonUp(int button)
{
    if (button >= 0 && button < kMouseButtonCount)
        m_currentMouseButtons[button] = false;
}

void InputManager::OnMouseWheel(int delta)
{
    m_wheelDelta += delta;
}

bool InputManager::IsKeyDown(int virtualKey) const
{
    return (virtualKey >= 0 && virtualKey < kKeyCount) && m_currentKeys[virtualKey];
}

bool InputManager::IsKeyPressed(int virtualKey) const
{
    return (virtualKey >= 0 && virtualKey < kKeyCount) &&
        m_currentKeys[virtualKey] && !m_previousKeys[virtualKey];
}

bool InputManager::IsKeyReleased(int virtualKey) const
{
    return (virtualKey >= 0 && virtualKey < kKeyCount) &&
        !m_currentKeys[virtualKey] && m_previousKeys[virtualKey];
}

bool InputManager::IsMouseButtonDown(int button) const
{
    return (button >= 0 && button < kMouseButtonCount) && m_currentMouseButtons[button];
}

bool InputManager::IsMouseButtonPressed(int button) const
{
    return (button >= 0 && button < kMouseButtonCount) &&
        m_currentMouseButtons[button] && !m_previousMouseButtons[button];
}

bool InputManager::IsMouseButtonReleased(int button) const
{
    return (button >= 0 && button < kMouseButtonCount) &&
        !m_currentMouseButtons[button] && m_previousMouseButtons[button];
}
