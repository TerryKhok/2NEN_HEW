
std::pair<std::shared_ptr<RenderNode>, std::shared_ptr<RenderNode>> RenderManager::m_rendererList[LAYER::LATER_MAX];
ComPtr<ID3D11Buffer> RenderManager::m_vertexBuffer = nullptr;
ComPtr<ID3D11Buffer> RenderManager::m_indexBuffer = nullptr;
ComPtr<ID3D11ShaderResourceView> RenderManager::m_pTextureView = nullptr;

Renderer::Renderer(GameObject* _pObject)
{
	RenderNode* node = new RenderNode();
	node->m_object = _pObject;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, _pObject->GetLayer());
}

Renderer::Renderer(GameObject* _pObject,const wchar_t* texpath)
{
	RenderNode* node = new RenderNode(texpath);
	node->m_object = _pObject;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, _pObject->GetLayer());
}

void Renderer::Delete()
{
	m_node->Delete();
	m_node.reset();
}

RenderNode::RenderNode()
{
	m_pTextureView = RenderManager::m_pTextureView;
	NextEnd();
}

RenderNode::RenderNode(const wchar_t* texpath)
{
	TextureAssets::LoadTexture(m_pTextureView, texpath);
	NextEnd();
}

void RenderNode::Draw()
{
	auto& cb = m_object->GetContantBuffer();
	cb.color = m_color;

	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, m_pTextureView.GetAddressOf());

	cb.tex = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	cb.tex = DirectX::XMMatrixTranspose(cb.tex);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(6, 0, 0);

	//次のポインタにつなぐ
	Excute();
}

void RenderNode::Delete()
{
	m_object = nullptr;
	back->next = next;
	if (next == nullptr)
	{
		back->NextEnd();
	}
	else
	{
		next->back = back;
	}
}


HRESULT RenderManager::Init()
{
	HRESULT  hr;

	Vector2 size = { DEFULT_OBJECT_SIZE / 2,DEFULT_OBJECT_SIZE / 2 };
	Vertex vertexList[] =
	{
		{-size.x, size.y,0.5f,1.0f,1.0f,1.0f,1.0f,	0.0f,		0.0f},
		{ size.x, size.y,0.5f,1.0f,1.0f,1.0f,1.0f,	1.0f / 1.0f,	0.0f},
		{-size.x,-size.y,0.5f,1.0f,1.0f,1.0f,1.0f,	0.0f,		1.0f / 1.0f},
		{ size.x,-size.y,0.5f,1.0f,1.0f,1.0f,1.0f,	1.0f / 1.0f,	1.0f / 1.0f}
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

	TextureAssets::LoadTexture(m_pTextureView, DEFUALT_TEXTURE_FILEPATH);

	//リストの初期化
	for (auto& node : m_rendererList)
	{
		RenderNode* renderNode = new RenderNode();
		node.first = std::shared_ptr<RenderNode>(renderNode);
		node.second = node.first;
	}
	
	return S_OK;
}

void RenderManager::Draw()
{
	for (auto& node : m_rendererList)
	{
		node.first->Excute();
	}
}

void RenderManager::AddRenderList(std::shared_ptr<RenderNode> _node, LAYER _layer)
{
	//ポインタでつなぐ
	m_rendererList[_layer].second->next = _node;
	m_rendererList[_layer].second->NextContinue();

	_node->back = m_rendererList[_layer].second;

	m_rendererList[_layer].second = _node;
}
