#pragma once

class Permeation : public Component
{
	SAFE_POINTER(Renderer, rend)
	SAFE_POINTER(Box2DBody, rb)

	int inCount = 0;

	const XMFLOAT4 enterColor = { 0.2f,0.2f,0.2f,0.5f };
	const XMFLOAT4 exitColor = { 0.2f,0.2f,0.2f,0.2f };

	std::unordered_map<Box2DBody*, FILTER> m_enterBox2dFilters;

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

		std::unordered_map<GameObject*, b2ShapeId> exitObjects;
		rb->GetOverlapObject(exitObjects);
		for (auto object : exitObjects)
		{
			Box2DBody* rb = nullptr;
			if (object.first->TryGetComponent<Box2DBody>(&rb))
			{
				switch (rb->GetFilter())
				{
				case F_OBSTACLE:
					rb->SetFilter(F_PEROBSTACLE);
					m_enterBox2dFilters.insert(std::make_pair(rb, F_OBSTACLE));
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
			enters.emplace(_other);
			FILTER filter = rb->GetFilter();
			switch (filter)
			{
			case F_OBSTACLE:
				rb->SetFilter(F_PEROBSTACLE);
				break;
			default:
				rb->SetFilter(F_PERMEATION);
				break;
			}
			m_enterBox2dFilters.insert(std::make_pair(rb, filter));
			
			rend->SetColor(enterColor);
			inCount++;
		}
	}

	void OnColliderExit(GameObject* _other) override
	{
		Box2DBody* rb = nullptr;
		if (_other->TryGetComponent(&rb))
		{
			auto iter = m_enterBox2dFilters.find(rb);
			if (iter != m_enterBox2dFilters.end())
			{
				rb->SetFilter(iter->second);
				m_enterBox2dFilters.erase(iter);
			}
			enters.erase(enters.find(_other));
			inCount--;
			if (inCount <= 0)
			{
				rend->SetColor(exitColor);
			}
		}
	}

	std::unordered_map<GameObject*, b2ShapeId> enterObjects;
	Vector2 enterPos;

	void OnWindowEnter(HWND _hWnd) override
	{
		rb->GetOverlapObject(enterObjects);
		enterPos = m_this->transform.position;

		for (auto object : m_barrier)
		{
			DeleteObject(object);
		}

		m_barrier.clear();
	}

	void OnWindowMove(HWND _hWnd,RECT* _rect) override
	{

	}

	void OnWindowExit(HWND _hWnd) override
	{
		CreateObstacleSegment();
		
		Vector2 pos = GetWindowPosition(_hWnd);
		rb->SetPosition(pos);

		std::unordered_map<GameObject*, b2ShapeId> exitObjects;
		rb->GetOverlapObject(exitObjects);

		for (auto object : enterObjects)
		{
			auto iter = exitObjects.find(object.first);
			if (iter == exitObjects.end())
			{
				Box2DBody* rb = nullptr;
				if (object.first->TryGetComponent(&rb))
				{
					auto iter = m_enterBox2dFilters.find(rb);
					if (iter != m_enterBox2dFilters.end())
					{
						rb->SetFilter(iter->second);
						m_enterBox2dFilters.erase(iter);
					}

					auto iterator = enters.find(object.first);
					if (iterator != enters.end()) enters.erase(iterator);

					inCount--;
					if (inCount <= 0)
					{
						rend->SetColor(exitColor);
					}
				}
			}
		}
		for (auto object : exitObjects)
		{
			auto iter = enterObjects.find(object.first);
			if (iter == enterObjects.end())
			{
				Box2DBody* rb = nullptr;
				if (object.first->TryGetComponent<Box2DBody>(&rb))
				{
					switch (rb->GetFilter())
					{
					case F_OBSTACLE:
						rb->SetFilter(F_PEROBSTACLE);
						m_enterBox2dFilters.insert(std::make_pair(rb, F_OBSTACLE));
						break;
					default:
						rb->SetAwake(true);
						break;
					}
				}
			}
		}

		enterObjects.clear();
	}
};
