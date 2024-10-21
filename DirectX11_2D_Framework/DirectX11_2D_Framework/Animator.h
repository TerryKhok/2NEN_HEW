#pragma once

class RenderNode;

struct AnimationFrame
{
	//�������m�ۋ֎~
	void* operator new(size_t) = delete;

	const wchar_t* texPath;
	float scaleX = 0.5f;
	float scaleY = 0.5f;
	int frameX = 0;
	int frameY = 0;
	long long waitCount = 0;
};

class AnimationClip
{
	friend class Animator;
protected:
	// Define a member function pointer type
	using functionPointer = void (AnimationClip::*)(long long, UVRenderNode*);

	//�����֎~
	AnimationClip(){
		pUpdate = &AnimationClip::UpdateCount;
	}
	//�t���[����ǉ�
	void AddFrame(AnimationFrame _frame) {
		frames.push_back(_frame);
	}
	//���݂�frameIndex�ɍX�V
	void SetUVRenderNode(UVRenderNode* _renderNode);
	//����������
	virtual void Awake(UVRenderNode* _renderNode);
	//�X�V����
	virtual void Update(long long _count, UVRenderNode* _renderNode)
	{
		(this->*pUpdate)(_count, _renderNode);
	}
	//�J�E���g�𑝂₷
	void UpdateCount(long long _count, UVRenderNode* _renderNode);
	//�Ȃɂ����Ȃ�
	void UpdateVoid(long long, UVRenderNode*) {}
private:
	functionPointer pUpdate;
protected:
	//�A�j���[�V�����̃R�}���i�[
	std::vector<AnimationFrame> frames;
	//�t���[���̔ԍ�
	int frameIndex = 0;
	//���݂̃J�E���g
	long long nowCount = 0;
};

class AnimationClipLoop : public AnimationClip
{
	friend class Animator;
private:
	AnimationClipLoop(){}
	//�J�E���g�𑝂₷
	void Update(long long _count,UVRenderNode* _renderNode);
	//����������
	void Awake(UVRenderNode* _renderNode);
};

class Animator : public Component
{
	friend class GameObject;
	friend class Renderer;

	Animator(GameObject* _gameObject);
	//�폜�֎~
	~Animator() = default;
	void Update() override;
public:
	//�f�o�b�O�p
	//============================================================================
	void AddClip(std::string _name, std::vector<AnimationFrame>& _frames,bool _loop = true)
	{
		AnimationClip* clip;
		if (_loop){
			clip = new AnimationClipLoop();
		}
		else {
			clip = new AnimationClip();
		}
		for (auto& frame : _frames)
		{
			clip->AddFrame(frame);
		}
		m_clip.insert(std::make_pair(_name, clip));
	}
	//============================================================================

	void Play(const std::string& _clipName);
	void Pause();
	void Resume();
private:
	AnimationClip::functionPointer pUpdate = &AnimationClip::Update;
private:
	std::unordered_map<std::string, std::shared_ptr<AnimationClip>> m_clip;
	std::shared_ptr<AnimationClip> m_currentClip;
	UVRenderNode* m_uvNode = nullptr;
};

class AnimatorManager
{
	friend class Animator;
	friend class Window;

private:
	static long long deltaCount;
};
