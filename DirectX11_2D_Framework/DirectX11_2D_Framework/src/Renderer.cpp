
thread_local RenderManager::RenderList* RenderManager::currentList = RenderManager::m_rendererList;
std::mutex RenderManager::listMutex;

Vector2 RenderManager::renderOffset = { 0.0f,0.0f };
Vector2 RenderManager::renderZoom = { 1.0f,1.0f };

RenderManager::RenderList RenderManager::m_rendererList[LAYER::LAYER_MAX];
RenderManager::RenderList RenderManager::m_nextRendererList[LAYER::LAYER_MAX];
ComPtr<ID3D11Buffer> RenderManager::m_vertexBuffer = nullptr;
ComPtr<ID3D11Buffer> RenderManager::m_indexBuffer = nullptr;

#ifdef DEBUG_TRUE
//当たり判定の描画
bool RenderManager::drawHitBox = true;
//rayの描画
bool RenderManager::drawRay = true;
//rayを描画するための線の頂点
ComPtr<ID3D11Buffer> RenderManager::m_lineVertexBuffer;
//共通のrayを描画するための線のインデックス
ComPtr<ID3D11Buffer> RenderManager::m_lineIndexBuffer;
//rayを描画するための要素
std::vector<RenderManager::DrawRayNode> RenderManager::m_drawRayNode;
#endif


Renderer::Renderer(GameObject* _pObject)
{
	RenderNode* node = new RenderNode();
	node->m_object = _pObject;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, m_layer);
}

Renderer::Renderer(GameObject* _pObject,const wchar_t* _texPath)
{
	RenderNode* node = new RenderNode(_texPath);
	node->m_object = _pObject;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, m_layer);
#ifdef DEBUG_TRUE
	m_node->texPath = _texPath;
#endif
}

Renderer::Renderer(GameObject* _pObject,Animator* _animator)
{
	auto node = new UVRenderNode();
	node->m_object = _pObject;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, m_layer);

	_animator->m_uvNode = node;
}

Renderer::Renderer(GameObject* _pObject, SERIALIZE_INPUT& ar)
{
	ar(CEREAL_NVP(m_layer), CEREAL_NVP(m_node));
	m_node->m_object = _pObject;
	RenderManager::AddRenderList(m_node, m_layer);
}

void Renderer::SetActive(bool _active)
{
	m_node->Active(_active);
}

void Renderer::Delete()
{
	m_node->Delete(m_layer);
	m_node.reset();
}

void Renderer::SetLayer(const LAYER _layer)
{
	if (m_layer == _layer) return;

	m_node->Delete(m_layer);
	RenderManager::AddRenderList(m_node, _layer);
	m_layer = _layer;
}

void Renderer::SetUVRenderNode(Animator* _animator)
{
	m_node->Delete(m_layer);
	auto node = new UVRenderNode();
	node->m_object = m_this;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, m_layer);

	_animator->m_uvNode = node;
}

void Renderer::Serialize(SERIALIZE_OUTPUT& ar)
{
	ar(CEREAL_NVP(m_layer),CEREAL_NVP(m_node));
}

void Renderer::DrawImGui(ImGuiApp::HandleUI& _handle)
{
#ifdef DEBUG_TRUE
	ImGui::Text(" Layer : %s", magic_enum::enum_name(m_layer).data());

	ImGui::Text(" path  : %s", wstring_to_string(m_node->texPath).c_str());
	ImGui::SameLine();
	if (ImGui::Button("LinkTex"))
	{
		std::string str = m_this->GetName() + "Texture";
		_handle.SetUploadFile(str, [&](GameObject* obj, std::filesystem::path path)
			{
				if (obj == nullptr || obj != m_this) return;

				auto render = obj->GetComponent<Renderer>();
				if (render != nullptr)
				{
					render->SetTexture(path.wstring().c_str());
				}
			}, { ".png" ,".jpg",".dds" });
	}
	ImGui::ColorEdit4("color", &m_node->m_color.x);
	ImVec4 color(m_node->m_color.x, m_node->m_color.y, m_node->m_color.z, m_node->m_color.w);
	ImGui::Image((ImTextureID)(m_node->m_pTextureView.Get()), ImVec2(50, 50), ImVec2(0, 0), ImVec2(1, 1), color);
#endif
}

void Renderer::SetTexture(const wchar_t* _texPath)
{
	m_node->SetTexture(_texPath);
}

void Renderer::SetTexture(const std::string& _filePath)
{
	m_node->SetTexture(_filePath);
}

void Renderer::SetColor(XMFLOAT4 _color)
{
	m_node->m_color = _color;
}

