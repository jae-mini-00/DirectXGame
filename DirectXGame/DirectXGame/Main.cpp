#include "Core/Renderer.h"
#include "Scene/GameObject.h"
#include "Core/Camera.h"
#include "Graphics/ModelLoader.h"
#include "Scene/Scene.h"
#include "Core/Logger/Logger.h"
#include "Core/Paths.h"
#include "Graphics/Animator.h"
#include "Core/Input/Input.h"
#include "Scene/Movement.h"
#include "Physics/Collision.h"
#include "Graphics/DebugRenderer.h"

#include <Windows.h>
#include <DirectXMath.h>
#include <memory>
#include <utility>
#include <chrono>

Camera* gCamera = nullptr;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_MOUSEWHEEL:
        {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);

            if (gCamera)
            {
                if (delta > 0)
                    gCamera->Zoom(-0.5f);
                else
                    gCamera->Zoom(0.5f);
            }

            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}


HRESULT AddModelToScene(const Renderer& renderer, Scene& scene, const char* filePath,
    DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotation)
{
    ModelData modelData;
    std::unique_ptr<GameObject> model = std::make_unique<GameObject>();
    LoadResult lr = ModelLoader::Load(filePath, modelData);

    if (!lr.success) {
        Logger::Error("AddModelToScene", lr.message.c_str());
        return E_FAIL;
    }
    model->transform.scale = scale;
    model->transform.position = position;
    model->transform.rotation = rotation;
    HRESULT modelHr = model->model.Initialize(
        renderer.GetDevice(), renderer.GetContext(),
        modelData);

    if (FAILED(modelHr))
        return modelHr;


    const auto& clips = model->model.GetAnimationClip();

    if (!clips.empty())
    {
        Logger::Info("AnimationTest", clips[1].name.c_str());
        model->animator.Play(&clips[0], true);
    }

    scene.AddObject(std::move(model));
    return S_OK;
}

void SceneUpdate(Scene& scene, double deltaTime)
{
    for (std::size_t i = 0; i < scene.size(); ++i)
    {
        GameObject* object = scene[i];

        object->animator.Update(deltaTime,
            object->model.GetNodeHierarchy(), object->model.GetSkeleton());
    }
}

