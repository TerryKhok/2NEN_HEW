#include "Permeation.h"
#include "clipper2/clipper.h"

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
	bool inside = false;
	bool insideList[VERTICES_MAX] = { false };
	bool unHit[VERTICES_MAX] = { false };
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
				inside = true;
			}
			else if (fromStartSize < fromEndSize)
			{
				fromStartPos.push_back(vertices[start]);
				inside = false;
			}
			else
			{
				switch (i)
				{
				case LEFT_T:
					if (fromStartPos[fromStartSize - 1].x > fromEndPos[0].x)
					{
						inside = true;
						fromStartPos.push_back(vertices[start]);
						fromEndPos.push_back(vertices[end]);
					}
					break;
				case RIGHT_T:
					if (fromStartPos[fromStartSize - 1].y < fromEndPos[0].y)
					{
						inside = true;
						fromStartPos.push_back(vertices[start]);
						fromEndPos.push_back(vertices[end]);
					}
					break;
				case RIGHT_B:
					if (fromStartPos[fromStartSize - 1].x < fromEndPos[0].x)
					{
						inside = true;
						fromStartPos.push_back(vertices[start]);
						fromEndPos.push_back(vertices[end]);
						std::reverse(fromStartPos.begin(), fromStartPos.end());
						std::reverse(fromEndPos.begin(), fromEndPos.end());
					}
					break;
				case LEFT_B:
					if (fromStartPos[fromStartSize - 1].y > fromEndPos[0].y)
					{
						inside = true;
						fromStartPos.push_back(vertices[start]);
						fromEndPos.push_back(vertices[end]);
					}
					break;
				}
			}
		}
		else 
		{
			unHit[i] = true;
			/*fromStartPos.push_back(vertices[start]);
			fromEndPos.push_back(vertices[end]);*/

			/*int hEnd = (i + 2) % VERTICES_MAX;
			if (Box2D::WorldManager::RayCast(vertices[hEnd], vertices[start], F_ONLYOBSTACLE))
			{
				fromStartPos.push_back(vertices[start]);
				fromEndPos.push_back(vertices[end]);
			}
			else
			{
				const int hStart = (i + 1) % VERTICES_MAX;
				hEnd = (i + 3) % VERTICES_MAX;
				if (Box2D::WorldManager::RayCast(vertices[hEnd], vertices[hStart], F_ONLYOBSTACLE))
				{
					fromStartPos.push_back(vertices[start]);
					fromEndPos.push_back(vertices[end]);
				}
			}*/
		}

		insideList[end] = inside;

		for (auto& pos : fromStartPos)
		{
			if (pos.x == fromEndPos.back().x && pos.y == fromEndPos.back().y)
			{
				fromEndPos.back() -= Vector2(1.0f, 1.0f);
			}

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

			/*LOG("start x :%f ,y :%f", pos.x, pos.y);
			LOG("end   x :%f ,y :%f", fromEndPos.back().x, fromEndPos.back().y);*/
			fromEndPos.pop_back();

			box2d->CreateSegment(points);
			m_barrier.push_back(object->GetName());
		}
	}

	for (int i = 0; i < VERTICES_MAX; i++)
	{
		if (insideList[i] && unHit[i])
		{
			int nextIndex = (i + 1) % VERTICES_MAX;
			if (unHit[nextIndex])
			{
				insideList[nextIndex] = true;
			}

			auto& startPos = vertices[i];
			auto& endPos = vertices[nextIndex];

			auto object = Instantiate("Barrier");
			b2BodyDef bodyDef = b2DefaultBodyDef();
			auto box2d = object->AddComponent<Box2DBody>(&bodyDef);
			box2d->SetFilter(F_TERRAIN);

			std::vector<b2Vec2> points =
			{
				{startPos.x ,startPos.y ,},
				{endPos.x ,endPos.y,}
			};

			box2d->CreateSegment(points);
			m_barrier.push_back(object->GetName());
		}
	}

	return hit;
}

