#include "Renderer.h"
#include "GameObject.h"
#include "Camera.h"

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

    Vertex vertices[24] = {
        //앞 0, 1, 2, 3
        {-0.5,  0.5,  0.5, 1, 1, 1, 1, 0.0f, 0.0f},
        {0.5,  0.5,  0.5, 1, 1, 1, 1, 1.0f, 0.0f},
        {0.5, -0.5,  0.5, 1, 1, 1, 1, 1.0f, 1.0f},
        {-0.5, -0.5,  0.5, 1, 1, 1, 1, 0.0f, 1.0f},
        //뒤 4, 5, 6, 7
        {-0.5,  0.5,  -0.5, 1, 1, 1, 1, 0.0f, 0.0f},
        {0.5,  0.5,  -0.5, 1, 1, 1, 1, 1.0f, 0.0f},
        {0.5, -0.5,  -0.5, 1, 1, 1, 1, 1.0f, 1.0f},
        {-0.5, -0.5,  -0.5, 1, 1, 1, 1, 0.0f, 1.0f},
        //오른쪽 8, 9, 10, 11
        {0.5,  0.5,  0.5, 1, 1, 1, 1, 0.0f, 0.0f},
        {0.5,  0.5,  -0.5, 1, 1, 1, 1, 1.0f, 0.0f},
        {0.5, -0.5,  -0.5, 1, 1, 1, 1, 1.0f, 1.0f},
        {0.5, -0.5,  0.5, 1, 1, 1, 1, 0.0f, 1.0f},
        //왼쪽 12, 13, 14, 15
        {-0.5,  0.5,  -0.5, 1, 1, 1, 1, 0.0f, 0.0f},
        {-0.5,  0.5,  0.5, 1, 1, 1, 1, 1.0f, 0.0f},
        {-0.5, -0.5,  0.5, 1, 1, 1, 1, 1.0f, 1.0f},
        {-0.5, -0.5,  -0.5, 1, 1, 1, 1, 0.0f, 1.0f},
        //위 16, 17, 18, 19
        {-0.5,  0.5,  -0.5, 1, 1, 1, 1, 0.0f, 0.0f},
        {0.5,  0.5,  -0.5, 1, 1, 1, 1, 1.0f, 0.0f},
        {0.5,  0.5,  0.5, 1, 1, 1, 1, 1.0f, 1.0f},
        {-0.5,  0.5,  0.5, 1, 1, 1, 1, 0.0f, 1.0f},
        //아래 20, 21, 22, 23
        {-0.5, -0.5,  0.5, 1, 1, 1, 1, 0.0f, 0.0f},
        {0.5, -0.5,  0.5, 1, 1, 1, 1, 1.0f, 0.0f},
        {0.5, -0.5,  -0.5, 1, 1, 1, 1, 1.0f, 1.0f},
        {-0.5, -0.5,  -0.5, 1, 1, 1, 1, 0.0f, 1.0f},
    };

    unsigned int indices[36] = {
        //앞
        0, 2, 1,
        0, 3, 2,
        //뒤
        4, 5, 6,
        4, 6, 7,
        //오른쪽
        8, 10, 9,
        8, 11, 10,
        //왼쪽
        12, 14, 13,
        12, 15, 14,
        //위
        16, 18, 17,
        16, 19, 18,
        //아래
        20, 22, 21,
        20, 23, 22
    };

    HRESULT meshHr = cube.mesh.Initialize(
        renderer.GetDevice(),
        vertices,
        24,
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