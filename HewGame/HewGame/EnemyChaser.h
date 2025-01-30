#pragma once

class EnemyChaser : public Component
{
	SAFE_POINTER(Box2DBody, rb)

	void Start() override
	{
		if (m_this->TryGetComponent<Box2DBody>(&rb))
		{
			switch (rb->GetType())
			{
			case b2_staticBody:
				pRunFunc = &EnemyChaser::RunWithStaticBody;
				break;
			case b2_dynamicBody:
				pRunFunc = &EnemyChaser::RunWithDynamicBody;
				break;
			case b2_kinematicBody:
				pRunFunc = &EnemyChaser::RunWithKinematicBody;
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

	void(EnemyChaser::* pRunFunc)(Vector2) = &EnemyChaser::Run;

	bool lost = false;

	void Update() override
	{
		auto target = ObjectManager::Find(targetName);
		if (target == nullptr) return;

		Vector2 rayStart = m_this->transform.position;
		Vector2 rayEnd = target->transform.position;

		float dis = rayStart.Distance(rayEnd);
		if (dis > chaseRange)
		{
			if (!lost) rb->SetVelocity({ 0.0f,0.0f });
			lost = true;
			return;
		}

		lost = false;

		if (Box2D::WorldManager::RayCastShape(rayStart, rayEnd, rb, F_MAPRAY)) 
		{
			int targetIndex = -1;
			float targetMinDist = chaseRange;
			int closeIndex = -1;
			float minDist = chaseRange;
			int pointNum = (int)routePoints.size();
			//自身から一番近いポイントと対象に一番近いポイントを詮索する
			for (int i = 0; i < pointNum; i++)
			{
				auto& point = routePoints[i];
				float dist = rayStart.Distance(point);
				float targetDist = rayEnd.Distance(point);
				if (dist < minDist)
				{
					minDist = dist;
					closeIndex = i;
				}
				if (targetDist < targetMinDist)
				{
					targetMinDist = targetDist;
					targetIndex = i;
				}
			}

			if (closeIndex < 0 || targetIndex < 0) return;

			int plusDir = (targetIndex - closeIndex + pointNum) % pointNum;
			int minusDir = (closeIndex - targetIndex + pointNum) % pointNum;
			LOG("%d,%d,%d", targetIndex,plusDir, minusDir);

			int nextIndex = closeIndex;
			if (plusDir <= minusDir)
			{
				nextIndex = (nextIndex + 1) % pointNum;
			}
			else
			{
				nextIndex = (nextIndex + pointNum - 1) % pointNum;
			}

			Vector2 vec = routePoints[nextIndex] - rayStart;
			vec.Normalize();
			vec *= chaseSpeed;
			rb->SetVelocity({ vec.x,vec.y });
		}
		else
		{
			Vector2 vec = rayEnd - rayStart;
			vec.Normalize();
			vec *= chaseSpeed;
			rb->SetVelocity({ vec.x,vec.y });
		}
	}

public:
	void SetRoutePoint(std::vector<Vector2>&& _points)
	{
		routePoints = _points;
	}
private:

#ifdef DEBUG_TRUE
	void DrawImGui(ImGuiApp::HandleUI& _handle) override
	{
		static char str[128] = {};

		memset(str, '\0', sizeof(str) / sizeof(char));
		memcpy(str, targetName.c_str(), targetName.size());
		if (ImGui::InputText("TargetName##EnemyChaser", str, sizeof(str), ImGuiInputTextFlags_EnterReturnsTrue) && str[0] != '\0')
		{
			targetName = str;
		}

		ImGui::InputFloat("Speed##FixedRouteTime", &chaseSpeed);
		ImGui::InputFloat("Range##FixedRouteTime", &chaseRange);


		bool edit = _handle.DrawLockButton("FixedRoutePoint");
		/*if (ImGui::Checkbox("EditLock##fixedRoutePoint", &edit))
		{
			_handle.LockHandle(edit, "FixedRoutePoint");
		}*/

		if (edit)
		{
			if (ImGui::Button("AddPoint##RunFixedRoute"))
			{
				routePoints.emplace_back(0.0f, 0.0f);
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
				rayNode.color = XMFLOAT4(0.6f, 0.0f, 0.0f, 1.0f);

				RenderManager::AddDrawRayNode(std::move(rayNode));

				RenderManager::DrawBoxNode boxNode;
				boxNode.center = routePoints[i];
				boxNode.size = { 1.0f / RenderManager::renderZoom.x,1.0f / RenderManager::renderZoom.y };
				if (selectIndex == i)
					boxNode.color = XMFLOAT4(1.0f, 0.0f, colorSeg * i , 1.0f);
				else
					boxNode.color = XMFLOAT4(0.6f, 0.0f, colorSeg * i, 1.0f);

				RenderManager::AddDrawBoxNode(std::move(boxNode));
			}
		}
	}
#endif

private:
	std::string targetName;
	std::vector<Vector2> routePoints = { {-100.0f,0.0f},{100.0f,0.0f} };
	int currentIndex = 0;
	float chaseSpeed = 5.0f;
	float chaseRange = 100.0f;


	void Serialize(SERIALIZE_OUTPUT& ar) override {
		ar(CEREAL_NVP(targetName), CEREAL_NVP(routePoints),
			 CEREAL_NVP(chaseSpeed), CEREAL_NVP(chaseRange));
	}

	void Deserialize(SERIALIZE_INPUT& ar) override {
		ar(CEREAL_NVP(targetName), CEREAL_NVP(routePoints),
			 CEREAL_NVP(chaseSpeed), CEREAL_NVP(chaseRange));
	}
};

SetReflectionComponent(EnemyChaser)
