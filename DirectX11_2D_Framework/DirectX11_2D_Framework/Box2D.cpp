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

#ifdef WORLD_UPDATE_MULTITHERD
void Box2D::WorldManager::WorldUpdate()
{
	LOG("box2d world thred Start");

	running = true;

	LARGE_INTEGER liWork;
	long long frequency = 0;
	long long oldCount = 0;
	long long oldTick = GetTickCount64();	//前回計測時
	long long nowTick = oldTick;	//今回計測時
	long long nowCount = oldCount;

	//FPS固定用変数
	QueryPerformanceFrequency(&liWork);
	frequency = liWork.QuadPart;
	QueryPerformanceCounter(&liWork);
	oldCount = liWork.QuadPart;

	while (running)
	{
		// 一時停止
		std::unique_lock<std::mutex> lock(threadMutex);
		cv.wait(lock, [] { return !paused || !running; });

		if (!running) break;

		// 排他解除
		lock.unlock();

		//現在時間を取得
		QueryPerformanceCounter(&liWork);
		nowCount = liWork.QuadPart;
		if (nowCount >= oldCount + frequency / FPS)
		{
			//ワールドの更新
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
	//排他制御
	std::lock_guard<std::mutex> lock(threadMutex);
	//すでにスレッドが立っている
	if (running) {
		LOG_WARNING("box2d WorldUpdate thread is already running.\n");
		return;
	}
	
	//前のスレッドが正しく結合されていることを確認する
	if (worldUpdateThread.joinable()) {
		worldUpdateThread.join();
	}
	//スレッドをたてる
	worldUpdateThread = std::thread(WorldUpdate);
}

void Box2D::WorldManager::StopWorldUpdate()
{
	{
		//排他制御
		std::lock_guard<std::mutex> lock(threadMutex);
		//スレッドが立っていない
		if (!running) {
			LOG("box2d WorldUpdate thread is not running.\n");
			return;
		}
		running = false;

		//排他解除
	}
	//待機している全てのスレッドを起床させる
	cv.notify_all();
	//スレッドの結合
	if (worldUpdateThread.joinable()) {
		worldUpdateThread.join();
	}
}

void Box2D::WorldManager::PauseWorldUpdate()
{
	//排他制御
	std::lock_guard<std::mutex> lock(threadMutex);
	if (running && !paused) {
		paused = true;
		LOG("box2d WorldUpdate thread paused.\n");
	}
}

void Box2D::WorldManager::ResumeWorldUpdate()
{
	{
		//排他制御
		std::lock_guard<std::mutex> lock(threadMutex);
		//スレッドが立っていて一時停止されているとき
		if (running && paused) {
			paused = false;
			std::cout << "PhysicUpdate thread resumed.\n";
		}
	}
	//待機している全てのスレッドを起床させる
	cv.notify_all(); // Wake up the paused thread
}
#else
void Box2D::WorldManager::WorldUpdate()
{
	//ワールドの更新
	b2World_Step(worldId, timeStep, subStepCount);
}
#endif



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
