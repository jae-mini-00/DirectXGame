#pragma once

#include <DirectXMath.h>

class Camera
{
    public:
        Camera(float width, float height);

        void Zoom(float delta);
        const DirectX::XMMATRIX& GetView() const;
        const DirectX::XMMATRIX& GetProjection() const;

    private:
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        float distance;

        void UpdateView();
};