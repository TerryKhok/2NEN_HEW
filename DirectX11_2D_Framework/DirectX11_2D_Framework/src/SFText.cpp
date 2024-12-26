#ifdef SFTEXT_TRUE

#include "../font/msGothic.c"

//std::list<SFTextManager::StringNode> SFTextManager::m_stringNode;
//std::list<SFTextManager::StringNode> SFTextManager::m_oldStringNode;
DirectX::XMMATRIX SFTextManager::worldMatrix;
std::unique_ptr<SpriteBatch> SFTextManager::spriteBatch;
std::unique_ptr<SpriteFont> SFTextManager::spriteFont;
thread_local SFTextManager::SFTextNodeList* SFTextManager::currentList;
SFTextManager::SFTextNodeList SFTextManager::textNodeList;
SFTextManager::SFTextNodeList SFTextManager::nextTextNodeList;

SFText::SFText(GameObject* _object)
{
	m_node.reset(new SFTextNode(_object));
	SFTextManager::AddNode(m_node);
}

SFText::SFText(GameObject* _object, std::string _str)
{
	m_node.reset(new SFTextNode(_object));
	SFTextManager::AddNode(m_node);
	SetString(_str);
}

void SFText::SetActive(bool _active)
{
	m_node->pDrawFunc = _active ? &SFTextNode::Draw : &SFTextNode::UnDraw;
}

void SFText::Delete()
{
	m_node->Delete();
}

//void SFText::Update()
//{
//	const auto& transform = m_this->transform;
//
//	SFTextManager::m_stringNode.push_back(
//		SFTextManager::StringNode(
//			m_str,													//std::wstring str;
//			XMFLOAT2(transform.position.x * DISPALY_ASPECT_WIDTH, transform.position.y * -DISPALY_ASPECT_WIDTH),	//XMFLOAT2 pos;
//			color,													//FXMVECTOR color;
//			static_cast<float>(transform.angle.z.Get()),			//float rot;
//			XMFLOAT2(m_origin.x + offset.x, m_origin.y + offset.y), //XMFLOAT2 origin;
//			scale													//float scale;
//		)
//	);
//}

void SFText::SetString(std::string _str)
{
	float size = static_cast<float>(_str.find('\n'));

	if (size == std::string::npos)
	{
		size = static_cast<float>(_str.size());
	}

	m_node->str = string_to_wstring(_str);

	m_node->origin = DirectX::XMFLOAT2(size * 4, 0);

#ifdef DEBUG_TRUE
	memset(text, '\0', strlen(text));
	memcpy(text, _str.c_str(), _str.size());
#endif
}

void SFText::DrawImGui(ImGuiApp::HandleUI& _handle)
{
	if (ImGui::InputText("text : %s", text, sizeof(text)))
	{
		std::string str(text, strlen(text));
		SetString(std::move(str));
	}
	//ImGui::InputText("Japanese Input", input_buffer, IM_ARRAYSIZE(input_buffer));
	ImGui::ColorEdit3("color##text", m_node->color.m128_f32);
	ImGui::InputFloat2("offset", m_node->offset.data());
	ImGui::InputFloat("scale", &m_node->scale);
}

void SFTextNode::Draw()
{
	const auto& tf = object->transform;

	SFTextManager::spriteFont->DrawString(
		SFTextManager::spriteBatch.get(),
		str.c_str(),
		XMFLOAT2(tf.position.x * DISPALY_ASPECT_WIDTH, tf.position.y * -DISPALY_ASPECT_WIDTH),
		color,					
		static_cast<float>(tf.angle.z.Get()),                 
		XMFLOAT2(origin.x + offset.x, origin.y + offset.y),				
		scale,              
		DirectX::SpriteEffects_None,
		0.0                        
	);

	(this->*pConnectFunc)();
}

