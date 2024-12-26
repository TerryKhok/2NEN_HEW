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
	//初期化処理
	static void Init();
	//要求のあった文字列を表示
	static void Draw();
	//リストを削除しない
	//static void KeepExecuteDrawString();
	//リストに要素を追加する
	static void AddNode(std::shared_ptr<SFTextNode> _node);
	//新しいリストを生成する
	static void GenerateList();
	//スレッドの現在のリストを次のリストに変更する
	static void ChangeNextTextList();
	//次のリストをリンクする
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
