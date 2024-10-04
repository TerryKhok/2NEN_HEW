#include"HelloBox2d.h"

//box2d�`���[�g���A��
//=============================================
//https://box2d.org/documentation/hello.html
//=============================================

b2WorldId worldId;
b2BodyId bodyId;
b2DebugDraw debugDraw;

void HelloBox2dInit()
{
	//================================================================
	// Creating a Ground Box
	//================================================================
	
	//���[���h��`�A������
	b2WorldDef worldDef = b2DefaultWorldDef();

	//�d�͂̐ݒ�
	worldDef.gravity = { 0.0f, -10.0f };

	//���[���h�I�u�W�F�N�g�쐬
	worldId = b2CreateWorld(&worldDef);

	//�{�f�B��`�ƃ��[���hID���g���ăO���E���h�E�{�f�B���쐬����
	b2BodyDef groundBodyDef = b2DefaultBodyDef();
	groundBodyDef.position = { 0.0f, -10.0f };

	//�O�����h�{�f�B�̍쐬
	b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

	//�n�ʃ|���S�������B b2MakeBox()�w���p�[�֐����g���A�n�ʃ|���S���𔠌^�ɂ���B���̒��S�͐e�{�f�B�̌��_�ł���B
	b2Polygon groundBox = b2MakeBox(50.0f, 10.0f);

	//�V�F�C�v���쐬���Ēn�ʂ̃{�f�B���d�グ��
	b2ShapeDef groundShapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);


	//================================================================
	// Creating a Dynamic Body
	//================================================================

	//CreateBody���g���ă{�f�B���쐬����B �f�t�H���g�ł̓{�f�B�͐ÓI�Ȃ̂ŁA�쐬����b2BodyType��ݒ肵�ă{�f�B�𓮓I�ɂ���
	//�{�f�B���쐬���Ă���ړ�������͔̂��ɔ�����I�̂��ߍ쐬����O�ɓ�����
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { 0.0f, 4.0f };
	bodyId = b2CreateBody(worldId, &bodyDef);

	//������ �{�f�B��́i�d�͂Ȃǁj�ɔ������ē����������ꍇ�́A�{�f�B�^�C�v��b2_dynamicBody�ɐݒ肷��K�v������܂��B

	//�V�F�C�v��`���g���ă|���S���`����쐬���A�A�^�b�`����B �܂��A�ʂ̃{�b�N�X�V�F�C�v���쐬����
	b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);

	//�{�b�N�X�̌`���`���쐬����B �f�t�H���g�̖��x��1�Ȃ̂ŁA����͕s�v�ł���B �܂��A�`��̖��C��0.3�ɐݒ肳��Ă���B
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.friction = 0.3f;

	//������ �_�C�i�~�b�N�{�f�B�́A���x���[���łȂ��`������Ȃ��Ƃ�1���ׂ��ł���B �����łȂ��ƁA��ȓ���ɂȂ�

	//�V�F�C�v�̒�`���g���āA�V�F�C�v����邱�Ƃ��ł���B ����Ń{�f�B�̎��ʂ������I�ɍX�V�����B 
	//�{�f�B�ɂ͍D���Ȃ����`���ǉ��ł��܂��B ���ꂼ�ꂪ�����ʂɊ�^����B
	b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);
}

void HelloBox2dUpdate()
{
	//================================================================
	// Simulating the World
	//================================================================

	//Box2D�̃^�C���E�X�e�b�v��I��
	//�܂��A�^�C���X�e�b�v���t���[�����ƂɈقȂ�̂��悭����܂���B �^�C���X�e�b�v���ς��ƌ��ʂ��ς��A�f�o�b�O������Ȃ�܂��B
	//  ������A�^�C���X�e�b�v���t���[�����[�g�Ɍ��т��Ă͂����Ȃ�
	float timeStep = 1.0f / 60.0f;

	//Box2D�́A����̔����̎�i�Ƃ��ăT�u�X�e�b�v���g�p���܂��B Box2D�̐����T�u�X�e�b�v����4�ł��B
	int subStepCount = 4;

	//�^�C���X�e�b�v�ƃT�u�X�e�b�v���͊֘A���Ă��邱�Ƃɒ��ӁB �^�C���X�e�b�v���������Ȃ�ƁA�T�u�X�e�b�v�̃T�C�Y���������Ȃ�B 
	//�Ⴆ�΁A60Hz�̃^�C���X�e�b�v��4�̃T�u�X�e�b�v�ł́A�T�u�X�e�b�v��240Hz�œ��삵�܂�
	//�Q�[���ł́A�V�~�����[�V�������[�v���Q�[�����[�v�Ƀ}�[�W���邱�Ƃ��ł��܂��B �Q�[�����[�v��ʉ߂��邽�тɁAb2World_Step()���Ăяo���B 
	//�t���[�����[�g�╨�����Z�̃^�C���X�e�b�v�ɂ���邪�A�ʏ��1��Ăяo�������ŏ\�����B
	for (int i = 0; i < 90; ++i)
	{
		b2World_Step(worldId, timeStep, subStepCount);
		b2Vec2 position = b2Body_GetPosition(bodyId);
		b2Rot rotation = b2Body_GetRotation(bodyId);
		printf("%4.2f %4.2f %4.2f\n", position.x, position.y, b2Rot_GetAngle(rotation));
	}
}

void HelloBox2dDraw()
{
	debugDraw.drawShapes = true;
	b2World_Draw(worldId, &debugDraw);
}

void HelloBox2dUninit()
{
	//�V�~�����[�V�������I�������A���E��j�󂵂Ȃ���΂Ȃ�Ȃ��B
	b2DestroyWorld(worldId);
}


