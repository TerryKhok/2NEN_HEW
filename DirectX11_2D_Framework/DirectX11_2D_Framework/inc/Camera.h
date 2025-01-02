#pragma once

//====================================================
// CameraComponent�������ăJ�����̐�������₷������
//====================================================

class CameraManager
{
	friend class Window;
	friend class RenderManager;
	friend class TileMap;

private:
	CameraManager() = delete;
	//�J�����̍s��v�Z������
	static void SetCameraMatrix();
private:
	//�e�E�B���h�E���ߗp
	static Vector2 cameraPosition;
	static Vector2 cameraZoom;
public:
	static float cameraRotation;
private:
	static VSCameraConstantBuffer m_cb;
};
