#pragma once


class Renderer : public Component
{
	friend class GameObject;
	friend class Animator;
	
private:
	//�����֎~
	Renderer(GameObject* _pObject);
	Renderer(GameObject* _pObject,const wchar_t* _texpath);
	Renderer(GameObject* _pObject,Animator* _animator);
	//�f�V���A���C�Y�p
	Renderer(GameObject* _pObject, SERIALIZE_INPUT& ar);
	~Renderer() = default;
	//�A�N�e�B�u�ύX
	void SetActive(bool _active);
	//�Ή������m�[�h�̍폜
	void Delete();
	//UVRenderNode�֐؂�ւ���
	void SetUVRenderNode(Animator* _animator);
	//�V���A���C�Y
	void Serialize(SERIALIZE_OUTPUT& ar) override;
	//imGui�̕`��
	void DrawImGui(ImGuiApp::HandleUI& _handle) override;
public:
	//���C���[�̕ύX
	void SetLayer(const LAYER _layer);
	//�e�N�X�`���̕ύX
	void SetTexture(const wchar_t* _texPath);
	void SetTexture(const std::string& _filePath);
	//�F�̕ύX
	void SetColor(XMFLOAT4 _color);
	//uv���W�̕ύX(������Əd��)
	void SetUV(int _splitX, int _splitY, int _frameX, int _frameY);
private:
	//�Ή������m�[�h
	std::shared_ptr<RenderNode> m_node;
	//�`�悷�鏇��
	LAYER m_layer = LAYER::LAYER_01;
//private:
//	template<class Archive>
//	void save(const Archive& archive) const
//	{
//		archive(CEREAL_NVP(m_node), CEREAL_NVP(m_layer));
//	}
//
//	template<class Archive>
//	void load(const Archive& archive) const
//	{
//		archive(CEREAL_NVP(m_node), CEREAL_NVP(m_layer));
//		RenderManager::AddRenderList(m_node, m_layer);
//	}
};


//�o�������X�g�m�[�h
class RenderNode
{
	friend class Renderer;
	friend class RenderManager;
	friend class Box2DBody;
	friend class Animator;

	//�`��֘A
protected:
	RenderNode();
	RenderNode(const wchar_t* _texpath);
	//�A�N�e�B�u��؂�ւ���
	void Active(bool _active);
	//�`��֐��̎��s
	virtual void Execute() { (this->*pDrawFunc)(); }
	//�`�悵�Ď��ɂȂ�
	virtual inline void Draw();
	//�Ȃɂ������Ɏ��ɂȂ�
	void VoidNext() { NextFunc(); }
	//�e�N�X�`���[�̐ݒ�
	void SetTexture(const wchar_t* _texpath);
	//�e�N�X�`���[�̐ݒ�
	void SetTexture(const std::string& _filePath);
private:
	//�`��֐��|�C���^�[
	void(RenderNode::* pDrawFunc)(void) = &RenderNode::Draw;
protected:
	GameObject* m_object = nullptr;
	DirectX::XMFLOAT4 m_color = { 1.0f,1.0f,1.0f,1.0f };
	//�e�N�X�`���p�X
	bool active = false;
	std::wstring texPath = L"null";
	ComPtr<ID3D11ShaderResourceView> m_pTextureView = nullptr;

	//���X�g�֘A
protected:
	//�֐��|�C���^�[�̎��s
	void NextFunc() { (this->*pConnectFunc)(); }
	//���X�g�̎���ݒ肷��
	void NextContinue() { pConnectFunc = &RenderNode::Continue; }
	//���X�g�̏I����ݒ肷��
	void NextEnd() { pConnectFunc = &RenderNode::End; }
	//���̃m�[�h�̕`�������
	void Continue() { next->Execute(); }
	//�m�[�h�̐؂�[
	void End() {}
	//���X�g���炱�̃C���X�^���X���폜����
	void Delete(LAYER _nodeLayer);
	//���X�g�̍폜
	inline void DeleteList();
private:
	//���X�g���q���֐��|�C���^�[
	void(RenderNode::* pConnectFunc)() = &RenderNode::End;
	//���X�g�|�C���^
	std::shared_ptr<RenderNode> back = nullptr;
	std::shared_ptr<RenderNode> next = nullptr;
private:
	//template<class Archive>
	//void serialize(Archive& ar) 
	//{
	//	ar(CEREAL_NVP(m_color), CEREAL_NVP(active)/*,CEREAL_NVP(texPath)*/);
	//}

	template <class Archive>
	void save(Archive& archive) const 
	{
		std::string path = wstring_to_string(texPath);
		archive(CEREAL_NVP(active), CEREAL_NVP(m_color), CEREAL_NVP(path));
	}

