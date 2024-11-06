
std::unordered_map<std::string, std::function<void()>> Box2DBodyManager::moveFunctions;

std::unordered_map<FILTER, unsigned int> Box2DBodyManager::m_layerFilterBit;

//shapeIdに対応したオブジェクトの名前を格納
std::unordered_map<int32_t, std::string> Box2DBodyManager::m_bodyObjectName;

#ifdef DEBUG_TRUE
const int Box2DBodyManager::numSegments = 36;
//box用インデックス
ComPtr<ID3D11Buffer> Box2DBodyManager::m_boxIndexBuffer;
//Circle用頂点データ
ComPtr<ID3D11Buffer> Box2DBodyManager::m_circleVertexBuffer;
//Circle用インデックス
ComPtr<ID3D11Buffer> Box2DBodyManager::m_circleIndexBuffer;

void SetDebugBodyColor(b2BodyId _bodyId, DirectX::XMFLOAT4& _color);
#endif

Box2DBody::Box2DBody(GameObject* _object)
{
	auto& position = _object->transform.position;
	//ボディ定義とワールドIDを使ってグラウンド・ボディを作成する
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { position.x / DEFAULT_OBJECT_SIZE, position.y / DEFAULT_OBJECT_SIZE };
	bodyDef.rotation = b2MakeRot(static_cast<float>(_object->transform.angle.z.Get()));

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif
	Box2D::WorldManager::GenerataeBody(m_bodyId, &bodyDef);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

	Box2DBodyManager::m_bodyObjectName.insert(std::pair(m_bodyId.index1, _object->GetName()));
}

Box2DBody::Box2DBody(GameObject* _object, b2BodyDef* _bodyDef)
{
	auto& position = _object->transform.position;
	_bodyDef->position = { position.x / DEFAULT_OBJECT_SIZE, position.y / DEFAULT_OBJECT_SIZE };

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif
	Box2D::WorldManager::GenerataeBody(m_bodyId, _bodyDef);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

	Box2DBodyManager::m_bodyObjectName.insert(std::pair(m_bodyId.index1, _object->GetName()));
}

inline void Box2DBody::Update()
{
	auto func = [&]() 
		{
			auto pos = b2Body_GetPosition(m_bodyId);
			pos *= DEFAULT_OBJECT_SIZE;
			m_this->transform.position = Vector3(pos.x, pos.y, 0.5f);
			auto rot = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
			m_this->transform.angle.z.Set(rot);
		};

	Box2DBodyManager::moveFunctions.emplace(m_this->GetName(), func);
}

#ifdef DEBUG_TRUE
void Box2DBody::Delete()
{
	for (auto& node : m_nodeList)
	{
		node->Delete(LAYER_BOX2D_DEBUG);
	}
	auto& list = Box2DBodyManager::m_bodyObjectName;
	auto it = list.find(m_bodyId.index1);
	if (it != list.end()) {
		list.erase(it);
	}
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif
	b2DestroyBody(m_bodyId);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif
}

#else
void Box2DBody::Delete()
{
	auto& list = Box2DBodyManager::m_bodyObjectName;
	auto it = list.find(m_bodyId.index1);
	if (it != list.end()) {
		list.erase(it);
	}
	b2DestroyBody(m_bodyId);
}
#endif

void Box2DBody::SetActive(bool _active)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif
	_active ? b2Body_Enable(m_bodyId) : b2Body_Disable(m_bodyId);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

#ifdef DEBUG_TRUE
	for (auto& node : m_nodeList) 
	{
		node->Active(_active);
	}
#endif
}

void Box2DBody::SetFilter(const FILTER _filter)
{
	m_filter = _filter;

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	for (auto shape : m_shapeList)
	{
		auto filter = b2Shape_GetFilter(shape);
		filter.categoryBits = m_filter;
		filter.maskBits = Box2DBodyManager::GetMaskLayerBit(m_filter);
		b2Shape_SetFilter(shape, filter);
	}
	
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif 
}

