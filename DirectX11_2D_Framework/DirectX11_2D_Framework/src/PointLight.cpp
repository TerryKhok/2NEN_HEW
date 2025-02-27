ComPtr<ID3D11ComputeShader> PointLight::m_lightComputeShader;
ComPtr<ID3D11PixelShader> PointLight::m_lightPixelShader;
ComPtr<ID3D11Buffer> PointLight::m_pCSLightMappingBuffer;
ComPtr<ID3D11Buffer> PointLight::m_pCSCameraBuffer;

ComPtr<ID3D11UnorderedAccessView> PointLight::lightMapUAV;
ComPtr<ID3D11ShaderResourceView> PointLight::lightMapSRV;

ComputeShaderConstants PointLight::m_computeBuffer;
ComputeCameraBuffer PointLight::m_cameraBuffer;

std::vector<PointLight*> PointLight::m_useLights;
std::vector<PointLight*> PointLight::m_newLights;

thread_local void(PointLight::*PointLight::pAddFunc)() = &PointLight::AddPointLight;

void PointLight::Start()
{
	if (m_useLights.size() >= MAX_LIGHT_NUM) return;
	
	(this->*pAddFunc)();
}

void PointLight::Update()
{
	
}

void PointLight::SetActive(bool _active)
{
	if (_active)
	{
		if (m_useLights.size() >= MAX_LIGHT_NUM || m_index >= 0) return;

		m_useLights.push_back(this);
		m_index = (int)m_useLights.size() - 1;
		return;
	}

	if (m_index < 0) return;

	int backIndex = (int)m_useLights.size() - 1;
	if (m_index != backIndex)
	{
		//インデックス番号の入れ替え
		m_useLights[backIndex]->m_index = m_index;
		std::swap(m_useLights[m_index], m_useLights[backIndex]);
	}

	m_index = -1;
	m_useLights.pop_back();
}

void PointLight::Delete()
{
	if (m_index < 0) return;

	int backIndex = (int)m_useLights.size() - 1;
	if (m_index != backIndex)
	{
		//インデックス番号の入れ替え
		m_useLights[backIndex]->m_index = m_index;
		std::swap(m_useLights[m_index], m_useLights[backIndex]);
	}

	m_index = -1;
	m_useLights.pop_back();
}

void PointLight::DrawImGui(ImGuiApp::HandleUI& _handleUi)
{
	if (m_index <  0 || m_index >= MAX_LIGHT_NUM)
	{
		ImGui::Text("light num max!!");
		return;
	}

	ImGui::Text("index : %d", m_index);
	ImGui::ColorEdit3("Color##PointLight", &m_color.x);
	ImGui::DragFloat("Intensity##PointLight", &m_intensity, 0.01f, 0.0f);
	ImGui::DragFloat("Radius##PointLight", &m_radius, 1.0f, 0.0f);
}

void PointLight::Serialize(SERIALIZE_OUTPUT& ar)
{
	ar(CEREAL_NVP(m_color), CEREAL_NVP(m_intensity), CEREAL_NVP(m_radius));
}

void PointLight::Deserialize(SERIALIZE_INPUT& ar)
{
	ar(CEREAL_NVP(m_color), CEREAL_NVP(m_intensity), CEREAL_NVP(m_radius));
}

void PointLight::SetColor(DirectX::XMFLOAT3 _color)
{
	m_color = _color;
}

void PointLight::SetIntensity(float _intensity)
{
	m_intensity = _intensity;
}

void PointLight::SetLightParamater(POINT_LIGHT& _light)
{
	auto& cameraPos = CameraManager::cameraPosition;
	auto& cameraZoom = CameraManager::cameraZoom;

	_light.pos = {
		(m_this->transform.position.x - cameraPos.x) * cameraZoom.x * DISPALY_ASPECT_WIDTH + Window::MONITER_HALF_WIDTH,
		(m_this->transform.position.y - cameraPos.y) * -cameraZoom.y * DISPALY_ASPECT_HEIGHT + Window::MONITER_HALF_HEIGHT };

	_light.color = m_color;
	_light.intensity = m_intensity;
	_light.radius = m_radius;
}


