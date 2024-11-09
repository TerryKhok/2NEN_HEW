
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

std::vector<std::function<void()>> Box2D::WorldManager::worldTask;
std::mutex Box2D::WorldManager::worldTaskMutex;
#endif

bool Box2D::WorldManager::RayCast(Vector2 _start, Vector2 _end)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2Vec2 rayStart = { _start.x / DEFAULT_OBJECT_SIZE,_start.y / DEFAULT_OBJECT_SIZE };
	b2Vec2 rayEnd =	{ _end.x / DEFAULT_OBJECT_SIZE,_end.y / DEFAULT_OBJECT_SIZE };

	b2RayResult result = b2World_CastRayClosest(*currentWorldId, rayStart, b2Sub(rayEnd, rayStart), b2DefaultQueryFilter());

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

#ifdef DEBUG_TRUE
	Vector2 hitPoint = _end;
	if (result.hit)
	{
		hitPoint = { result.point.x * DEFAULT_OBJECT_SIZE ,result.point.y * DEFAULT_OBJECT_SIZE };
	}
	RenderManager::DrawRayNode rayNode;
	Vector2 dis = hitPoint - _start;
	rayNode.center = _start + dis / 2;
	//rayNode.length = Math::PointDistance(_start.x, _start.y, hitPoint.x, hitPoint.y);
	rayNode.length = sqrt(dis.x * dis.x + dis.y * dis.y);
	rayNode.radian = Math::PointRadian(_start.x, _start.y, hitPoint.x, hitPoint.y);
	rayNode.color = result.hit ? XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) : XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	RenderManager::m_drawRayNode.push_back(rayNode);
#endif

	return result.hit;
}

bool Box2D::WorldManager::RayCast(Vector2 _start, Vector2 _end, Vector2& _output)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2Vec2 rayStart = { _start.x / DEFAULT_OBJECT_SIZE,_start.y / DEFAULT_OBJECT_SIZE };
	b2Vec2 rayEnd = { _end.x / DEFAULT_OBJECT_SIZE,_end.y / DEFAULT_OBJECT_SIZE };

	b2RayResult result = b2World_CastRayClosest(*currentWorldId, rayStart, b2Sub(rayEnd, rayStart), b2DefaultQueryFilter());

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

	Vector2 hitPoint = _end;
	if (result.hit)
	{
		hitPoint = { result.point.x * DEFAULT_OBJECT_SIZE ,result.point.y * DEFAULT_OBJECT_SIZE };
	}
	_output = hitPoint;

#ifdef DEBUG_TRUE
	RenderManager::DrawRayNode rayNode;
	Vector2 dis = hitPoint - _start;
	rayNode.center = _start + dis / 2;
	//rayNode.length = Math::PointDistance(_start.x, _start.y, hitPoint.x, hitPoint.y);
	rayNode.length = sqrt(dis.x * dis.x + dis.y * dis.y);
	rayNode.radian = Math::PointRadian(_start.x, _start.y, hitPoint.x, hitPoint.y);
	rayNode.color = result.hit ? XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) : XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	RenderManager::m_drawRayNode.push_back(rayNode);
#endif

	return result.hit;
}

bool Box2D::WorldManager::RayCast(Vector2 _start, Vector2 _end, FILTER _filter)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2Vec2 rayStart = { _start.x / DEFAULT_OBJECT_SIZE,_start.y / DEFAULT_OBJECT_SIZE };
	b2Vec2 rayEnd = { _end.x / DEFAULT_OBJECT_SIZE,_end.y / DEFAULT_OBJECT_SIZE };

	b2QueryFilter filter;
	filter.categoryBits = _filter;
	filter.maskBits = Box2DBodyManager::GetMaskFilterBit(_filter);
	b2RayResult result = b2World_CastRayClosest(*currentWorldId, rayStart, b2Sub(rayEnd, rayStart), filter);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

#ifdef DEBUG_TRUE
	Vector2 hitPoint = _end;
	if (result.hit)
	{
		hitPoint = { result.point.x * DEFAULT_OBJECT_SIZE ,result.point.y * DEFAULT_OBJECT_SIZE };
	}
	RenderManager::DrawRayNode rayNode;
	Vector2 dis = hitPoint - _start;
	rayNode.center = _start + dis / 2;
	//rayNode.length = Math::PointDistance(_start.x, _start.y, hitPoint.x, hitPoint.y);
	rayNode.length = sqrt(dis.x * dis.x + dis.y * dis.y);
	rayNode.radian = Math::PointRadian(_start.x, _start.y, hitPoint.x, hitPoint.y);
	rayNode.color = result.hit ? XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) : XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	RenderManager::m_drawRayNode.push_back(rayNode);
