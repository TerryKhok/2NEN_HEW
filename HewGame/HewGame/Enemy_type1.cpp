#include "YkTestScene.h"
#include "Enemy_type1.h"
#include "MovePlayer.h"

void Enemy_type1::Start()
{
	rb = m_this->GetComponent<Box2DBody>();
}

void Enemy_type1::Update()
{
	searchCount++;
	if (searchCount > 10)
	{
		GameObject* player = ObjectManager::Find("Player");
		goalPos = player->transform.position;
		Vector2 dis = goalPos - m_this->transform.position;
		float Renge = sqrtf(pow(dis.x, 2) + pow(dis.y, 2));
		if (Renge < searchRenge)
		{
			checkPos.clear();
			Search(m_this->transform.position);

			if (checkPos.size() > 0)
			{
				Vector2 vec = checkPos[0] - m_this->transform.position;
				vec.Normalize();
				vec *= speed;
				rb->AddForceImpulse({ vec.x,vec.y });
			}
			
		}

		searchCount = 0;
	}
		
}

void Enemy_type1::Search(Vector3 targetPos)
{
	Vector3 evec;

	evec = goalPos - targetPos;

	bool touchFg = Box2D::WorldManager::RayCastShape(targetPos,goalPos, rb, F_MAPRAY);
	float renge = 0;
	

	if (touchFg == true)
	{
		while (renge < 135 / searchAngle)
		{
			renge += searchAngle;
			Vector3 vec;
			evec.Normalize();
		    float rad = Math::DegToRad(-renge);
			vec.x = evec.x * cos(rad) - evec.y * sin(rad);
			vec.y = evec.x * sin(rad) - evec.y * cos(rad);
			/*touchFg = Box2D::WorldManager::RayCastShape(targetPos, targetPos + vec * dis, rb, F_MAPRAY);*/
			touchFg = Box2D::WorldManager::RayCast(targetPos, targetPos + vec * searchDis, F_MAPRAY);
			if (touchFg == false)
			{
				checkPos.push_back(targetPos);
				Search(targetPos + vec * searchDis);
				break;
			}
		}
		//renge = 0.0f;
		//while (renge > -135 / searchAngle)
		//{
		//	renge -= searchAngle;
		//	Vector3 vec;
		//	evec.Normalize();
		//	float rad = Math::DegToRad(renge);
		//	vec.x = evec.x * cos(rad) - evec.y * sin(rad);
		//	vec.y = evec.x * sin(rad) - evec.y * cos(rad);
		//	/*touchFg = Box2D::WorldManager::RayCastShape(targetPos, targetPos + vec * dis, rb, F_MAPRAY);*/
		//	touchFg = Box2D::WorldManager::RayCast(targetPos, targetPos + vec * searchDis, F_MAPRAY);
		//	if (touchFg == false)
		//	{
		//		checkPos.push_back(targetPos);
		//		Search(targetPos + vec * searchDis);
		//		break;
		//	}
		//}
	}
	if (touchFg == false)
	{
		checkPos.push_back(goalPos);
		LOG("find");
		return;
	}
}

void Enemy_type1::OnColliderEnter(GameObject* _other)
{
	MovePlayer* rb = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&rb))
	{
		LOG(_other->GetName().c_str());
		SceneManager::LoadScene<YkTestScene>();
	}
}

void Enemy_type1::DrawImGui(ImGuiApp::HandleUI& _handle)
{
	ImGui::InputFloat("speed##Enemy", &speed);
	ImGui::InputFloat("Renge##Enemy", &searchRenge);
	ImGui::InputFloat("Angle##Enemy", &searchAngle);
	ImGui::InputFloat("Dis##Enemy", &searchDis);
}