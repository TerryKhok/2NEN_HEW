#include "Permeation.h"

enum
{
	LEFT_T,
	RIGHT_T,
	RIGHT_B,
	LEFT_B,
	VERTICES_MAX
};

void SearchHitPos(Vector2 start, Vector2 end, std::list<Vector2>& list,Vector2 add)
{
	using namespace Box2D;

	Vector2 output;
	if (WorldManager::RayCast(start, end, output))
	{
		list.push_back(output);
		Vector2 sOutput;
		if (WorldManager::RayCast(output, start, sOutput))
		{
			auto it = list.end();
			--it;
			list.insert(it, sOutput);
		}
		SearchHitPos(output + add, end, list, add);
	}
}

bool Permeation::CreateObstacleSegment()
{
	Vector2 center = m_this->transform.position;
	Vector2 halfScale = m_this->transform.scale * DEFAULT_OBJECT_SIZE / 2;
	Vector2 vertices[VERTICES_MAX] = {
		{center.x - halfScale.x,center.y + halfScale.y},
		{center.x + halfScale.x,center.y + halfScale.y},
		{center.x + halfScale.x,center.y - halfScale.y},
		{center.x - halfScale.x,center.y - halfScale.y}
	};

	bool hit = false;
	for (int i = 0; i < VERTICES_MAX; i++)
	{
		const int start = i;
		const int end = (i + 1) % VERTICES_MAX;

		std::vector<Vector2> fromStartPos;
		std::vector<Vector2> fromEndPos;
		Box2D::WorldManager::RayCastAll(vertices[start], vertices[end], fromStartPos, F_ONLYOBSTACLE);
		Box2D::WorldManager::RayCastAll(vertices[end], vertices[start], fromEndPos, F_ONLYOBSTACLE);

		auto fromStartSize = fromStartPos.size();
		auto fromEndSize = fromEndPos.size();
		if (fromStartSize > 0 || fromEndSize > 0)
		{
			if (fromStartSize > fromEndSize)
			{
				fromEndPos.push_back(vertices[end]);
			}
			else if (fromStartSize < fromEndSize)
			{
				fromStartPos.push_back(vertices[start]);
			}
			else
			{
				switch (i)
				{
				case LEFT_T:
					if (fromStartPos[fromStartSize - 1].x > fromEndPos[0].x)
					{
						fromStartPos.push_back(vertices[start]);
						fromEndPos.push_back(vertices[end]);
					}
					break;
				case RIGHT_T:
					if (fromStartPos[fromStartSize - 1].y < fromEndPos[0].y)
					{
						fromStartPos.push_back(vertices[start]);
						fromEndPos.push_back(vertices[end]);
					}
					break;
				case RIGHT_B:
					if (fromStartPos[fromStartSize - 1].x < fromEndPos[0].x)
					{
						fromStartPos.push_back(vertices[start]);
						fromEndPos.push_back(vertices[end]);
						std::reverse(fromStartPos.begin(), fromStartPos.end());
						std::reverse(fromEndPos.begin(), fromEndPos.end());
					}
					break;
				case LEFT_B:
					if (fromStartPos[fromStartSize - 1].y > fromEndPos[0].y)
					{
						fromStartPos.push_back(vertices[start]);
						fromEndPos.push_back(vertices[end]);
					}
					break;
				}
			}

			for (auto& pos : fromStartPos)
			{
				hit = true;

				auto object = Instantiate("Barrier");
				b2BodyDef bodyDef = b2DefaultBodyDef();
				auto box2d = object->AddComponent<Box2DBody>(&bodyDef);
				box2d->SetFilter(F_TERRAIN);

				std::vector<b2Vec2> points =
				{
					{pos.x ,pos.y ,},
					{fromEndPos.back().x ,fromEndPos.back().y,}
				};
				LOG("start x :%f ,y :%f", pos.x, pos.y);
				LOG("end   x :%f ,y :%f", fromEndPos.back().x, fromEndPos.back().y);
				fromEndPos.pop_back();

				box2d->CreateSegment(points);
				m_barrier.push_back(object->GetName());
			}
		}
	}

	return hit;
}
