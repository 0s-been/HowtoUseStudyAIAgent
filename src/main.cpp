#include "Core/Game.h"

// MSVC 기본 CRT 시작 루틴(WinMainCRTStartup)은 WinMain 을 호출하므로,
// 링커 엔트리 포인트를 별도로 지정하지 않아도 되도록 wWinMain 대신 WinMain 을 사용한다.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    Game game;
    if (!game.Initialize(hInstance))
        return -1;

    game.Run();
    game.Shutdown();
    return 0;
}
