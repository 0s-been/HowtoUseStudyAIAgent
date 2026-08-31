#pragma once

#include "Window.h"
#include "Graphics.h"
#include "Engine/GameObject.h"
#include <memory>

// 윈도우, 그래픽스, 입력, 씬을 총괄하고 메인 루프를 실행하는 클래스.
class Game
{
public:
    bool Initialize(HINSTANCE hInstance);
    void Run();
    void Shutdown();

private:
    void BuildDefaultScene();
    void Update(float deltaTime);
    void Render();

    Window m_window;
    std::unique_ptr<Graphics> m_graphics;
    bool m_running = false;

    GameObject* m_player = nullptr; // 데모용: 방향키/WASD 로 이동
};
