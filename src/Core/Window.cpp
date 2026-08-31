#include "Window.h"
#include "Input/InputManager.h"
#include <windowsx.h>

namespace
{
    int ToMouseButtonIndex(UINT msg)
    {
        switch (msg)
        {
        case WM_LBUTTONDOWN: case WM_LBUTTONUP: return 0;
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: return 1;
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: return 2;
        default: return -1;
        }
    }
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto& input = InputManager::Instance();

    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        input.OnKeyDown(static_cast<int>(wParam));
        return 0;

    case WM_KEYUP:
        input.OnKeyUp(static_cast<int>(wParam));
        return 0;

    case WM_MOUSEMOVE:
        input.OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        input.OnMouseButtonDown(ToMouseButtonIndex(msg));
        return 0;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        input.OnMouseButtonUp(ToMouseButtonIndex(msg));
        return 0;

    case WM_MOUSEWHEEL:
        input.OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool Window::Create(HINSTANCE hInstance, const std::wstring& title, int width, int height)
{
    m_width = width;
    m_height = height;

    const wchar_t* className = L"DX11GameFrameworkWindowClass";

    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = className;
    RegisterClassEx(&wc);

    RECT rect{ 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowEx(
        0, className, title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr);

    return m_hwnd != nullptr;
}

void Window::Show()
{
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
}

bool Window::ProcessMessages()
{
    MSG msg{};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}