void Renderer::SetUV(int _splitX, int _splitY, int _frameX, int _frameY)
{
	m_node->Delete(m_layer);
	auto node = new UVRenderNode();
	node->m_object = m_this;
	node->m_pTextureView = m_node->m_pTextureView;
	node->m_scaleX = 1.0f / _splitX;
	node->m_scaleY = 1.0f / _splitY;
	node->m_frameX = _frameX;
	node->m_frameY = _frameY;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, m_layer);
}

RenderNode::RenderNode()
{
	m_pTextureView = DirectX11::m_pTextureView;
	NextEnd();
	Active(true);
}

RenderNode::RenderNode(const wchar_t* _texpath)
{
	TextureAssets::pLoadTexture(m_pTextureView, _texpath);

	NextEnd();
	Active(true);
}

void RenderNode::Active(bool _active)
{
	active = _active;
	pDrawFunc = active ? &RenderNode::Draw : &RenderNode::VoidNext;
}

inline void RenderNode::Draw()
{
	auto& cb = m_object->GetConstantBuffer();
	cb.color = m_color;

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, m_pTextureView.GetAddressOf());

	cb.uvScale = XMFLOAT2(1, 1);
	cb.uvOffset = XMFLOAT2(0, 0);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(6, 0, 0);

#ifdef DEBUG_TRUE
	bool flg = true;
	if (m_object->isSelected != GameObject::SELECT_NONE)
	{
		Box2DBody* rb = nullptr;
		flg = m_object->TryGetComponent(&rb);
		if (flg)
		{
			flg = !rb->m_shapeList.empty();
		}
	}
	if (!flg)
	{
		UINT strides = sizeof(Vertex);
		UINT offsets = 0;

		DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
		DirectX11::m_pDeviceContext->IASetIndexBuffer(Box2DBodyManager::m_boxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		DirectX11::m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		//テクスチャをピクセルシェーダーに渡す
		DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, DirectX11::m_pTextureView.GetAddressOf());

		//ワールド変換行列の作成
		//ー＞オブジェクトの位置・大きさ・向きを指定
		cb.world = DirectX::XMMatrixScaling(m_object->transform.scale.x + 1.0f, m_object->transform.scale.y + 1.0f, 1.0f);
		cb.world *= DirectX::XMMatrixRotationZ(static_cast<float>(m_object->transform.angle.z.Get()));
		cb.world *= DirectX::XMMatrixTranslation(m_object->transform.position.x, m_object->transform.position.y, 0.0f);
		cb.world = DirectX::XMMatrixTranspose(cb.world);
		if (m_object->isSelected == GameObject::SELECTED)
			cb.color = Box2D::b2_colorSelected;
		else if (m_object->isSelected == GameObject::ON_MOUSE)
			cb.color = Box2D::b2_colorOnMouse;

		//行列をシェーダーに渡す
		DirectX11::m_pDeviceContext->UpdateSubresource(
			DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

		DirectX11::m_pDeviceContext->DrawIndexed(8, 0, 0);

		DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
		DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		DirectX11::m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
#endif

	//次のポインタにつなぐ
	NextFunc();
}

void RenderNode::SetTexture(const wchar_t* _texPath)
{
	TextureAssets::pLoadTexture(m_pTextureView, _texPath);
	texPath = _texPath;
}

void RenderNode::SetTexture(const std::string& _filePath)
{
	TextureAssets::StbiLoad(m_pTextureView, _filePath);
	texPath = string_to_wstring(_filePath);
}

void RenderNode::Delete(LAYER _nodeLayer)
{
	back->next = next;
	if (next == nullptr)
	{
		back->NextEnd();

		//このノードがレイヤーの最後の場合
		auto& listEnd = RenderManager::currentList[_nodeLayer].second;
		if (listEnd.get() == this)
		{
			listEnd = back;
		}
	}
	else
	{
		next->back = back;
	}

	back = nullptr;
	next = nullptr;
	NextEnd();
}

inline void RenderNode::DeleteList()
{
	if (next != nullptr)
	{
		next->DeleteList();
	}

	back = nullptr;
	next = nullptr;

	//NextEnd();
}

inline void UVRenderNode::Draw()
{
	auto& cb = m_object->GetConstantBuffer();
	cb.color = m_color;

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, m_pTextureView.GetAddressOf());

	cb.uvScale = XMFLOAT2(m_scaleX, m_scaleY);
	cb.uvOffset = XMFLOAT2(m_frameX * m_scaleX, m_frameY * m_scaleY);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(6, 0, 0);

	//次のポインタにつなぐ
	NextFunc();
}



