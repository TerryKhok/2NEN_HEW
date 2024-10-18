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
	//ワールド定義、初期化
	b2WorldDef worldDef = b2DefaultWorldDef();
	//重力の設定
	worldDef.gravity = { 0.0f, GRAVITY };
	//ワールドオブジェクト作成
	worldId = b2CreateWorld(&worldDef);
	//次のワールドも作っておく（更新はしないよ）
	eraseWorldId = b2CreateWorld(&worldDef);

	currentWorldId = &worldId;
}

void Box2D::WorldManager::GenerataeBody(b2BodyId& _bodyId,const b2BodyDef* _bodyDef)
{
	//グランドボディの作成
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
		//排他制御
		std::unique_lock<std::mutex> lock(threadMutex);
		
		//一時停止でメイン処理が通知を待ってる時
		if (paused && !actuallyPaused) {
			actuallyPaused = true;
			//メイン処理の方に通知をする
			pauseCv.notify_one(); 
			//一時停止中なら処理を止める
			cv.wait(lock, [] { return !paused || !running; });
		}

		if (!running) break;

		//メイン処理待ち解除
		actuallyPaused = false;
		// 排他解除
		lock.unlock();

		//現在時間を取得
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

#ifdef BOX2D_UPDATE_MULTITHREAD
	//ワールド更新を止めないように設定
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
	//待っているスレッドを叩き起こす
	cv.notify_all();

	//停止フラグが解除されるまで待つ
	std::unique_lock<std::mutex> lock(threadMutex);
	pauseCv.wait(lock, [] { return actuallyPaused.load(); });

	//LOG("WorldUpdate thread paused.");
}

void Box2D::WorldManager::ResumeWorldUpdate()
{
	{
		//排他制御
		std::lock_guard<std::mutex> lock(threadMutex);
		//スレッドが立っていて一時停止されているとき
		if (running && paused) {
			paused = false;
			//LOG("PhysicUpdate thread resumed");
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



void Box2D::WorldManager::DeleteAllWorld()
{
	//シミュレーションが終わったら、世界を破壊しなければならない。
	b2DestroyWorld(worldId);
	if (b2World_IsValid(nextWorldId))
	{
		b2DestroyWorld(nextWorldId);
	}
	DeleteOldWorld();
}

void Box2D::WorldManager::ChengeNextWorld()
{
	//ワールド定義、初期化
	b2WorldDef worldDef = b2DefaultWorldDef();
	//重力の設定
	worldDef.gravity = { 0.0f, GRAVITY };
	//ワールドオブジェクト作成
	nextWorldId = b2CreateWorld(&worldDef);

	//ワールドを次のワールドにする
	currentWorldId = &nextWorldId;

#ifdef BOX2D_UPDATE_MULTITHREAD
	//ワールド更新を止めないように設定
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
