#pragma once

#include "Common.h"

// Win32 윈도우 생성과 메시지 펌프를 담당한다.
// 키보드/마우스 메시지는 WndProc 에서 InputManager 로 전달한다.
class Window
{
public:
    bool Create(HINSTANCE hInstance, const std::wstring& title, int width, int height);
    void Show();

    // 큐에 쌓인 윈도우 메시지를 처리한다. WM_QUIT 를 받으면 false 를 반환한다.
    bool ProcessMessages();

    HWND GetHandle() const { return m_hwnd; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd = nullptr;
    int m_width = 0;
    int m_height = 0;
};
