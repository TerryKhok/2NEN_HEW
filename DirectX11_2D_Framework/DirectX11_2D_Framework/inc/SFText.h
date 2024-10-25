#pragma once

#define SFTEXT_TRUE

#ifdef SFTEXT_TRUE

// string��wstring�֕ϊ�����
std::wstring StringToWString(std::string oString);

class SFText :public Component
{
	friend class GameObject;

private:
	SFText(){}
	SFText(std::string _str) {
		SetString(_str);
	}
	void Update() override;
	void SetString(std::string _str);
public:
	XMVECTOR color = Colors::White;
	Vector2 offset = { 0,0 };
	float scale = 1.0f;
private:
	std::wstring m_str = L" ";
	XMFLOAT2 m_origin = DirectX::XMFLOAT2(0, 0);
};

class SFTextManager
{
	friend class Window;
	friend class SFText;

	struct StringNode
	{
		std::wstring str;
		XMFLOAT2 pos;
		FXMVECTOR color;
		float rot;
		XMFLOAT2 origin;
		float scale;
	};
private:
	//����������
	static void Init();
	//�v���̂������������\��
	static void ExcuteDrawString();
	//���X�g���폜���Ȃ�
	static void KeepExcuteDrawString();
private:
	static std::vector<StringNode> m_stringNode;
	static std::vector<StringNode> m_oldStringNode;
	static DirectX::XMMATRIX worldMatrix;
	static std::unique_ptr<SpriteBatch> spriteBatch;
	static std::unique_ptr<SpriteFont> spriteFont;
};

#endif