#endif

	return result.hit;
}

bool Box2D::WorldManager::RayCast(Vector2 _start, Vector2 _end, Vector2& _output, FILTER _filter)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2Vec2 rayStart = { _start.x / DEFAULT_OBJECT_SIZE,_start.y / DEFAULT_OBJECT_SIZE };
	b2Vec2 rayEnd = { _end.x / DEFAULT_OBJECT_SIZE,_end.y / DEFAULT_OBJECT_SIZE };

	b2QueryFilter filter;
	filter.categoryBits = _filter;
	filter.maskBits = Box2DBodyManager::GetMaskFilterBit(_filter);
	b2RayResult result = b2World_CastRayClosest(*currentWorldId, rayStart, b2Sub(rayEnd, rayStart), filter);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif
	Vector2 hitPoint = _end;
	if (result.hit)
	{
		hitPoint = { result.point.x * DEFAULT_OBJECT_SIZE ,result.point.y * DEFAULT_OBJECT_SIZE };
	}
	_output = hitPoint;

#ifdef DEBUG_TRUE
	RenderManager::DrawRayNode rayNode;
	Vector2 dis = hitPoint - _start;
	rayNode.center = _start + dis / 2;
	//rayNode.length = Math::PointDistance(_start.x, _start.y, hitPoint.x, hitPoint.y);
	rayNode.length = sqrt(dis.x * dis.x + dis.y * dis.y);
	rayNode.radian = Math::PointRadian(_start.x, _start.y, hitPoint.x, hitPoint.y);
	rayNode.color = result.hit ? XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) : XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	RenderManager::m_drawRayNode.push_back(rayNode);
#endif

	return result.hit;
}

float b2CastResultPosFcn(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
{
	std::vector<b2Vec2>* posList = (std::vector<b2Vec2>*)context;
	posList->push_back(point);

	return 1;
}

bool Box2D::WorldManager::RayCastAll(Vector2 _start, Vector2 _end, std::vector<Vector2>& _output)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2Vec2 rayStart = { _start.x / DEFAULT_OBJECT_SIZE,_start.y / DEFAULT_OBJECT_SIZE };
	b2Vec2 rayEnd = { _end.x / DEFAULT_OBJECT_SIZE,_end.y / DEFAULT_OBJECT_SIZE };

	std::vector<b2Vec2> posList;
	b2World_CastRay(*currentWorldId, rayStart, b2Sub(rayEnd, rayStart), b2DefaultQueryFilter(), b2CastResultPosFcn, &posList);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

	for (auto& vec2 : posList)
	{
		_output.push_back(
			{
				vec2.x * DEFAULT_OBJECT_SIZE,
				vec2.y * DEFAULT_OBJECT_SIZE,
			}
		);
	}

	bool hit = posList.size() != 0;

#ifdef DEBUG_TRUE
	RenderManager::DrawRayNode rayNode;
	Vector2 dis = _end - _start;
	rayNode.center = _start + dis / 2;
	//rayNode.length = Math::PointDistance(_start.x, _start.y, hitPoint.x, hitPoint.y);
	rayNode.length = sqrt(dis.x * dis.x + dis.y * dis.y);
	rayNode.radian = Math::PointRadian(_start.x, _start.y, _end.x, _end.y);
	rayNode.color = hit ? XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) : XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	RenderManager::m_drawRayNode.push_back(rayNode);
#endif

	return hit;
}

bool Box2D::WorldManager::RayCastAll(Vector2 _start, Vector2 _end, std::vector<Vector2>& _output, FILTER _filter)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2Vec2 rayStart = { _start.x / DEFAULT_OBJECT_SIZE,_start.y / DEFAULT_OBJECT_SIZE };
	b2Vec2 rayEnd = { _end.x / DEFAULT_OBJECT_SIZE,_end.y / DEFAULT_OBJECT_SIZE };

	b2QueryFilter filter;
	filter.categoryBits = _filter;
	filter.maskBits = Box2DBodyManager::GetMaskFilterBit(_filter);
	std::vector<b2Vec2> posList;
	b2World_CastRay(*currentWorldId, rayStart, b2Sub(rayEnd, rayStart), filter, b2CastResultPosFcn, &posList);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

	for (auto& vec2 : posList)
	{
		_output.push_back(
			{
				vec2.x * DEFAULT_OBJECT_SIZE,
				vec2.y * DEFAULT_OBJECT_SIZE,
			}
		);
	}

	bool hit = posList.size() != 0;

