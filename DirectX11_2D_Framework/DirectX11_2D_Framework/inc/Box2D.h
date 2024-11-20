#pragma once

#ifndef BOX2D_H
#define BOX2D_H

#include"../box2d/include/box2d/box2d.h"
#include"../box2d/include/box2d/box2d.h"
#include"../box2d/include/box2d/collision.h"
#include"../box2d/include/box2d/id.h"
#include"../box2d/include/box2d/math_functions.h"
#include"../box2d/include/box2d/types.h"

//box2dサイト
//=============================================
//https://box2d.org/documentation/index.html
//=============================================

//重力
#define GRAVITY -10.0f

//ワールド更新をマルチスレッドにする
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

	//ワールド更新ステップ
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
		//StartからEndまでBox2dBodyがあるか
		static bool RayCast(Vector2 _start, Vector2 _end);
		//StartからEndまでBox2dBodyがあるか(outputに衝突位置を代入)
		static bool RayCast(Vector2 _start, Vector2 _end, Vector2& _output);
		//StartからEndまでBox2dBodyがあるか(filter指定)
		static bool RayCast(Vector2 _start, Vector2 _end, FILTER _filter);
		//StartからEndまでBox2dBodyがあるか(outputに衝突位置を代入,filter指定)
		static bool RayCast(Vector2 _start, Vector2 _end, Vector2& _output, FILTER _filter);

		static bool RayCastAll(Vector2 _start, Vector2 _end, std::vector<Vector2>& _output);
		static bool RayCastAll(Vector2 _start, Vector2 _end, std::vector<Vector2>& _output, FILTER _filter);

	private:
		//生成禁止
		WorldManager() = delete;
		//シュミレーションワールドの作成
		static void CreateWorld();
		//bodyの作成
		static void GenerateBody(b2BodyId& _bodyId, const b2BodyDef* _bodyDef = &bodyDef);
#ifdef BOX2D_UPDATE_MULTITHREAD
		//ワールドの更新
		static void WorldUpdate();
		//ワールドの更新を始める
		static void StartWorldUpdate();
		//ワールドの更新を止める
		static void StopWorldUpdate();
		//ワールドの更新を一時停止
		static void PauseWorldUpdate();
		//ワールドの更新を再開
		static void ResumeWorldUpdate();
		//ワールドの更新を変更する機能再開
		static void EnableWorldUpdate();
		//ワールドの更新を変更する機能停止
		static void DisableWorldUpdate();
		//ワールドのタスクを追加する
		static void AddWorldTask(std::function<void()>&& _task);
#else
		//ワールドの更新
		static void WorldUpdate();
#endif
		//センサーイベントの確認と実行
		static void ExecuteSensorEvent();
		//ワールドのかたずけ
		static void DeleteAllWorld();
		//次のワールドにする
		static void ChangeNextWorld();
		//次のワールドに繋げる
		static void LinkNextWorld();
		//古いワールドを削除する
		static void DeleteOldWorld();
	private:
		static thread_local b2WorldId* currentWorldId;
		static b2WorldId worldId;
		static b2WorldId nextWorldId;
		static b2WorldId eraseWorldId;
		static b2BodyDef bodyDef;

#ifdef BOX2D_UPDATE_MULTITHREAD
	private:
		//非同期ロードの時にワールド更新を
		//止めないようにするため関数を切り替える
		static thread_local void(*pPauseWorldUpdate)();
		static thread_local void(*pResumeWorldUpdate)();
	private:
		//マルチスレッド用変数
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

	//デバッグ表示のα値
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
