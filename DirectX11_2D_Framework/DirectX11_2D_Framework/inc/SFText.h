#pragma once

#define SFTEXT_TRUE

#ifdef SFTEXT_TRUE

class SFTextNode;

class SFText :public Component
{
	friend class GameObject;

	static constexpr int maxStringNum = 256;

private:
	SFText(GameObject* _object);
	SFText(GameObject* _object,std::string _str);
	~SFText() = default;

	void SetActive(bool _active) override;
	void Delete() override;
public:
	void SetString(std::string _str);
	/*XMVECTOR color = Colors::White;
	Vector2 offset = { 0.0f,0.0f };
	float scale = 1.0f;*/
private:
	std::shared_ptr<SFTextNode> m_node;
	//std::wstring m_str = L" ";
	//XMFLOAT2 m_origin = DirectX::XMFLOAT2(0, 0);
private:
#ifdef DEBUG_TRUE
	char text[maxStringNum];

	void DrawImGui(ImGuiApp::HandleUI& _handle) override;
#endif
};

class SFTextNode
{
	friend class SFText;
	friend class SFTextNode;
	friend class SFTextManager;

	SFTextNode(GameObject* _object) :object(_object) {}
private:
	void(SFTextNode::* pDrawFunc)(void) = &SFTextNode::Draw;
	void(SFTextNode::* pConnectFunc)(void) = &SFTextNode::End;
	std::shared_ptr<SFTextNode> nextNode = nullptr;
	std::shared_ptr<SFTextNode> prevNode = nullptr;
private:
	void Execute() { (this->*pDrawFunc)();}
	void NextFunc() { (this->*pConnectFunc)(); }
	void Draw();
	void UnDraw() { (this->*pConnectFunc)(); }
	void Continue() { nextNode->Execute(); }
	void End(){}
	void Delete();
private:
	GameObject* object;
	std::wstring str = L" ";
	XMVECTOR color = Colors::White;
	Vector2 offset = { 0.0f,0.0f };
	float scale = 1.0f;
	XMFLOAT2 origin = DirectX::XMFLOAT2(0, 0);
};

class SFTextManager
{
	friend class Window;
	friend class SFText;
	friend class SFTextNode;
	friend class SceneManager;

	/*struct StringNode
	{
		std::wstring str;
		XMFLOAT2 pos;
		FXMVECTOR color;
		float rot;
		XMFLOAT2 origin;
		float scale;
	};*/

	using SFTextNodeList = std::pair<std::shared_ptr<SFTextNode>, std::shared_ptr<SFTextNode>>;
private:
	//����������
	static void Init();
	//�v���̂������������\��
	static void Draw();
	//���X�g���폜���Ȃ�
	//static void KeepExecuteDrawString();
	//���X�g�ɗv�f��ǉ�����
	static void AddNode(std::shared_ptr<SFTextNode> _node);
	//�V�������X�g�𐶐�����
	static void GenerateList();
	//�X���b�h�̌��݂̃��X�g�����̃��X�g�ɕύX����
	static void ChangeNextTextList();
	//���̃��X�g�������N����
	static void LinkNextTextList();
private:
	//static std::list<StringNode> m_stringNode;
	//static std::list<StringNode> m_oldStringNode;
	static DirectX::XMMATRIX worldMatrix;
	static std::unique_ptr<SpriteBatch> spriteBatch;
	static std::unique_ptr<SpriteFont> spriteFont;
	static thread_local SFTextNodeList* currentList;
	static SFTextNodeList textNodeList;
	static SFTextNodeList nextTextNodeList;
};

#endif