HRESULT RenderManager::Init()
{
	HRESULT  hr;

	Vector2 size = { HALF_OBJECT_SIZE,HALF_OBJECT_SIZE };
	Vertex vertexList[] =
	{
		{-size.x, size.y,0.5f,	1.0f,1.0f,1.0f,1.0f,	0.0f,0.0f},
		{ size.x, size.y,0.5f,	1.0f,1.0f,1.0f,1.0f,	1.0f,0.0f},
		{-size.x,-size.y,0.5f,	1.0f,1.0f,1.0f,1.0f,	0.0f,1.0f},
		{ size.x,-size.y,0.5f,	1.0f,1.0f,1.0f,1.0f,	1.0f,1.0f}
	};

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(vertexList);// 確保するバッファサイズを指定
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;// 頂点バッファ作成を指定
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = vertexList;// VRAMに送るデータを指定
	subResourceData.SysMemPitch = 0;
	subResourceData.SysMemSlicePitch = 0;

	hr = DirectX11::m_pDevice->CreateBuffer(&bufferDesc, &subResourceData, m_vertexBuffer.GetAddressOf());
	if (FAILED(hr)) return hr;

	WORD indexList[]{
		0, 1, 2,
		1, 3, 2,
	};

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.ByteWidth = sizeof(WORD) * 6;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA irData;
	irData.pSysMem = indexList;
	irData.SysMemPitch = 0;
	irData.SysMemSlicePitch = 0;

	hr = DirectX11::m_pDevice->CreateBuffer(&ibDesc, &irData, m_indexBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "頂点バッファー作成失敗", "エラー", MB_ICONERROR | MB_OK);

		return hr;
	}

	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	//TextureAssets::LoadTexture(m_pTextureView, DEFUALT_TEXTURE_FILEPATH);

	GenerateList();

	//リストの初期化
	for (auto& node : m_nextRendererList)
	{
		RenderNode* renderNode = new RenderNode();
		node.first = std::shared_ptr<RenderNode>(renderNode);
		node.second = node.first;
	}

#ifdef DEBUG_TRUE
	Vertex lineVertexList[] =
	{
		{-0.5f, 0.0f, 0.5f,	1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f},
		{ 0.5f, 0.0f, 0.5f,	1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f},
	};

	bufferDesc.ByteWidth = sizeof(lineVertexList);// 確保するバッファサイズを指定
	subResourceData.pSysMem = lineVertexList;// VRAMに送るデータを指定

	hr = DirectX11::m_pDevice->CreateBuffer(&bufferDesc, &subResourceData, m_lineVertexBuffer.GetAddressOf());
	if (FAILED(hr)) return hr;

	WORD lineIndexList[]{
		0, 1
	};

	ibDesc.ByteWidth = sizeof(WORD) * 2;
	irData.pSysMem = lineIndexList;

	hr = DirectX11::m_pDevice->CreateBuffer(&ibDesc, &irData, m_lineIndexBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "頂点バッファー作成失敗", "エラー", MB_ICONERROR | MB_OK);

		return hr;
	}
#endif
	
	return S_OK;
}

void RenderManager::GenerateList()
{
	//リストの初期化
	for (auto& node : m_rendererList)
	{
		RenderNode* renderNode = new RenderNode();
		node.first.reset(renderNode);
		node.second = node.first;
	}
}