void Box2DBody::CreateBoxShape(bool _sensor)
{
	auto scale = m_this->transform.scale;
	CreateBoxShape({ scale.x, scale.y }, { 0.0f,0.0f }, 0.0f, _sensor);
}

void Box2DBody::CreateBoxShape(float _offsetX, float _offsetY, float _angle, bool _sensor)
{
	auto scale = m_this->transform.scale;
	CreateBoxShape({ scale.x, scale.y }, { _offsetX ,_offsetY }, _angle, _sensor);
}

void Box2DBody::CreateBoxShape(Vector2 _size, Vector2 _offset, float _angle,bool _sensor)
{
	float rad = Math::DegToRad(_angle);

	//地面ポリゴンを作る。 b2MakeBox()ヘルパー関数を使い、地面ポリゴンを箱型にする。箱の中心は親ボディの原点である。
	b2Polygon polygonBox = 
		b2MakeOffsetBox(_size.x / 2, _size.y / 2, { _offset.x / DEFAULT_OBJECT_SIZE,_offset.y / DEFAULT_OBJECT_SIZE }, b2MakeRot(rad));

	//シェイプを作成して地面のボディを仕上げる
	b2ShapeDef shapeDef = b2DefaultShapeDef();

	shapeDef.isSensor = _sensor;
	shapeDef.filter.categoryBits = m_filter;
	shapeDef.filter.maskBits = Box2DBodyManager::GetMaskLayerBit(m_filter);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	auto shape = b2CreatePolygonShape(m_bodyId, &shapeDef, &polygonBox);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

#ifdef DEBUG_TRUE

	Vector2 offset;
	offset.x = _offset.x * cos(rad) - offset.y * sin(rad);
	offset.y = (_offset.x * sin(rad) + offset.y * cos(rad)) / 2;
	auto node = std::shared_ptr<RenderNode>(new Box2DBoxRenderNode(_offset, { _size.x,_size.y }, rad, m_bodyId));
	//auto node = std::shared_ptr<RenderNode>(new Box2DCircleRenderNode(scale.x, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif

	m_shapeList.push_back(shape);
}

void Box2DBody::CreateCircleShape()
{
	auto scale = m_this->transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCircleShape(diameter);
}

void Box2DBody::CreateCircleShape(Vector2 _offset)
{
	auto scale = m_this->transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCircleShape(diameter, _offset);
}

