#pragma once

#define SFTEXT_TRUE

#ifdef SFTEXT_TRUE

// stringをwstringへ変換する
std::wstring StringToWString(std::string oString);

class SFText :public Component
{
	friend class GameObject;

private:
	SFText(){}
	SFText(std::string _str) {
		SetString(_str);
	}
	~SFText() = default;

	void Update() override;
public:
	void SetString(std::string _str);
	XMVECTOR color = Colors::White;
	Vector2 offset = { 0.0f,0.0f };
	float scale = 1.0f;
private:
	std::wstring m_str = L" ";
	XMFLOAT2 m_origin = DirectX::XMFLOAT2(0, 0);
private:
#ifdef DEBUG_TRUE
	std::string text;

	void DrawImGui() override
	{
		ImGui::Text("text : %s", text.c_str());
		static char input_buffer[256] = "";
		ImGui::InputText("Japanese Input", input_buffer, IM_ARRAYSIZE(input_buffer));
		ImGui::ColorEdit3("color", color.m128_f32);
		ImGui::InputFloat2("offset", offset.data());
		ImGui::InputFloat("scale", &scale);
	}
#endif
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
	//初期化処理
	static void Init();
	//要求のあった文字列を表示
	static void ExecuteDrawString();
	//リストを削除しない
	static void KeepExecuteDrawString();
private:
	static std::list<StringNode> m_stringNode;
	static std::list<StringNode> m_oldStringNode;
	static DirectX::XMMATRIX worldMatrix;
	static std::unique_ptr<SpriteBatch> spriteBatch;
	static std::unique_ptr<SpriteFont> spriteFont;
};

#endif
