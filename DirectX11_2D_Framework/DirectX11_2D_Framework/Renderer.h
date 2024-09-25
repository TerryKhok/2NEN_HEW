#pragma once


//�e�N�X�`���̎w�肪�Ȃ��������̑�p�e�N�X�`��
//============================================================================
#define DEFUALT_TEXTURE_FILEPATH L"asset/pic/noTextureImg.png"
//============================================================================


class Renderer : public Component
{
	friend class GameObject;
private:
	Renderer(GameObject* _pObject);
	Renderer(GameObject* _pObject,const wchar_t* texpath);
public:
	//�Ή������m�[�h�̍폜
	void Delete();
private:
	//�Ή������m�[�h
	std::shared_ptr<RenderNode> m_node;
};

//�o�������X�g�m�[�h
class RenderNode
{
	friend class Renderer;
	friend class RenderManager;

	//�`��֘A
private:
	RenderNode();
	RenderNode(const wchar_t* texpath);
	inline void Draw();
private:
	GameObject* m_object = nullptr;
	XMFLOAT4 m_color = { 1.0f,1.0f,1.0f,1.0f };
	ComPtr<ID3D11ShaderResourceView> m_pTextureView = nullptr;

	//���X�g�֘A
public:
	//�֐��|�C���^�[�̎��s
	void Excute() { (this->*pFunc)(); }
	//���X�g���q���֐�
	void(RenderNode::* pFunc)() = nullptr;
	//���X�g�̎���ݒ肷��
	void NextContinue() { pFunc = &RenderNode::Continue; }
	//���X�g�̏I����ݒ肷��
	void NextEnd() { pFunc = &RenderNode::End; }
	//���̃m�[�h�̕`�������
	void Continue() { next->Draw(); }
	//�m�[�h�̐؂�[
	void End() {}
	//���X�g���炱�̃C���X�^���X���폜����
	void Delete();
private:
	//���X�g�|�C���^
	std::shared_ptr<RenderNode> back = nullptr;
	std::shared_ptr<RenderNode> next = nullptr;
};

class RenderManager
{
	friend class Window;
	friend class RenderNode;
	friend class Renderer;

	RenderManager() = delete;
private:
	//����������
	static HRESULT Init();
	//���X�g�m�[�h�̈�ĕ`��
	static void Draw();
	//���X�g�Ƀm�[�h��ǉ�����
	static void AddRenderList(std::shared_ptr<RenderNode> _node, LAYER _layer);
private:
	//�m�[�h���X�g(���C���[�̐�����������)
	static std::pair < std::shared_ptr<RenderNode>, std::shared_ptr<RenderNode>> m_rendererList[LAYER::LATER_MAX];
	//���ʂ̒��_�o�b�t�@�[
	static ComPtr<ID3D11Buffer> m_vertexBuffer;
	//���ʂ̃C���f�b�N�X�o�b�t�@�[
	static ComPtr<ID3D11Buffer> m_indexBuffer;
	//�w��Ȃ��̃e�N�X�`���r���[
	static ComPtr<ID3D11ShaderResourceView> m_pTextureView;
};