#ifdef DEBUG_TRUE
	RenderManager::DrawRayNode rayNode;
	Vector2 dis = _end - _start;
	rayNode.center = _start + dis / 2;
	//rayNode.length = Math::PointDistance(_start.x, _start.y, hitPoint.x, hitPoint.y);
	rayNode.length = sqrt(dis.x * dis.x + dis.y * dis.y);
	rayNode.radian = Math::PointRadian(_start.x, _start.y, _end.x, _end.y);
	rayNode.color = hit ? XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) : XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	RenderManager::m_drawRayNode.push_back(rayNode);
#endif

	return hit;
}

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
	//long long oldTick = GetTickCount64();	//�O��v����
	//long long nowTick = oldTick;	//����v����
	long long nowCount = oldCount;

	//FPS�Œ�p�ϐ�
	QueryPerformanceFrequency(&liWork);
	frequency = liWork.QuadPart;
	QueryPerformanceCounter(&liWork);
	oldCount = liWork.QuadPart;

	const long long frameCount = frequency / WORLD_FPS;

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
		if (nowCount >= oldCount + frameCount)
		{
			//���[���h�̃^�X�N���ڂ�
			//=================================================================
			std::unique_lock<std::mutex> lock(worldTaskMutex);
			std::vector<std::function<void()>> tempTasks = std::move(worldTask);
			worldTask.clear();
			//=================================================================

			lock.unlock();

			// Execute all tasks in the temporary list
			std::vector<std::future<void>> futures;
			for (auto& task : tempTasks) {
				futures.push_back(std::async(std::launch::async, task));
			}

			// Wait for all tasks to complete
			for (auto& future : futures) {
				future.get();
			}

			//���[���h�̍X�V
			b2World_Step(worldId, timeStep, subStepCount);

			oldCount = nowCount;
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

void Box2D::WorldManager::EnableWorldUpdate()
{
	//���[���h�X�V�ɕύX���������悤�ɐݒ�
	pPauseWorldUpdate = PauseWorldUpdate;
	pResumeWorldUpdate = ResumeWorldUpdate;
}

void Box2D::WorldManager::DisableWorldUpdate()
{
	//���[���h�X�V�ɕύX��������Ȃ��悤�ɐݒ�
	pPauseWorldUpdate = []() {};
	pResumeWorldUpdate = []() {};
}
void Box2D::WorldManager::AddWorldTask(std::function<void()>&& _task)
{
	std::lock_guard<std::mutex> lock(worldTaskMutex);
	worldTask.push_back(_task);
}
#else
void Box2D::WorldManager::WorldUpdate()
{
	//���[���h�̍X�V
	b2World_Step(worldId, timeStep, subStepCount);
}
#endif

void Box2D::WorldManager::ExcuteSensorEvent()
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	pPauseWorldUpdate();
#endif
	b2SensorEvents sensorEvents = b2World_GetSensorEvents(worldId);
#ifdef BOX2D_UPDATE_MULTITHREAD
	pResumeWorldUpdate();
#endif
	for (int i = 0; i < sensorEvents.beginCount; i++)
	{
		b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];
		auto iter = Box2DBodyManager::m_bodyObjectName.find(b2Shape_GetBody(event.sensorShapeId).index1);
		if (iter != Box2DBodyManager::m_bodyObjectName.end())
		{
			GameObject* sensorObject = ObjectManager::Find(iter->second);
			if (sensorObject != nullptr)
			{
				auto it = Box2DBodyManager::m_bodyObjectName.find(b2Shape_GetBody(event.visitorShapeId).index1);
				if (it != Box2DBodyManager::m_bodyObjectName.end())
				{
					GameObject* visitorObject = ObjectManager::Find(it->second);
					if (visitorObject != nullptr)
					{
						for (auto& component : sensorObject->m_componentList)
						{
							component.second->OnColliderEnter(visitorObject);
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < sensorEvents.endCount; i++)
	{
		b2SensorEndTouchEvent event = sensorEvents.endEvents[i];
		auto iter = Box2DBodyManager::m_bodyObjectName.find(b2Shape_GetBody(event.sensorShapeId).index1);
		if (iter != Box2DBodyManager::m_bodyObjectName.end())
		{
			GameObject* sensorObject = ObjectManager::Find(iter->second);
			auto it = Box2DBodyManager::m_bodyObjectName.find(b2Shape_GetBody(event.visitorShapeId).index1);
			if (it != Box2DBodyManager::m_bodyObjectName.end())
			{
				GameObject* visitorObject = ObjectManager::Find(it->second);
				for (auto& component : sensorObject->m_componentList)
				{
					component.second->OnColliderExit(visitorObject);
				}
			}
		}
	}
}

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


