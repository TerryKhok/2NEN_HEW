

Vector2 CameraManager::cameraPosition = { 0.0f,0.0f };
Vector2 CameraManager::cameraZoom = { 2.0f,2.0f };
float CameraManager::cameraRotation = 0.0f;
VSCameraConstantBuffer CameraManager::m_cb;


void CameraManager::SetCameraMatrix()
{
    //auto& cb = GameObject::m_cb;

    //View変換
    static XMVECTOR cameraPos = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
    static XMVECTOR focusPoint = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    static XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    float rad = Math::DegToRad(cameraRotation);

    cameraPos =  XMVectorSet(cameraPosition.x, cameraPosition.y, -2.0f, 0.0f);
    focusPoint = XMVectorSet(cameraPosition.x, cameraPosition.y, 0.0f, 0.0f);
    upDirection = XMVectorSet(sin(rad), cos(rad), 0.0f, 0.0f);

    m_cb.view = XMMatrixLookAtLH(cameraPos, focusPoint, upDirection);
    m_cb.view = DirectX::XMMatrixTranspose(m_cb.view);

    //Projection変換
    float width = PROJECTION_WIDTH;
    float height = PROJECTION_HEIGHT;

    cameraZoom.x = max(0.1f, cameraZoom.x);
    cameraZoom.y = max(0.1f, cameraZoom.y);

    width /= cameraZoom.x;
    height /= cameraZoom.y;

    m_cb.projection = DirectX::XMMatrixOrthographicLH(width, height, 0.0f, 3.0f);

    //行列をシェーダーに渡す
    DirectX11::m_pDeviceContext->UpdateSubresource(
        DirectX11::m_pVSCameraConstantBuffer.Get(), 0, NULL, &m_cb, 0, 0);
}
