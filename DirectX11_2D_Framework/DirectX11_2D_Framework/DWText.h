#pragma once
//
//
//
//class DWTextManager
//{
//	friend class Window;
//	friend class DWText;
//
//	DWTextManager() = delete;
//
//	static void Init();
//	static void Draw();
//	static void Uninit();
//private:
//	static std::unique_ptr<DirectWriteCustomFont> write;
//	static std::vector<std::function<void()>> m_drawText;
//};
//
//class DWText : public Component
//{
//	friend class GameObject;
//
//	DWText() {}
//
//	void Update()
//	{
//		auto func = [&]() {
//			
//			};
//
//		DWTextManager::m_drawText.push_back(std::move(func));
//	}
//public:
//	void SetText(std::string _str);
//private:
//	ComPtr<IDWriteTextLayout> m_textLayout = NULL;
//};
