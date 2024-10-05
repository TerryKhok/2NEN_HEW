#include "Box2D.h"

thread_local b2WorldId Box2D::WorldManager::currentWorldId;
std::atomic<b2WorldId> Box2D::WorldManager::worldId;
b2WorldId Box2D::WorldManager::nextWorldId;
b2BodyDef Box2D::WorldManager::bodyDef;

#ifdef WORLD_UPDATE_MULTITHERD
std::atomic<bool> Box2D::WorldManager::running(false);
std::atomic<bool> Box2D::WorldManager::paused(false);
std::thread Box2D::WorldManager::worldUpdateThread;
std::mutex Box2D::WorldManager::threadMutex;
std::condition_variable Box2D::WorldManager::cv;
#endif

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

#ifdef WORLD_UPDATE_MULTITHERD
void Box2D::WorldManager::WorldUpdate()
{
	LOG("box2d world thred Start");

	running = true;

	LARGE_INTEGER liWork;
	long long frequency = 0;
	long long oldCount = 0;
	long long oldTick = GetTickCount64();	//�O��v����
	long long nowTick = oldTick;	//����v����
	long long nowCount = oldCount;

	//FPS�Œ�p�ϐ�
	QueryPerformanceFrequency(&liWork);
	frequency = liWork.QuadPart;
	QueryPerformanceCounter(&liWork);
	oldCount = liWork.QuadPart;

	while (running)
	{
		// �ꎞ��~
		std::unique_lock<std::mutex> lock(threadMutex);
		cv.wait(lock, [] { return !paused || !running; });

		if (!running) break;

		// �r������
		lock.unlock();

		//���ݎ��Ԃ��擾
		QueryPerformanceCounter(&liWork);
		nowCount = liWork.QuadPart;
		if (nowCount >= oldCount + frequency / FPS)
		{
			//���[���h�̍X�V
			b2World_Step(worldId, timeStep, subStepCount);

			oldCount = nowCount;
			nowTick = GetTickCount64();
			if (nowTick >= oldTick + 1000)
			{
				oldTick = nowTick;
			}
		}
	}

	LOG("box2d world thred End");
}

void Box2D::WorldManager::StartWorldUpdate()
{
	//�r������
	std::lock_guard<std::mutex> lock(threadMutex);
	//���łɃX���b�h�������Ă���
	if (running) {
		LOG_WARNING("box2d WorldUpdate thread is already running.\n");
		return;
	}
	
	//�O�̃X���b�h����������������Ă��邱�Ƃ��m�F����
	if (worldUpdateThread.joinable()) {
		worldUpdateThread.join();
	}
	//�X���b�h�����Ă�
	worldUpdateThread = std::thread(WorldUpdate);
}

void Box2D::WorldManager::StopWorldUpdate()
{
	{
		//�r������
		std::lock_guard<std::mutex> lock(threadMutex);
		//�X���b�h�������Ă��Ȃ�
		if (!running) {
			LOG("box2d WorldUpdate thread is not running.\n");
			return;
		}
		running = false;

		//�r������
	}
	//�ҋ@���Ă���S�ẴX���b�h���N��������
	cv.notify_all();
	//�X���b�h�̌���
	if (worldUpdateThread.joinable()) {
		worldUpdateThread.join();
	}
}

void Box2D::WorldManager::PauseWorldUpdate()
{
	//�r������
	std::lock_guard<std::mutex> lock(threadMutex);
	if (running && !paused) {
		paused = true;
		LOG("box2d WorldUpdate thread paused.\n");
	}
}

void Box2D::WorldManager::ResumeWorldUpdate()
{
	{
		//�r������
		std::lock_guard<std::mutex> lock(threadMutex);
		//�X���b�h�������Ă��Ĉꎞ��~����Ă���Ƃ�
		if (running && paused) {
			paused = false;
			std::cout << "PhysicUpdate thread resumed.\n";
		}
	}
	//�ҋ@���Ă���S�ẴX���b�h���N��������
	cv.notify_all(); // Wake up the paused thread
}
#else
void Box2D::WorldManager::WorldUpdate()
{
	//���[���h�̍X�V
	b2World_Step(worldId, timeStep, subStepCount);
}
#endif



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
