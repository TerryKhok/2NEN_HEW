#pragma once

class RunFixedRoute : public Component
{
	SAFE_POINTER(Box2DBody, rb)

	void Start() override
	{
		if(m_this->TryGetComponent<Box2DBody>(&rb))
		{
			switch (rb->GetType())
			{
			case b2_staticBody:
				pRunFunc = &RunFixedRoute::RunWithStaticBody;
				break;
			case b2_dynamicBody:
				pRunFunc = &RunFixedRoute::RunWithDynamicBody;
				break;
			case b2_kinematicBody:
				pRunFunc = &RunFixedRoute::RunWithKinematicBody;
				break;
			}
		}
	}

	void Run(Vector2 _targetPos)
	{
		m_this->transform.position = _targetPos;
	}
	void RunWithStaticBody(Vector2 _targetPos)
	{
		rb->SetPosition(_targetPos);
	}
	void RunWithDynamicBody(Vector2 _targetPos)
	{
		Vector2 vec = _targetPos - m_this->transform.position;
		rb->AddForce({ vec.x ,vec.y });
	}
	void RunWithKinematicBody(Vector2 _targetPos)
	{
		Vector2 vec = _targetPos - m_this->transform.position;
		rb->SetVelocity({ vec.x ,vec.y });
	}

	void(RunFixedRoute::* pRunFunc)(Vector2) = &RunFixedRoute::Run;

	float SmoothRotateRad(float current, float target, float& velocity, float smoothTime, float deltaTime) {
		float omega = 2.0f / smoothTime;
		float x = omega * deltaTime;
		float expFactor = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

		float difference = fmod(target - current + Math::PI, 2 * Math::PI) - Math::PI; // -π ～ π に正規化
		float temp = (velocity + omega * difference) * deltaTime;
		velocity = (velocity - omega * temp) * expFactor;

		return current + (difference + temp) * expFactor;
	}

	// ラジアン角度を一定速度で補間し、オフセットを適用
	float LerpAngleRad(float current, float target) {
		float offsetTarget = target + offsetAngle; // 目標角度にオフセットを適用
		float difference = fmod(offsetTarget - current + Math::PI, 2 * Math::PI) - Math::PI; // -π ～ π の範囲に正規化
		float step = turnSpeed * deltaTime;

		// 目標角度を超えないように調整
		if (std::fabs(difference) < step)
			return offsetTarget;

		return current + (difference > 0 ? step : -step);
	}

	float deltaTime = 1.0f / UPDATE_FPS; // 16ms (60FPS)

	void Update() override
	{
		int pointNum = (int)routePoints.size();
		int nextIndex = (currentIndex + (reverse ? pointNum - 1 : 1)) % pointNum;

		auto& goalTime = reverse ? pointTimes[nextIndex] : pointTimes[currentIndex];
		float t = time / goalTime;

		Vector2 targetPos;
		targetPos.Lerp(t, routePoints[currentIndex], routePoints[nextIndex]);

		(this->*pRunFunc)(targetPos);

		if (rotation)
		{
			//Vector2 vec = targetPos - m_this->transform.position;
			Vector2 vec = routePoints[nextIndex] - routePoints[currentIndex];
			float targetRadZ = std::atan2(vec.y, vec.x) + Math::PI;
			auto& angelZ = m_this->transform.angle.z;
			float rad = LerpAngleRad((float)angelZ.Get(), targetRadZ);
			if (Math::RadToDeg(rad) >= 359.0f) rad = 0.0f;
			angelZ.Set(rad);
		}

		time += 1.0f / UPDATE_FPS;

		if (time >= goalTime)
		{
			time = 0.0f;
			currentIndex = nextIndex;
		}
	}

