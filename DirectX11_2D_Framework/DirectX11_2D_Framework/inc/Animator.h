#pragma once


struct AnimationFrameData
{
	//�������m�ۋ֎~
	void* operator new(size_t) = delete;

	std::wstring texPath;
	float scaleX = 0.5f;
	float scaleY = 0.5f;
	int frameX = 0;
	int frameY = 0;
	long long waitCount = 0;
};

struct AnimationFrame
{
	//�������m�ۋ֎~
	void* operator new(size_t) = delete;

	int texIndex = 0;
#ifdef DEBUG_TRUE
	std::wstring texPath = L"";
#endif
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
	void AddFrame(const AnimationFrameData& _frame) {
		AnimationFrame frame;
		ComPtr<ID3D11ShaderResourceView> texture;
		TextureAssets::pLoadTexture(texture, _frame.texPath.c_str());
		auto iter = std::find(textureList.begin(), textureList.end(), texture);
		if (iter != textureList.end())
		{
			frame.texIndex = (int)std::distance(textureList.begin(), iter);
		}
		else
		{
			textureList.push_back(texture);
			frame.texIndex = (int)textureList.size() - 1;
		}
#ifdef DEBUG_TRUE
		frame.texPath = _frame.texPath;
#endif
		frame.frameX = _frame.frameX;
		frame.frameY = _frame.frameY;
		frame.scaleX = _frame.scaleX;
		frame.scaleY = _frame.scaleY;
		frame.waitCount = _frame.waitCount;

		frames.push_back(std::move(frame));
	}
	//���݂�frameIndex�ɍX�V
	void SetUVRenderNode(UVRenderNode* _renderNode);
	//����������(Clip�؂�ւ����ɌĂяo��)
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

	virtual bool IsLoop() { return false; }
private:
	functionPointer pUpdate;
protected:
	//�A�j���[�V�����̃R�}���i�[
	std::vector<AnimationFrame> frames;
	std::vector<ComPtr<ID3D11ShaderResourceView>> textureList;
	//�t���[���̔ԍ�
	int frameIndex = 0;
	//���݂̃J�E���g
	long long nowCount = 0;
	//�V���A���C�Y�p
	std::filesystem::path clipFilePath;
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

	bool IsLoop() override { return true; }
};


class Animator : public Component
{
	friend class GameObject;
	friend class Renderer;

	Animator(GameObject* _gameObject);
	Animator(GameObject* _gameObject,SERIALIZE_INPUT& ar);
	~Animator() = default;
	void Update() override;

	void SetActive(bool _active);
	void DrawImGui(ImGuiApp::HandleUI& _handle) override;
	void Serialize(SERIALIZE_OUTPUT& ar) override;
public:
	void AddClip(std::string _name,std::string _path, bool _loop = true);

	void Reverse(bool _reverse);
	void Play(const std::string& _clipName);
	void Pause();
	void Resume();	
private:
	AnimationClip::functionPointer pUpdate = &AnimationClip::Update;
private:
	std::map<std::string, std::shared_ptr<AnimationClip>> m_clip;
#ifdef DEBUG_TRUE
	std::filesystem::path currentClipPath;
#endif
	std::string_view m_currentClipName;
	std::shared_ptr<AnimationClip> m_currentClip;
	UVRenderNode* m_uvNode = nullptr;
};


class AnimatorManager
{
	friend class Animator;
	friend class Window;
	friend class ImGuiApp;

	AnimatorManager() = delete;
private:
	static std::shared_ptr<AnimationClip> m_commonClip;
	static long long deltaCount;
};
