#include "Box2D.h"

#ifdef BOX2D_UPDATE_MULTITHREAD
thread_local void(*Box2D::WorldManager::pPauseWorldUpdate)() = PauseWorldUpdate;
thread_local void(*Box2D::WorldManager::pResumeWorldUpdate)() = ResumeWorldUpdate;
#endif

thread_local b2WorldId* Box2D::WorldManager::currentWorldId;
b2WorldId Box2D::WorldManager::worldId;
b2WorldId Box2D::WorldManager::nextWorldId;
b2WorldId Box2D::WorldManager::eraseWorldId;
b2BodyDef Box2D::WorldManager::bodyDef;

#ifdef BOX2D_UPDATE_MULTITHREAD
std::atomic<bool> Box2D::WorldManager::running(false);
std::atomic<bool> Box2D::WorldManager::paused(false);
std::atomic<bool> Box2D::WorldManager::actuallyPaused(false);
std::thread Box2D::WorldManager::worldUpdateThread;
std::mutex Box2D::WorldManager::threadMutex;
std::condition_variable Box2D::WorldManager::cv;
std::condition_variable Box2D::WorldManager::pauseCv;

//std::vector<std::function<void()>> Box2D::WorldManager::worldFunc;
//std::mutex Box2D::WorldManager::worldFuncMutex;
#endif

void Box2D::WorldManager::CreateWorld()
{
	//���[���h��`�A������
	b2WorldDef worldDef = b2DefaultWorldDef();
	//�d�͂̐ݒ�
	worldDef.gravity = { 0.0f, GRAVITY };
	//���[���h�I�u�W�F�N�g�쐬
	worldId = b2CreateWorld(&worldDef);
	//���̃��[���h������Ă����i�X�V�͂��Ȃ���j
	eraseWorldId = b2CreateWorld(&worldDef);

	currentWorldId = &worldId;
}

void Box2D::WorldManager::GenerataeBody(b2BodyId& _bodyId,const b2BodyDef* _bodyDef)
{
	//�O�����h�{�f�B�̍쐬
	_bodyId = b2CreateBody(*currentWorldId, _bodyDef);
}


#ifdef BOX2D_UPDATE_MULTITHREAD
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
		//�r������
		std::unique_lock<std::mutex> lock(threadMutex);
		
		//�ꎞ��~�Ń��C���������ʒm��҂��Ă鎞
		if (paused && !actuallyPaused) {
			actuallyPaused = true;
			//���C�������̕��ɒʒm������
			pauseCv.notify_one(); 
			//�ꎞ��~���Ȃ珈�����~�߂�
			cv.wait(lock, [] { return !paused || !running; });
		}

		if (!running) break;

		//���C�������҂�����
		actuallyPaused = false;
		// �r������
		lock.unlock();

		//���ݎ��Ԃ��擾
		QueryPerformanceCounter(&liWork);
		nowCount = liWork.QuadPart;
		if (nowCount >= oldCount + frequency / FPS)
		{
			/*{
				std::lock_guard<std::mutex> lock(worldFuncMutex);
				for (auto& func : worldFunc)
				{
					func();
				}

				worldFunc.clear();
			}*/

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

#ifdef BOX2D_UPDATE_MULTITHREAD
	//���[���h�X�V���~�߂Ȃ��悤�ɐݒ�
	pPauseWorldUpdate = []() {};
	pResumeWorldUpdate = []() {};
#endif
}

void Box2D::WorldManager::PauseWorldUpdate()
{
	{
		std::lock_guard<std::mutex> lock(threadMutex);
		if (running && !paused) {
			paused = true;
		}
	}
	//�҂��Ă���X���b�h��@���N����
	cv.notify_all();

	//��~�t���O�����������܂ő҂�
	std::unique_lock<std::mutex> lock(threadMutex);
	pauseCv.wait(lock, [] { return actuallyPaused.load(); });

	//LOG("WorldUpdate thread paused.");
}

void Box2D::WorldManager::ResumeWorldUpdate()
{
	{
		//�r������
		std::lock_guard<std::mutex> lock(threadMutex);
		//�X���b�h�������Ă��Ĉꎞ��~����Ă���Ƃ�
		if (running && paused) {
			paused = false;
			//LOG("PhysicUpdate thread resumed");
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



void Box2D::WorldManager::DeleteAllWorld()
{
	//�V�~�����[�V�������I�������A���E��j�󂵂Ȃ���΂Ȃ�Ȃ��B
	b2DestroyWorld(worldId);
	if (b2World_IsValid(nextWorldId))
	{
		b2DestroyWorld(nextWorldId);
	}
	DeleteOldWorld();
}

void Box2D::WorldManager::ChengeNextWorld()
{
	//���[���h��`�A������
	b2WorldDef worldDef = b2DefaultWorldDef();
	//�d�͂̐ݒ�
	worldDef.gravity = { 0.0f, GRAVITY };
	//���[���h�I�u�W�F�N�g�쐬
	nextWorldId = b2CreateWorld(&worldDef);

	//���[���h�����̃��[���h�ɂ���
	currentWorldId = &nextWorldId;

#ifdef BOX2D_UPDATE_MULTITHREAD
	//���[���h�X�V���~�߂Ȃ��悤�ɐݒ�
	pPauseWorldUpdate = []() {};
	pResumeWorldUpdate = []() {};
#endif
}

void Box2D::WorldManager::LinkNextWorld()
{
	//DeleteWorld();
	eraseWorldId = worldId;
	worldId = nextWorldId;
}

void Box2D::WorldManager::DeleteOldWorld()
{
	if (b2World_IsValid(eraseWorldId))
	{
		b2DestroyWorld(eraseWorldId);
	}
}