	void CalculationPointTime()
	{
		float totalDistance = 0.0f;
		std::vector<float> distances;
		int pointNum = (int)routePoints.size();
		for (int i = 0; i < pointNum; i++)
		{
			int nextIndex = (i + 1) % pointNum;
			float dis = routePoints[i].Distance(routePoints[nextIndex]);
			distances.push_back(dis);
			totalDistance += dis;
		}

		pointTimes.clear();
		for (auto& dis : distances)
		{
			pointTimes.push_back(aroundTime * (dis / totalDistance));
		}
	}

public:
	void SetRoutePoint(std::vector<Vector2>&& _points)
	{
		routePoints = _points;
		CalculationPointTime();
	}
private:

#ifdef DEBUG_TRUE
	void DrawImGui(ImGuiApp::HandleUI& _handle) override
	{
		if (ImGui::InputFloat("AroundTime##FixedRouteTime",&aroundTime))
		{
			CalculationPointTime();
		}
		ImGui::Checkbox("Reverse##FixedRouteTime", &reverse);
		ImGui::Checkbox("Rotation##FixedRouteTime", &rotation);
		if (rotation)
		{
			ImGui::InputFloat("turnTime##FixedRouteTime", &turnSpeed);

			float angle = Math::RadToDeg(offsetAngle);
			if (ImGui::InputFloat("offsetAngle##FixedRouteTime", &angle))
			{
				offsetAngle = Math::DegToRad(angle);
			}
		}

		bool edit = _handle.DrawLockButton("FixedRoutePoint");
		/*if (ImGui::Checkbox("EditLock##fixedRoutePoint", &edit))
		{
			_handle.LockHandle(edit, "FixedRoutePoint");
		}*/

		if (edit)
		{
			if(ImGui::Button("AddPoint##RunFixedRoute"))
			{
				routePoints.emplace_back(0.0f, 0.0f);
				pointTimes.push_back(2.5f);
				CalculationPointTime();
			}

			ImGui::SeparatorText("Points");

			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::BeginChild("ClipListChild", ImVec2(300, 100), ImGuiChildFlags_Borders);
			{
				int i = 0;
				for (auto iter = routePoints.begin(); iter != routePoints.end();)
				{
					ImGui::PushID(i);
					if (routePoints.size() > 2)
					{
						if (ImGui::Button("erase##fixedRoutePoint"))
						{
							iter = routePoints.erase(iter);
							if (i >= currentIndex)
							{
								currentIndex = (int)routePoints.size() - 1;
							}
							ImGui::PopID();
							CalculationPointTime();
							continue;
						}
						ImGui::SameLine();
					}
					
					ImGui::InputFloat2("point", iter->data(), "%.1f");
					ImGui::PopID();

					i++;
					iter++;
				}

				ImGui::EndChild();
			}
			ImGui::PopStyleVar();

			static int selectIndex = -1;
			Vector2 worldPos = Input::Get().MousePoint();
			worldPos.x = worldPos.x * DISPALY_ASPECT_WIDTH / RenderManager::renderZoom.x + RenderManager::renderOffset.x;
			worldPos.y = worldPos.y * DISPALY_ASPECT_HEIGHT / RenderManager::renderZoom.y + RenderManager::renderOffset.y;

			int pointNum = (int)routePoints.size();
			if (selectIndex < 0)
			{
				if (Input::Get().MouseLeftTrigger())
				{
					for (int i = 0; i < pointNum; i++)
					{
						const Vector2& pos = routePoints[i];
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
				routePoints[selectIndex] = worldPos;

				if (Input::Get().MouseLeftRelease())
				{
					CalculationPointTime();
					selectIndex = -1;
				}
			}

			const float colorSeg = 1.0f / pointNum;
			for (int i = 0; i < pointNum; i++)
			{
				int nextIndex = (i + 1) % pointNum;
				auto& start = routePoints[i];
				auto& end = routePoints[nextIndex];
				RenderManager::DrawRayNode rayNode;
				Vector2 dis = end - start;
				rayNode.center = start + dis / 2;
				rayNode.length = sqrt(dis.x * dis.x + dis.y * dis.y);
				rayNode.radian = Math::PointRadian(start.x, start.y, end.x, end.y);
				rayNode.color = XMFLOAT4(0.6f, 0.6f, 1.0f, 1.0f);

				RenderManager::AddDrawRayNode(std::move(rayNode));

				RenderManager::DrawBoxNode boxNode;
				boxNode.center = routePoints[i];
				boxNode.size = { 1.0f / RenderManager::renderZoom.x,1.0f / RenderManager::renderZoom.y };
				if (selectIndex == i)
					boxNode.color = XMFLOAT4(1.0f, colorSeg * i, 1.0f, 1.0f);
				else
					boxNode.color = XMFLOAT4(0.6f, colorSeg * i, 0.6f, 1.0f);

				RenderManager::AddDrawBoxNode(std::move(boxNode));
			}
		}
	}
#endif
	
private:
	std::vector<Vector2> routePoints = { {-100.0f,0.0f},{100.0f,0.0f} };
	std::vector<float> pointTimes = { 2.5f,2.5f };
	int currentIndex = 0;
	float aroundTime = 5.0f;
	float time = 0.0f;
	bool reverse = true;

	bool rotation = false;
	float turnSpeed = Math::PI;
	float offsetAngle = 0.0f;

	void Serialize(SERIALIZE_OUTPUT& ar) override {
		ar(CEREAL_NVP(routePoints), CEREAL_NVP(pointTimes), CEREAL_NVP(currentIndex),
			CEREAL_NVP(aroundTime), CEREAL_NVP(time), CEREAL_NVP(reverse),
			CEREAL_NVP(rotation), CEREAL_NVP(turnSpeed), CEREAL_NVP(offsetAngle));
	} 
	
	void Deserialize(SERIALIZE_INPUT& ar) override {
		ar(CEREAL_NVP(routePoints), CEREAL_NVP(pointTimes), CEREAL_NVP(currentIndex),
			CEREAL_NVP(aroundTime), CEREAL_NVP(time), CEREAL_NVP(reverse),
			CEREAL_NVP(rotation), CEREAL_NVP(turnSpeed), CEREAL_NVP(offsetAngle));
	}
};

SetReflectionComponent(RunFixedRoute)