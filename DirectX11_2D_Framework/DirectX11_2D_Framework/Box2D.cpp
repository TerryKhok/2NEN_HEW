#include "Box2D.h"

thread_local b2WorldId Box2D::WorldManager::currentWorldId;
b2WorldId Box2D::WorldManager::worldId;
b2WorldId Box2D::WorldManager::nextWorldId;
b2BodyDef Box2D::WorldManager::bodyDef;


void Box2D::WorldManager::CreateWorld()
{
	//���[���h��`�A������
	b2WorldDef worldDef = b2DefaultWorldDef();
	//�d�͂̐ݒ�
	worldDef.gravity = { 0.0f, GRAVITY };
	//���[���h�I�u�W�F�N�g�쐬
	worldId = b2CreateWorld(&worldDef);

	currentWorldId = worldId;
}

void Box2D::WorldManager::GenerataeBody(b2BodyId& _bodyId,const b2BodyDef* _bodyDef)
{
	//�O�����h�{�f�B�̍쐬
	_bodyId = b2CreateBody(currentWorldId, _bodyDef);
}

void Box2D::WorldManager::UpdateWorld()
{
	b2World_Step(worldId, timeStep, subStepCount);
}

void Box2D::WorldManager::DeleteWorld()
{
	//�V�~�����[�V�������I�������A���E��j�󂵂Ȃ���΂Ȃ�Ȃ��B
	b2DestroyWorld(worldId);
}

void Box2D::WorldManager::ChengeNextWorld()
{
	//���[���h��`�A������
	b2WorldDef worldDef = b2DefaultWorldDef();
	//�d�͂̐ݒ�
	worldDef.gravity = { 0.0f, GRAVITY };
	//���[���h�I�u�W�F�N�g�쐬
	nextWorldId = b2CreateWorld(&worldDef);

	currentWorldId = nextWorldId;
}

void Box2D::WorldManager::LinkNextWorld()
{
	DeleteWorld();
	worldId = nextWorldId;
}
