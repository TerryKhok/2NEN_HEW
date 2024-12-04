#pragma once


//�e�N�X�`���̎w�肪�Ȃ��������̑�p�e�N�X�`��
//============================================================================
#define DEFUALT_TEXTURE_FILEPATH L"asset/pic/noTextureImg.png"
//============================================================================

class Renderer : public Component
{
	friend class GameObject;
	friend class Animator;
	
private:
	//�����֎~
	Renderer(GameObject* _pObject);
	Renderer(GameObject* _pObject,const wchar_t* _texpath);
	Renderer(GameObject* _pObject,Animator* _animator);
	//�A�N�e�B�u�ύX
	void SetActive(bool _active);
	//�Ή������m�[�h�̍폜
	void Delete();
	//UVRenderNode�֐؂�ւ���
	void SetUVRenderNode(Animator* _animator);
	//imGui�̕`��
	void DrawImGui() override;

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
private:
	friend class cereal::access;
	//Restrict access to the serialize function
	template <class Archive>
	void serialize(Archive& ar) {
		//ar(CEREAL_NVP(id), CEREAL_NVP(name));
	}

	// Static method for load_and_construct
	template <class Archive>
	static void load_and_construct(Archive& ar, cereal::construct<Renderer>& construct) {
		GameObject* object;
		construct(object);           // Construct the Texture object
	}
};

REGISTER_COMPONENT(Renderer)

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
	virtual void Active(bool _active);
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
	void(RenderNode::* pDrawFunc)(void) = nullptr;
protected:
	GameObject* m_object = nullptr;
	XMFLOAT4 m_color = { 1.0f,1.0f,1.0f,1.0f };
	ComPtr<ID3D11ShaderResourceView> m_pTextureView = nullptr;

#ifdef DEBUG_TRUE
	//�e�N�X�`���p�X
	std::string texPath = "not texture path";
#endif

	//���X�g�֘A
protected:
	//�֐��|�C���^�[�̎��s
	void NextFunc() { (this->*pFunc)(); }
	//���X�g�̎���ݒ肷��
	void NextContinue() { pFunc = &RenderNode::Continue; }
	//���X�g�̏I����ݒ肷��
	void NextEnd() { pFunc = &RenderNode::End; }
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
	void(RenderNode::* pFunc)() = nullptr;
	//���X�g�|�C���^
	std::shared_ptr<RenderNode> back = nullptr;
	std::shared_ptr<RenderNode> next = nullptr;
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
};

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
	static RenderList m_rendererList[LAYER::LATER_MAX];
	//���̃����_�[���X�g�i��ɏ�̃��X�g�ɃR�s�[����j
	static RenderList m_nextRendererList[LAYER::LATER_MAX];
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
