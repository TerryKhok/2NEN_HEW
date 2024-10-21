//#include "DWText.h"
//
//std::unique_ptr<DirectWriteCustomFont> DWTextManager::write = nullptr;
//std::vector<std::function<void()>> DWTextManager::m_drawText;
//
//void DWTextManager::Init()
//{
//	FontData data;
//
//	write.reset(new DirectWriteCustomFont(&data));
//	write->Init(DirectX11::m_pSwapChain.Get());
//
//	write->GetFontFamilyName(write->fontCollection.Get(), L"ja-JP");
//
//	data.fontSize = 60;
//	data.fontWeight = DWRITE_FONT_WEIGHT_ULTRA_BLACK;
//	data.Color = D2D1::ColorF(D2D1::ColorF::White);
//	data.font = write->GetFontName(0);
//
//	write->SetFont(data);
//}
//
//void DWTextManager::Draw()
//{
//	for (auto& node : m_drawText)
//	{
//		node();
//	}
//	m_drawText.clear();
//}
//
//void DWTextManager::Uninit()
//{
//	//delete write;
//}
//
//void DWText::SetText(std::string _str)
//{
//}
//