int WINAPI WinMain
(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int nCmdShow)
{
    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GameWindow";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx( 0,
        L"GameWindow", L"DirectX Game",
        WS_OVERLAPPEDWINDOW,
        100, 100, 1280, 720,
        nullptr, nullptr, hInstance, nullptr);

    if (hwnd == nullptr)
        return 0;

    Renderer renderer;
    HRESULT hr = renderer.Initialize(hwnd);

    if (FAILED(hr))
    {
        MessageBox(hwnd, L"Renderer Initialize Failed", L"Error", MB_OK);
        return 0;
    }

    DebugRenderer debugRenderer;
    hr = debugRenderer.Initialize(renderer.GetDevice(), renderer.GetContext());

    if (FAILED(hr))
    {
        MessageBox(hwnd, L"DebugRenderer Initialize Failed", L"Error", MB_OK);
        return 0;
    }

    bool showColliderDebug = false;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    Scene scene;
    DirectX::XMFLOAT3 scale;
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 rotation;


    //scale = { 0.1f, 0.1f, 0.1f };
    scale = { 0.01f, 0.01f, 0.01f };
    position = { 0.0f, -5.0f, 0.0f };
    rotation = { 0.0f, 0.0f, 0.0f };
    //hr = AddModelToScene(renderer, scene, Paths::Player, position, scale, rotation);
    hr = AddModelToScene(renderer, scene, Paths::ManModel, position, scale, rotation);
    if (FAILED(hr))
    {
        MessageBox(hwnd, L"Player Add Failed", L"Error", MB_OK);
        return 0;
    }
    scale = { 1.0f, 1.0f, 1.0f };
    hr = AddModelToScene(renderer, scene, Paths::BarrelModel, position, scale, rotation);
    if (FAILED(hr))
    {
        MessageBox(hwnd, L"Player Add Failed", L"Error", MB_OK);
        return 0;
    }
    // 캐릭터
    scene[0]->collider.center = { 0.0f, 0.9f, 0.0f };
    scene[0]->collider.halfExtents = { 0.3f, 0.9f, 0.3f };

    // 드럼통
    scene[1]->collider.center = { 0.0f, 0.6f, 0.0f };
    scene[1]->collider.halfExtents = { 0.6f, 0.6f, 0.6f };

    RECT rect;
    GetClientRect(hwnd, &rect);
    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);

    Camera camera(width, height);
    gCamera = &camera;
    auto previousTime = std::chrono::steady_clock::now();

    Input input;

    WorldAABB box{
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }
    };

    // 테스트 1: 캡슐 중심 선분이 AABB를 통과
    WorldCapsule capsule1{
        { -2.0f, 0.0f, 0.0f },
        {  2.0f, 0.0f, 0.0f },
        0.25f
    };

    // 테스트 2: AABB 측면과 정확히 접촉
    WorldCapsule capsule2{
        { 1.5f, -2.0f, 0.0f },
        { 1.5f,  2.0f, 0.0f },
        0.5f
    };

    // 테스트 3: AABB와 분리
    WorldCapsule capsule3{
        { 2.0f, -2.0f, 0.0f },
        { 2.0f,  2.0f, 0.0f },
        0.5f
    };

    // 테스트 4: 중심 선분 길이가 0인 경우
    WorldCapsule capsule4{
        { 1.2f, 0.0f, 0.0f },
        { 1.2f, 0.0f, 0.0f },
        0.25f
    };

    bool test1 = Collision::IntersectsAABBCapsule(box, capsule1);
    bool test2 = Collision::IntersectsAABBCapsule(box, capsule2);
    bool test3 = Collision::IntersectsAABBCapsule(box, capsule3);
    bool test4 = Collision::IntersectsAABBCapsule(box, capsule4);

    OutputDebugStringA(test1 ? "Test 1: PASS\n" : "Test 1: FAIL\n");
    OutputDebugStringA(test2 ? "Test 2: PASS\n" : "Test 2: FAIL\n");
    OutputDebugStringA(!test3 ? "Test 3: PASS\n" : "Test 3: FAIL\n");
    OutputDebugStringA(test4 ? "Test 4: PASS\n" : "Test 4: FAIL\n");

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr,
            0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            input.Update();

            if (input.IsKeyPressed(VK_F1))
                showColliderDebug = !showColliderDebug;

            DirectX::XMFLOAT3 direction = { 0, 0, 0 };
            auto currentTime = std::chrono::steady_clock::now();

            double deltaTime =
                std::chrono::duration<double>(currentTime - previousTime).count();

            previousTime = currentTime;

            if (input.IsKeyDown('A'))
            {
                direction.x -= 1;
            }
            if (input.IsKeyDown('D'))
            {
                direction.x += 1;
            }
            if (input.IsKeyDown('S'))
            {
                direction.z -= 1;
            }
            if (input.IsKeyDown('W'))
            {
                direction.z += 1;
            }
            scene[0]->movement.Move(scene[0]->transform, direction, static_cast<float>(deltaTime));

            WorldAABB worldA = Collision::ToWorldAABB(scene[0]->collider, scene[0]->transform.position,
                scene[0]->transform.scale);
            WorldAABB worldB = Collision::ToWorldAABB(scene[1]->collider, scene[1]->transform.position,
                scene[1]->transform.scale);

            bool isColliding = Collision::IntersectsAABB(worldA, worldB);
            //OutputDebugStringA(isColliding ? "Collision: TRUE\n" : "Collision: FALSE\n");

            SceneUpdate(scene, deltaTime);
            renderer.BeginFrame();
            renderer.Draw(scene, camera);

            if (showColliderDebug)
                debugRenderer.DrawColliders(scene, camera);

            renderer.EndFrame();
        }
    }

    return 0;
}