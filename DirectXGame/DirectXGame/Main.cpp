#include "Renderer.h"
#include "GameObject.h"
#include "Camera.h"
#include "ModelLoader.h"
#include "Scene.h"

#include <chrono>

Camera* gCamera = nullptr;

LRESULT CALLBACK WindowProc
(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
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
        MessageBox( hwnd, L"Renderer Initialize Failed",
            L"Error", MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    ModelData modelChair;
    ModelData modelBarrel;

    if (!ModelLoader::Load("chair01_textured.obj", modelChair))
    {
        MessageBox( hwnd, L"Model Load Failed",
            L"Error", MB_OK);

        return 0;
    }
    if (!ModelLoader::Load("model.obj", modelBarrel))
    {
        MessageBox(hwnd, L"Model Load Failed",
            L"Error", MB_OK);

        return 0;
    }

    std::unique_ptr<GameObject> chair = std::make_unique<GameObject>();
    chair->transform.scale = { 0.01f, 0.01f, 0.01f };
    HRESULT modelHr = chair->model.Initialize(
        renderer.GetDevice(), renderer.GetContext(),
        modelChair);

    if (FAILED(modelHr))
    {
        MessageBox(hwnd, L"Model Initialize Failed",
            L"Error", MB_OK);

        return 0;
    }

    std::unique_ptr<GameObject> barrel = std::make_unique<GameObject>();
    barrel->transform.scale = { 0.50f, 0.50f, 0.50f };
    barrel->transform.position = { 3.01f, 0.01f, 0.01f };
    modelHr = barrel->model.Initialize(
        renderer.GetDevice(), renderer.GetContext(),
        modelBarrel);

    if (FAILED(modelHr))
    {
        MessageBox(hwnd, L"Model Initialize Failed",
            L"Error", MB_OK);

        return 0;
    }



    Scene scene;
    scene.AddObject(std::move(chair));
    scene.AddObject(std::move(barrel));

    RECT rect;
    GetClientRect(hwnd, &rect);
    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);

    Camera camera(width, height);
    gCamera = &camera;
    auto lastTime = std::chrono::steady_clock::now();
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
            auto currentTime = std::chrono::steady_clock::now();

            std::chrono::duration<float> elapsed =
                currentTime - lastTime;

            float deltaTime = elapsed.count();

            lastTime = currentTime;

            scene.GetObjects()[0]->transform.rotation.y += 30.0f * deltaTime;
            renderer.BeginFrame();

            renderer.Draw(scene, camera);

            renderer.EndFrame();
        }
    }

    return 0;
}