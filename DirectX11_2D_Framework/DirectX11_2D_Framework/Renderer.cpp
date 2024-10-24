#include "Renderer.h"

thread_local RenderManager::RenderList* RenderManager::currentList = RenderManager::m_rendererList;
std::mutex RenderManager::listMutex;

Vector2 RenderManager::renderOffset = { 0.0f,0.0f };
Vector2 RenderManager::renderZoom = { 1.0f,1.0f };

RenderManager::RenderList RenderManager::m_rendererList[LAYER::LATER_MAX];
RenderManager::RenderList RenderManager::m_nextRendererList[LAYER::LATER_MAX];
ComPtr<ID3D11Buffer> RenderManager::m_vertexBuffer = nullptr;
ComPtr<ID3D11Buffer> RenderManager::m_indexBuffer = nullptr;

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

Renderer::Renderer(GameObject* _pObject,Animator* _animator)
{
	auto node = new UVRenderNode();
	node->m_object = _pObject;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, _pObject->GetLayer());

	_animator->m_uvNode = node;
}

void Renderer::SetActive(bool _active)
{
	m_node->Active(_active);
}

void Renderer::Delete()
{
	m_node->Delete();
	m_node.reset();
}

void Renderer::SetLayer(const LAYER _layer)
{
	m_node->Delete();
	RenderManager::AddRenderList(m_node, _layer);
}

void Renderer::SetUVRenderNode(Animator* _animator)
{
	m_node->Delete();
	auto node = new UVRenderNode();
	node->m_object = m_this;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, m_this->GetLayer());

	_animator->m_uvNode = node;
}

void Renderer::SetTexture(const wchar_t* _texPath)
{
	m_node->SetTexture(_texPath);
}

void Renderer::SetColor(XMFLOAT4 _color)
{
	m_node->m_color = _color;
}

void Renderer::SetTexcode(int _splitX, int _splitY, int _frameX, int _frameY)
{
	m_node->Delete();
	auto node = new UVRenderNode();
	node->m_object = m_this;
	node->m_pTextureView = m_node->m_pTextureView;
	node->m_scaleX = 1.0f / _splitX;
	node->m_scaleY = 1.0f / _splitY;
	node->m_frameX = _frameX;
	node->m_frameY = _frameY;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, m_this->GetLayer());
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
	pDrawFunc = _active ? &RenderNode::Draw : &RenderNode::VoidNext;
}

inline void RenderNode::Draw()
{
	auto& cb = m_object->GetContantBuffer();
	cb.color = m_color;

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, m_pTextureView.GetAddressOf());

	cb.uvScale = XMFLOAT2(1, 1);
	cb.uvOffset = XMFLOAT2(0, 0);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(6, 0, 0);

	//次のポインタにつなぐ
	NextFunc();
}

void RenderNode::SetTexture(const wchar_t* _texPath)
{
	TextureAssets::pLoadTexture(m_pTextureView, _texPath);
}

void RenderNode::Delete()
{
	back->next = next;
	if (next == nullptr)
	{
		back->NextEnd();

		//このノードがレイヤーの最後の場合
		auto& listEnd = RenderManager::currentList[m_object->GetLayer()].second;
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
	auto& cb = m_object->GetContantBuffer();
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


	RECT rect;
	for (auto& view : DirectX11::m_pRenderTargetViewList)
	{
		//モニターの解像度所得
		static const int monitorHalfWidth = GetSystemMetrics(SM_CXSCREEN) / 2;
		static const int monitorHalfHeight = GetSystemMetrics(SM_CYSCREEN) / 2;

		if (GetWindowRect(view.first, &rect))
		{
			CameraManager::cameraPosition = { 
				(static_cast<float>(rect.left + rect.right) / 2 - monitorHalfWidth + renderOffset.x) / renderZoom.x,
				(static_cast<float>(rect.top + rect.bottom) / -2 + monitorHalfHeight + renderOffset.y) / renderZoom.y
			};
		}

		if (GetClientRect(view.first, &rect))
		{
			CameraManager::cameraZoom.x = PROJECTION_WIDTH / static_cast<float>(rect.right) * renderZoom.x;
			CameraManager::cameraZoom.y = PROJECTION_HEIGHT / static_cast<float>(rect.bottom) * renderZoom.y;
		}

		CameraManager::SetCameraMatrix();

		// 描画先のキャンバスと使用する深度バッファを指定する
		DirectX11::m_pDeviceContext->OMSetRenderTargets(1, view.second.GetAddressOf(), DirectX11::m_pDepthStencilView.Get());

#ifndef DEBUG_TRUE
		for (int i = 0; i < LAYER::LAYER_UI; i++)
		{
			auto& node = m_rendererList[i];
			node.first->NextFunc();
		}
#else
		
		for (int i = 0; i < LAYER::LAYER_BOX2D_DEBUG; i++)
		{
			auto& node = m_rendererList[i];
			node.first->NextFunc();
		}

		DirectX11::m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		//テクスチャをピクセルシェーダーに渡す
		DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, DirectX11::m_pTextureView.GetAddressOf());

		m_rendererList[LAYER::LAYER_BOX2D_DEBUG].first->NextFunc();

		//設定戻す
		//======================================================================================================================
		DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
		DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		DirectX11::m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//======================================================================================================================
#endif
	}

	// 描画先のキャンバスと使用する深度バッファを指定する
	DirectX11::m_pDeviceContext->OMSetRenderTargets(1,
		DirectX11::m_pRenderTargetViewList.begin()->second.GetAddressOf(), DirectX11::m_pDepthStencilView.Get());

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
	for (int i = 0; i < LATER_MAX; i++)
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


