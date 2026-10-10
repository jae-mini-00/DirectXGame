#include "Camera.h"

const DirectX::XMMATRIX& Camera::GetView() const { return view; }

const DirectX::XMMATRIX& Camera::GetProjection() const { return projection; }

Camera::Camera(float width, float height)
{
    DirectX::XMVECTOR eye =
        DirectX::XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);

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