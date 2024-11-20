#pragma once

#ifndef BOX2D_H
#define BOX2D_H

#include"../box2d/include/box2d/box2d.h"
#include"../box2d/include/box2d/box2d.h"
#include"../box2d/include/box2d/collision.h"
#include"../box2d/include/box2d/id.h"
#include"../box2d/include/box2d/math_functions.h"
#include"../box2d/include/box2d/types.h"

//box2d�T�C�g
//=============================================
//https://box2d.org/documentation/index.html
//=============================================

//�d��
#define GRAVITY -10.0f

//���[���h�X�V���}���`�X���b�h�ɂ���
//=============================================
//#define BOX2D_UPDATE_MULTITHREAD
//=============================================

class GameObject;
class Box2DBody;
class SceneManager;

namespace Box2D
{
	enum BOX2D_SHAPE
	{
		BOX,
		CIRCLE,
		CUPSULE,
		COUPOUND,
		SHAPE_MAX
	};

	//���[���h�X�V�X�e�b�v
	//======================================
	const float timeStep = 1.0f / WORLD_FPS;
	const int subStepCount = 4;
	//======================================

	class WorldManager
	{
		friend class Window;
		friend class GameObject;
		friend class Box2DBody;
		friend class SceneManager;

	public:
		//Start����End�܂�Box2dBody�����邩
		static bool RayCast(Vector2 _start, Vector2 _end);
		//Start����End�܂�Box2dBody�����邩(output�ɏՓˈʒu����)
		static bool RayCast(Vector2 _start, Vector2 _end, Vector2& _output);
		//Start����End�܂�Box2dBody�����邩(filter�w��)
		static bool RayCast(Vector2 _start, Vector2 _end, FILTER _filter);
		//Start����End�܂�Box2dBody�����邩(output�ɏՓˈʒu����,filter�w��)
		static bool RayCast(Vector2 _start, Vector2 _end, Vector2& _output, FILTER _filter);

		static bool RayCastAll(Vector2 _start, Vector2 _end, std::vector<Vector2>& _output);
		static bool RayCastAll(Vector2 _start, Vector2 _end, std::vector<Vector2>& _output, FILTER _filter);

	private:
		//�����֎~
		WorldManager() = delete;
		//�V���~���[�V�������[���h�̍쐬
		static void CreateWorld();
		//body�̍쐬
		static void GenerateBody(b2BodyId& _bodyId, const b2BodyDef* _bodyDef = &bodyDef);
#ifdef BOX2D_UPDATE_MULTITHREAD
		//���[���h�̍X�V
		static void WorldUpdate();
		//���[���h�̍X�V���n�߂�
		static void StartWorldUpdate();
		//���[���h�̍X�V���~�߂�
		static void StopWorldUpdate();
		//���[���h�̍X�V���ꎞ��~
		static void PauseWorldUpdate();
		//���[���h�̍X�V���ĊJ
		static void ResumeWorldUpdate();
		//���[���h�̍X�V��ύX����@�\�ĊJ
		static void EnableWorldUpdate();
		//���[���h�̍X�V��ύX����@�\��~
		static void DisableWorldUpdate();
		//���[���h�̃^�X�N��ǉ�����
		static void AddWorldTask(std::function<void()>&& _task);
#else
		//���[���h�̍X�V
		static void WorldUpdate();
#endif
		//�Z���T�[�C�x���g�̊m�F�Ǝ��s
		static void ExecuteSensorEvent();
		//���[���h�̂�������
		static void DeleteAllWorld();
		//���̃��[���h�ɂ���
		static void ChangeNextWorld();
		//���̃��[���h�Ɍq����
		static void LinkNextWorld();
		//�Â����[���h���폜����
		static void DeleteOldWorld();
	private:
		static thread_local b2WorldId* currentWorldId;
		static b2WorldId worldId;
		static b2WorldId nextWorldId;
		static b2WorldId eraseWorldId;
		static b2BodyDef bodyDef;

#ifdef BOX2D_UPDATE_MULTITHREAD
	private:
		//�񓯊����[�h�̎��Ƀ��[���h�X�V��
		//�~�߂Ȃ��悤�ɂ��邽�ߊ֐���؂�ւ���
		static thread_local void(*pPauseWorldUpdate)();
		static thread_local void(*pResumeWorldUpdate)();
	private:
		//�}���`�X���b�h�p�ϐ�
		static std::atomic<bool> running;
		static std::atomic<bool> paused;
		static std::atomic<bool> actuallyPaused;
		static std::thread worldUpdateThread;
		static std::mutex threadMutex;
		static std::condition_variable cv;
		static std::condition_variable pauseCv;

		static std::vector<std::function<void()>> worldTask;
		static std::mutex worldTaskMutex;
#endif
	};

	//�f�o�b�O�\���̃��l
	//========================================================
	const float b2_colorAlpha = 1.0f;
	//========================================================

	const XMFLOAT4 b2_colorRed =	{ 1.0f,0.0f,0.0f,b2_colorAlpha };
	const XMFLOAT4 b2_colorGreen =	{ 0.0f,1.0f,0.0f,b2_colorAlpha };
	const XMFLOAT4 b2_colorBlue =	{ 0.4f,0.4f,1.0f,b2_colorAlpha };
	const XMFLOAT4 b2_colorGray =	{ 0.6f,0.6f,0.6f,b2_colorAlpha };
	const XMFLOAT4 b2_colorPink =	{ 1.0f,0.4f,0.4f,b2_colorAlpha };
};


#endif