void SFTextNode::Delete()
{
	prevNode->nextNode = nextNode;
	if (nextNode == nullptr)
	{
		prevNode->pConnectFunc = &SFTextNode::End;

		//このノードがレイヤーの最後の場合
		auto& listEnd = SFTextManager::currentList->second;
		if (listEnd.get() == this)
		{
			listEnd = prevNode;
		}
	}
	else
	{
		nextNode->prevNode = prevNode;
	}

	prevNode = nullptr;
	nextNode = nullptr;
	pConnectFunc = &SFTextNode::End;
}

void SFTextManager::Init()
{
	spriteBatch = std::make_unique<SpriteBatch>(DirectX11::m_pDeviceContext.Get());
	spriteFont = std::make_unique<SpriteFont>(DirectX11::m_pDevice.Get(), _acmsGothic, sizeof(_acmsGothic));

	spriteBatch->SetRotation(DXGI_MODE_ROTATION_IDENTITY);// これで行列が適用されなくなります。

	worldMatrix = DirectX::XMMatrixScaling(SCREEN_WIDTH / PROJECTION_WIDTH, SCREEN_HEIGHT / PROJECTION_HEIGHT, 1); // World座標はY座標が逆になっている為修正します。
	worldMatrix *= DirectX::XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.5f);

	currentList = &textNodeList;
	textNodeList.first.reset(new SFTextNode(nullptr));
	textNodeList.second = textNodeList.first;
}

void SFTextManager::Draw()
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

		textNodeList.first->NextFunc();

		// End()関数により、描画の終了を行います。
		spriteBatch->End();
	}
	catch (const std::exception&)
	{
		throw;
	}
}

//void SFTextManager::KeepExecuteDrawString()
//{
//	//try
//	//{
//	//	// テストの為にテキストとテキスト枠を表示させます。
//	//	HRESULT    hr = S_OK;
//	//	// Begin()関数により、描画の開始を行います。
//
//	//	spriteBatch->SpriteBatch::Begin(
//	//		DirectX::SpriteSortMode_Deferred,
//	//		DirectX11::m_pBlendState.Get(),
//	//		nullptr,
//	//		nullptr,
//	//		nullptr,/*DirectX11::m_pWireframeRasterState.Get(),*///m_pWireframeRasterState,
//	//		nullptr,
//	//		worldMatrix
//	//	);    // world_view_proj行列はここで渡します。
//
//
//	//	// SpriteFontによりテキストを描画します。
//	//	for (auto& node : m_oldStringNode)
//	//	{
//	//		spriteFont->DrawString(
//	//			spriteBatch.get(),
//	//			node.str.c_str(),
//	//			node.pos,					// XMFLOAT2 const& position,
//	//			node.color,					// FXMVECTOR color = Colors::White,
//	//			node.rot,                   // float rotation = 0,
//	//			node.origin,				//*DirectX::XMFLOAT2(size * 4, 0)*/ // XMFLOAT2 const& origin = Float2Zero,
//	//			node.scale,                 // float scale = 1,
//	//			DirectX::SpriteEffects_None,// SpriteEffects effects = SpriteEffects_None,
//	//			0.0                         // float layerDepth = 0
//	//		);
//	//	}
//	//	// End()関数により、描画の終了を行います。
//	//	spriteBatch->End();
//	//}
//	//catch (const std::exception&)
//	//{
//	//	throw;
//	//}
//}

void SFTextManager::AddNode(std::shared_ptr<SFTextNode> _node)
{
	//ポインタでつなぐ
	currentList->second->nextNode = _node;
	currentList->second->pConnectFunc = &SFTextNode::Continue;

	_node->prevNode = currentList->second;

	currentList->second = _node;
}

void SFTextManager::GenerateList()
{
	textNodeList.first.reset(new SFTextNode(nullptr));
	textNodeList.second = textNodeList.first;
}

void SFTextManager::ChangeNextTextList()
{
	currentList = &nextTextNodeList;
}

void SFTextManager::LinkNextTextList()
{
	textNodeList = std::move(nextTextNodeList);
	nextTextNodeList.first.reset(new SFTextNode(nullptr));
	nextTextNodeList.second = nextTextNodeList.first;
}

#endif


