#ifdef SFTEXT_TRUE

#include "../font/msGothic.c"

std::list<SFTextManager::StringNode> SFTextManager::m_stringNode;
std::list<SFTextManager::StringNode> SFTextManager::m_oldStringNode;
DirectX::XMMATRIX SFTextManager::worldMatrix;
std::unique_ptr<SpriteBatch> SFTextManager::spriteBatch;
std::unique_ptr<SpriteFont> SFTextManager::spriteFont;

// string��wstring�֕ϊ�����
std::wstring StringToWString(std::string oString)
{
	// SJIS �� wstring
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, (wchar_t*)NULL, 0);

	// �o�b�t�@�̎擾
	wchar_t* cpUCS2 = new wchar_t[iBufferSize];

	// SJIS �� wstring
	MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2, iBufferSize);

	// string�̐���
	std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

	// �o�b�t�@�̔j��
	delete[] cpUCS2;

	// �ϊ����ʂ�Ԃ�
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

	spriteBatch->SetRotation(DXGI_MODE_ROTATION_IDENTITY);// ����ōs�񂪓K�p����Ȃ��Ȃ�܂��B

	worldMatrix = DirectX::XMMatrixScaling(SCREEN_WIDTH / PROJECTION_WIDTH, SCREEN_HEIGHT / PROJECTION_HEIGHT, 1); // World���W��Y���W���t�ɂȂ��Ă���׏C�����܂��B
	worldMatrix *= DirectX::XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.5f);
}

void SFTextManager::ExecuteDrawString()
{
	try
	{
		// �e�X�g�ׂ̈Ƀe�L�X�g�ƃe�L�X�g�g��\�������܂��B
		HRESULT    hr = S_OK;
		// Begin()�֐��ɂ��A�`��̊J�n���s���܂��B

		spriteBatch->SpriteBatch::Begin(
			DirectX::SpriteSortMode_Deferred,
			DirectX11::m_pBlendState.Get(),
			nullptr,
			nullptr,
			nullptr,/*DirectX11::m_pWireframeRasterState.Get(),*///m_pWireframeRasterState,
			nullptr,
			worldMatrix
		);    // world_view_proj�s��͂����œn���܂��B


		// SpriteFont�ɂ��e�L�X�g��`�悵�܂��B
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

		// End()�֐��ɂ��A�`��̏I�����s���܂��B
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
		// �e�X�g�ׂ̈Ƀe�L�X�g�ƃe�L�X�g�g��\�������܂��B
		HRESULT    hr = S_OK;
		// Begin()�֐��ɂ��A�`��̊J�n���s���܂��B

		spriteBatch->SpriteBatch::Begin(
			DirectX::SpriteSortMode_Deferred,
			DirectX11::m_pBlendState.Get(),
			nullptr,
			nullptr,
			nullptr,/*DirectX11::m_pWireframeRasterState.Get(),*///m_pWireframeRasterState,
			nullptr,
			worldMatrix
		);    // world_view_proj�s��͂����œn���܂��B


		// SpriteFont�ɂ��e�L�X�g��`�悵�܂��B
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
		// End()�֐��ɂ��A�`��̏I�����s���܂��B
		spriteBatch->End();
	}
	catch (const std::exception&)
	{
		throw;
	}
}

#endif