void RenderManager::Draw()
{
	//フォントで変わったバッファを戻す
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

#ifdef CAMERA_ON_WINDOW
	//ズーム修正
	renderZoom.x = max(0.2f, renderZoom.x);
	renderZoom.y = max(0.2f, renderZoom.y);

	RECT rect;
#endif

#ifdef DEBUG_TRUE
	static VSObjectConstantBuffer rayCb;
#endif
	for (auto& view : DirectX11::m_pRenderTargetViewList)
	{
#ifdef CAMERA_ON_WINDOW
		if (GetWindowRect(view.first, &rect))
		{
			CameraManager::cameraPosition = { 
				(static_cast<float>(rect.left + rect.right) / 2 - Window::MONITER_HALF_WIDTH) / renderZoom.x + renderOffset.x,
				(static_cast<float>(rect.top + rect.bottom) / -2 + Window::MONITER_HALF_HEIGHT) / renderZoom.y + renderOffset.y
			};
		}

		if (GetClientRect(view.first, &rect))
		{
			rect.right = max(rect.right, 1);
			rect.bottom = max(rect.bottom, 1);
			CameraManager::cameraZoom.x = PROJECTION_WIDTH / static_cast<float>(rect.right) * renderZoom.x;
			CameraManager::cameraZoom.y = PROJECTION_HEIGHT / static_cast<float>(rect.bottom) * renderZoom.y;
		}

		CameraManager::SetCameraMatrix();
#endif

		// 描画先のキャンバスと使用する深度バッファを指定する
		DirectX11::m_pDeviceContext->OMSetRenderTargets(1, view.second.first.GetAddressOf(), DirectX11::m_pDepthStencilView.Get());

#ifndef DEBUG_TRUE
		for (const auto layer : view.second.second)
		{
			auto& node = m_rendererList[layer];
			node.first->NextFunc();
		}
#else
		/*for (int i = 0; i < LAYER::LAYER_BOX2D_DEBUG; i++)
		{
			auto& node = m_rendererList[i];
			node.first->NextFunc();
		}*/

		for (const auto layer : view.second.second)
		{
			auto& node = m_rendererList[layer];
			node.first->NextFunc();
		}

		if (drawHitBox)
		{
			DirectX11::m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			//テクスチャをピクセルシェーダーに渡す
			DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, DirectX11::m_pTextureView.GetAddressOf());

			m_rendererList[LAYER::LAYER_BOX2D_DEBUG].first->NextFunc();
		}
		
		if (drawRay)
		{
			//rayの描画
			DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_lineVertexBuffer.GetAddressOf(), &strides, &offsets);
			DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_lineIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

			for (auto& rayNode : m_drawRayNode)
			{
				//ワールド変換行列の作成
				//ー＞オブジェクトの位置・大きさ・向きを指定
				rayCb.world = DirectX::XMMatrixScaling(rayNode.length, 1.0f, 1.0f);
				rayCb.world *= DirectX::XMMatrixRotationZ(rayNode.radian);
				rayCb.world *= DirectX::XMMatrixTranslation(rayNode.center.x, rayNode.center.y, 0.5f);
				rayCb.world = DirectX::XMMatrixTranspose(rayCb.world);
				rayCb.color = rayNode.color;

				//行列をシェーダーに渡す
				DirectX11::m_pDeviceContext->UpdateSubresource(
					DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &rayCb, 0, 0);

				DirectX11::m_pDeviceContext->DrawIndexed(2, 0, 0);
			}
		}

		//設定戻す
		//======================================================================================================================
		DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
		DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		DirectX11::m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//======================================================================================================================
#endif
	}

#ifdef DEBUG_TRUE
	m_drawRayNode.clear();
#endif

	// 描画先のキャンバスと使用する深度バッファを指定する
	DirectX11::m_pDeviceContext->OMSetRenderTargets(1,
		DirectX11::m_pRenderTargetViewList[Window::GetMainHWnd()].first.GetAddressOf(), DirectX11::m_pDepthStencilView.Get());

	static VSCameraConstantBuffer cb = {
			XMMatrixIdentity(),
			DirectX::XMMatrixOrthographicLH(PROJECTION_WIDTH, PROJECTION_HEIGHT, 0.0f, 5.0f)
	};

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSCameraConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	m_rendererList[LAYER::LAYER_UI].first->NextFunc();
}

void RenderManager::AddRenderList(std::shared_ptr<RenderNode> _node, LAYER _layer)
{
	std::lock_guard<std::mutex> lock(listMutex);  // Protect access to arrayB if necessary

	//ポインタでつなぐ
	currentList[_layer].second->next = _node;
	currentList[_layer].second->NextContinue();

	_node->back = currentList[_layer].second;

	currentList[_layer].second = _node;
}

void RenderManager::ChangeNextRenderList()
{
	currentList = m_nextRendererList;
}

void RenderManager::LinkNextRenderList()
{
	//リストのコピー
	for (int i = 0; i < LAYER_MAX; i++)
	{
		m_rendererList[i] = m_nextRendererList[i];
	}

	//リストの初期化
	for (auto& node : m_nextRendererList)
	{
		RenderNode* renderNode = new RenderNode();
		node.first = std::shared_ptr<RenderNode>(renderNode);
		node.second = node.first;
	}
}

void RenderManager::SetMainCameraMatrix()
{
	RECT rect;
	if (GetWindowRect(Window::GetMainHWnd(), &rect))
	{
		CameraManager::cameraPosition = {
			(static_cast<float>(rect.left + rect.right) / 2 - Window::MONITER_HALF_WIDTH) / renderZoom.x + renderOffset.x,
			(static_cast<float>(rect.top + rect.bottom) / -2 + Window::MONITER_HALF_HEIGHT) / renderZoom.y + renderOffset.y
		};
	}

	if (GetClientRect(Window::GetMainHWnd(), &rect))
	{
		rect.right = max(rect.right, 1);
		rect.bottom = max(rect.bottom, 1);
		CameraManager::cameraZoom.x = PROJECTION_WIDTH / static_cast<float>(rect.right) * renderZoom.x;
		CameraManager::cameraZoom.y = PROJECTION_HEIGHT / static_cast<float>(rect.bottom) * renderZoom.y;
	}

	CameraManager::SetCameraMatrix();
}

