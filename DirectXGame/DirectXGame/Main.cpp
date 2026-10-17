#include "Core/Renderer.h"
#include "Scene/GameObject.h"
#include "Core/Camera.h"
#include "Graphics/ModelLoader.h"
#include "Scene/Scene.h"

#include <Windows.h>
#include <DirectXMath.h>
#include <memory>
#include <utility>

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


HRESULT AddModelToScene(const Renderer& renderer, Scene& scene, const char* filePath,
    DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale)
{
    ModelData modelData;

    if (!ModelLoader::Load(filePath, modelData))
        return E_FAIL;
    std::unique_ptr<GameObject> model = std::make_unique<GameObject>();
    model->transform.scale = scale;
    model->transform.position = position;
    HRESULT modelHr = model->model.Initialize(
        renderer.GetDevice(), renderer.GetContext(),
        modelData);

    if (FAILED(modelHr))
        return modelHr;

    scene.AddObject(std::move(model));
    return S_OK;
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
    Scene scene;
    DirectX::XMFLOAT3 scale;
    DirectX::XMFLOAT3 position;

    scale = { 0.3f, 0.3f, 0.3f };
    position = { 0.0f, 0.0f, 0.0f };
    hr = AddModelToScene(renderer, scene, "Assets/Barrel/model.obj", position, scale);
    if (FAILED(hr))
    {
        MessageBox(hwnd, L"model Add Failed", L"Error", MB_OK);
        return 0;
    }
    scale = { 0.01f, 0.01f, 0.01f };
    position = { 3.0f, 0.0f, 0.0f };
    hr = AddModelToScene(renderer, scene, "Assets/Chair/chair01_textured.obj", position, scale);
    if (FAILED(hr))
    {
        MessageBox(hwnd, L"Chair Add Failed", L"Error", MB_OK);
        return 0;
    }

    RECT rect;
    GetClientRect(hwnd, &rect);
    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);

    Camera camera(width, height);
    gCamera = &camera;
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
            renderer.BeginFrame();
            renderer.Draw(scene, camera);
            renderer.EndFrame();
        }
    }

    return 0;
}