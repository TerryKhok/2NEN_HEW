#include "Camera.h"

Vector2 CameraManager::cameraPosition = { 0.0f,0.0f };
float CameraManager::cameraRotation = 0.0f;
float CameraManager::cameraZoom = 2.0f;

void CameraManager::CameraMatrixCalculation()
{
    //auto& cb = GameObject::m_cb;
    static VSCameraConstantBuffer cb;

    //View変換
    static XMVECTOR cameraPos = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
    static XMVECTOR focusPoint = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    static XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    float rad = Math::DegToRad(cameraRotation);

    cameraPos =  XMVectorSet(cameraPosition.x, cameraPosition.y, -4.0f, 0.0f);
    focusPoint = XMVectorSet(cameraPosition.x, cameraPosition.y, 0.0f, 0.0f);
    upDirection = XMVectorSet(sin(rad), cos(rad), 0.0f, 0.0f);

    cb.view = XMMatrixLookAtLH(cameraPos, focusPoint, upDirection);
    cb.view = DirectX::XMMatrixTranspose(cb.view);

    //Projection変換
    float width = PROJECTION_WIDTH;
    float height = PROJECTION_HEIGHT;

    cameraZoom = max(0.1f, cameraZoom);

    width /= cameraZoom;
    height /= cameraZoom;

    cb.projection = DirectX::XMMatrixOrthographicLH(width, height, 0.0f, 5.0f);

    //行列をシェーダーに渡す
    DirectX11::m_pDeviceContext->UpdateSubresource(
        DirectX11::m_pVSCameraConstantBuffer.Get(), 0, NULL, &cb, 0, 0);
}
