#pragma once

#include "DecorativePlayerMove.h"

class AntiGravity : public Component
{
	SAFE_POINTER(Renderer, rend)
	SAFE_POINTER(Box2DBody, rb)

	const XMFLOAT4 enterColor = { 1.0f,1.0f,1.0f,1.0f };
	const XMFLOAT4 exitColor = { 1.0f,1.0f,1.0f,0.2f };

	void Start() override
	{
		rend = m_this->GetComponent<Renderer>();
		if (rend != nullptr)
		{
			rend->SetColor(exitColor);
		}

		if (!m_this->TryGetComponent<Box2DBody>(&rb))
		{
			rb = m_this->AddComponent<Box2DBody>();
		}
		
		if (!rb->IsBullet())
			rb->SetBullet(true);

		if (rb->GetType() != b2_kinematicBody)
			rb->SetType(b2_kinematicBody);
	}

protected:
	static inline std::unordered_map<GameObject*, int> enterLib;

	std::unordered_set<GameObject*> enters;

	virtual void EnterEvent(GameObject* target)
	{
		auto iter = enterLib.find(target);
		if (iter != enterLib.end())
			iter->second++;
		else
		{
			enterLib.emplace(target, 1);

			Box2DBody* rb = nullptr;
			if (target->TryGetComponent<Box2DBody>(&rb))
			{
				rb->SetGravityScale(rb->GetGravityScale() * -1.0f);
				//rb->SetAngle((double)(b2Rot_GetAngle(b2Body_GetRotation(rb->GetBodyId())) + Math::PI));
			}

			MovePlayer* player = nullptr;
			if (target->TryGetComponent<MovePlayer>(&player))
			{
				player->inFloat = true;
				player->PushMode(ANTI_GRAVITY);
			}
		}
	}

	virtual void ExitEvent(GameObject* target)
	{
		auto iter = enterLib.find(target);
		if (iter == enterLib.end()) return;
		iter->second--;
		if (iter->second > 0)return;
		enterLib.erase(iter);
		
		Box2DBody* rb = nullptr;
		if (target->TryGetComponent(&rb))
		{
			float gravity = rb->GetGravityScale();
			rb->SetGravityScale(gravity * -1.0f);
			//rb->SetAngle((double)(b2Rot_GetAngle(b2Body_GetRotation(rb->GetBodyId())) + Math::PI));
			//rb->AddForceImpulse({ 0.0f,rb->GetMass() });
		}

		MovePlayer* player = nullptr;
		if (target->TryGetComponent<MovePlayer>(&player))
		{
			player->inFloat = false;
			player->PopMode(ANTI_GRAVITY);
		}
	}

private:
	int overlapCount = 0;
	void Update() override
	{
		overlapCount++;
		if (overlapCount > 2)
		{
			overlapCount = 0;
			std::unordered_map<GameObject*, b2ShapeId> insideObjects;
			rb->GetOverlapObject(insideObjects);

			for (auto iter = enters.begin(); iter != enters.end();)
			{
				auto it = insideObjects.find(*iter);
				if (it == insideObjects.end())
				{
					ExitEvent(*iter);
					iter = enters.erase(iter);
					if (enters.empty())
						rend->SetColor(exitColor);
					continue;
				}
				iter++;
			}
		}
	}

	void OnColliderEnter(GameObject* _other) override
	{
		if (enters.find(_other) != enters.end()) return;

		enters.insert(_other);
		rend->SetColor(enterColor);
		EnterEvent(_other);
	}

	void OnColliderExit(GameObject* _other) override
	{
		auto iter = enters.find(_other);
		if (iter == enters.end()) return;
		
		enters.erase(iter);
		if (enters.empty())
			rend->SetColor(exitColor);

		ExitEvent(_other);
	}

	void Delete() override
	{
		for (auto& obj : enters)
		{
			auto iter = enterLib.find(obj);
			if (iter == enterLib.end()) continue;
			iter->second--;
			if (iter->second > 0)continue;
			enterLib.erase(iter);
		}
	}
};

SetReflectionComponent(AntiGravity)

class DecorativeAntiGravity : public AntiGravity
{
	void EnterEvent(GameObject* target) override
	{
		auto iter = enterLib.find(target);
		if (iter != enterLib.end())
			iter->second++;
		else
		{
			enterLib.emplace(target, 1);

			Box2DBody* rb = nullptr;
			if (target->TryGetComponent<Box2DBody>(&rb))
			{
				rb->SetGravityScale(rb->GetGravityScale() * -1.0f);
				//rb->SetAngle((double)(b2Rot_GetAngle(b2Body_GetRotation(rb->GetBodyId())) + Math::PI));
			}

			DecorativePlayerMove* player = nullptr;
			if (target->TryGetComponent<DecorativePlayerMove>(&player))
			{
				player->inFloat = true;
				player->PushMode(ANTI_GRAVITY);
			}
		}
	}

	void ExitEvent(GameObject* target) override
	{
		auto iter = enterLib.find(target);
		if (iter == enterLib.end()) return;
		iter->second--;
		if (iter->second > 0)return;
		enterLib.erase(iter);

		Box2DBody* rb = nullptr;
		if (target->TryGetComponent(&rb))
		{
			float gravity = rb->GetGravityScale();
			rb->SetGravityScale(gravity * -1.0f);
			//rb->SetAngle((double)(b2Rot_GetAngle(b2Body_GetRotation(rb->GetBodyId())) + Math::PI));
			rb->AddForceImpulse({ 0.0f,rb->GetMass() });
		}

		DecorativePlayerMove* player = nullptr;
		if (target->TryGetComponent<DecorativePlayerMove>(&player))
		{
			player->inFloat = false;
			player->PopMode(ANTI_GRAVITY);
		}
	}
};

SetReflectionComponent(DecorativeAntiGravity)