
std::unordered_map<FILTER, unsigned int> Box2DBodyManager::m_layerFilterBit;

//shapeIdに対応したオブジェクトの名前を格納
std::unordered_map<int32_t, const std::string&> Box2DBodyManager::m_bodyObjectName;

#ifdef DEBUG_TRUE
const int Box2DBodyManager::numSegments = 36;
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
	Box2D::WorldManager::GenerateBody(m_bodyId, &bodyDef);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

	Box2DBodyManager::m_bodyObjectName.emplace(m_bodyId.index1, _object->GetName());
}

Box2DBody::Box2DBody(GameObject* _object, b2BodyDef* _bodyDef)
{
	auto& position = _object->transform.position;
	_bodyDef->position = { position.x / DEFAULT_OBJECT_SIZE, position.y / DEFAULT_OBJECT_SIZE };

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif
	Box2D::WorldManager::GenerateBody(m_bodyId, _bodyDef);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

	Box2DBodyManager::m_bodyObjectName.emplace(m_bodyId.index1, _object->GetName());
}

Box2DBody::Box2DBody(GameObject* _object, SERIALIZE_INPUT& ar)
{
	BodySaveData data;
	std::vector<b2ShapeType> types;
	ar(CEREAL_NVP(m_filter), CEREAL_NVP(data), CEREAL_NVP(types));

	auto& position = _object->transform.position;
	//ボディ定義とワールドIDを使ってグラウンド・ボディを作成する
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = data.type;
	bodyDef.position = data.pos;
	bodyDef.rotation = data.rot;
	bodyDef.linearVelocity = data.lineVec;
	bodyDef.automaticMass = data.autoMass;
	bodyDef.angularVelocity = data.angleVec;
	bodyDef.gravityScale = data.gravity;
	bodyDef.isBullet = data.bullet;
	bodyDef.fixedRotation = data.fixRot;
	bodyDef.isAwake = data.awake;

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif
	Box2D::WorldManager::GenerateBody(m_bodyId, &bodyDef);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

	Box2DBodyManager::m_bodyObjectName.emplace(m_bodyId.index1, _object->GetName());

	std::unordered_map<int32_t, std::pair<std::vector<b2Vec2>,std::vector<ShapeSaveData>>> chainVertex;
	for (auto& type : types)
	{
		ShapeSaveData shapeData;
		ar(CEREAL_NVP(shapeData));
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.isSensor = shapeData.sensor;
		shapeDef.friction = shapeData.friction;
		shapeDef.density = shapeData.density;
		shapeDef.restitution = shapeData.restitution;
		shapeDef.filter.categoryBits = m_filter;
		shapeDef.filter.maskBits = Box2DBodyManager::GetMaskFilterBit(m_filter);

		switch (type)
		{
		case b2_circleShape:
		{
			b2Circle circle;
			ar(CEREAL_NVP(circle));

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pPauseWorldUpdate();
#endif 
			auto shape = b2CreateCircleShape(m_bodyId, &shapeDef, &circle);

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pResumeWorldUpdate();
#endif
			m_shapeList.push_back(shape);

#ifdef DEBUG_TRUE
			Vector2 offset = { circle.center.x * DEFAULT_OBJECT_SIZE,circle.center.y * DEFAULT_OBJECT_SIZE };
			float diameter = circle.radius * 2;
			auto node = std::shared_ptr<RenderNode>(new Box2DCircleRenderNode(offset, diameter, m_bodyId));
			node->m_object = _object;
			m_nodeList.push_back((node));
			RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif
			break;
		}
		case b2_capsuleShape:
		{
			b2Capsule capsule;
			ar(CEREAL_NVP(capsule));

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pPauseWorldUpdate();
#endif 
			auto shape = b2CreateCapsuleShape(m_bodyId, &shapeDef, &capsule);

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pResumeWorldUpdate();
#endif
			m_shapeList.push_back(shape);

#ifdef DEBUG_TRUE
			auto& p1 = capsule.center1;
			auto& p2 = capsule.center2;
			b2Vec2 center = p1 + p2;
			Vector2 offset = {center.x * HALF_OBJECT_SIZE,center.y * HALF_OBJECT_SIZE };
			float diameter = capsule.radius * 2;
			float height = Math::PointDistance(p1.x, p1.y, p2.x, p2.y);
			float rad = Math::PointRadian(p1.x, p1.y, p2.x, p2.y) - Math::hPI;
			auto node = std::shared_ptr<RenderNode>(
				new Box2DCapsuleRenderNode(offset, diameter, height * HALF_OBJECT_SIZE, height, rad, m_bodyId));
			node->m_object = _object;
			m_nodeList.push_back((node));
			RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif

			break;
		}
		case b2_segmentShape:
		{
			b2Segment segment;
			ar(CEREAL_NVP(segment));

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pPauseWorldUpdate();
#endif 
			auto shape = b2CreateSegmentShape(m_bodyId, &shapeDef, &segment);

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pResumeWorldUpdate();
#endif
			m_shapeList.push_back(shape);

#ifdef DEBUG_TRUE
			Vector2 start = { segment.point1.x * DEFAULT_OBJECT_SIZE,segment.point1.y * DEFAULT_OBJECT_SIZE };
			Vector2 end = { segment.point2.x * DEFAULT_OBJECT_SIZE,segment.point2.y * DEFAULT_OBJECT_SIZE };
			Vector2 dis = end - start;
			Vector2 center = start + dis / 2;
			float length = sqrt(dis.x * dis.x + dis.y * dis.y);
			float radian = Math::PointRadian(start.x, start.y, end.x, end.y);

			auto node = std::shared_ptr<RenderNode>(
				new Box2DLineRenderNode(length, radian, center, m_bodyId));
			node->m_object = _object;
			m_nodeList.push_back((node));
			RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif
			break;
		}
		case b2_polygonShape:
		{
			b2Polygon polygon;
			ar(CEREAL_NVP(polygon));

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pPauseWorldUpdate();
#endif 
			auto shape = b2CreatePolygonShape(m_bodyId, &shapeDef, &polygon);

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pResumeWorldUpdate();
#endif
			m_shapeList.push_back(shape);

#ifdef DEBUG_TRUE
			bool box = false;
			auto& vertex = polygon.vertices;
			if (polygon.count == 4)
			{
				if (vertex[0].x == vertex[3].x &&
					vertex[0].y == vertex[1].y &&
					vertex[1].x == vertex[2].x &&
					vertex[2].y == vertex[3].y)
					box = true;
			}

			if (box)
			{
				Vector2 offset = { polygon.centroid.x * DEFAULT_OBJECT_SIZE,polygon.centroid.y * DEFAULT_OBJECT_SIZE };
				Vector2 size = { vertex[2].x - vertex[3].x,vertex[2].y - vertex[1].y};
				auto& p1 = vertex[0];
				auto& p2 = vertex[1];
				float rad = Math::PointRadian(p1.x, p1.y, p2.x, p2.y);
				auto node = std::shared_ptr<RenderNode>(
					new Box2DBoxRenderNode(offset, size, rad, m_bodyId));
				node->m_object = _object;
				m_nodeList.push_back((node));
				RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
			}
			else
			{
				std::vector<b2Vec2> pointList;
				//pointList.resize(polygon.count);
				for (int i = 0; i < polygon.count; i++)
				{
					pointList.push_back(
						{ polygon.vertices[i].x * DEFAULT_OBJECT_SIZE,polygon.vertices[i].y * DEFAULT_OBJECT_SIZE });
				}
				//Polygon用の描画ノードの生成
				auto node = std::shared_ptr<RenderNode>(
					new Box2DConvexMeshRenderNode(pointList, m_bodyId));
				node->m_object = _object;
				m_nodeList.push_back((node));
				RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
			}
#endif
			break;
		}
		case b2_chainSegmentShape:
		{
			b2ChainSegment chainSegment;
			ar(CEREAL_NVP(chainSegment));
			auto iter = chainVertex.find(chainSegment.chainId);
			if (iter == chainVertex.end())
			{
				iter = chainVertex.emplace(chainSegment.chainId, 
					std::make_pair(std::vector<b2Vec2>(), std::vector<ShapeSaveData>())).first;
			}
			iter->second.first.push_back(chainSegment.segment.point1);
			iter->second.second.push_back(shapeData);

#ifdef DEBUG_TRUE
			Vector2 start = { chainSegment.segment.point1.x * DEFAULT_OBJECT_SIZE,
				chainSegment.segment.point1.y * DEFAULT_OBJECT_SIZE };
			Vector2 end = { chainSegment.segment.point2.x * DEFAULT_OBJECT_SIZE,
				chainSegment.segment.point2.y * DEFAULT_OBJECT_SIZE };
			Vector2 dis = end - start;
			Vector2 center = start + dis / 2;
			float length = sqrt(dis.x * dis.x + dis.y * dis.y);
			float radian = Math::PointRadian(start.x, start.y, end.x, end.y);

			auto node = std::shared_ptr<RenderNode>(
				new Box2DLineRenderNode(length, radian, center, m_bodyId));
			node->m_object = _object;
			m_nodeList.push_back((node));
			RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif
			break;
		}
		default:
			break;
		}
	}

	for (auto& chainData : chainVertex)
	{
		b2ChainDef chainDef = b2DefaultChainDef();
		chainDef.filter.categoryBits = m_filter;
		chainDef.filter.maskBits = Box2DBodyManager::GetMaskFilterBit(m_filter);

		chainDef.points = chainData.second.first.data();
		chainDef.count = static_cast<int32_t>(chainData.second.first.size());
		chainDef.isLoop = true;

#ifdef BOX2D_UPDATE_MULTITHREAD
		Box2D::WorldManager::pPauseWorldUpdate();
#endif 
		b2ChainId chainId = b2CreateChain(m_bodyId, &chainDef);

#ifdef BOX2D_UPDATE_MULTITHREAD
		Box2D::WorldManager::pResumeWorldUpdate();
#endif

		int chainCount = b2Chain_GetSegmentCount(chainId);
		std::vector<b2ShapeId> shapeArray;
		shapeArray.resize(chainCount);
		int num = b2Chain_GetSegments(chainId, shapeArray.data(), chainCount);
		for (int i = 0;i < num;i++)
		{
			auto& shape = shapeArray[i];
			auto& data = chainData.second.second[i];
			b2Shape_SetFriction(shape, data.friction);
			b2Shape_SetDensity(shape, data.density);
			b2Shape_SetRestitution(shape, data.restitution);
			m_shapeList.push_back(shape);
		}
	}

	auto massData = b2Body_GetMassData(m_bodyId);
	massData.mass = data.mass;
	b2Body_SetMassData(m_bodyId, massData);

#ifdef RELEASE_SERIALIZE_VIEW_HITBOX
	ar(CEREAL_NVP(renderData));


#ifdef DEBUG_TRUE
	for (auto& data : renderData)
	{
		auto node = std::shared_ptr<RenderNode>(data->CreateRenderNode(m_bodyId));
		node->m_object = _object;
		m_nodeList.push_back((node));
		RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
	}
#endif

#endif

}

