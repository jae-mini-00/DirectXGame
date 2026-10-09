#include "Renderer.h"
#include <chrono>

LRESULT CALLBACK WindowProc(
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

int WINAPI WinMain(
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

    Vertex vertices[8] = {
        // +Z
        { -0.5f,  0.5f,  0.5f, 0.0f, 0.8f, 1.0f, 1.0f }, // 0 Ã»·Ï
        {  0.5f,  0.5f,  0.5f, 1.0f, 0.85f, 0.0f, 1.0f }, // 1 ³ë¶û

        {  0.5f, -0.5f,  0.5f, 1.0f, 0.85f, 0.0f, 1.0f }, // 2 ³ë¶û
        { -0.5f, -0.5f,  0.5f, 0.0f, 0.8f, 1.0f, 1.0f }, // 3 Ã»·Ï

        // -Z
        { -0.5f,  0.5f, -0.5f, 0.35f, 0.0f, 1.0f, 1.0f }, // 4 º¸¶ó
        {  0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.25f, 1.0f }, // 5 »¡°­/ÇÎÅ©

        {  0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.25f, 1.0f }, // 6 »¡°­/ÇÎÅ©
        { -0.5f, -0.5f, -0.5f, 0.35f, 0.0f, 1.0f, 1.0f }  // 7 º¸¶ó
    };

    unsigned int indices[36] = {
        // +Z
        0, 2, 1,
        0, 3, 2,

        // +X
        1, 6, 5,
        1, 2, 6,

        // -Z
        4, 5, 6,
        4, 6, 7,

        // -X
        0, 7, 3,
        0, 4, 7,

        // +Y
        4, 1, 5,
        4, 0, 1,

        // -Y
        3, 6, 2,
        3, 7, 6
    };

    HRESULT meshHr = cube.mesh.Initialize(
        renderer.GetDevice(),
        vertices,
        8,
        indices,
        36
    );

    if (FAILED(meshHr))
    {
        MessageBox(
            hwnd,
            L"Mesh Initialize Failed",
            L"Error",
            MB_OK
        );

        return 0;
    }
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

            renderer.Draw(cube);

            renderer.EndFrame();
        }
    }

    return 0;
}