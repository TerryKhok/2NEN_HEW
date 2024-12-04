#ifdef SFTEXT_TRUE

#include "../font/msGothic.c"

std::list<SFTextManager::StringNode> SFTextManager::m_stringNode;
std::list<SFTextManager::StringNode> SFTextManager::m_oldStringNode;
DirectX::XMMATRIX SFTextManager::worldMatrix;
std::unique_ptr<SpriteBatch> SFTextManager::spriteBatch;
std::unique_ptr<SpriteFont> SFTextManager::spriteFont;

// stringをwstringへ変換する
std::wstring StringToWString(std::string oString)
{
	// SJIS → wstring
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, (wchar_t*)NULL, 0);

	// バッファの取得
	wchar_t* cpUCS2 = new wchar_t[iBufferSize];

	// SJIS → wstring
	MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2, iBufferSize);

	// stringの生成
	std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

	// バッファの破棄
	delete[] cpUCS2;

	// 変換結果を返す
	return(oRet);
}

void SFText::Update()
{
	const auto& transform = m_this->transform;

	SFTextManager::m_stringNode.push_back(
		SFTextManager::StringNode(
			m_str,													//std::wstring str;
			XMFLOAT2(transform.position.x * DISPALY_ASPECT_WIDTH, transform.position.y * -DISPALY_ASPECT_WIDTH),	//XMFLOAT2 pos;
			color,													//FXMVECTOR color;
			static_cast<float>(transform.angle.z.Get()),			//float rot;
			XMFLOAT2(m_origin.x + offset.x, m_origin.y + offset.y), //XMFLOAT2 origin;
			scale													//float scale;
		)
	);
}

void SFText::SetString(std::string _str)
{
	float size = static_cast<float>(_str.find('\n'));

	if (size == std::string::npos)
	{
		size = static_cast<float>(_str.size());
	}

	m_str = StringToWString(_str);

	m_origin = DirectX::XMFLOAT2(size * 4, 0);

#ifdef DEBUG_TRUE
	text = _str;
#endif
}

void SFTextManager::Init()
{
	spriteBatch = std::make_unique<SpriteBatch>(DirectX11::m_pDeviceContext.Get());
	spriteFont = std::make_unique<SpriteFont>(DirectX11::m_pDevice.Get(), _acmsGothic, sizeof(_acmsGothic));

	spriteBatch->SetRotation(DXGI_MODE_ROTATION_IDENTITY);// これで行列が適用されなくなります。

	worldMatrix = DirectX::XMMatrixScaling(SCREEN_WIDTH / PROJECTION_WIDTH, SCREEN_HEIGHT / PROJECTION_HEIGHT, 1); // World座標はY座標が逆になっている為修正します。
	worldMatrix *= DirectX::XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.5f);
}

void SFTextManager::ExecuteDrawString()
{
	try
	{
		// テストの為にテキストとテキスト枠を表示させます。
		HRESULT    hr = S_OK;
		// Begin()関数により、描画の開始を行います。

		spriteBatch->SpriteBatch::Begin(
			DirectX::SpriteSortMode_Deferred,
			DirectX11::m_pBlendState.Get(),
			nullptr,
			nullptr,
			nullptr,/*DirectX11::m_pWireframeRasterState.Get(),*///m_pWireframeRasterState,
			nullptr,
			worldMatrix
		);    // world_view_proj行列はここで渡します。


		// SpriteFontによりテキストを描画します。
		for (auto& node : m_stringNode)
		{
			spriteFont->DrawString(
				spriteBatch.get(),
				node.str.c_str(),
				node.pos,					// XMFLOAT2 const& position,
				node.color,					// FXMVECTOR color = Colors::White,
				node.rot,                   // float rotation = 0,
				node.origin,				//*DirectX::XMFLOAT2(size * 4, 0)*/ // XMFLOAT2 const& origin = Float2Zero,
				node.scale,                 // float scale = 1,
				DirectX::SpriteEffects_None,// SpriteEffects effects = SpriteEffects_None,
				0.0                         // float layerDepth = 0
			);
		}

		m_oldStringNode = std::move(m_stringNode);

		// End()関数により、描画の終了を行います。
		spriteBatch->End();
	}
	catch (const std::exception&)
	{
		throw;
	}
}

void SFTextManager::KeepExecuteDrawString()
{
	try
	{
		// テストの為にテキストとテキスト枠を表示させます。
		HRESULT    hr = S_OK;
		// Begin()関数により、描画の開始を行います。

		spriteBatch->SpriteBatch::Begin(
			DirectX::SpriteSortMode_Deferred,
			DirectX11::m_pBlendState.Get(),
			nullptr,
			nullptr,
			nullptr,/*DirectX11::m_pWireframeRasterState.Get(),*///m_pWireframeRasterState,
			nullptr,
			worldMatrix
		);    // world_view_proj行列はここで渡します。


		// SpriteFontによりテキストを描画します。
		for (auto& node : m_oldStringNode)
		{
			spriteFont->DrawString(
				spriteBatch.get(),
				node.str.c_str(),
				node.pos,					// XMFLOAT2 const& position,
				node.color,					// FXMVECTOR color = Colors::White,
				node.rot,                   // float rotation = 0,
				node.origin,				//*DirectX::XMFLOAT2(size * 4, 0)*/ // XMFLOAT2 const& origin = Float2Zero,
				node.scale,                 // float scale = 1,
				DirectX::SpriteEffects_None,// SpriteEffects effects = SpriteEffects_None,
				0.0                         // float layerDepth = 0
			);
		}
		// End()関数により、描画の終了を行います。
		spriteBatch->End();
	}
	catch (const std::exception&)
	{
		throw;
	}
}

#endif