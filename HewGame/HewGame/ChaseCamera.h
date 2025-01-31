#pragma once

class ChaseCamera : public Component
{
	void Update() override
	{
		int pointNum = (int)routePoints.size();
		int nextIndex = (currentIndex + (reverse ? pointNum - 1 : 1)) % pointNum;

		auto& goalTime = reverse ? pointTimes[nextIndex] : pointTimes[currentIndex];
		float t = time / goalTime;

		Vector2 targetPos;
		targetPos.Lerp(t, routePoints[currentIndex], routePoints[nextIndex]);

		RenderManager::renderOffset = targetPos;

		time += 1.0f / UPDATE_FPS;

		if (time >= goalTime)
		{
			m_this->SetActive(false);
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
		if (ImGui::InputFloat("AroundTime##ChaseCameraTime", &aroundTime))
		{
			CalculationPointTime();
		}
		ImGui::Checkbox("Reverse##ChaseCameraTime", &reverse);


		bool edit = _handle.DrawLockButton("ChaseCameraPoint");
		/*if (ImGui::Checkbox("EditLock##ChaseCameraPoint", &edit))
		{
			_handle.LockHandle(edit, "ChaseCameraPoint");
		}*/

		if (edit)
		{
			if (ImGui::Button("AddPoint##RunChaseCamera"))
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
						if (ImGui::Button("erase##ChaseCameraPoint"))
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

	void Serialize(SERIALIZE_OUTPUT& ar) override {
		ar(CEREAL_NVP(routePoints), CEREAL_NVP(pointTimes), CEREAL_NVP(currentIndex),
			CEREAL_NVP(aroundTime), CEREAL_NVP(time), CEREAL_NVP(reverse));
	}

	void Deserialize(SERIALIZE_INPUT& ar) override {
		ar(CEREAL_NVP(routePoints), CEREAL_NVP(pointTimes), CEREAL_NVP(currentIndex),
			CEREAL_NVP(aroundTime), CEREAL_NVP(time), CEREAL_NVP(reverse));
	}
};


SetReflectionComponent(ChaseCamera)

class MoveCamera : public Component
{
	void Update() override
	{
		if (wait)
		{
			time += 1.0f / UPDATE_FPS;
			if (time >= waitTime)
			{
				time = 0.0f;
				wait = false;
			}
			return;
		}

		int pointNum = (int)routePoints.size();
		int nextIndex = (currentIndex + (reverse ? pointNum - 1 : 1)) % pointNum;

		auto& goalTime = reverse ? pointTimes[nextIndex] : pointTimes[currentIndex];
		float t = time / goalTime;

		Vector2 targetPos;
		targetPos.Lerp(t, routePoints[currentIndex], routePoints[nextIndex]);

		RenderManager::renderOffset = targetPos;

		time += 1.0f / UPDATE_FPS;

		if (time >= goalTime)
		{
			sumTime += time;
			time = 0.0f;
			currentIndex = nextIndex;
		}

		if (sumTime >= aroundTime)
		{
			m_this->SetActive(false);
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
		ImGui::InputFloat("waitTime##MoveCamera", &waitTime);
		if (ImGui::InputFloat("AroundTime##MoveCameraTime", &aroundTime))
		{
			CalculationPointTime();
		}
		ImGui::Checkbox("Reverse##MoveCameraTime", &reverse);

		bool edit = _handle.DrawLockButton("MoveCameraPoint");
		/*if (ImGui::Checkbox("EditLock##MoveCameraPoint", &edit))
		{
			_handle.LockHandle(edit, "MoveCameraPoint");
		}*/

		if (edit)
		{
			if (ImGui::Button("AddPoint##RunMoveCamera"))
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
						if (ImGui::Button("erase##MoveCameraPoint"))
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
	bool wait = true;
	float waitTime = 1.0f;
	float sumTime = 0.0f;
	float aroundTime = 5.0f;
	float time = 0.0f;
	bool reverse = true;

	void Serialize(SERIALIZE_OUTPUT& ar) override {
		ar(CEREAL_NVP(currentIndex),CEREAL_NVP(routePoints), CEREAL_NVP(pointTimes), CEREAL_NVP(wait), CEREAL_NVP(time),
			CEREAL_NVP(sumTime),CEREAL_NVP(waitTime), CEREAL_NVP(aroundTime), CEREAL_NVP(reverse));
	}

	void Deserialize(SERIALIZE_INPUT& ar) override {
		ar(CEREAL_NVP(currentIndex), CEREAL_NVP(routePoints), CEREAL_NVP(pointTimes), CEREAL_NVP(wait), CEREAL_NVP(time),
			CEREAL_NVP(sumTime), CEREAL_NVP(waitTime), CEREAL_NVP(aroundTime), CEREAL_NVP(reverse));
	}
};


SetReflectionComponent(MoveCamera)


