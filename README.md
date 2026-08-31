# HowtoUseStudyAIAgent

고급프로그래밍기법 강의 학습용 레포지토리입니다.
AIAgent를 통한 코드 작업 연습이 주 내용이며, 업로드된 요청 명세(`9fd77653-___________.md`)에 따라
DirectX 11 기반의 미니 게임 프레임워크를 단계별로 구현했습니다.

## 주요 기능

1. **GameObject - Component 아키텍처**
   - 모든 `GameObject` 는 생성 시 기본적으로 `Transform` 컴포넌트를 가진다.
   - `Component` 는 `Start / Update / Render / OnDestroy` 생명주기를 가진 추상 클래스다.
   - `GetComponent<T>()` / `GetComponents<T>()` 는 벡터 순회가 아닌 해시맵(`type_index`) 조회라 빠르고,
     같은 타입의 컴포넌트를 여러 개 등록해도 모두 찾을 수 있다.
   - 모든 `GameObject` / `Component` 는 생성 시 고유 ID를 부여받고 `ObjectRegistry` 전역 레지스트리에
     등록되어, ID만으로 어디서든 즉시 찾아올 수 있다.

2. **계층 구조 + Transform 최적화**
   - `GameObject::SetParent()` 로 부모-자식 계층을 구성할 수 있다.
   - `Transform` 은 로컬 좌표(위치/회전/스케일)와, 부모 체인을 반영한 월드 행렬을 모두 제공한다.
   - 값이 바뀔 때만 Dirty 플래그를 세우고, 이미 dirty 상태인 서브트리는 다시 순회하지 않아
     불필요한 행렬 재계산을 막는다. 부모가 움직이면 자식 Transform 도 자동으로 dirty 전파된다.

3. **실시간(지연) 컴포넌트/오브젝트 관리**
   - `AddComponent<T>()` 로 추가된 컴포넌트는 즉시 활성화되지 않고, 해당 프레임의 렌더링이 끝난 뒤
     `ApplyPendingChanges()` 시점에 `Start()` 가 호출되며 반영된다.
   - `RemoveComponent()` 를 호출하면 그 즉시 Update/Render 대상에서 제외되고, 프레임 종료 후
     실제 메모리에서 정리된다. `GameObject` 파괴도 동일하게 지연 처리된다.

4. **씬(Scene) JSON 저장 / 로드**
   - `Scene::SaveToFile()` 은 모든 GameObject 의 ID/이름/부모-자식 관계/Transform 값/컴포넌트
     데이터를 JSON 으로 저장한다.
   - `Scene::LoadFromFile()` 은 `ComponentFactory` 를 이용해 타입 이름으로부터 실제 컴포넌트를
     복원하고, 모든 객체 생성이 끝난 뒤 부모-자식 관계를 재구성한다.
   - 실행 중 **F5** 로 저장, **F9** 로 로드할 수 있다. (`Assets/scene.json`)

5. **입력 / 시간**
   - `InputManager` 싱글톤이 키보드(전체 키), 마우스 좌표/버튼/휠을 폴링 방식으로 제공한다.
   - `TimeManager` 싱글톤이 `QueryPerformanceCounter` 기반의 델타 타임과 총 경과 시간을 제공한다.
   - 데모 씬에서 **W/A/S/D** 또는 방향키로 Player 스프라이트를 이동할 수 있다.

6. **스프라이트 렌더링**
   - `SpriteRenderer` 컴포넌트가 공용 unit quad(정점+UV)를 텍스처 크기만큼 스케일링해서 그린다.
   - `TextureManager` 싱글톤이 WIC(Windows Imaging Component)로 이미지를 로드하고 경로별로 캐싱한다.
   - `Graphics` 가 파란색 배경 클리어, 알파 블렌딩, 상수 버퍼(WVP 행렬) 갱신 등 렌더 파이프라인을 담당한다.

7. **셰이더 Hot Reload**
   - `Shader` 클래스가 두 가지 경로를 지원한다.
     - **핫 리로드(개발) 모드**: `.hlsl` → 즉시 컴파일 → Blob 을 바로 `CreateVertexShader/PixelShader` 에 전달.
     - **캐시(출시) 모드**: `.hlsl` → 컴파일 → `.cso` 저장 → 다음부터는 `.cso` 를 그대로 로드.
   - `ShaderManager` 가 0.5초 간격으로 `.hlsl` 파일의 수정 시각을 감시하다가 변경을 감지하면
     자동으로 다시 컴파일해서 교체한다.
   - `src/Core/EngineConfig.h` 의 `HOT_RELOAD_ENABLED` 매크로 유무로 두 모드를 전환한다.
     (주석 처리하면 `.cso` 캐시 기반 출시 모드로 빌드된다.)

## 프로젝트 구조

```
src/
  Core/       Window, Graphics(D3D11), Game(메인 루프), TimeManager, EngineConfig
  Input/      InputManager
  Engine/     Component, Transform, GameObject, Scene, SceneManager,
              ObjectRegistry, ComponentFactory, Shader, ShaderManager,
              Texture, TextureManager, SpriteRenderer
  main.cpp    WinMain 진입점
Shaders/      SpriteVS.hlsl, SpritePS.hlsl
Assets/       Textures/player.png, Textures/child.png (데모용 단색 텍스처)
third_party/  nlohmann/json.hpp (헤더 온리 JSON 라이브러리)
```

## 빌드 방법 (Windows)

Visual Studio 2019 이상 + Windows 10/11 SDK 가 설치되어 있어야 합니다.

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
build\Debug\DX11GameFramework.exe
```

CMake 빌드 후 `Shaders/`, `Assets/` 폴더가 실행 파일 옆으로 자동 복사됩니다.
(`.hlsl` 은 런타임에 텍스트 그대로 읽어서 컴파일하므로, Hot Reload 모드에서는 실행 중에
`Shaders/SpriteVS.hlsl` 등을 수정 후 저장하면 다음 검사 주기(0.5초)에 자동으로 반영됩니다.)

## 조작법

| 키 | 동작 |
|---|---|
| W / A / S / D, 방향키 | Player 이동 (자식 오브젝트도 함께 따라옴) |
| F5 | 현재 씬을 `Assets/scene.json` 으로 저장 |
| F9 | `Assets/scene.json` 을 다시 로드 |

## 참고

이 저장소는 Linux 컨테이너 환경에서 작성되었기 때문에(DirectX 11 은 Windows 전용 API)
이 세션에서는 실제 컴파일/실행 검증을 하지 못했습니다. Windows + Visual Studio 환경에서
위 빌드 방법대로 확인해 주세요.
