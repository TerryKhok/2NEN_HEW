#pragma once

#ifndef BOX2D_H
#define BOX2D_H

#include"box2d/include/box2d.h"
#include"box2d/include/box2d.h"
#include"box2d/include/collision.h"
#include"box2d/include/id.h"
#include"box2d/include/math_functions.h"
#include"box2d/include/types.h"

//box2dサイト
//=============================================
//https://box2d.org/documentation/index.html
//=============================================

#define GRAVITY -100.0f

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
	const float timeStep = 1.0f / 60.0f;
	const int subStepCount = 4;
	//======================================

	class WorldManager
	{
		friend class Window;
		friend class Box2DBody;
		friend class SceneManager;

	private:
		//シュミレーションワールドの作成
		static void CreateWorld();
		//bodyの作成
		static void GenerataeBody(b2BodyId& _bodyId, const b2BodyDef* _bodyDef = &bodyDef);
		//ワールドの更新
		static void UpdateWorld();
		//ワールドのかたずけ
		static void DeleteWorld();
		//次のワールドにする
		static void ChengeNextWorld();
		//次のワールドに繋げる
		static void LinkNextWorld();
	private:
		static thread_local b2WorldId currentWorldId;
		static b2WorldId worldId;
		static b2WorldId nextWorldId;
		static b2BodyDef bodyDef;
	};

	//デバッグ表示のα値
	//========================================================
	const float b2_colorAlpha = 0.2f;
	//========================================================

	const XMFLOAT4 b2_colorRed =	{ 1.0f,0.0f,0.0f,b2_colorAlpha };
	const XMFLOAT4 b2_colorGreen =	{ 0.0f,1.0f,0.0f,b2_colorAlpha };
	const XMFLOAT4 b2_colorBlue =	{ 0.0f,0.0f,1.0f,b2_colorAlpha };
	const XMFLOAT4 b2_colorGray =	{ 0.5f,0.5f,0.5f,b2_colorAlpha };
	const XMFLOAT4 b2_colorPink =	{ 1.0f,0.4f,0.4f,b2_colorAlpha };
};


#endif
