#include "Game.h"
#include "TimeManager.h"
#include "Input/InputManager.h"
#include "Engine/TextureManager.h"
#include "Engine/ShaderManager.h"
#include "Engine/SceneManager.h"
#include "Engine/SpriteRenderer.h"

bool Game::Initialize(HINSTANCE hInstance)
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED); // WIC(텍스처 로딩) 사용을 위해 필요

    if (!m_window.Create(hInstance, L"DirectX 11 Game Framework", 1280, 720))
        return false;
    m_window.Show();

    m_graphics = std::make_unique<Graphics>();
    if (!m_graphics->Initialize(m_window.GetHandle(), m_window.GetWidth(), m_window.GetHeight()))
        return false;

    TimeManager::Instance().Init();
    TextureManager::Instance().Init(m_graphics->GetDevice());

    SceneManager::Instance().CreateScene("MainScene");
    BuildDefaultScene();
    SceneManager::Instance().GetActiveScene()->Start();

    return true;
}

void Game::BuildDefaultScene()
{
    Scene* scene = SceneManager::Instance().GetActiveScene();

    GameObject* player = scene->CreateGameObject("Player");
    player->AddComponent<SpriteRenderer>()->SetTexturePath("Assets/Textures/player.png");
    m_player = player;

    // 계층 구조 데모: Player 의 자식으로 Child 를 붙여서 부모를 움직이면
    // 자식도 같이 따라오는지(Dirty Flag 전파) 확인할 수 있다.
    GameObject* child = scene->CreateGameObject("Child");
    child->AddComponent<SpriteRenderer>()->SetTexturePath("Assets/Textures/child.png");
    child->GetTransform()->SetLocalPosition(XMFLOAT3(80.0f, 0.0f, 0.0f));
    child->GetTransform()->SetLocalScale(XMFLOAT3(0.5f, 0.5f, 0.5f));
    child->SetParent(player);

    // AddComponent/RemoveComponent 는 프레임 종료 후 반영되므로,
    // 이 시점에 등록한 컴포넌트들도 다음 ApplyPendingChanges() 에서 Start() 된다.
}

void Game::Run()
{
    m_running = true;

    while (m_running)
    {
        InputManager::Instance().BeginFrame();

        if (!m_window.ProcessMessages())
        {
            m_running = false;
            break;
        }

        TimeManager::Instance().Tick();
        float deltaTime = TimeManager::Instance().GetDeltaTime();

        ShaderManager::Instance().Update(deltaTime);

        Update(deltaTime);
        Render();

        // 렌더링이 끝난 뒤, 이번 프레임 동안 쌓인 컴포넌트/오브젝트 추가·삭제 요청을 일괄 처리한다.
        SceneManager::Instance().ApplyPendingChanges();
    }
}

void Game::Update(float deltaTime)
{
    auto& input = InputManager::Instance();

    if (m_player)
    {
        Transform* transform = m_player->GetTransform();
        XMFLOAT3 pos = transform->GetLocalPosition();
        const float speed = 200.0f * deltaTime;

        if (input.IsKeyDown('W') || input.IsKeyDown(VK_UP))    pos.y += speed;
        if (input.IsKeyDown('S') || input.IsKeyDown(VK_DOWN))  pos.y -= speed;
        if (input.IsKeyDown('A') || input.IsKeyDown(VK_LEFT))  pos.x -= speed;
        if (input.IsKeyDown('D') || input.IsKeyDown(VK_RIGHT)) pos.x += speed;

        transform->SetLocalPosition(pos);
    }

    // F5: 현재 씬을 JSON 으로 저장 / F9: 저장된 JSON 을 다시 로드
    if (input.IsKeyPressed(VK_F5))
        SceneManager::Instance().SaveActiveScene("Assets/scene.json");

    if (input.IsKeyPressed(VK_F9) && SceneManager::Instance().LoadScene("Assets/scene.json"))
    {
        // 씬을 새로 로드하면 기존 GameObject 들은 모두 파괴되므로 포인터를 다시 찾는다.
        m_player = nullptr;
        for (auto& go : SceneManager::Instance().GetActiveScene()->GetGameObjects())
        {
            if (go->GetName() == "Player")
            {
                m_player = go.get();
                break;
            }
        }
    }

    SceneManager::Instance().Update(deltaTime);
}

void Game::Render()
{
    m_graphics->BeginFrame();
    SceneManager::Instance().Render();
    m_graphics->EndFrame();
}

void Game::Shutdown()
{
    if (m_graphics)
        m_graphics->Shutdown();
    CoUninitialize();
}