	template<class Archive>
	void load(Archive& archive)
	{
		std::string path;
		archive(CEREAL_NVP(active), CEREAL_NVP(m_color), CEREAL_NVP(path));
		Active(active);
		if (path != "null")
		{
			texPath = string_to_wstring(path);
			SetTexture(texPath.c_str());
		}
	}

	friend class cereal::access;
};

class UVRenderNode : public RenderNode
{
	friend class Renderer;
	friend class AnimationClip;
	friend class AnimationClipLoop;

private:
	UVRenderNode() {}
	UVRenderNode(const wchar_t* _texPath) :RenderNode(_texPath) {}
private:
	//�`�悵�Ď��ɂȂ�
	inline void Draw();
private:
	float m_scaleX = 0.5f;
	float m_scaleY = 0.5f;
	int m_frameX = 0;
	int m_frameY = 0;
private:
	/*template<class Archive>
	void serialize(Archive& ar) const
	{
		ar(CEREAL_NVP(m_scaleX), CEREAL_NVP(m_scaleY), CEREAL_NVP(m_frameX), CEREAL_NVP(m_frameY));
	}*/

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(CEREAL_NVP(m_scaleX), CEREAL_NVP(m_scaleY), CEREAL_NVP(m_frameX), CEREAL_NVP(m_frameY));
	}

	template<class Archive>
	void load(Archive& archive)
	{
		archive(CEREAL_NVP(m_scaleX), CEREAL_NVP(m_scaleY), CEREAL_NVP(m_frameX), CEREAL_NVP(m_frameY));
	}

	friend class cereal::access;
};

// Register the types with Cereal
CEREAL_REGISTER_TYPE(RenderNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(RenderNode, UVRenderNode)

class RenderManager final
{
	friend class Window;
	friend class RenderNode;
	friend class UVRenderNode;
	friend class Renderer;
	friend class RenderManager;
	friend class SceneManager;
	friend class Box2D::WorldManager;
	friend class Box2DBody;
	friend class Box2DBoxRenderNode;
	friend class Box2DCapsuleRenderNode;
	friend class ImGuiApp;

	using RenderList = std::pair < std::shared_ptr<RenderNode>, std::shared_ptr<RenderNode>>;

	RenderManager() = delete;
private:
	//����������
	static HRESULT Init();
	//�V�������X�g�ɓ���ւ���
	static void GenerateList();
	//���X�g�m�[�h�̈�ĕ`��
	static void Draw();
	//���X�g�Ƀm�[�h��ǉ�����
	static void AddRenderList(std::shared_ptr<RenderNode> _node, LAYER _layer);
	//�X���b�h�̌��݂̃��X�g�����̃��X�g�ɕύX
	static void ChangeNextRenderList();
	//���̃m�[�h���X�g�Ɍq��
	static void LinkNextRenderList();
	//���C���J�����̍s��ϊ����Z�b�g����
	static void SetMainCameraMatrix();
public:
	//�E�B���h�E�S�̂̈ʒu
	static Vector2 renderOffset;
	//�E�B���h�E�S�̂̊g�嗦
	static Vector2 renderZoom;
private:
	// �X���b�h���Ƃ̌��݂̃��X�g
	static thread_local RenderList* currentList;
	//���X�g�����b�N����
	static std::mutex listMutex;
	//�m�[�h���X�g(���C���[�̐�����������)
	static RenderList m_rendererList[LAYER::LAYER_MAX];
	//���̃����_�[���X�g�i��ɏ�̃��X�g�ɃR�s�[����j
	static RenderList m_nextRendererList[LAYER::LAYER_MAX];
	//���ʂ̒��_�o�b�t�@�[
	static ComPtr<ID3D11Buffer> m_vertexBuffer;
	//���ʂ̃C���f�b�N�X�o�b�t�@�[
	static ComPtr<ID3D11Buffer> m_indexBuffer;
#ifdef DEBUG_TRUE
	struct DrawRayNode
	{
		Vector2 center;
		float length;
		float radian;
		XMFLOAT4 color;
	};
	//�����蔻��̕`��
	static bool drawHitBox;
	//ray�̕`��
	static bool drawRay;
	//ray��`�悷�邽�߂̐��̒��_
	static ComPtr<ID3D11Buffer> m_lineVertexBuffer;
	//���ʂ�Ray��`�悷�邽�߂̐��̃C���f�b�N�X
	static ComPtr<ID3D11Buffer> m_lineIndexBuffer;
	//ray��`�悷�邽�߂̗v�f
	static std::vector<DrawRayNode> m_drawRayNode;
#endif
};