void Box2DBody::CreateCircleShape(float _diameter, Vector2 _offset)
{
	//地面ポリゴンを作る。 b2MakeBox()ヘルパー関数を使い、地面ポリゴンを箱型にする。箱の中心は親ボディの原点である。
	b2Circle circle = { {_offset.x / DEFAULT_OBJECT_SIZE,_offset.y / DEFAULT_OBJECT_SIZE},_diameter / 2 };

	//シェイプを作成して地面のボディを仕上げる
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.filter.categoryBits = m_filter;
	shapeDef.filter.maskBits = Box2DBodyManager::GetMaskLayerBit(m_filter);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	auto shape = b2CreateCircleShape(m_bodyId, &shapeDef, &circle);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif


#ifdef DEBUG_TRUE
	auto node = std::shared_ptr<RenderNode>(new Box2DCircleRenderNode(_offset,_diameter, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif

	m_shapeList.push_back(shape);
}

void Box2DBody::CreateCapsuleShape()
{
	auto& transform = m_this->transform;
	auto scale = transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCapsuleShape(diameter, scale.y / 2);
}

void Box2DBody::CreateCapsuleShape(float _height, Vector2 _offset, float _angle)
{
	auto& transform = m_this->transform;
	auto scale = transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCapsuleShape(diameter, _height, _angle, _offset);
}

void Box2DBody::CreateCapsuleShape(float _diameter, float _height, float _angle, Vector2 _offset)
{
	float rad = Math::DegToRad(_angle);
	float halfHeight = _height / 2;

	Vector2 offset = _offset;
	offset /= DEFAULT_OBJECT_SIZE;

	//地面ポリゴンを作る。 b2MakeBox()ヘルパー関数を使い、地面ポリゴンを箱型にする。箱の中心は親ボディの原点である。
	b2Capsule capsule = {
				{ offset.x + sin(rad) * halfHeight,  offset.y + -cos(rad) * halfHeight },
				{ offset.x + -sin(rad) * halfHeight, offset.y + cos(rad) * halfHeight }, _diameter / 2.0f };

	//シェイプを作成して地面のボディを仕上げる
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.filter.categoryBits = m_filter;
	shapeDef.filter.maskBits = Box2DBodyManager::GetMaskLayerBit(m_filter);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	auto shape = b2CreateCapsuleShape(m_bodyId, &shapeDef, &capsule);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif


#ifdef DEBUG_TRUE
	halfHeight = _height / 2;

	auto node = std::shared_ptr<RenderNode>(
		new Box2DCapsuleRenderNode(_offset, _diameter, halfHeight * DEFAULT_OBJECT_SIZE, _height, rad, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif

	m_shapeList.push_back(shape);
}

void Box2DBody::CreatePolygonShape(std::vector<b2Vec2> _pointList)
{
	int count = static_cast<int>(_pointList.size());
	if (_pointList.size() < 4)
	{
		LOG_ERROR("Not enough vertices in Polygon");
		return;
	}

#ifdef DEBUG_TRUE
	auto node = std::shared_ptr<RenderNode>(
		new Box2DConvexMeshRenderNode(_pointList, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif

	for (auto& point : _pointList)
	{
		point.x /= DEFAULT_OBJECT_SIZE;
		point.y /= DEFAULT_OBJECT_SIZE;
	}

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.filter.categoryBits = m_filter;
	shapeDef.filter.maskBits = Box2DBodyManager::GetMaskLayerBit(m_filter);

	b2Hull hull = b2ComputeHull(_pointList.data(), count);
	b2Polygon polygon = b2MakePolygon(&hull, 0.0f);
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	auto shape = b2CreatePolygonShape(m_bodyId, &shapeDef, &polygon);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

	m_shapeList.push_back(shape);
}

void Box2DBody::CreateSegment(std::vector<b2Vec2> _pointList)
{
	int count = static_cast<int>(_pointList.size());

	if (count < 2)
	{
		LOG_ERROR("Not enough vertices in Segment");
		return;
	}


#ifdef DEBUG_TRUE
	auto node = std::shared_ptr<RenderNode>(
		new Box2DMeshRenderNode(_pointList, m_bodyId, false));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif

	for (auto& point : _pointList)
	{
		point.x /= DEFAULT_OBJECT_SIZE;
		point.y /= DEFAULT_OBJECT_SIZE;
	}
	

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.filter.categoryBits = m_filter;
	shapeDef.filter.maskBits = Box2DBodyManager::GetMaskLayerBit(m_filter);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	for (int i = 0; i < count - 1; i++)
	{
		b2Segment segment1 = { _pointList[i], _pointList[i + 1] };
		auto shape = b2CreateSegmentShape(m_bodyId, &shapeDef, &segment1);
		m_shapeList.push_back(shape);
	}

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif
}

void Box2DBody::CreateChain(std::vector<b2Vec2>& _pointList)
{
	if (_pointList.size() < 4)
	{
		LOG_ERROR("Not enough vertices in Chain");
		return;
	}

#ifdef DEBUG_TRUE
	auto node = std::shared_ptr<RenderNode>(
		new Box2DMeshRenderNode(_pointList, m_bodyId, true));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif

	for (auto& point : _pointList)
	{
		point.x /= DEFAULT_OBJECT_SIZE;
		point.y /= DEFAULT_OBJECT_SIZE;
	}

	b2ChainDef chainDef = b2DefaultChainDef();
	chainDef.filter.categoryBits = m_filter;
	chainDef.filter.maskBits = Box2DBodyManager::GetMaskLayerBit(m_filter);

	chainDef.points = _pointList.data();
	chainDef.count = static_cast<int32_t>(_pointList.size());
	chainDef.isLoop = true;

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2CreateChain(m_bodyId, &chainDef);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif
}

void Box2DBody::SetPosition(Vector2 _pos)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	b2BodyId id = m_bodyId;
	Box2D::WorldManager::AddWorldTask(std::move([id, _pos]()
		{
			b2Vec2 pos = { _pos.x / DEFAULT_OBJECT_SIZE,_pos.y / DEFAULT_OBJECT_SIZE };
			b2Body_SetTransform(id, pos, b2Body_GetRotation(id));
		})
	);
#else
	b2Vec2 pos = { _pos.x / DEFAULT_OBJECT_SIZE,_pos.y / DEFAULT_OBJECT_SIZE };
	b2Body_SetTransform(m_bodyId, pos, b2Body_GetRotation(m_bodyId));
#endif
}

void Box2DBody::SetVelocity(b2Vec2 _velocity)
{
	/*_velocity.x /= DEFAULT_OBJECT_SIZE;
	_velocity.y /= DEFAULT_OBJECT_SIZE;*/
	b2Body_SetLinearVelocity(m_bodyId, _velocity);
}

void Box2DBody::SetVelocityX(float _velocityX)
{
	b2Vec2 vec = b2Body_GetLinearVelocity(m_bodyId);
	vec.x = _velocityX;
	b2Body_SetLinearVelocity(m_bodyId, vec);
}

void Box2DBody::SetVelocityY(float _velocityY)
{
	b2Vec2 vec = b2Body_GetLinearVelocity(m_bodyId);
	vec.y = _velocityY;
	b2Body_SetLinearVelocity(m_bodyId, vec);
}

const b2Vec2 Box2DBody::GetVelocity() const
{
	return b2Body_GetLinearVelocity(m_bodyId);
}

void Box2DBody::AddForce(b2Vec2 _force)
{
	_force.x *= DEFAULT_OBJECT_SIZE;
	_force.y *= DEFAULT_OBJECT_SIZE;
	b2Body_ApplyForceToCenter(m_bodyId, _force, true);
}

void Box2DBody::AddForceImpule(b2Vec2 _force)
{
	_force.x *= DEFAULT_OBJECT_SIZE;
	_force.y *= DEFAULT_OBJECT_SIZE;
	b2Body_ApplyLinearImpulseToCenter(m_bodyId, _force, true);
}

float  Box2DBody::GetGravityScale() const
{
	return b2Body_GetGravityScale(m_bodyId);
}

void Box2DBody::SetGravityScale(float _scale)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	b2BodyId id = m_bodyId;
	Box2D::WorldManager::AddWorldTask(std::move([id, _scale]()
		{
			b2Body_SetGravityScale(id, _scale);
		})
	);
#else
	b2Body_SetGravityScale(m_bodyId, _scale);
#endif
}

void Box2DBody::SetFixedRotation(bool _flag)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	b2BodyId id = m_bodyId;
	Box2D::WorldManager::AddWorldTask(std::move([id, _flag]()
		{
			b2Body_SetFixedRotation(id, _flag);
		})
	);
#else
	b2Body_SetFixedRotation(m_bodyId, _flag);
#endif
}

void Box2DBody::SetAwake(bool _awake)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	b2BodyId id = m_bodyId;
	Box2D::WorldManager::AddWorldTask(std::move([id, _awake]()
		{
			b2Body_SetAwake(id, _awake);
		})
	);
#else
	b2Body_SetAwake(m_bodyId, _awake);
#endif
}

bool OverlapResultFcn(b2ShapeId shapeId, void* context)
{
	std::vector<b2ShapeId>* shpeIds = (std::vector<b2ShapeId>*)context;

	shpeIds->push_back(shapeId);
	// continue the query
	return true;
}


void Box2DBody::GetOverlapObject(std::vector<GameObject*>& _objects)
{
	const b2Transform b2tf = b2Body_GetTransform(m_bodyId);
	GetOverlapObject(_objects, b2tf);
}

void Box2DBody::GetOverlapObject(std::vector<GameObject*>& _objects, b2Transform _tf)
{
	std::vector<b2ShapeId> shpeIds;
	b2QueryFilter filter;
	filter.categoryBits = m_filter;
	filter.maskBits = Box2DBodyManager::GetMaskLayerBit(m_filter);
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	for (auto& shpeId : m_shapeList)
	{
		switch (b2Shape_GetType(shpeId))
		{
		case b2_polygonShape:
		{
			auto polygon = b2Shape_GetPolygon(shpeId);
			b2World_OverlapPolygon(*Box2D::WorldManager::currentWorldId, &polygon, _tf, filter, OverlapResultFcn, &shpeIds);
		}
		break;
		case b2_circleShape:
		{
			auto circle = b2Shape_GetCircle(shpeId);
			b2World_OverlapCircle(*Box2D::WorldManager::currentWorldId, &circle, _tf, filter, OverlapResultFcn, &shpeIds);
		}
		break;
		case b2_capsuleShape:
		{
			auto causule = b2Shape_GetCapsule(shpeId);
			b2World_OverlapCapsule(*Box2D::WorldManager::currentWorldId, &causule, _tf, filter, OverlapResultFcn, &shpeIds);
		}
		break;
		}
		}
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif
	for (auto& id : shpeIds)
	{
		auto bodyId = b2Shape_GetBody(id);
		//if (bodyId.index1 == m_bodyId.index1) continue;

		auto iter = Box2DBodyManager::m_bodyObjectName.find(bodyId.index1);
		if (iter != Box2DBodyManager::m_bodyObjectName.end())
		{
			auto object = ObjectManager::Find(iter->second);
			if (object != nullptr)
			{
				_objects.push_back(object);
			}
		}
	}
}

void Box2DBody::GetOverlapObject(std::unordered_map<GameObject*, b2ShapeId>& _objects)
{
	const b2Transform b2tf = b2Body_GetTransform(m_bodyId);
	GetOverlapObject(_objects, b2tf, m_filter);
}

void Box2DBody::GetOverlapObject(std::unordered_map<GameObject*, b2ShapeId>& _objects, b2Transform _tf)
{
	GetOverlapObject(_objects, _tf, m_filter);
}

void Box2DBody::GetOverlapObject(std::unordered_map<GameObject*, b2ShapeId>& _objects, b2Transform _tf, FILTER _filter)
{
	std::vector<b2ShapeId> shpeIds;
	b2QueryFilter filter;
	filter.categoryBits = _filter;
	filter.maskBits = Box2DBodyManager::GetMaskLayerBit(_filter);
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	for (auto& shpeId : m_shapeList)
	{
		switch (b2Shape_GetType(shpeId))
		{
		case b2_polygonShape:
		{
			auto polygon = b2Shape_GetPolygon(shpeId);
			b2World_OverlapPolygon(*Box2D::WorldManager::currentWorldId, &polygon, _tf, filter, OverlapResultFcn, &shpeIds);
		}
		break;
		case b2_circleShape:
		{
			auto circle = b2Shape_GetCircle(shpeId);
			b2World_OverlapCircle(*Box2D::WorldManager::currentWorldId, &circle, _tf, filter, OverlapResultFcn, &shpeIds);
		}
		break;
		case b2_capsuleShape:
		{
			auto causule = b2Shape_GetCapsule(shpeId);
			b2World_OverlapCapsule(*Box2D::WorldManager::currentWorldId, &causule, _tf, filter, OverlapResultFcn, &shpeIds);
		}
		break;
		}
		}
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif
	for (auto& id : shpeIds)
	{
		auto bodyId = b2Shape_GetBody(id);
		//if (bodyId.index1 == m_bodyId.index1) continue;

		auto iter = Box2DBodyManager::m_bodyObjectName.find(bodyId.index1);
		if (iter != Box2DBodyManager::m_bodyObjectName.end())
		{
			auto object = ObjectManager::Find(iter->second);
			if (object != nullptr)
			{
				_objects.insert(std::make_pair(object, id));
			}
		}
	}
}

void Box2DBodyManager::DisableCollisionFilter(FILTER _filter01, FILTER _filter02)
{
	FILTER filter = _filter01;
	FILTER target = _filter02;
	for (int i = 0; i < 2; i++)
	{
		auto iter = m_layerFilterBit.find(filter);
		if (iter == m_layerFilterBit.end())
		{
			m_layerFilterBit.insert(std::make_pair(filter, ALL_BITS));
			iter = m_layerFilterBit.find(filter);
		}
		iter->second &= ~target;

		filter = _filter02;
		target = _filter01;
	}
}

void Box2DBodyManager::EnableCollisionFilter(FILTER _filter01, FILTER _filter02)
{
	FILTER filter = _filter01;
	FILTER target = _filter02;
	for (int i = 0; i < 2; i++)
	{
		auto iter = m_layerFilterBit.find(filter);
		if (iter == m_layerFilterBit.end())
		{
			m_layerFilterBit.insert(std::make_pair(filter, ALL_BITS));
			iter = m_layerFilterBit.find(filter);
		}
		iter->second |= target;

		filter = _filter02;
		target = _filter01;
	}
}

void Box2DBodyManager::OnlyCollisionFilter(FILTER _filter, FILTER _target)
{
	auto iter = m_layerFilterBit.find(_filter);
	if (iter == m_layerFilterBit.end())
	{
		m_layerFilterBit.insert(std::make_pair(_filter, ALL_BITS));
		iter = m_layerFilterBit.find(_filter);
	}
	iter->second = _target;
}

#ifdef DEBUG_TRUE
void Box2DBodyManager::Init()
{
	std::vector<WORD> indexList{
		2, 0 ,1, 3, 0, 1, 3, 2
	};

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.ByteWidth = sizeof(WORD) * static_cast<UINT>(indexList.size());
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA irData;
	irData.pSysMem = indexList.data();
	irData.SysMemPitch = 0;
	irData.SysMemSlicePitch = 0;

	DirectX11::m_pDevice->CreateBuffer(&ibDesc, &irData, m_boxIndexBuffer.GetAddressOf());

	float radius = HALF_OBJECT_SIZE;
	std::vector<Vertex> vertices;

	for (int i = 0; i <= numSegments; i++) {
		float theta = (float)i / (float)numSegments * DirectX::XM_2PI;
		float x = radius * cosf(theta);
		float y = radius * sinf(theta);

		vertices.push_back({ x, y, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f ,1.0f,1.0f });
	}

	vertices.push_back({ 0.0f,0.0f,0.5f, 1.0f,1.0f,1.0f, 1.0f,0.0f,0.0f });

	std::vector<WORD> indices;

	for (int i = 0; i < numSegments; i++) {
		indices.push_back(i);
		indices.push_back(i + 1);
	}

	indices.push_back(numSegments);
	indices.push_back(numSegments + 1);

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * static_cast<WORD>(vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices.data();

	DirectX11::m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, m_circleVertexBuffer.GetAddressOf());

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(WORD) * static_cast<WORD>(indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices.data();

	DirectX11::m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, m_circleIndexBuffer.GetAddressOf());
}
#endif


void Box2DBodyManager::ExcuteMoveFunction()
{
	for (const auto& func : moveFunctions) {
		func.second();
	}

	moveFunctions.clear();
}

unsigned int Box2DBodyManager::GetMaskLayerBit(FILTER _layer)
{
	auto iter = m_layerFilterBit.find(_layer);
	if (iter != m_layerFilterBit.end())
	{
		return iter->second;
	}

	m_layerFilterBit.insert(std::make_pair(_layer, ALL_BITS));
	return m_layerFilterBit.find(_layer)->second;
}

#ifdef DEBUG_TRUE

void Box2DBoxRenderNode::Draw()
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(Box2DBodyManager::m_boxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	static VSObjectConstantBuffer cb;

	const auto& transform = m_object->transform;
	const auto& objectCb = m_object->GetContantBuffer();

	auto rad = static_cast<float>(transform.angle.z.Get());
	Vector2 offset;
	offset.x = m_offset.x * cos(rad) - m_offset.y * sin(rad);
	offset.y = m_offset.x * sin(rad) + m_offset.y * cos(rad);

	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_size.x, m_size.y, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(rad + m_angle);
	cb.world *= DirectX::XMMatrixTranslation(transform.position.x + offset.x, transform.position.y + offset.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	/*cb.view = objectCb.view;
	cb.projection = objectCb.projection;*/

	SetDebugBodyColor(m_bodyId, cb.color);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(8, 0, 0);

	//次のポインタにつなぐ
	NextFunc();
}

inline void Box2DCircleRenderNode::Draw()
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, Box2DBodyManager::m_circleVertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(Box2DBodyManager::m_circleIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	static VSObjectConstantBuffer cb;

	const auto& transform = m_object->transform;
	const auto& objectCb = m_object->GetContantBuffer();


	auto rad = static_cast<float>(transform.angle.z.Get());
	Vector2 offset;
	offset.x = m_offset.x * cos(rad) - m_offset.y * sin(rad);
	offset.y = m_offset.x * sin(rad) + m_offset.y * cos(rad);

	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_size, m_size, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(rad);
	cb.world *= DirectX::XMMatrixTranslation(transform.position.x + offset.x, transform.position.y + offset.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	//cb.view = objectCb.view;
	//cb.projection = objectCb.projection;

	SetDebugBodyColor(m_bodyId, cb.color);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(Box2DBodyManager::numSegments * 2 + 2, 0, 0);

	//次のポインタにつなぐ
	NextFunc();
}

inline void Box2DCapsuleRenderNode::Draw()
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, Box2DBodyManager::m_circleVertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(Box2DBodyManager::m_circleIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	static VSObjectConstantBuffer cb;

	const auto& transform = m_object->transform;
	const auto& objectCb = m_object->GetContantBuffer();

	const float objectRad = static_cast<float>(transform.angle.z.Get());
	const float rad = objectRad + m_angle;

	Vector2 offset;
	offset.x = transform.position.x + m_offset.x * cos(objectRad) - m_offset.y * sin(objectRad);
	offset.y = transform.position.y + m_offset.x * sin(objectRad) + m_offset.y * cos(objectRad);

	Vector2 heightOffset;
	heightOffset.x = m_sizeY * sin(rad);
	heightOffset.y = m_sizeY * cos(rad);

	//上の半球描画
	//======================================================================================================================
	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_diameter, m_diameter, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(rad);
	cb.world *= DirectX::XMMatrixTranslation(offset.x - heightOffset.x, offset.y + heightOffset.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	/*cb.view = objectCb.view;
	cb.projection = objectCb.projection;*/

	SetDebugBodyColor(m_bodyId, cb.color);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(Box2DBodyManager::numSegments, 0, 0);

	//下の半球描画
	//======================================================================================================================
	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_diameter, m_diameter, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(rad + Math::PI);
	cb.world *= DirectX::XMMatrixTranslation(offset.x + heightOffset.x, offset.y - heightOffset.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(Box2DBodyManager::numSegments, 0, 0);

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(Box2DBodyManager::m_boxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	//胴体のbox描画
	//======================================================================================================================
	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_diameter, m_height, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(objectRad + m_angle);
	cb.world *= DirectX::XMMatrixTranslation(offset.x, offset.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(4, 0, 0);

	//次のポインタにつなぐ
	NextFunc();
}

Box2DMeshRenderNode::Box2DMeshRenderNode(std::vector<b2Vec2>& _pointList, b2BodyId _bodyId, bool _loop) :m_bodyId(_bodyId)
{
	std::vector<Vertex> vertices;

	int count = static_cast<int>(_pointList.size());

	for (int i = 0; i < count; i++) {
		vertices.push_back({ _pointList[i].x, _pointList[i].y, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f ,1.0f,1.0f });
	}

	std::vector<WORD> indices;

	for (int i = 0; i < count - 1; i++) {
		indices.push_back(i);
		indices.push_back(i + 1);
	}

	if (_loop)
	{
		indices.push_back(count - 1);
		indices.push_back(0);
	}

	indexCount = static_cast<int>(indices.size());

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * static_cast<WORD>(vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices.data();

	DirectX11::m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, m_chainVertexBuffer.GetAddressOf());

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(WORD) * static_cast<WORD>(indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices.data();

	DirectX11::m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, m_chainIndexBuffer.GetAddressOf());
}

inline void Box2DMeshRenderNode::Draw()
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, m_chainVertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(m_chainIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	static VSObjectConstantBuffer cb;

	const auto& transform = m_object->transform;
	const auto& objectCb = m_object->GetContantBuffer();

	auto rad = static_cast<float>(transform.angle.z.Get());

	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(1.0f, 1.0f, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(rad);
	cb.world *= DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	/*cb.view = objectCb.view;
	cb.projection = objectCb.projection;*/

	SetDebugBodyColor(m_bodyId, cb.color);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(indexCount, 0, 0);


	//次のポインタにつなぐ
	NextFunc();
}

Box2DConvexMeshRenderNode::Box2DConvexMeshRenderNode(std::vector<b2Vec2>& _pointList, b2BodyId _bodyId/*, bool _loop*/) :
	Box2DMeshRenderNode(_bodyId)
{
	std::vector<Vertex> vertices;

	int count = static_cast<int>(_pointList.size());

	for (int i = 0; i < count; i++) {
		vertices.push_back({ _pointList[i].x, _pointList[i].y, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f ,1.0f,1.0f });
	}

	std::vector<WORD> indices;

	for (int i = 0; i < count - 1; i++) {
		for (int j = i + 1; j < count;j++)
		{
			indices.push_back(i);
			indices.push_back(j);
		}
	}

	/*if (_loop)
	{
		indices.push_back(count - 1);
		indices.push_back(0);
	}*/

	indexCount = static_cast<int>(indices.size());

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * static_cast<WORD>(vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices.data();

	DirectX11::m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, m_chainVertexBuffer.GetAddressOf());

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(WORD) * static_cast<WORD>(indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices.data();

	DirectX11::m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, m_chainIndexBuffer.GetAddressOf());
}

void SetDebugBodyColor(b2BodyId _bodyId, DirectX::XMFLOAT4& _color)
{
	b2BodyType bodyType = b2Body_GetType(_bodyId);
	b2Vec2 vec = b2Body_GetLinearVelocity(_bodyId);
	if (bodyType == b2_dynamicBody && (abs(vec.x) > 150 || abs(vec.y) > 150))
	{
		_color = Box2D::b2_colorRed;
	}
	if (bodyType == b2_dynamicBody && (abs(vec.x) > 0 || abs(vec.y) > 0))
	{
		_color = Box2D::b2_colorPink;
	}
	else if (bodyType == b2_staticBody)
	{
		_color = Box2D::b2_colorGreen;
	}
	else if (bodyType == b2_kinematicBody)
	{
		_color = Box2D::b2_colorBlue;
	}
	else
	{
		_color = Box2D::b2_colorGray;
	}
}

#endif

