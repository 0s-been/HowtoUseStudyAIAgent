#pragma once

#include "Core/Common.h"

// 키보드/마우스 입력을 폴링 방식으로 제공하는 싱글톤.
// Window 의 WndProc 에서 OnKeyDown/OnKeyUp 등을 호출해 상태를 갱신하고,
// 매 프레임 시작 시 BeginFrame() 을 호출해 "이전 프레임" 상태를 저장한다.
class InputManager
{
public:
    static InputManager& Instance();

    void BeginFrame();

    void OnKeyDown(int virtualKey);
    void OnKeyUp(int virtualKey);
    void OnMouseMove(int x, int y);
    void OnMouseButtonDown(int button); // 0=Left, 1=Right, 2=Middle
    void OnMouseButtonUp(int button);
    void OnMouseWheel(int delta);

    bool IsKeyDown(int virtualKey) const;
    bool IsKeyPressed(int virtualKey) const;  // 이번 프레임에 처음 눌림
    bool IsKeyReleased(int virtualKey) const; // 이번 프레임에 처음 떼어짐

    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonReleased(int button) const;

    int GetMouseX() const { return m_mouseX; }
    int GetMouseY() const { return m_mouseY; }
    int GetMouseWheelDelta() const { return m_wheelDelta; }

private:
    InputManager() = default;

    static constexpr int kKeyCount = 256;
    static constexpr int kMouseButtonCount = 3;

    bool m_currentKeys[kKeyCount]{};
    bool m_previousKeys[kKeyCount]{};

    bool m_currentMouseButtons[kMouseButtonCount]{};
    bool m_previousMouseButtons[kMouseButtonCount]{};

    int m_mouseX = 0;
    int m_mouseY = 0;
    int m_wheelDelta = 0;
};
