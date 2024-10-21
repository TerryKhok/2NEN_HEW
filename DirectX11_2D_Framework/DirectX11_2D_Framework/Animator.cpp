#include "Animator.h"

long long AnimatorManager::deltaCount;

void AnimationClip::SetUVRenderNode(UVRenderNode* _renderNode)
{
	auto& frame = frames[frameIndex];
	_renderNode->SetTexture(frame.texPath);
	_renderNode->m_frameX = frame.frameX;
	_renderNode->m_frameY = frame.frameY;
	_renderNode->m_scaleX = frame.scaleX;
	_renderNode->m_scaleY = frame.scaleY;
}

void AnimationClip::Awake(UVRenderNode* _renderNode)
{
	frameIndex = 0;
	nowCount = 0;
	pUpdate = &AnimationClip::UpdateCount;

	SetUVRenderNode(_renderNode);
}

void AnimationClip::UpdateCount(long long _count, UVRenderNode* _renderNode)
{
	nowCount += _count;

	const auto& waitCount = frames[frameIndex].waitCount;
	if (nowCount > waitCount)
	{
		nowCount -= waitCount;
		frameIndex++;

		if (frameIndex >= static_cast<int>(frames.size()))
		{
			pUpdate = &AnimationClip::UpdateVoid;
			return;
		}

		SetUVRenderNode(_renderNode);
	}
}

void AnimationClipLoop::Awake(UVRenderNode* _renderNode)
{
	frameIndex = 0;
	nowCount = 0;
	SetUVRenderNode(_renderNode);
}

void AnimationClipLoop::Update(long long _count, UVRenderNode* _renderNode)
{
	nowCount += _count;

	const auto& waitCount = frames[frameIndex].waitCount;
	if (nowCount > waitCount)
	{
		nowCount -= waitCount;
		frameIndex = (frameIndex + 1) % static_cast<int>(frames.size());

		SetUVRenderNode(_renderNode);
	}
}

Animator::Animator(GameObject* _gameObject)
{
	auto renderer = _gameObject->GetComponent<Renderer>();
	if (renderer == nullptr)
	{
		_gameObject->AddComponent<Renderer>(this);
		LOG("so we added an Renderer. Never mind the warning above.\n");
		return;
	}

	renderer->SetUVRenderNode(this);
}

void Animator::Update()
{
	(m_currentClip.get()->*pUpdate)(AnimatorManager::deltaCount, m_uvNode);
}

void Animator::Play(const std::string& _clipName)
{
	m_currentClip = m_clip.find(_clipName)->second;
	m_currentClip->Awake(m_uvNode);
}

void Animator::Pause()
{
	pUpdate = &AnimationClip::UpdateVoid;
}

void Animator::Resume()
{
	pUpdate = &AnimationClip::Update;
}