void Permeation::ClipObject()
{
	using namespace Clipper2Lib;

	Vector2 center = m_this->transform.position;
	int64_t centerX = (int64_t)center.x;
	int64_t centerY = (int64_t)center.y;
	Vector2 halfScale = m_this->transform.scale * DEFAULT_OBJECT_SIZE / 2;
	int64_t halfScaleX = (int64_t)halfScale.x;
	int64_t halfScaleY = (int64_t)halfScale.y;
	// 四角形の切り抜き領域を定義
	Paths64 clip = {
		{{centerX - halfScaleX,centerY + halfScaleY},
		{centerX + halfScaleX,centerY + halfScaleY},
		{centerX + halfScaleX,centerY - halfScaleY},
		{centerX - halfScaleX,centerY - halfScaleY}} // 切り抜き領域
	};

	rb->SetFilter(F_WINDOW);

	std::vector<GameObject*> overlapList;
	rb->GetOverlapObject(overlapList, F_ONLYOBSTACLE);

	if (overlapList.empty())
	{
		rb->ChangeFilter(F_PERWINDOW);
		return;
	}

	overlapList.erase(std::unique(overlapList.begin(), overlapList.end()), overlapList.end());

	std::vector<Permeation*> perWindows;

	for (auto& object : overlapList)
	{
		Permeation* perWnd;
		if (object->TryGetComponent<Permeation>(&perWnd))
		{
			perWindows.push_back(perWnd);
			continue;
		}

		std::vector<Vector2> vertices;

		Box2DBody* box2d = object->GetComponent<Box2DBody>();
		auto& shapeList = box2d->GetShapeId();
		for (auto& shape : shapeList)
		{
			auto shapeType = b2Shape_GetType(shape);
			switch (shapeType)
			{
			case b2_polygonShape:
			{
				auto polygon = b2Shape_GetPolygon(shape);
				for (int i = 0; i < polygon.count; i++)
				{
					auto& pos = polygon.vertices[i];
					vertices.emplace_back(pos.x * DEFAULT_OBJECT_SIZE, pos.y * DEFAULT_OBJECT_SIZE);
				}
			}
				break;
			case b2_segmentShape:
			{
				auto segment = b2Shape_GetSegment(shape);
				auto& pos = segment.point1;
				vertices.emplace_back(pos.x * DEFAULT_OBJECT_SIZE, pos.y * DEFAULT_OBJECT_SIZE);
			}
				break;
			case b2_chainSegmentShape:
			{
				auto chainSegment = b2Shape_GetChainSegment(shape);
				auto& pos = chainSegment.segment.point1;
				vertices.push_back({ pos.x * DEFAULT_OBJECT_SIZE, pos.y * DEFAULT_OBJECT_SIZE });
			}
				break;
			}
		}

		// 元のポリゴンを定義 (例: 四角形)
		Paths64 subject;

		//ローカル座標に中心点を加える
		Vector2 center = box2d->GetPosition();
		Path64 path;
		for (auto& pos : vertices)
		{
			pos += center;
			path.emplace_back((int)pos.x, (int)pos.y);
		}
		subject.push_back(path);

		// 結果を格納するための変数
		Paths64 solution;

		Clipper64 clipper;
		clipper.AddSubject(subject);
		clipper.AddClip(clip);
		clipper.Execute(ClipType::Difference, FillRule::EvenOdd, solution);  // 差分操作

		std::vector<b2Vec2> points;
		for (const auto& path : solution) {
			auto instance = Instantiate("clipped");
			instance->transform.position = center;
			b2BodyDef bodyDef = b2DefaultBodyDef();;
			auto iBox2D = instance->AddComponent<Box2DBody>(&bodyDef);
			iBox2D->SetFilter(box2d->GetFilter());

			for (const auto& point : path) {
				Vector2 pos = { (float)point.x, (float)point.y };
				pos -= center;
				points.emplace_back(pos.x, pos.y);
			}
			if (points.size() > 3)
				iBox2D->CreateChain(points);
			else
			{
				points.push_back(points.front());
				iBox2D->CreateSegment(points);
			}
			points.clear();
		}

		object->RemoveComponent<Box2DBody>();
	}

	for (auto& perWnd : perWindows)
	{
		perWnd->ClipObject();
	}
}
