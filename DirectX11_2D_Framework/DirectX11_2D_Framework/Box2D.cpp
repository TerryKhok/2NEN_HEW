#include "Box2D.h"

thread_local b2WorldId Box2D::WorldManager::currentWorldId;
b2WorldId Box2D::WorldManager::worldId;
b2WorldId Box2D::WorldManager::nextWorldId;
b2BodyDef Box2D::WorldManager::bodyDef;


void Box2D::WorldManager::CreateWorld()
{
	//ワールド定義、初期化
	b2WorldDef worldDef = b2DefaultWorldDef();
	//重力の設定
	worldDef.gravity = { 0.0f, GRAVITY };
	//ワールドオブジェクト作成
	worldId = b2CreateWorld(&worldDef);

	currentWorldId = worldId;
}

void Box2D::WorldManager::GenerataeBody(b2BodyId& _bodyId,const b2BodyDef* _bodyDef)
{
	//グランドボディの作成
	_bodyId = b2CreateBody(currentWorldId, _bodyDef);
}

void Box2D::WorldManager::UpdateWorld()
{
	b2World_Step(worldId, timeStep, subStepCount);
}

void Box2D::WorldManager::DeleteWorld()
{
	//シミュレーションが終わったら、世界を破壊しなければならない。
	b2DestroyWorld(worldId);
}

void Box2D::WorldManager::ChengeNextWorld()
{
	//ワールド定義、初期化
	b2WorldDef worldDef = b2DefaultWorldDef();
	//重力の設定
	worldDef.gravity = { 0.0f, GRAVITY };
	//ワールドオブジェクト作成
	nextWorldId = b2CreateWorld(&worldDef);

	currentWorldId = nextWorldId;
}

void Box2D::WorldManager::LinkNextWorld()
{
	DeleteWorld();
	worldId = nextWorldId;
}
