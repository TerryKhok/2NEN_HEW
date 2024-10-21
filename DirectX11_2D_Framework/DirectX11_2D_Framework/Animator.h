#pragma once

class RenderNode;

struct AnimationFrame
{
	//メモリ確保禁止
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

	//生成禁止
	AnimationClip(){
		pUpdate = &AnimationClip::UpdateCount;
	}
	//フレームを追加
	void AddFrame(AnimationFrame _frame) {
		frames.push_back(_frame);
	}
	//現在のframeIndexに更新
	void SetUVRenderNode(UVRenderNode* _renderNode);
	//初期化処理
	virtual void Awake(UVRenderNode* _renderNode);
	//更新処理
	virtual void Update(long long _count, UVRenderNode* _renderNode)
	{
		(this->*pUpdate)(_count, _renderNode);
	}
	//カウントを増やす
	void UpdateCount(long long _count, UVRenderNode* _renderNode);
	//なにもしない
	void UpdateVoid(long long, UVRenderNode*) {}
private:
	functionPointer pUpdate;
protected:
	//アニメーションのコマを格納
	std::vector<AnimationFrame> frames;
	//フレームの番号
	int frameIndex = 0;
	//現在のカウント
	long long nowCount = 0;
};

class AnimationClipLoop : public AnimationClip
{
	friend class Animator;
private:
	AnimationClipLoop(){}
	//カウントを増やす
	void Update(long long _count,UVRenderNode* _renderNode);
	//初期化処理
	void Awake(UVRenderNode* _renderNode);
};

class Animator : public Component
{
	friend class GameObject;
	friend class Renderer;

	Animator(GameObject* _gameObject);
	//削除禁止
	~Animator() = default;
	void Update() override;
public:
	//デバッグ用
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
