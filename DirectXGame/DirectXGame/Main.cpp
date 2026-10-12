#include "Renderer.h"
#include "GameObject.h"
#include "Camera.h"
#include "ModelLoader.h"

#include <chrono>

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

    HWND hwnd = CreateWindowEx(
        0,
        L"GameWindow",
        L"DirectX Game",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        1280,
        720,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hwnd == nullptr)
        return 0;

    Renderer renderer;
    HRESULT hr = renderer.Initialize(hwnd);

    if (FAILED(hr))
    {
        MessageBox(
            hwnd,
            L"Renderer Initialize Failed",
            L"Error",
            MB_OK
        );
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    GameObject cube;
    ModelData modelData;

    if (!ModelLoader::Load("cube.obj", modelData))
    {
        MessageBox(
            hwnd,
            L"Model Load Failed",
            L"Error",
            MB_OK
        );

        return 0;
    }

    HRESULT modelHr = cube.model.Initialize(
        renderer.GetDevice(),
        renderer.GetContext(),
        modelData
    );

    if (FAILED(modelHr))
    {
        MessageBox(
            hwnd,
            L"Model Initialize Failed",
            L"Error",
            MB_OK
        );

        return 0;
    }
    RECT rect;
    GetClientRect(hwnd, &rect);
    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);

    Camera camera(width, height);

    cube.transform.rotation.x = 30.0f;
    cube.transform.rotation.y = 45.0f;
    auto lastTime = std::chrono::steady_clock::now();
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(
            &msg,
            nullptr,
            0,
            0,
            PM_REMOVE))
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

            cube.transform.rotation.y += 90.0f * deltaTime;
            renderer.BeginFrame();

            renderer.Draw(cube, camera);

            renderer.EndFrame();
        }
    }

    return 0;
}