#pragma once

//====================================================
// CameraComponentをつくってカメラの制御をしやすくする
//====================================================

class CameraManager
{
	friend class Window;
	friend class RenderManager;
	friend class TileMap;

private:
	CameraManager() = delete;
	//カメラの行列計算をする
	static void SetCameraMatrix();
private:
	//各ウィンドウ調節用
	static Vector2 cameraPosition;
	static Vector2 cameraZoom;
public:
	static float cameraRotation;
private:
	static VSCameraConstantBuffer m_cb;
};
