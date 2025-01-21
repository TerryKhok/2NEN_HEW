#pragma once

#include "MovePlayer.h"

class Permeation : public Component
{
	SAFE_POINTER(Renderer, rend)
	SAFE_POINTER(Box2DBody, rb)

	int inCount = 0;

	const XMFLOAT4 enterColor = { 1.0f,1.0f,1.0f,1.0f };
	const XMFLOAT4 exitColor = { 1.0f,1.0f,1.0f,0.2f };
	
	//共有の元のフィルターを格納したリスト
	static std::unordered_map<Box2DBody*, std::pair<FILTER,int>> m_box2dFiltersCount;

	//std::vector<Box2DBody*> m_enterBox2d;

	void EnterInsideBox2dObject(Box2DBody* _rb)
	{
		auto iter = m_box2dFiltersCount.find(_rb);
		if (iter != m_box2dFiltersCount.end())
		{
			auto& count = iter->second.second;
			count++;
			return;
		}

		m_box2dFiltersCount.emplace(_rb, std::make_pair(_rb->GetFilter(), 1));
	}

	void ExitInsideBox2dObject(Box2DBody* _rb)
	{
		auto iter = m_box2dFiltersCount.find(_rb);
		if (iter != m_box2dFiltersCount.end())
		{
			auto& count = iter->second.second;
			count--;
			if (count <= 0)
			{
				_rb->SetFilter(iter->second.first);
				m_box2dFiltersCount.erase(iter);
			}
		}
	}

	void Start() override
	{
		rend = m_this->GetComponent<Renderer>();
		rend->SetColor(exitColor);

		if (!m_this->TryGetComponent<Box2DBody>(&rb))
		{
			rb = m_this->AddComponent<Box2DBody>();
		}

		if (!rb->IsBullet())
			rb->SetBullet(true);

		if (rb->GetType() != b2_kinematicBody)
			rb->SetType(b2_kinematicBody);

		CreateObstacleSegment();

		std::unordered_map<GameObject*, b2ShapeId> enterObjects;
		rb->GetOverlapObject(enterObjects);
		for (auto object : enterObjects)
		{
			Box2DBody* rb = nullptr;
			if (object.first->TryGetComponent<Box2DBody>(&rb))
			{
				switch (rb->GetFilter())
				{
				case F_OBSTACLE:
					EnterInsideBox2dObject(rb);
					rb->SetFilter(F_PEROBSTACLE);
					break;
				default:
					rb->SetAwake(true);
					break;
				}
			}
		}
	}

	std::vector<std::string> m_barrier;

	bool CreateObstacleSegment();

	std::unordered_set<GameObject*> enters;

	void OnColliderEnter(GameObject* _other) override
	{
		if (enters.find(_other) != enters.end()) return;

		Box2DBody* rb = nullptr;
		if (_other->TryGetComponent<Box2DBody>(&rb))
		{
			enters.insert(_other);
			FILTER filter = rb->GetFilter();
			EnterInsideBox2dObject(rb);
			switch (filter)
			{
			case F_OBSTACLE:
				//rb->SetFilter(F_PEROBSTACLE);
				break;
			case F_PLAYER:
			{
				rb->SetFilter(F_PERMEATION);
				MovePlayer* player = nullptr;
				if (_other->TryGetComponent<MovePlayer>(&player))
				{
					player->SetMode(PERMEATION);
				}
				break;
			}
			case F_PERMEATION:
			{
				MovePlayer* player = nullptr;
				if (_other->TryGetComponent<MovePlayer>(&player))
				{
					player->SetMode(PERMEATION);
				}
				break;
			}
			default:
				rb->SetFilter(F_PERMEATION);
				break;
			}
			
			rend->SetColor(enterColor);
			inCount++;
		}
	}

	void OnColliderExit(GameObject* _other) override
	{
		Box2DBody* rb = nullptr;
		if (_other->TryGetComponent(&rb))
		{
			ExitInsideBox2dObject(rb);
			enters.erase(enters.find(_other));
			inCount--;
			if (inCount <= 0)
			{
				rend->SetColor(exitColor);
			}
		}
		MovePlayer* player = nullptr;
		if (_other->TryGetComponent(&player))
		{
			player->BackMode();
		}
	}

	std::unordered_map<GameObject*, b2ShapeId> insideObjects;

	void OnWindowEnter(HWND _hWnd) override
	{
		rb->GetOverlapObject(insideObjects);

		for (auto object : m_barrier)
		{
			DeleteObject(object);
		}

		m_barrier.clear();
	}


	void OnWindowExit(HWND _hWnd) override
	{
		CreateObstacleSegment();
		
		Vector2 pos = GetWindowPosition(_hWnd);
		rb->SetPosition(pos);

		std::unordered_map<GameObject*, b2ShapeId> enterObjects;
		rb->GetOverlapObject(enterObjects);

		for (auto object : insideObjects)
		{
			auto iter = enterObjects.find(object.first);
			if (iter == enterObjects.end())
			{
				Box2DBody* rb = nullptr;
				if (object.first->TryGetComponent(&rb))
				{
					ExitInsideBox2dObject(rb);

					auto iterator = enters.find(object.first);
					if (iterator != enters.end()) enters.erase(iterator);

					inCount--;
					if (inCount <= 0)
					{
						rend->SetColor(exitColor);
					}
				}
				MovePlayer* player = nullptr;
				if (object.first->TryGetComponent<MovePlayer>(&player))
				{
					player->BackMode();
				}
			}
		}
		for (auto object : enterObjects)
		{
			auto iter = insideObjects.find(object.first);
			if (iter == insideObjects.end())
			{
				Box2DBody* rb = nullptr;
				if (object.first->TryGetComponent<Box2DBody>(&rb))
				{
					switch (rb->GetFilter())
					{
					case F_OBSTACLE:
						EnterInsideBox2dObject(rb);
						rb->SetFilter(F_PEROBSTACLE);
						break;
					/*case F_PLAYER:
					{
						rb->SetAwake(true);
						MovePlayer* player = nullptr;
						if (object.first->TryGetComponent<MovePlayer>(&player))
						{
							player->SetMode(PERMEATION);
						}
						break;
					}*/
					default:
						rb->SetAwake(true);
						break;
					}
				}
			}
		}

		insideObjects.clear();
	}
};

SetReflectionComponent(Permeation)
