#pragma once

//====================================================
// CameraComponent�������ăJ�����̐�������₷������
//====================================================

class CameraManager
{
	friend class Window;
private:
	CameraManager() = delete;
	//�J�����̍s��v�Z������
	static void CameraMatrixCalculation();
public:
	static Vector2 cameraPosition;
	static float cameraRotation;
	static float cameraZoom;
};
