#pragma once

#include <DirectXMath.h>

class Camera
{
public:
    Camera(float width, float height);

    const DirectX::XMMATRIX& GetView() const;
    const DirectX::XMMATRIX& GetProjection() const;

private:
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
};