HRESULT PointLight::Init(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	for (int i = 0; i < MAX_LIGHT_NUM; i++)
	{
		m_computeBuffer.Lights[i].pos = { 0.0f,0.0f };
		m_computeBuffer.Lights[i].color = { 1.0f,1.0f,1.0f };
		m_computeBuffer.Lights[i].intensity = 1.0f;
		m_computeBuffer.Lights[i].radius = 100.0f;
	}

	//ライト用変数初期化
	//=====================================================================================================
	HRESULT hr = DirectX11::CreateComputeShader("CSLightShader.hlsl", "cs_main", "cs_5_0", m_lightComputeShader.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateComputeShader error", "error", MB_OK);
		return E_FAIL;
	}
	hr = DirectX11::CreatePixelShader("PSLightShader.hlsl", "ps_main", "ps_5_0", m_lightPixelShader.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreatePixelShader error", "error", MB_OK);
		return E_FAIL;
	}

	//セット
	_pDeviceContext->CSSetShader(m_lightComputeShader.Get(), NULL, 0);

	// ライトマップ用の UAV (Unordered Access View) テクスチャ
	ID3D11Texture2D* lightMapTex = nullptr;
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = Window::MONITER_WIDTH;  // 画面サイズ
	texDesc.Height = Window::MONITER_HEIGHT;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1;
	hr = _pDevice->CreateTexture2D(&texDesc, nullptr, &lightMapTex);
	if (FAILED(hr)) return S_FALSE;

	// UAV を作成
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	hr = _pDevice->CreateUnorderedAccessView(lightMapTex, &uavDesc, lightMapUAV.GetAddressOf());
	if (FAILED(hr)) return S_FALSE;

	// SRV (Shader Resource View) を作成（ピクセルシェーダーで利用）
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	hr = _pDevice->CreateShaderResourceView(lightMapTex, &srvDesc, lightMapSRV.GetAddressOf());
	if (FAILED(hr)) return S_FALSE;

	lightMapTex->Release();
	//=====================================================================================================

	//ライト用定数バッファ
	//========================================================================================
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(ComputeShaderConstants);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = _pDevice->CreateBuffer(&cbDesc, NULL, m_pCSLightMappingBuffer.GetAddressOf());
	if (FAILED(hr)) return hr;

	D3D11_BUFFER_DESC cbDesc1 = {};
	cbDesc1.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc1.ByteWidth = sizeof(ComputeCameraBuffer);
	cbDesc1.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc1.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = _pDevice->CreateBuffer(&cbDesc1, NULL, m_pCSCameraBuffer.GetAddressOf());
	if (FAILED(hr)) return hr;
	//========================================================================================

	return S_OK;
}

void PointLight::SetAndUpdateLight(ID3D11DeviceContext* _pDeviceContext)
{
	//バッファのデータを更新する
	//======================================================================================
	m_computeBuffer.LightCount = (int)m_useLights.size();
	for (unsigned int i = 0; i < m_computeBuffer.LightCount; i++)
	{
		m_useLights[i]->SetLightParamater(m_computeBuffer.Lights[i]);
	}

	m_cameraBuffer.pos = RenderManager::renderOffset;
	m_cameraBuffer.zoom = RenderManager::renderZoom;
	//======================================================================================

	// 1. コンピュートシェーダーでライトマップを生成
	//======================================================================================
	_pDeviceContext->CSSetShader(m_lightComputeShader.Get(), nullptr, 0);
	_pDeviceContext->PSSetShader(m_lightPixelShader.Get(), NULL, 0);

	//定数バッファをコンピュートシェーダーにセットする
	_pDeviceContext->CSSetConstantBuffers(0, 1, m_pCSLightMappingBuffer.GetAddressOf());
	//定数バッファをコンピュートシェーダーにセットする
	_pDeviceContext->CSSetConstantBuffers(1, 1, m_pCSCameraBuffer.GetAddressOf());

	//定数バッファにデータを送る(D3D11_USAGE_DYNAMICなのでMap,UnMapを使用する)
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = _pDeviceContext->Map(m_pCSLightMappingBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) {
		std::cerr << "Failed to map computeConstantBuffer!" << std::endl;
	}
	memcpy(mappedResource.pData, &m_computeBuffer, sizeof(ComputeShaderConstants));
	_pDeviceContext->Unmap(m_pCSLightMappingBuffer.Get(), 0);

	hr = _pDeviceContext->Map(m_pCSCameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) {
		std::cerr << "Failed to map computeCameraBuffer!" << std::endl;
	}
	memcpy(mappedResource.pData, &m_cameraBuffer, sizeof(ComputeCameraBuffer));
	_pDeviceContext->Unmap(m_pCSCameraBuffer.Get(), 0);

	_pDeviceContext->CSSetUnorderedAccessViews(0, 1, lightMapUAV.GetAddressOf(), nullptr);


	// スレッドグループサイズ = (画面サイズ / 8)（8x8 ピクセルごとに処理）
	static const int threadGroupX = (Window::MONITER_WIDTH + 7) / 8;
	static const int threadGroupY = (Window::MONITER_HEIGHT + 7) / 8;

	_pDeviceContext->Dispatch(threadGroupX, threadGroupY, 1);

	// 使用後は UAV を解除（これをしないと描画時にエラーになる）
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	_pDeviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

	// 2. ライトマップテクスチャをピクセルシェーダーにセット
	_pDeviceContext->PSSetShaderResources(1, 1, lightMapSRV.GetAddressOf());
	//======================================================================================
}

void PointLight::StartAsync()
{
	pAddFunc = &PointLight::AddPointLightAsync;
	m_newLights.clear();
}

void PointLight::LinkLight()
{
	//ここで削除しておく
	for (auto& light : m_useLights)
	{
		light->m_index = -1;
	}
	m_useLights.clear();

	for (auto& light : m_newLights)
	{
		if (m_useLights.size() >= MAX_LIGHT_NUM) break;

		m_useLights.push_back(light);
		light->m_index = (int)m_useLights.size() - 1;
	}

	m_newLights.clear();
}

void PointLight::AddPointLight()
{
	m_useLights.push_back(this);
	m_index = (int)m_useLights.size() - 1;
}

void PointLight::AddPointLightAsync()
{
	m_newLights.push_back(this);
}

void PointLight::SetLightPSShader(ID3D11DeviceContext* _pDeviceContext)
{
	_pDeviceContext->PSSetShader(m_lightPixelShader.Get(), NULL, 0);

	// 2. ライトマップテクスチャをピクセルシェーダーにセット
	_pDeviceContext->PSSetShaderResources(1, 1, lightMapSRV.GetAddressOf());
}

