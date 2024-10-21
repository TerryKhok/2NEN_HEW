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
	//�폜�֎~
	~Renderer() = default;
	//�A�N�e�B�u�ύX
	void SetActive(bool _active);
	//�Ή������m�[�h�̍폜
	void Delete();
	//���C���[�̕ύX
	void SetLayer(const LAYER _layer);
	//Debug�p�̃|�C���^�[�擾
	Renderer* GetDevide(){
		return this;
	}
	//UVRenderNode�֐؂�ւ���
	void SetUVRenderNode(Animator* _animator);
public:
	void SetTexture(const wchar_t* _texPath);
	void SetColor(XMFLOAT4 _color);
	void SetTexcode(int _splitX, int _splitY, int _frameX, int _frameY);
private:
	//�Ή������m�[�h
	std::shared_ptr<RenderNode> m_node;
};

//�o�������X�g�m�[�h
class RenderNode
{
	friend class Renderer;
	friend class RenderManager;
	friend class Box2DBody;

	//�`��֘A
protected:
	RenderNode();
	RenderNode(const wchar_t* _texpath);
	//�A�N�e�B�u��؂�ւ���
	virtual void Active(bool _active);
	//�`��֐��̎��s
	virtual void Excute() { (this->*pDrawFunc)(); }
	//�`�悵�Ď��ɂȂ�
	virtual inline void Draw();
	//�Ȃɂ������Ɏ��ɂȂ�
	void VoidNext() { NextFunc(); }
	//�e�N�X�`���[�̐ݒ�
	void SetTexture(const wchar_t* _texpath);
private:
	//�`��֐��|�C���^�[
	void(RenderNode::* pDrawFunc)(void) = nullptr;
protected:
	GameObject* m_object = nullptr;
	XMFLOAT4 m_color = { 1.0f,1.0f,1.0f,1.0f };
	ComPtr<ID3D11ShaderResourceView> m_pTextureView = nullptr;

	//���X�g�֘A
protected:
	//�֐��|�C���^�[�̎��s
	void NextFunc() { (this->*pFunc)(); }
	//���X�g�̎���ݒ肷��
	void NextContinue() { pFunc = &RenderNode::Continue; }
	//���X�g�̏I����ݒ肷��
	void NextEnd() { pFunc = &RenderNode::End; }
	//���̃m�[�h�̕`�������
	void Continue() { next->Excute(); }
	//�m�[�h�̐؂�[
	void End() {}
	//���X�g���炱�̃C���X�^���X���폜����
	void Delete();	
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
	friend class Box2DBody;
	friend class Box2DBoxRenderNode;
	friend class Box2DCapsuleRenderNode;

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
};