inline void Box2DBody::Update()
{
	//auto func = [&]() 
		{
			auto pos = b2Body_GetPosition(m_bodyId);
			pos *= DEFAULT_OBJECT_SIZE;
			m_this->transform.position = Vector3(pos.x, pos.y, 0.5f);
			auto rot = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
			m_this->transform.angle.z.Set(rot);
		};

	//Box2DBodyManager::moveFunctions.emplace(m_this->GetName(), func);

	//Box2DBodyManager::m_moveBodyObjects.emplace(std::make_pair(m_this, m_bodyId));
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

void Box2DBody::Serialize(SERIALIZE_OUTPUT& ar)
{
	BodySaveData data;
	data.pos = b2Body_GetPosition(m_bodyId);
	data.rot = b2Body_GetRotation(m_bodyId);
	data.type = b2Body_GetType(m_bodyId);
	data.lineVec = b2Body_GetLinearVelocity(m_bodyId);
	data.angleVec = b2Body_GetAngularVelocity(m_bodyId);
	data.gravity = b2Body_GetGravityScale(m_bodyId);
	data.autoMass = b2Body_GetAutomaticMass(m_bodyId);
	data.mass = b2Body_GetMass(m_bodyId);
	data.bullet = b2Body_IsBullet(m_bodyId);
	data.fixRot = b2Body_IsFixedRotation(m_bodyId);
	data.awake = b2Body_IsAwake(m_bodyId);

	std::vector<b2ShapeType> types;
	for (auto& shape : m_shapeList)
	{
		b2ShapeType type = b2Shape_GetType(shape);
		types.push_back(type);
	}

	ar(CEREAL_NVP(m_filter),CEREAL_NVP(data),CEREAL_NVP(types));

	for (auto& shape : m_shapeList)
	{
		ShapeSaveData shapeData;
		shapeData.sensor = b2Shape_IsSensor(shape);
		shapeData.friction = b2Shape_GetFriction(shape);
		shapeData.density = b2Shape_GetDensity(shape);
		shapeData.restitution = b2Shape_GetRestitution(shape);

		ar(CEREAL_NVP(shapeData));

		b2ShapeType type = b2Shape_GetType(shape);
		switch (type)
		{
		case b2_circleShape:
			b2Circle circle = b2Shape_GetCircle(shape);
			ar(CEREAL_NVP(circle));
			break;
		case b2_capsuleShape:
			b2Capsule capsule = b2Shape_GetCapsule(shape);
			ar(CEREAL_NVP(capsule));
			break;
		case b2_segmentShape:
			b2Segment segment = b2Shape_GetSegment(shape);
			ar(CEREAL_NVP(segment));
			break;
		case b2_polygonShape:
			b2Polygon polygon = b2Shape_GetPolygon(shape);
			ar(CEREAL_NVP(polygon));
			break;		
		case b2_chainSegmentShape:
			b2ChainSegment chainSegment = b2Shape_GetChainSegment(shape);
			ar(CEREAL_NVP(chainSegment));
			break;
		default:
			break;
		}
	}

#ifdef RELEASE_SERIALIZE_VIEW_HITBOX
	ar(CEREAL_NVP(renderData));
#endif
}


void Box2DBody::DrawImGui(ImGuiApp::HandleUI& _handle)
{
#ifdef DEBUG_TRUE
	if (ImGui::Button("<>##type"))
	{
		ImGui::OpenPopup("changeType");
	}
	ImGui::SetItemTooltip("change type");
	ImGui::SameLine();

	b2BodyType bodyType = GetType();
	ImGui::Text("type : %s", magic_enum::enum_name(bodyType).data());

	if (ImGui::BeginPopup("changeType"))
	{
		for (int i = 0; i < b2_bodyTypeCount; i++)
		{
			b2BodyType type = (b2BodyType)i;
			bool same = type == bodyType;
			if (ImGui::Selectable(magic_enum::enum_name(type).data(), same))
			{
				if (!same)SetType(type);
			}
		}
		ImGui::EndPopup();
	}

	if (ImGui::Button("<>##filter"))
	{
		ImGui::OpenPopup("changeFilter");
	}
	ImGui::SetItemTooltip("change filer");
	ImGui::SameLine();
	ImGui::Text("Filter : %s", magic_enum::enum_name(m_filter).data());

	if (ImGui::BeginPopup("changeFilter"))
	{
		static long long numFilter = magic_enum::enum_count<FILTER>();
		for (int i = 0; i < numFilter; i++)
		{
			FILTER filter = (FILTER)pow(2, i);
			bool same = filter == m_filter;
			if (ImGui::Selectable(magic_enum::enum_name(filter).data(), &same))
			{
				SetFilter(filter);
			}
		}
		ImGui::EndPopup();
	}

	auto vec = b2Body_GetLinearVelocity(m_bodyId);
	float velocity[2] = { vec.x,vec.y };
	//ImGui::Text(" LinerVelocity\n  <x: %.2f   y:%.2f>", vec.x, vec.y);
	if (ImGui::InputFloat2("Velocity", velocity))
	{
		b2Body_SetLinearVelocity(m_bodyId, { velocity[0],velocity[1] });
	}

	float angulerVelocity = b2Body_GetAngularVelocity(m_bodyId);
	if (ImGui::InputFloat("AnglerVelocity", &angulerVelocity))
	{
		b2Body_SetAngularVelocity(m_bodyId, angulerVelocity);
	}
	
	auto massData = b2Body_GetMassData(m_bodyId);
	if (ImGui::InputFloat("Mass", &massData.mass))
	{
		b2Body_SetMassData(m_bodyId, massData);
	}

	float gravity = b2Body_GetGravityScale(m_bodyId);
	if (ImGui::InputFloat("Gravity", &gravity))
	{
		b2Body_SetGravityScale(m_bodyId, gravity);
	}

	bool isBullet = b2Body_IsBullet(m_bodyId);
	if (ImGui::Checkbox("Bullet", &isBullet))
	{
		b2Body_SetBullet(m_bodyId, isBullet);
	}

	bool isFixRot = b2Body_IsFixedRotation(m_bodyId);
	if (ImGui::Checkbox("FixedRotation", &isFixRot))
	{
		b2Body_SetFixedRotation(m_bodyId, isFixRot);
	}

	bool isAwake = b2Body_IsAwake(m_bodyId);
	if (ImGui::Checkbox("Awake", &isAwake))
	{
		b2Body_SetAwake(m_bodyId, isAwake);
	}

	ImGui::SeparatorText("+Create");

	static const char* shapeTypes[] =
	{
		"Box",
		"Circle",
		"Capsule",
		"Segment",
		"Polygon",
		"Chain"
	};
	static Box2DShapeType selectType = BOX;
	if (ImGui::BeginListBox("##shapeType", ImVec2(100, 110)))
	{
		for (int i = 0; i < TYPE_MAX; i++)
		{
			if (ImGui::Selectable(magic_enum::enum_name((Box2DShapeType)i).data(), i == selectType))
			{
				selectType = (Box2DShapeType)i;
			}
		}
		ImGui::EndListBox();
	}

	ImGui::SameLine();

	ImGui::BeginGroup();

	bool create = false;
	static bool editVertex = false;

	if (ImGui::Button("Create"))create = true;

	ImGui::SameLine();
	if (ImGui::Button("Swap"))
	{
		if (selectType < SEGMENT || editVertex)
		{
			for (auto& node : m_nodeList)
			{
				node->Delete(LAYER_BOX2D_DEBUG);
			}
			m_nodeList.clear();
			for (auto& shape : m_shapeList)
			{
				b2DestroyShape(shape);
			}
			m_shapeList.clear();
			create = true;
		}
	}

	if (ImGui::BeginChild("parameter", ImVec2(IMGUI_WINDOW_WIDTH - 100, 100)))
	{
		static bool sensor = false;
		switch (selectType)
		{
		case BOX:
		{
			static Vector2 size = { 5.0f,5.0f };
			static Vector2 offset;
			static float angle = 0.0f;
			static bool decideSize = false;
			static bool decideOffset = false;
			static bool decideAngle = false;
			ImGui::Checkbox("sensor", &sensor);
			ImGui::Checkbox("##size", &decideSize);
			ImGui::SameLine();
			if (!decideSize)ImGui::BeginDisabled();
			ImGui::DragFloat2("size", size.data(), 0.1f, 0.0f, 100.0f);
			if (!decideSize)ImGui::EndDisabled();

			ImGui::Checkbox("##offset", &decideOffset);
			ImGui::SameLine();
			if (!decideOffset)ImGui::BeginDisabled();
			ImGui::InputFloat2("offset", offset.data());
			if (!decideOffset)ImGui::EndDisabled();

			ImGui::Checkbox("##angle", &decideAngle);
			ImGui::SameLine();
			if (!decideAngle)ImGui::BeginDisabled();
			ImGui::DragFloat("angle", &angle, 1.0f, -180.0f, 180.0f);
			if (!decideAngle)ImGui::EndDisabled();

			if (create)
			{
				if (decideSize) CreateBoxShape(size, decideOffset ? offset : Vector2(0.0f, 0.0f), decideAngle ? angle : 0.0f, sensor);
				else CreateBoxShape(decideOffset ? offset : Vector2(0.0f, 0.0f), decideAngle ? angle : 0.0f, sensor);
			}
			break;
		}
		case CIRCLE:
		{
			static float diameter = 1.0f;
			static Vector2 offset = { 0.0f,0.0f };
			static bool decideDiameter = false;
			static bool decideOffset = false;

			ImGui::Checkbox("sensor", &sensor);
			ImGui::Checkbox("##diameter", &decideDiameter);
			ImGui::SameLine();
			if (!decideDiameter)ImGui::BeginDisabled();
			ImGui::InputFloat("diameter", &diameter);
			if (!decideDiameter)ImGui::EndDisabled();

			ImGui::Checkbox("##offset", &decideOffset);
			ImGui::SameLine();
			if (!decideOffset)ImGui::BeginDisabled();
			ImGui::InputFloat2("offset", offset.data());
			if (!decideOffset)ImGui::EndDisabled();

			if (create)
			{
				if (decideDiameter)CreateCircleShape(diameter, decideOffset ? offset : Vector2(0.0f, 0.0f), sensor);
				else CreateCircleShape(decideOffset ? offset : Vector2(0.0f, 0.0f), sensor);
			}
		}
		break;
		case CAPSULE:
		{
			static float diameter = 1.0f;
			static float height = 1.0f;
			static float angle = 0.0f;
			static Vector2 offset = { 0.0f,0.0f };
			static bool decideDiameter = false;
			static bool decideHeight = false;
			static bool decideAngle = false;
			static bool decideOffset = false;

			ImGui::Checkbox("sensor", &sensor);
			ImGui::Checkbox("##diameter", &decideDiameter);
			ImGui::SameLine();
			if (!decideDiameter)ImGui::BeginDisabled();
			ImGui::InputFloat("diameter", &diameter);
			if (!decideDiameter)ImGui::EndDisabled();

			ImGui::Checkbox("##height", &decideHeight);
			ImGui::SameLine();
			if (!decideHeight)ImGui::BeginDisabled();
			ImGui::DragFloat("height", &height, 0.1f, 0.0f, 100.0f);
			if (!decideHeight)ImGui::EndDisabled();

			ImGui::Checkbox("##angle", &decideAngle);
			ImGui::SameLine();
			if (!decideAngle)ImGui::BeginDisabled();
			ImGui::DragFloat("angle", &angle, 1.0f, -180.0f, 180.0f);
			if (!decideAngle)ImGui::EndDisabled();

			ImGui::Checkbox("##offset", &decideOffset);
			ImGui::SameLine();
			if (!decideOffset)ImGui::BeginDisabled();
			ImGui::InputFloat2("offset", offset.data());
			if (!decideOffset)ImGui::EndDisabled();

			if (create)
			{
				if (decideDiameter && decideHeight)
					CreateCapsuleShape(diameter, height, decideAngle ? angle : 0.0f, decideOffset ? offset : Vector2(0.0f, 0.0f), sensor);
				else if (decideHeight)
					CreateCapsuleShape(height, decideOffset ? offset : Vector2(0.0f, 0.0f), decideAngle ? angle : 0.0f, sensor);
				else
					CreateCapsuleShape(decideOffset ? offset : Vector2(0.0f, 0.0f), decideAngle ? angle : 0.0f, sensor);
			}
		}
		break;

		
		case SEGMENT:
		case POLYGON:
		case CHAIN:
		{
			static std::vector<Vector2> pointList =
			{
				{-100.0f,100.0f},{100.0f,100.0f},{100.0f,-100.0f},{-100.0f,-100.0f},
			};

			int pointNum = (int)pointList.size();

			static int selectIndex = -1;
			if (editVertex)
			{
				Vector2 worldPos = Input::Get().MousePoint();
				worldPos.x = worldPos.x * DISPALY_ASPECT_WIDTH / RenderManager::renderZoom.x + RenderManager::renderOffset.x;
				worldPos.y = worldPos.y * DISPALY_ASPECT_HEIGHT / RenderManager::renderZoom.y + RenderManager::renderOffset.y;

				if (selectIndex < 0)
				{
					if (Input::Get().MouseLeftTrigger())
					{
						for (int i = 0; i < pointNum; i++)
						{
							const Vector2& pos = pointList[i] + m_this->transform.position;
							Vector2 scale = { HALF_OBJECT_SIZE,HALF_OBJECT_SIZE };
							scale.x /= RenderManager::renderZoom.x;
							scale.y /= RenderManager::renderZoom.y;
							if ((pos.x - scale.x) < worldPos.x &&
								(pos.x + scale.x) > worldPos.x &&
								(pos.y - scale.y) < worldPos.y &&
								(pos.y + scale.y) > worldPos.y)
							{
								selectIndex = i;
								break;
							}
						}
					}
				}
				else
				{
					pointList[selectIndex] = worldPos - m_this->transform.position;

					if (Input::Get().MouseLeftRelease())
					{
						selectIndex = -1;
					}
				}
			}

			editVertex = _handle.DrawLockButton("editVertex");
			/*if (ImGui::Checkbox("EditLock##vertex", &editVertex))
			{
				_handle.LockHandle(editVertex, "editVertex");
			}*/
			if (editVertex)
			{
				bool notLoop = false;
				static bool out = false;
				switch (selectType)
				{
				case SEGMENT:
					notLoop = true;
					break;
				case POLYGON:
					break;
				case CHAIN:
					ImGui::Checkbox("out##vertex", &out);
					break;
				}

				if (create)
				{
					std::vector<b2Vec2> points;
					if (out)
					{
						for (int i = (int)pointList.size() - 1; i >= 0; i--)
						{
							auto& point = pointList[i];
							points.emplace_back(
								point.x, point.y
							);
						}
					}
					else
					{
						for (auto& point : pointList)
						{
							points.emplace_back(
								point.x, point.y
							);
						}
					}
					
					switch (selectType)
					{
					case SEGMENT:
						CreateSegment(points, sensor);
						break;
					case POLYGON:
						CreatePolygonShape(points, sensor);
						break;
					case CHAIN:
						CreateChain(points);
						break;
					}
				}

				if (ImGui::Button("+AddVertex"))
				{
					pointList.emplace_back(0.0f, 0.0f);
				}
				ImGui::SameLine();
				if (ImGui::Button("-Reset"))
				{
					pointList.clear();
					pointList.resize(4);
					pointList = { { -100.0f,100.0f }, { 100.0f,100.0f }, { 100.0f,-100.0f }, { -100.0f,-100.0f } };
				}
				ImGui::SameLine();
				if (ImGui::Button("=Load"))
				{
					pointList.clear();
					for (auto& shape : m_shapeList)
					{
						b2ShapeType type = b2Shape_GetType(shape);
						switch (type)
						{
							/// A circle with an offset
						case b2_circleShape:
						{
							auto circle = b2Shape_GetCircle(shape);
							pointList.emplace_back(circle.center.x * DEFAULT_OBJECT_SIZE,
								circle.center.y * DEFAULT_OBJECT_SIZE);
							break;
						}
						/// A capsule is an extruded circle
						case b2_capsuleShape:
						{
							auto capsule = b2Shape_GetCapsule(shape);
							pointList.emplace_back(capsule.center1.x * DEFAULT_OBJECT_SIZE,
								capsule.center1.y * DEFAULT_OBJECT_SIZE);
							break;
						}
						/// A line segment
						case b2_segmentShape:
						{
							auto segment = b2Shape_GetSegment(shape);
							pointList.emplace_back(segment.point1.x * DEFAULT_OBJECT_SIZE,
								segment.point1.y * DEFAULT_OBJECT_SIZE);
							break;
						}
						/// A convex polygon
						case b2_polygonShape:
						{
							auto polygon = b2Shape_GetPolygon(shape);
							for (int i = 0; i < polygon.count; i++)
							{
								pointList.emplace_back(
									polygon.vertices[i].x * DEFAULT_OBJECT_SIZE,
									polygon.vertices[i].y * DEFAULT_OBJECT_SIZE
								);
							}
							break;
						}
						/// A line segment owned by a chain shape

						case b2_chainSegmentShape:
						{
							auto chainSegment = b2Shape_GetChainSegment(shape);
							pointList.emplace_back(chainSegment.segment.point1.x * DEFAULT_OBJECT_SIZE,
								chainSegment.segment.point1.y * DEFAULT_OBJECT_SIZE);

							break;
						}
						}
					}
					while (pointList.size() < 4)
					{
						pointList.emplace_back(0.0f, 0.0f);
					}
				}

				ImGui::SeparatorText("Vertices");

				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
				if (ImGui::BeginChild("ListChild", ImVec2(280, 100), ImGuiChildFlags_Borders))
				{
					int count = 0;
					for (auto iter = pointList.begin(); iter != pointList.end();)
					{
						count++;

						ImGui::BulletText("%d", count);
						ImGui::SameLine();

						ImGui::PushID(count);
						if (pointNum > 4)
						{
							if (ImGui::Button("-##erase"))
							{
								iter = pointList.erase(iter);
								ImGui::PopID();
								continue;
							}
							ImGui::SameLine();
						}
						ImGui::DragFloat2("##pos", iter->data());
						ImGui::SameLine();
						if (ImGui::Button("+##add"))
						{
							Vector2 addPos = *iter;
							addPos.Normalize();
							iter = pointList.insert(iter + 1, addPos);
							ImGui::PopID();
							continue;
						}

						ImGui::PopID();

						iter++;
					}

					ImGui::EndChild();
				}
				ImGui::PopStyleVar();

				pointNum = (int)pointList.size();
				const float colorSeg = 1.0f / pointNum;
				for (int i = 0; i < pointNum - notLoop; i++)
				{
					int nextIndex = (i + 1) % pointNum;
					auto& start = pointList[i];
					auto& end = pointList[nextIndex];
					RenderManager::DrawRayNode rayNode;
					Vector2 dis = end - start;
					rayNode.center = start + dis / 2 + m_this->transform.position;
					rayNode.length = sqrt(dis.x * dis.x + dis.y * dis.y);
					rayNode.radian = Math::PointRadian(start.x, start.y, end.x, end.y);
					rayNode.color = XMFLOAT4(0.6f, 1.0f, 0.6f, 1.0f);

					RenderManager::m_drawRayNode.push_back(std::move(rayNode));

					RenderManager::DrawBoxNode boxNode;
					boxNode.center = pointList[i] + m_this->transform.position;
					boxNode.size = { 1.0f / RenderManager::renderZoom.x,1.0f / RenderManager::renderZoom.y };
					if (selectIndex == i)
						boxNode.color = XMFLOAT4(1.0f, colorSeg * i, 1.0f, 1.0f);
					else
						boxNode.color = XMFLOAT4(0.6f, colorSeg * i, 0.6f, 1.0f);

					RenderManager::m_drawBoxNode.push_back(std::move(boxNode));
				}

				RenderManager::DrawBoxNode boxNode;
				boxNode.center = m_this->transform.position;
				boxNode.size = { 1.0f / RenderManager::renderZoom.x,1.0f / RenderManager::renderZoom.y };
				RenderManager::m_drawBoxNode.push_back(std::move(boxNode));

				if (notLoop)
				{
					RenderManager::DrawBoxNode boxNode;
					boxNode.center = pointList[pointNum - 1] + m_this->transform.position;
					boxNode.size = { 1.0f / RenderManager::renderZoom.x,1.0f / RenderManager::renderZoom.y };
					if (selectIndex == pointNum - 1)
						boxNode.color = XMFLOAT4(1.0f, colorSeg * pointNum - 1, 1.0f, 1.0f);
					else
						boxNode.color = XMFLOAT4(0.6f, colorSeg * pointNum - 1, 0.6f, 1.0f);

					RenderManager::m_drawBoxNode.push_back(std::move(boxNode));
				}
			}
			break;
		}
		}
		ImGui::EndChild();
	}

	ImGui::EndGroup();
	if (!m_shapeList.empty())
	{
		if (ImGui::TreeNode("ShapeList"))
		{
			ImGui::BeginChild("ShapeListChild", ImVec2(0, 200));
			int count = 0;
			//ImGui::BeginDisabled(true); // Disable interaction
			//for (auto& shape : m_shapeList)
			for(decltype(m_shapeList)::iterator i = m_shapeList.begin();i != m_shapeList.end();)
			{
				ImGui::PushID(count);
				auto& shape = *i;
				if(ImGui::Button("erase"))
				{
					b2DestroyShape(shape);
					size_t index = std::distance(m_shapeList.begin(), i);
					auto iter = m_nodeList.begin() + index;
					iter->get()->Delete(LAYER_BOX2D_DEBUG);
					m_nodeList.erase(iter);
					i = m_shapeList.erase(i);

					ImGui::PopID();
					count++;
					continue;
				}

				b2ShapeType type = b2Shape_GetType(shape);
				ImGui::Text("type : %s", magic_enum::enum_name(type).data());
				
				bool sensor = b2Shape_IsSensor(shape);
				ImGui::Text("sensor : %s", sensor ? "true" : "false");
				
				float friction = b2Shape_GetFriction(shape);
				if (ImGui::InputFloat(" friction", &friction))
				{
					b2Shape_SetFriction(shape, friction);
				}
				float density = b2Shape_GetDensity(shape);
				if (ImGui::InputFloat(" density", &density))
				{
					b2Shape_SetDensity(shape, density);
				}
				float restitution = b2Shape_GetRestitution(shape);
				if (ImGui::InputFloat(" restitution", &restitution))
				{
					b2Shape_SetRestitution(shape, restitution);
				}

				++i;
				ImGui::PopID();
				count++;
			}
			
			ImGui::EndChild();
			//ImGui::EndDisabled(); // Re-enable interaction
			ImGui::TreePop();
		}
		ImGui::SameLine();
		if (ImGui::Button("clear"))
		{
			for (auto& shape : m_shapeList)
			{
				b2DestroyShape(shape);
			}
			for (auto& node : m_nodeList)
			{
				node->Delete(LAYER_BOX2D_DEBUG);
			}
			m_nodeList.clear();
			m_shapeList.clear();
		}
	}
#endif
}

void Box2DBody::SetFilter(const FILTER _filter)
{
	if (m_filter == _filter) return;

	m_filter = _filter;

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	for (auto shape : m_shapeList)
	{
		auto filter = b2Shape_GetFilter(shape);
		filter.categoryBits = m_filter;
		filter.maskBits = Box2DBodyManager::GetMaskFilterBit(m_filter);
		b2Shape_SetFilter(shape, filter);
	}
	
#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif 
}

void Box2DBody::ChangeFilter(const FILTER _filter)
{
	if (m_filter == _filter) return;

	m_filter = _filter;

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	for (auto& shape : m_shapeList)
	{
		auto def = b2DefaultShapeDef();
		def.filter.categoryBits = m_filter;
		def.filter.maskBits = Box2DBodyManager::GetMaskFilterBit(m_filter);
		def.density = b2Shape_GetDensity(shape);
		def.friction = b2Shape_GetFriction(shape);
		def.isSensor = b2Shape_IsSensor(shape);
		def.restitution = b2Shape_GetRestitution(shape);

		auto type = b2Shape_GetType(shape);
		switch (type)
		{
		case b2_circleShape:
		{
			auto circle = b2Shape_GetCircle(shape);
			b2DestroyShape(shape);
			shape = b2CreateCircleShape(m_bodyId, &def, &circle);
		}
			break;
		case b2_capsuleShape:
		{
			auto capsule = b2Shape_GetCapsule(shape);
			b2DestroyShape(shape);
			shape = b2CreateCapsuleShape(m_bodyId, &def, &capsule);
		}
		break;
		case b2_polygonShape:
		{
			auto polygon = b2Shape_GetPolygon(shape);
			b2DestroyShape(shape);
			shape = b2CreatePolygonShape(m_bodyId, &def, &polygon);
		}
		break;
		case b2_segmentShape:
		{
			auto segment = b2Shape_GetSegment(shape);
			b2DestroyShape(shape);
			shape = b2CreateSegmentShape(m_bodyId, &def, &segment);
		}
			break;
		case b2_chainSegmentShape:
		{
			b2Shape_SetFilter(shape, def.filter);
		}
		break;
		}
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

void Box2DBody::CreateBoxShape(Vector2 _offset, float _angle, bool _sensor)
{
	auto scale = m_this->transform.scale;
	CreateBoxShape({ scale.x, scale.y }, _offset, _angle, _sensor);
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
	shapeDef.filter.maskBits = Box2DBodyManager::GetMaskFilterBit(m_filter);

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
	auto node = std::shared_ptr<RenderNode>(new Box2DBoxRenderNode(_offset, _size, rad, m_bodyId));
	//auto node = std::shared_ptr<RenderNode>(new Box2DCircleRenderNode(scale.x, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif

#ifdef RELEASE_SERIALIZE_VIEW_HITBOX
	Box2DBoxData* boxData = new Box2DBoxData(_offset, _size, rad);
	renderData.emplace_back(boxData);
#endif

	m_shapeList.push_back(shape);
}

void Box2DBody::CreateCircleShape(bool _sensor)
{
	auto scale = m_this->transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCircleShape(diameter, { 0.0f,0.0f }, _sensor);
}

void Box2DBody::CreateCircleShape(Vector2 _offset, bool _sensor)
{
	auto scale = m_this->transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCircleShape(diameter, _offset,_sensor);
}

void Box2DBody::CreateCircleShape(float _diameter, Vector2 _offset, bool _sensor)
{
	//地面ポリゴンを作る。 b2MakeBox()ヘルパー関数を使い、地面ポリゴンを箱型にする。箱の中心は親ボディの原点である。
	b2Circle circle = { {_offset.x / DEFAULT_OBJECT_SIZE,_offset.y / DEFAULT_OBJECT_SIZE},_diameter / 2 };

	//シェイプを作成して地面のボディを仕上げる
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.isSensor = _sensor;
	shapeDef.filter.categoryBits = m_filter;
	shapeDef.filter.maskBits = Box2DBodyManager::GetMaskFilterBit(m_filter);

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

#ifdef RELEASE_SERIALIZE_VIEW_HITBOX
	Box2DCircleData* circleData = new Box2DCircleData(_offset,_diameter);
	renderData.emplace_back(circleData);
#endif

	m_shapeList.push_back(shape);
}

void Box2DBody::CreateCapsuleShape(bool _sensor)
{
	auto& transform = m_this->transform;
	auto scale = transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCapsuleShape(diameter, scale.y / 2, 0.0f, { 0.0f,0.0f }, _sensor);
}

void Box2DBody::CreateCapsuleShape(Vector2 _offset, float _angle, bool _sensor)
{
	auto& transform = m_this->transform;
	auto scale = transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCapsuleShape(diameter, scale.y / 2,_angle, _offset, _sensor);
}

void Box2DBody::CreateCapsuleShape(float _height, Vector2 _offset, float _angle, bool _sensor)
{
	auto& transform = m_this->transform;
	auto scale = transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCapsuleShape(diameter, _height, _angle, _offset, _sensor);
}

void Box2DBody::CreateCapsuleShape(float _diameter, float _height, float _angle, Vector2 _offset, bool _sensor)
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
	shapeDef.isSensor = _sensor;
	shapeDef.filter.categoryBits = m_filter;
	shapeDef.filter.maskBits = Box2DBodyManager::GetMaskFilterBit(m_filter);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	auto shape = b2CreateCapsuleShape(m_bodyId, &shapeDef, &capsule);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif


#ifdef DEBUG_TRUE
	auto node = std::shared_ptr<RenderNode>(
		new Box2DCapsuleRenderNode(_offset, _diameter, halfHeight * DEFAULT_OBJECT_SIZE, _height, rad, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif

#ifdef RELEASE_SERIALIZE_VIEW_HITBOX
	Box2DCapsuleData* capsuleData = 
		new Box2DCapsuleData(_offset,_diameter, halfHeight * DEFAULT_OBJECT_SIZE,_height,rad);
	renderData.emplace_back(capsuleData);
#endif

	m_shapeList.push_back(shape);
}

void Box2DBody::CreatePolygonShape(std::vector<b2Vec2> _pointList, bool _sensor)
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

#ifdef RELEASE_SERIALIZE_VIEW_HITBOX
	Box2DConvexMeshData* convexMeshData = new Box2DConvexMeshData(_pointList);
	renderData.emplace_back(convexMeshData);
#endif

	for (auto& point : _pointList)
	{
		point.x /= DEFAULT_OBJECT_SIZE;
		point.y /= DEFAULT_OBJECT_SIZE;
	}

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.isSensor = _sensor;
	shapeDef.filter.categoryBits = m_filter;
	shapeDef.filter.maskBits = Box2DBodyManager::GetMaskFilterBit(m_filter);

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

void Box2DBody::CreateSegment(std::vector<b2Vec2> _pointList, bool _sensor)
{
	int count = static_cast<int>(_pointList.size());

	if (count < 2)
	{
		LOG_ERROR("Not enough vertices in Segment");
		return;
	}


#ifdef DEBUG_TRUE
	size_t pointNum = _pointList.size();
	for (size_t i = 0; i < pointNum - 1; ++i)
	{
		size_t j = (i + 1) % pointNum;
		Vector2 start = { _pointList[i].x,_pointList[i].y };
		Vector2 end = { _pointList[j].x,_pointList[j].y };
		Vector2 dis = end - start;
		Vector2 center = start + dis / 2;
		float length = sqrt(dis.x * dis.x + dis.y * dis.y);
		float radian = Math::PointRadian(start.x, start.y, end.x, end.y);

		auto node = std::shared_ptr<RenderNode>(
			new Box2DLineRenderNode(length, radian, center, m_bodyId));
		node->m_object = m_this;
		m_nodeList.push_back((node));
		RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
	}
#endif

#ifdef RELEASE_SERIALIZE_VIEW_HITBOX
	size_t pointNum2 = _pointList.size();
	for (size_t i = 0; i < pointNum2; ++i)
	{
		size_t j = (i + 1) % pointNum2;
		Vector2 start = { _pointList[i].x,_pointList[i].y };
		Vector2 end = { _pointList[j].x,_pointList[j].y };
		Vector2 dis = end - start;
		Vector2 center = start + dis / 2;
		float length = sqrt(dis.x * dis.x + dis.y * dis.y);
		float radian = Math::PointRadian(start.x, start.y, end.x, end.y);

		Box2DLineData* meshData = new Box2DLineData(length,radian,center);
		renderData.emplace_back(meshData);
	}
#endif

	for (auto& point : _pointList)
	{
		point.x /= DEFAULT_OBJECT_SIZE;
		point.y /= DEFAULT_OBJECT_SIZE;
	}
	

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.isSensor = _sensor;
	shapeDef.filter.categoryBits = m_filter;
	shapeDef.filter.maskBits = Box2DBodyManager::GetMaskFilterBit(m_filter);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	for (int i = 0; i < count - 1; i++)
	{
		b2Segment segment1 = { _pointList[i], _pointList[i + 1] };
		auto shape = b2CreateSegmentShape(m_bodyId, &shapeDef, &segment1);
		m_shapeList.push_back(shape);
	}

	//ループ用
	//b2Segment segment1 = { _pointList[count - 1], _pointList[0] };
	//auto shape = b2CreateSegmentShape(m_bodyId, &shapeDef, &segment1);
	//m_shapeList.push_back(shape);

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
	size_t pointNum = _pointList.size();
	for (size_t i = 0; i < pointNum; ++i)
	{
		size_t j = (i + 1) % pointNum;
		Vector2 start = { _pointList[i].x,_pointList[i].y };
		Vector2 end = { _pointList[j].x,_pointList[j].y };
		Vector2 dis = end - start;
		Vector2 center = start + dis / 2;
		float length = sqrt(dis.x * dis.x + dis.y * dis.y);
		float radian = Math::PointRadian(start.x, start.y, end.x, end.y);

		auto node = std::shared_ptr<RenderNode>(
			new Box2DLineRenderNode(length, radian, center, m_bodyId));
		node->m_object = m_this;
		m_nodeList.push_back((node));
		RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
	}
#endif

#ifdef RELEASE_SERIALIZE_VIEW_HITBOX
	size_t pointNum2 = _pointList.size();
	for (size_t i = 0; i < pointNum2; ++i)
	{
		size_t j = (i + 1) % pointNum2;
		Vector2 start = { _pointList[i].x,_pointList[i].y };
		Vector2 end = { _pointList[j].x,_pointList[j].y };
		Vector2 dis = end - start;
		Vector2 center = start + dis / 2;
		float length = sqrt(dis.x * dis.x + dis.y * dis.y);
		float radian = Math::PointRadian(start.x, start.y, end.x, end.y);

		Box2DLineData* meshData = new Box2DLineData(length, radian, center);
		renderData.emplace_back(meshData);
	}
#endif

	for (auto& point : _pointList)
	{
		point.x /= DEFAULT_OBJECT_SIZE;
		point.y /= DEFAULT_OBJECT_SIZE;
	}

	b2ChainDef chainDef = b2DefaultChainDef();
	chainDef.filter.categoryBits = m_filter;
	chainDef.filter.maskBits = Box2DBodyManager::GetMaskFilterBit(m_filter);

	chainDef.points = _pointList.data();
	chainDef.count = static_cast<int32_t>(_pointList.size());
	chainDef.isLoop = true;

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2ChainId chainId = b2CreateChain(m_bodyId, &chainDef);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif

	int chainCount = b2Chain_GetSegmentCount(chainId);
	std::vector<b2ShapeId> shapeArray;
	shapeArray.resize(chainCount);
	int num = b2Chain_GetSegments(chainId, shapeArray.data(), chainCount);
	for (auto& shape : shapeArray)
	{
		m_shapeList.push_back(shape);
	}
}

void Box2DBody::ClearShape()
{
#ifdef DEBUG_TRUE
	for (auto& node : m_nodeList)
	{
		node->Delete(LAYER_BOX2D_DEBUG);
	}
	m_nodeList.clear();
#endif

	for (auto& shape : m_shapeList)
	{
		b2DestroyShape(shape);
	}
	m_shapeList.clear();
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

const Vector2 Box2DBody::GetPosition() const
{
	b2Vec2 pos = b2Body_GetPosition(m_bodyId);
	return { pos.x * DEFAULT_OBJECT_SIZE,pos.y * DEFAULT_OBJECT_SIZE };
}

void Box2DBody::SetAngle(float _deg)
{
	Angle angle(_deg);
	SetAngle(angle);
}

void Box2DBody::SetAngle(double _rad)
{
	Angle angle(_rad);
	SetAngle(angle);
}

void Box2DBody::SetAngle(Angle _angle)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	b2BodyId id = m_bodyId;
	Box2D::WorldManager::AddWorldTask(std::move([id, _angle]()
		{
			b2Body_SetTransform(id, b2Body_GetPosition(id), b2MakeRot(_angle));
		})
	);
#else
	b2Body_SetTransform(m_bodyId, b2Body_GetPosition(m_bodyId), b2MakeRot(static_cast<float>(_angle.Get())));
#endif
}

void Box2DBody::SetVelocity(b2Vec2 _velocity)
{
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

void Box2DBody::AddForceImpulse(b2Vec2 _force)
{
	_force.x *= DEFAULT_OBJECT_SIZE;
	_force.y *= DEFAULT_OBJECT_SIZE;
	b2Body_ApplyLinearImpulseToCenter(m_bodyId, _force, true);
}

void Box2DBody::SetType(b2BodyType _type)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	b2BodyId id = m_bodyId;
	Box2D::WorldManager::AddWorldTask(std::move([id, _type]()
		{
			b2Body_SetType(id, _type);
		})
	);
#else
	b2Body_SetType(m_bodyId, _type);
#endif
}

b2BodyType Box2DBody::GetType()
{
	return b2Body_GetType(m_bodyId);
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

float Box2DBody::GetMass()
{
	return b2Body_GetMass(m_bodyId);
}

void Box2DBody::SetMass(float _mass)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	b2BodyId id = m_bodyId;
	Box2D::WorldManager::AddWorldTask(std::move([id, _mass]()
		{
			b2MassData massData;
			massData.mass = _mass;
			b2Body_SetMassData(id, massData);
		})
	);
#else
	b2MassData massData;
	massData.mass = _mass;
	b2Body_SetMassData(m_bodyId, massData);
#endif
}

void Box2DBody::SetBullet(bool _isBullet)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
	b2BodyId id = m_bodyId;
	Box2D::WorldManager::AddWorldTask(std::move([id, _isBullet]()
		{
			b2Body_SetBullet(id, _isBullet);
		})
	);
#else
	b2Body_SetBullet(m_bodyId, _isBullet);
#endif
}

bool Box2DBody::IsBullet()
{
	return b2Body_IsBullet(m_bodyId);
}

void Box2DBody::SetRestitution(float _restitution)
{
	for (auto shape : m_shapeList)
	{
		b2Shape_SetRestitution(shape, _restitution);
	}
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



void Box2DBody::GetOverlapObject(std::vector<GameObject*>& _objects)
{
	const b2Transform b2tf = b2Body_GetTransform(m_bodyId);
	GetOverlapObject(_objects, m_filter, b2tf);
}

void Box2DBody::GetOverlapObject(std::vector<GameObject*>& _objects, FILTER _filter)
{
	const b2Transform b2tf = b2Body_GetTransform(m_bodyId);
	GetOverlapObject(_objects, _filter, b2tf);
}

void Box2DBody::GetOverlapObject(std::vector<GameObject*>& _objects, FILTER _filter,b2Transform _tf)
{
	std::vector<b2ShapeId> shpeIds;
	b2QueryFilter filter;
	filter.categoryBits = _filter;
	filter.maskBits = Box2DBodyManager::GetMaskFilterBit(_filter);
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
			b2World_OverlapPolygon(*Box2D::WorldManager::currentWorldId, &polygon, _tf, filter,
				Box2D::OverlapResultVectorb2ShapeId, &shpeIds);
		}
		break;
		case b2_circleShape:
		{
			auto circle = b2Shape_GetCircle(shpeId);
			b2World_OverlapCircle(*Box2D::WorldManager::currentWorldId, &circle, _tf, filter,
				Box2D::OverlapResultVectorb2ShapeId, &shpeIds);
		}
		break;
		case b2_capsuleShape:
		{
			auto causule = b2Shape_GetCapsule(shpeId);
			b2World_OverlapCapsule(*Box2D::WorldManager::currentWorldId, &causule, _tf, filter,
				Box2D::OverlapResultVectorb2ShapeId, &shpeIds);
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
	filter.maskBits = Box2DBodyManager::GetMaskFilterBit(_filter);
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
			b2World_OverlapPolygon(*Box2D::WorldManager::currentWorldId, &polygon, _tf, filter, 
				Box2D::OverlapResultVectorb2ShapeId, &shpeIds);
		}
		break;
		case b2_circleShape:
		{
			auto circle = b2Shape_GetCircle(shpeId);
			b2World_OverlapCircle(*Box2D::WorldManager::currentWorldId, &circle, _tf, filter,
				Box2D::OverlapResultVectorb2ShapeId, &shpeIds);
		}
		break;
		case b2_capsuleShape:
		{
			auto causule = b2Shape_GetCapsule(shpeId);
			b2World_OverlapCapsule(*Box2D::WorldManager::currentWorldId, &causule, _tf, filter, 
				Box2D::OverlapResultVectorb2ShapeId, &shpeIds);
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

#pragma region Box2DBodyChain

//Box2DBodyChain::Box2DBodyChain(GameObject* _object)
//{
//	auto& position = _object->transform.position;
//	//ボディ定義とワールドIDを使ってグラウンド・ボディを作成する
//	b2BodyDef bodyDef = b2DefaultBodyDef();
//	bodyDef.type = b2_dynamicBody;
//	bodyDef.position = { position.x / DEFAULT_OBJECT_SIZE, position.y / DEFAULT_OBJECT_SIZE };
//	bodyDef.rotation = b2MakeRot(static_cast<float>(_object->transform.angle.z.Get()));
//
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	Box2D::WorldManager::pPauseWorldUpdate();
//#endif
//	Box2D::WorldManager::GenerateBody(m_bodyId, &bodyDef);
//
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	Box2D::WorldManager::pResumeWorldUpdate();
//#endif
//
//	Box2DBodyManager::m_bodyObjectName.insert(std::pair(m_bodyId.index1, _object->GetName()));
//}
//
//Box2DBodyChain::Box2DBodyChain(GameObject* _object, b2BodyDef* _bodyDef)
//{
//	auto& position = _object->transform.position;
//	_bodyDef->position = { position.x / DEFAULT_OBJECT_SIZE, position.y / DEFAULT_OBJECT_SIZE };
//
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	Box2D::WorldManager::pPauseWorldUpdate();
//#endif
//	Box2D::WorldManager::GenerateBody(m_bodyId, _bodyDef);
//
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	Box2D::WorldManager::pResumeWorldUpdate();
//#endif
//
//	Box2DBodyManager::m_bodyObjectName.insert(std::pair(m_bodyId.index1, _object->GetName()));
//}
//
//Box2DBodyChain::Box2DBodyChain(GameObject* _object, SERIALIZE_INPUT& ar)
//{
//
//}
//
//inline void Box2DBodyChain::Update()
//{
//	{
//		auto pos = b2Body_GetPosition(m_bodyId);
//		pos *= DEFAULT_OBJECT_SIZE;
//		m_this->transform.position = Vector3(pos.x, pos.y, 0.5f);
//		auto rot = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
//		m_this->transform.angle.z.Set(rot);
//	};
//}
//
//
//#ifdef DEBUG_TRUE
//void Box2DBodyChain::Delete()
//{
//	for (auto& node : m_nodeList)
//	{
//		node->Delete(LAYER_BOX2D_DEBUG);
//	}
//	auto& list = Box2DBodyManager::m_bodyObjectName;
//	auto it = list.find(m_bodyId.index1);
//	if (it != list.end()) {
//		list.erase(it);
//	}
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	Box2D::WorldManager::pPauseWorldUpdate();
//#endif
//	b2DestroyBody(m_bodyId);
//
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	Box2D::WorldManager::pResumeWorldUpdate();
//#endif
//}
//
//#else
//void Box2DBodyChain::Delete()
//{
//	auto& list = Box2DBodyManager::m_bodyObjectName;
//	auto it = list.find(m_bodyId.index1);
//	if (it != list.end()) {
//		list.erase(it);
//	}
//	b2DestroyBody(m_bodyId);
//}
//#endif
//
//void Box2DBodyChain::SetActive(bool _active)
//{
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	Box2D::WorldManager::pPauseWorldUpdate();
//#endif
//	_active ? b2Body_Enable(m_bodyId) : b2Body_Disable(m_bodyId);
//
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	Box2D::WorldManager::pResumeWorldUpdate();
//#endif
//
//#ifdef DEBUG_TRUE
//	for (auto& node : m_nodeList)
//	{
//		node->Active(_active);
//	}
//#endif
//}
//
//void Box2DBodyChain::Serialize(SERIALIZE_OUTPUT& ar)
//{
//	BodySaveData data;
//	data.type = b2Body_GetType(m_bodyId);
//	data.lineVec = b2Body_GetLinearVelocity(m_bodyId);
//	data.angleVec = b2Body_GetAngularVelocity(m_bodyId);
//	data.gravity = b2Body_GetGravityScale(m_bodyId);
//	data.mass = b2Body_GetAutomaticMass(m_bodyId);
//	data.bullet = b2Body_IsBullet(m_bodyId);
//	data.fixRot = b2Body_IsFixedRotation(m_bodyId);
//	data.awake = b2Body_IsAwake(m_bodyId);
//
//	ar(CEREAL_NVP(data),CEREAL_NVP(m_chainVertexList));
//}
//
//void Box2DBodyChain::DrawImGui(ImGuiApp::HandleUI& _handle)
//{
//#ifdef DEBUG_TRUE
//	if (ImGui::Button("<>##type"))
//	{
//		ImGui::OpenPopup("changeType");
//	}
//	ImGui::SetItemTooltip("change type");
//	ImGui::SameLine();
//
//	b2BodyType bodyType = GetType();
//	ImGui::Text("type : %s", magic_enum::enum_name(bodyType).data());
//
//	if (ImGui::BeginPopup("changeType"))
//	{
//		for (int i = 0; i < b2_bodyTypeCount; i++)
//		{
//			b2BodyType type = (b2BodyType)i;
//			bool same = type == bodyType;
//			if (ImGui::Selectable(magic_enum::enum_name(type).data(), same))
//			{
//				if (!same)SetType(type);
//			}
//		}
//		ImGui::EndPopup();
//	}
//
//	auto vec = b2Body_GetLinearVelocity(m_bodyId);
//	float velocity[2] = { vec.x,vec.y };
//	//ImGui::Text(" LinerVelocity\n  <x: %.2f   y:%.2f>", vec.x, vec.y);
//	if (ImGui::InputFloat2("Velocity", velocity))
//	{
//		b2Body_SetLinearVelocity(m_bodyId, { velocity[0],velocity[1] });
//	}
//
//	auto massData = b2Body_GetMassData(m_bodyId);
//	if (ImGui::InputFloat("Mass", &massData.mass))
//	{
//		b2Body_SetMassData(m_bodyId, massData);
//	}
//
//	float gravity = b2Body_GetGravityScale(m_bodyId);
//	if (ImGui::InputFloat(" Gravity", &gravity))
//	{
//		b2Body_SetGravityScale(m_bodyId, gravity);
//	}
//
//	bool isBullet = b2Body_IsBullet(m_bodyId);
//	if (ImGui::Checkbox("Bullet", &isBullet))
//	{
//		b2Body_SetBullet(m_bodyId, isBullet);
//	}
//
//	bool isFixRot = b2Body_IsFixedRotation(m_bodyId);
//	if (ImGui::Checkbox("FixedRotation", &isFixRot))
//	{
//		b2Body_SetFixedRotation(m_bodyId, isFixRot);
//	}
//
//	bool isAwake = b2Body_IsAwake(m_bodyId);
//	if (ImGui::Checkbox("Awake", &isAwake))
//	{
//		b2Body_SetAwake(m_bodyId, isAwake);
//	}
//#endif
//}
//
//
//void Box2DBodyChain::CreateChain(std::vector<b2Vec2>& _pointList, FILTER _filter)
//{
//	if (_pointList.size() < 4)
//	{
//		LOG_ERROR("Not enough vertices in Chain");
//		return;
//	}
//
//#ifdef DEBUG_TRUE
//	auto node = std::shared_ptr<RenderNode>(
//		new Box2DMeshRenderNode(_pointList, m_bodyId, true));
//	node->m_object = m_this;
//	m_nodeList.push_back((node));
//	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
//#endif
//
//	for (auto& point : _pointList)
//	{
//		point.x /= DEFAULT_OBJECT_SIZE;
//		point.y /= DEFAULT_OBJECT_SIZE;
//	}
//
//	b2ChainDef chainDef = b2DefaultChainDef();
//	chainDef.filter.categoryBits = _filter;
//	chainDef.filter.maskBits = Box2DBodyManager::GetMaskFilterBit(_filter);
//
//	chainDef.points = _pointList.data();
//	chainDef.count = static_cast<int32_t>(_pointList.size());
//	chainDef.isLoop = true;
//
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	Box2D::WorldManager::pPauseWorldUpdate();
//#endif 
//	auto id = b2CreateChain(m_bodyId, &chainDef);
//
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	Box2D::WorldManager::pResumeWorldUpdate();
//#endif
//
//	m_chainList.push_back(id);
//
//	m_chainVertexList.push_back(std::move(_pointList));
//}
//
//void Box2DBodyChain::SetPosition(Vector2 _pos)
//{
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	b2BodyId id = m_bodyId;
//	Box2D::WorldManager::AddWorldTask(std::move([id, _pos]()
//		{
//			b2Vec2 pos = { _pos.x / DEFAULT_OBJECT_SIZE,_pos.y / DEFAULT_OBJECT_SIZE };
//			b2Body_SetTransform(id, pos, b2Body_GetRotation(id));
//		})
//	);
//#else
//	b2Vec2 pos = { _pos.x / DEFAULT_OBJECT_SIZE,_pos.y / DEFAULT_OBJECT_SIZE };
//	b2Body_SetTransform(m_bodyId, pos, b2Body_GetRotation(m_bodyId));
//#endif
//}
//
//
//void Box2DBodyChain::SetAngle(float _deg)
//{
//	Angle angle(_deg);
//	SetAngle(angle);
//}
//
//void Box2DBodyChain::SetAngle(double _rad)
//{
//	Angle angle(_rad);
//	SetAngle(angle);
//}
//
//void Box2DBodyChain::SetAngle(Angle _angle)
//{
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	b2BodyId id = m_bodyId;
//	Box2D::WorldManager::AddWorldTask(std::move([id, _angle]()
//		{
//			b2Body_SetTransform(id, b2Body_GetPosition(id), b2MakeRot(_angle));
//		})
//	);
//#else
//	b2Body_SetTransform(m_bodyId, b2Body_GetPosition(m_bodyId), b2MakeRot(static_cast<float>(_angle.Get())));
//#endif
//}
//
//
//void Box2DBodyChain::SetVelocity(b2Vec2 _velocity)
//{
//	b2Body_SetLinearVelocity(m_bodyId, _velocity);
//}
//
//void Box2DBodyChain::SetVelocityX(float _velocityX)
//{
//	b2Vec2 vec = b2Body_GetLinearVelocity(m_bodyId);
//	vec.x = _velocityX;
//	b2Body_SetLinearVelocity(m_bodyId, vec);
//}
//
//void Box2DBodyChain::SetVelocityY(float _velocityY)
//{
//	b2Vec2 vec = b2Body_GetLinearVelocity(m_bodyId);
//	vec.y = _velocityY;
//	b2Body_SetLinearVelocity(m_bodyId, vec);
//}
//
//const b2Vec2 Box2DBodyChain::GetVelocity() const
//{
//	return b2Body_GetLinearVelocity(m_bodyId);
//}
//
//void Box2DBodyChain::AddForce(b2Vec2 _force)
//{
//	_force.x *= DEFAULT_OBJECT_SIZE;
//	_force.y *= DEFAULT_OBJECT_SIZE;
//	b2Body_ApplyForceToCenter(m_bodyId, _force, true);
//}
//
//void Box2DBodyChain::AddForceImpulse(b2Vec2 _force)
//{
//	_force.x *= DEFAULT_OBJECT_SIZE;
//	_force.y *= DEFAULT_OBJECT_SIZE;
//	b2Body_ApplyLinearImpulseToCenter(m_bodyId, _force, true);
//}
//
//void Box2DBodyChain::SetType(b2BodyType _type)
//{
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	b2BodyId id = m_bodyId;
//	Box2D::WorldManager::AddWorldTask(std::move([id, _type]()
//		{
//			b2Body_SetType(id, _type);
//		})
//	);
//#else
//	b2Body_SetType(m_bodyId, _type);
//#endif
//}
//
//b2BodyType Box2DBodyChain::GetType()
//{
//	return b2Body_GetType(m_bodyId);
//}
//
//float  Box2DBodyChain::GetGravityScale() const
//{
//	return b2Body_GetGravityScale(m_bodyId);
//}
//
//void Box2DBodyChain::SetGravityScale(float _scale)
//{
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	b2BodyId id = m_bodyId;
//	Box2D::WorldManager::AddWorldTask(std::move([id, _scale]()
//		{
//			b2Body_SetGravityScale(id, _scale);
//		})
//	);
//#else
//	b2Body_SetGravityScale(m_bodyId, _scale);
//#endif
//}
//
//float Box2DBodyChain::GetMass()
//{
//	return b2Body_GetMass(m_bodyId);
//}
//
//void Box2DBodyChain::SetMass(float _mass)
//{
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	b2BodyId id = m_bodyId;
//	Box2D::WorldManager::AddWorldTask(std::move([id, _mass]()
//		{
//			b2MassData massData;
//			massData.mass = _mass;
//			b2Body_SetMassData(id, massData);
//		})
//	);
//#else
//	b2MassData massData;
//	massData.mass = _mass;
//	b2Body_SetMassData(m_bodyId, massData);
//#endif
//}
//
//void Box2DBodyChain::SetBullet(bool _isBullet)
//{
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	b2BodyId id = m_bodyId;
//	Box2D::WorldManager::AddWorldTask(std::move([id, _isBullet]()
//		{
//			b2Body_SetBullet(id, _isBullet);
//		})
//	);
//#else
//	b2Body_SetBullet(m_bodyId, _isBullet);
//#endif
//}
//
//bool Box2DBodyChain::IsBullet()
//{
//	return b2Body_IsBullet(m_bodyId);
//}
//
//
//void Box2DBodyChain::SetRestitution(float _restitution)
//{
//	for (auto& id : m_chainList)
//	{
//		b2Chain_SetRestitution(id,_restitution);
//	}
//}
//
//void Box2DBodyChain::SetFixedRotation(bool _flag)
//{
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	b2BodyId id = m_bodyId;
//	Box2D::WorldManager::AddWorldTask(std::move([id, _flag]()
//		{
//			b2Body_SetFixedRotation(id, _flag);
//		})
//	);
//#else
//	b2Body_SetFixedRotation(m_bodyId, _flag);
//#endif
//}
//
//void Box2DBodyChain::SetAwake(bool _awake)
//{
//#ifdef BOX2D_UPDATE_MULTITHREAD
//	b2BodyId id = m_bodyId;
//	Box2D::WorldManager::AddWorldTask(std::move([id, _awake]()
//		{
//			b2Body_SetAwake(id, _awake);
//		})
//	);
//#else
//	b2Body_SetAwake(m_bodyId, _awake);
//#endif
//}

#pragma endregion

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

	DirectX11::m_pDevice->CreateBuffer(&ibDesc, &irData, RenderManager::m_lineBoxIndexBuffer.GetAddressOf());

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


unsigned int Box2DBodyManager::GetMaskFilterBit(FILTER _filter)
{
	auto iter = m_layerFilterBit.find(_filter);
	if (iter != m_layerFilterBit.end())
	{
		return iter->second;
	}

	m_layerFilterBit.insert(std::make_pair(_filter, ALL_BITS));
	return m_layerFilterBit.find(_filter)->second;
}


#ifdef RELEASE_SERIALIZE_VIEW_HITBOX

RenderNode* Box2DBoxData::CreateRenderNode(b2BodyId bodyId)
{
	RenderNode* node = new Box2DBoxRenderNode(m_offset, m_size, m_angle, bodyId);
	return node;
};

RenderNode* Box2DCircleData::CreateRenderNode(b2BodyId bodyId)
{
	RenderNode* node = new Box2DCircleRenderNode(m_offset, m_size, bodyId);
	return node;
}

RenderNode* Box2DCapsuleData::CreateRenderNode(b2BodyId bodyId)
{
	RenderNode* node = new Box2DCapsuleRenderNode(m_offset, m_diameter, m_sizeY, m_height, m_angle, bodyId);
	return node;
}

RenderNode* Box2DMeshData::CreateRenderNode(b2BodyId bodyId)
{
	RenderNode* node = new Box2DMeshRenderNode(m_pointList, bodyId, m_loop);
	return node;
}

RenderNode* Box2DConvexMeshData::CreateRenderNode(b2BodyId bodyId)
{
	RenderNode* node = new Box2DConvexMeshRenderNode(m_pointList, bodyId);
	return node;
}

RenderNode* Box2DLineData::CreateRenderNode(b2BodyId bodyId)
{
	RenderNode* node = new Box2DLineRenderNode(m_length, m_radian, m_center, bodyId);
	return node;
}

#endif

#ifdef DEBUG_TRUE

void Box2DBoxRenderNode::Draw()
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_lineBoxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	static VSObjectConstantBuffer cb;

	const auto& transform = m_object->transform;
	const auto& objectCb = m_object->GetConstantBuffer();

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

	switch (m_object->isSelected)
	{
	case GameObject::SELECT_NONE:
		SetDebugBodyColor(m_bodyId, cb.color);
		break;
	case GameObject::SELECTED:
		cb.color = Box2D::b2_colorSelected;
		break;
	case GameObject::ON_MOUSE:
		cb.color = Box2D::b2_colorOnMouse;
		break;
	}

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
	const auto& objectCb = m_object->GetConstantBuffer();


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

	switch (m_object->isSelected)
	{
	case GameObject::SELECT_NONE:
		SetDebugBodyColor(m_bodyId, cb.color);
		break;
	case GameObject::SELECTED:
		cb.color = Box2D::b2_colorSelected;
		break;
	case GameObject::ON_MOUSE:
		cb.color = Box2D::b2_colorOnMouse;
		break;
	}

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
	const auto& objectCb = m_object->GetConstantBuffer();

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

	switch (m_object->isSelected)
	{
	case GameObject::SELECT_NONE:
		SetDebugBodyColor(m_bodyId, cb.color);
		break;
	case GameObject::SELECTED:
		cb.color = Box2D::b2_colorSelected;
		break;
	case GameObject::ON_MOUSE:
		cb.color = Box2D::b2_colorOnMouse;
		break;
	}

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
	DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_lineBoxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	//胴体のBox描画
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
	//const auto& objectCb = m_object->GetConstantBuffer();

	auto rad = static_cast<float>(transform.angle.z.Get());

	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(1.0f, 1.0f, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(rad);
	cb.world *= DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	/*cb.view = objectCb.view;
	cb.projection = objectCb.projection;*/

	switch (m_object->isSelected)
	{
	case GameObject::SELECT_NONE:
		SetDebugBodyColor(m_bodyId, cb.color);
		break;
	case GameObject::SELECTED:
		cb.color = Box2D::b2_colorSelected;
		break;
	case GameObject::ON_MOUSE:
		cb.color = Box2D::b2_colorOnMouse;
		break;
	}

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

inline void Box2DLineRenderNode::Draw()
{
	static VSObjectConstantBuffer cb;

	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	//rayの描画
	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_lineVertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_lineIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	const float rad = (float)m_object->transform.angle.z.Get();
	auto& objectPos = m_object->transform.position;
	Vector2 pos = 
	{
		m_center.x * cosf(rad) - m_center.y * sinf(rad),
		m_center.x * sinf(rad) + m_center.y * cosf(rad),
	};

	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_length, 1.0f, 1.0f);
	cb.world *= DirectX::XMMatrixRotationZ(m_radian + rad);
	cb.world *= DirectX::XMMatrixTranslation(objectPos.x + pos.x, objectPos.y + pos.y, 0.5f);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	
	switch (m_object->isSelected)
	{
	case GameObject::SELECT_NONE:
		SetDebugBodyColor(m_bodyId, cb.color);
		break;
	case GameObject::SELECTED:
		cb.color = Box2D::b2_colorSelected;
		break;
	case GameObject::ON_MOUSE:
		cb.color = Box2D::b2_colorOnMouse;
		break;
	}

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(2, 0, 0);

	//次のポインタにつなぐ
	NextFunc();
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
