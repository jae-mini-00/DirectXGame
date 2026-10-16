#include "Core/Camera.h"

const DirectX::XMMATRIX& Camera::GetView() const { return view; }
const DirectX::XMMATRIX& Camera::GetProjection() const { return projection; }

Camera::Camera(float width, float height)
{
    distance = 3.0f;
    DirectX::XMVECTOR eye =
        DirectX::XMVectorSet(0.0f, 0.0f, -distance, 0.0f);

    DirectX::XMVECTOR target =
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

    DirectX::XMVECTOR up =
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    view = DirectX::XMMatrixLookAtLH(eye, target, up);

    float aspect = width / height;

    projection = DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(60.0f), aspect,
            0.1f, 100.0f);
}

void Camera::Zoom(float delta)
{
    distance += delta;

    if (distance >= 30.0f)
        distance = 30.0f;
    else if (distance <= 1.0f)
        distance = 1.0f;
    UpdateView();
}

void Camera::UpdateView()
{
    DirectX::XMVECTOR eye =
        DirectX::XMVectorSet(0.0f, 0.0f, -distance, 0.0f);

    DirectX::XMVECTOR target =
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

    DirectX::XMVECTOR up =
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    view = DirectX::XMMatrixLookAtLH(eye, target, up);

}