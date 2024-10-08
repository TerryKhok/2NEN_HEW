#pragma once

//====================================================
// CameraComponentをつくってカメラの制御をしやすくする
//====================================================

class CameraManager
{
	friend class Window;
private:
	CameraManager() = delete;
	//カメラの行列計算をする
	static void CameraMatrixCalculation();
public:
	static Vector2 cameraPosition;
	static float cameraRotation;
	static float cameraZoom;
};
