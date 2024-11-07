#include "shaders.h"

// ※ID3D11で始まるポインタ型の変数は、解放する必要がある
ComPtr<ID3D11Device> DirectX11::m_pDevice = nullptr; // デバイス＝DirectXの各種機能を作る
//スワップチェーンだけを生成するため(生成用インス
ComPtr<IDXGIFactory> DirectX11::m_pDxgiFactory;
// コンテキスト＝描画関連を司る機能
ComPtr<ID3D11DeviceContext> DirectX11::m_pDeviceContext = nullptr;
// スワップチェイン＝ダブルバッファ機能
std::unordered_map<HWND, ComPtr<IDXGISwapChain>> DirectX11::m_pSwapChainList;
// レンダーターゲット＝描画先を表す機能
std::unordered_map<HWND, ComPtr<ID3D11RenderTargetView>> DirectX11::m_pRenderTargetViewList;
//デプスステート
ComPtr<ID3D11DepthStencilState> DirectX11::m_pDSState;
// デプスバッファ
ComPtr<ID3D11DepthStencilView> DirectX11::m_pDepthStencilView = nullptr;
// インプットレイアウト
ComPtr<ID3D11InputLayout> DirectX11::m_pInputLayout = nullptr;
// 頂点シェーダーオブジェクト
ComPtr<ID3D11VertexShader> DirectX11::m_pVertexShader = nullptr;
// ピクセルシェーダーオブジェクト
ComPtr<ID3D11PixelShader> DirectX11::m_pPixelShader = nullptr;
//サンプラー用変数
ComPtr<ID3D11SamplerState> DirectX11::m_pSampler = nullptr;
//定数バッファ変数
ComPtr<ID3D11Buffer> DirectX11::m_pVSObjectConstantBuffer = nullptr;
//定数バッファ変数
ComPtr<ID3D11Buffer> DirectX11::m_pVSCameraConstantBuffer = nullptr;
//ブレンドステート変数
ComPtr<ID3D11BlendState> DirectX11::m_pBlendState = nullptr;
//ワイヤーフレームステート変数
ComPtr<ID3D11RasterizerState> DirectX11::m_pWireframeRasterState;
//whitePixelテクスチャビュー
ComPtr<ID3D11ShaderResourceView> DirectX11::m_pTextureView;

//--------------------------------------------------------------------------------------
// シェーダーをファイル拡張子に合わせてコンパイル
//--------------------------------------------------------------------------------------
HRESULT CompileShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, void** ShaderObject, size_t& ShaderObjectSize, ID3DBlob** ppBlobOut)
{
	*ppBlobOut = nullptr;
	//size_t aaa = strlen(szFileName);
	//拡張子をhlslからcsoに変更したファイル名を作成
	char* csoFileName = new char[strlen(szFileName)];
	const char* dot = strrchr(szFileName, '.'); // 最後の"."を探す
	if (dot != NULL) {
		size_t prefixLength = dot - szFileName;
		strncpy_s(csoFileName, prefixLength + 1, szFileName, _TRUNCATE); // 拡張子以外をコピー
		strcpy_s(csoFileName + prefixLength, 5, ".cso"); // 新しい拡張子を追加
	}
	else {
		strcpy_s(csoFileName, strlen(szFileName), szFileName); // 拡張子がない場合はそのままコピー
	}

	FILE* fp;
	// コンパイル済みシェーダーファイル(cso)があれば読み込む
	if (fopen_s(&fp, csoFileName, "rb") == 0)
	{
		long int size = _filelength(_fileno(fp));
		unsigned char* buffer = new unsigned char[size];
		fread(buffer, size, 1, fp);
		if (!buffer) return E_FAIL;
		*ShaderObject = buffer;
		ShaderObjectSize = size;
		fclose(fp);
	}
	// コンパイル済みシェーダーファイルが無ければシェーダーファイル(hlsl)をコンパイルする
	else
	{
		ID3DBlob* p1 = nullptr;
		HRESULT hr = S_OK;
		WCHAR	filename[512];
		size_t 	wLen = 0;
		int err = 0;

		// char -> wcharに変換
		setlocale(LC_ALL, "japanese");
		err = mbstowcs_s(&wLen, filename, 512, szFileName, _TRUNCATE);

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		// D3DCOMPILE_DEBUGフラグを設定すると、シェーダーにデバッグ情報が埋め込まれる
		dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* pErrorBlob = nullptr;
		hr = D3DCompileFromFile(filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
		if (FAILED(hr))
		{
			if (pErrorBlob != nullptr) {
				MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "Error", MB_OK);
			}
			if (pErrorBlob) pErrorBlob->Release();
			if (*ppBlobOut)(*ppBlobOut)->Release();
			return hr;
		}
		if (pErrorBlob) pErrorBlob->Release();

		*ShaderObject = (*ppBlobOut)->GetBufferPointer();
		ShaderObjectSize = (*ppBlobOut)->GetBufferSize();
	}

	delete[] csoFileName;
	return S_OK;
}


HRESULT DirectX11::CreateVertexShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, D3D11_INPUT_ELEMENT_DESC* layout, unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout)
{
	HRESULT   hr;
	ID3DBlob* pBlob = nullptr;
	void* ShaderObject;
	size_t	  ShaderObjectSize;

	// ファイルの拡張子に合わせてコンパイル
	hr = CompileShader(szFileName, szEntryPoint, szShaderModel, &ShaderObject, ShaderObjectSize, &pBlob);
	if (FAILED(hr))
	{
		if (pBlob)pBlob->Release();
		return E_FAIL;
	}

	// 頂点シェーダーを生成
	hr = m_pDevice->CreateVertexShader(ShaderObject, ShaderObjectSize, NULL, ppVertexShader);
	if (FAILED(hr))
	{
		if (pBlob)pBlob->Release();
		return E_FAIL;
	}

	// 頂点データ定義生成
	hr = m_pDevice->CreateInputLayout(
		layout,
		numElements,
		ShaderObject,
		ShaderObjectSize,
		ppVertexLayout);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateInputLayout error", "error", MB_OK);
		pBlob->Release();
		return E_FAIL;
	}

	return S_OK;
}


HRESULT DirectX11::CreateVertexShader(const BYTE* byteCode, SIZE_T size, D3D11_INPUT_ELEMENT_DESC* layout, unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout)
{
	HRESULT   hr;

	// 頂点シェーダーを生成
	hr = m_pDevice->CreateVertexShader(byteCode, size, NULL, ppVertexShader);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	// 頂点データ定義生成
	hr = m_pDevice->CreateInputLayout(
		layout,
		numElements,
		byteCode,
		size,
		ppVertexLayout);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateInputLayout error", "error", MB_OK);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT DirectX11::CreatePixelShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11PixelShader** ppPixelShader)
{
	HRESULT   hr;
	ID3DBlob* pBlob = nullptr;
	void* ShaderObject;
	size_t	  ShaderObjectSize;

	// ファイルの拡張子に合わせてコンパイル
	hr = CompileShader(szFileName, szEntryPoint, szShaderModel, &ShaderObject, ShaderObjectSize, &pBlob);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	// ピクセルシェーダーを生成
	hr = m_pDevice->CreatePixelShader(ShaderObject, ShaderObjectSize, NULL, ppPixelShader);
	if (FAILED(hr))
	{
		if (pBlob)pBlob->Release();
		return E_FAIL;
	}

	return S_OK;
}

HRESULT DirectX11::CreatePixelShader(const BYTE* byteCode, SIZE_T size, ID3D11PixelShader** ppPixelShader)
{
	HRESULT   hr;

	// ピクセルシェーダーを生成
	hr = m_pDevice->CreatePixelShader(byteCode, size, NULL, ppPixelShader);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT DirectX11::D3D_Create(HWND mainHwnd)
{
	HRESULT  hr; // HRESULT型・・・Windowsプログラムで関数実行の成功/失敗を受け取る

	D3D_FEATURE_LEVEL pLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL level;
	CRect              rect;
	::GetClientRect(mainHwnd, &rect);

	//生成ファクトリ作成
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)m_pDxgiFactory.GetAddressOf());
	if (FAILED(hr)) {
		return hr;
	}

	// デバイス、スワップチェーン作成
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	::ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = mainHwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	//新しいスワップチェインポインター追加
	m_pSwapChainList.insert(std::make_pair(mainHwnd, nullptr));
	auto& swapchain = m_pSwapChainList.find(mainHwnd)->second;

	// デバイスとスワップチェインを同時に作成する関数の呼び出し
	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		pLevels,
		1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapchain,
		&m_pDevice,
		&level,
		&m_pDeviceContext);
	if (FAILED(hr)) return hr; // 上の関数呼び出しが失敗してないかifでチェック

	m_pRenderTargetViewList.insert(std::make_pair(mainHwnd, nullptr));

	// レンダーターゲットビュー作成
	ID3D11Texture2D* renderTarget;
	hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
	if (FAILED(hr)) return hr;
	hr = m_pDevice->CreateRenderTargetView(renderTarget, NULL, m_pRenderTargetViewList.find(mainHwnd)->second.GetAddressOf());
	renderTarget->Release();
	if (FAILED(hr)) return hr;

	// デプスステンシルバッファ作成
	// ※（デプスバッファ = 深度バッファ = Zバッファ）→奥行を判定して前後関係を正しく描画できる
	ID3D11Texture2D* depthStencile;
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = swapChainDesc.BufferDesc.Width;
	textureDesc.Height = swapChainDesc.BufferDesc.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D16_UNORM;
	textureDesc.SampleDesc = swapChainDesc.SampleDesc;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	hr = m_pDevice->CreateTexture2D(&textureDesc, NULL, &depthStencile);
	if (FAILED(hr)) return hr;

	// デプスステンシルビュー作成
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	hr = m_pDevice->CreateDepthStencilView(depthStencile, &depthStencilViewDesc, m_pDepthStencilView.GetAddressOf());
	if (FAILED(hr)) return hr;
	depthStencile->Release();

	// ビューポートを作成（→画面分割などに使う、描画領域の指定のこと）
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)rect.Width();
	viewport.Height = (FLOAT)rect.Height();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	// インプットレイアウト作成
	D3D11_INPUT_ELEMENT_DESC layout[]
	{
		// 位置座標があるということを伝える
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 色情報があるということを伝える
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//UV座標
		{ "TEX",0,DXGI_FORMAT_R32G32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	unsigned int numElements = ARRAYSIZE(layout);

	// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	//hr = CreateVertexShader(UV_VS, sizeof(UV_VS), layout, numElements, m_pVertexShader.GetAddressOf(), m_pInputLayout.GetAddressOf());
	hr = CreateVertexShader(UNLIT_TEXTURE_VS, sizeof(UNLIT_TEXTURE_VS), layout, numElements, m_pVertexShader.GetAddressOf(), m_pInputLayout.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateVertexShader error", "error", MB_OK);
		return E_FAIL;
	}
	
	// ピクセルシェーダーオブジェクトを生成
	hr = CreatePixelShader(UNLIT_TEXTURE_PS, sizeof(UNLIT_TEXTURE_PS), m_pPixelShader.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreatePixelShader error", "error", MB_OK);
		return E_FAIL;
	}

	//サンプラー作成
	//→テクスチャをポリゴンに貼る時に、拡大縮小される際のアルゴリズム
	D3D11_SAMPLER_DESC smpDesc;
	::ZeroMemory(&smpDesc, sizeof(D3D11_SAMPLER_DESC));
	smpDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	smpDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	smpDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	smpDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	hr = m_pDevice->CreateSamplerState(&smpDesc, m_pSampler.GetAddressOf());
	if (FAILED(hr)) return hr;

	//定数バッファ作成
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(VSObjectConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	hr = m_pDevice->CreateBuffer(&cbDesc, NULL, m_pVSObjectConstantBuffer.GetAddressOf());
	if (FAILED(hr)) return hr;

	//定数バッファ作成
	D3D11_BUFFER_DESC cbDesc2;
	cbDesc2.ByteWidth = sizeof(VSCameraConstantBuffer);
	cbDesc2.Usage = D3D11_USAGE_DEFAULT;
	cbDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc2.CPUAccessFlags = 0;
	cbDesc2.MiscFlags = 0;
	cbDesc2.StructureByteStride = 0;
	hr = m_pDevice->CreateBuffer(&cbDesc2, NULL, m_pVSCameraConstantBuffer.GetAddressOf());
	if (FAILED(hr)) return hr;

	//ブレンディングステート生成
	D3D11_BLEND_DESC BlendStateDesc;
	BlendStateDesc.AlphaToCoverageEnable = FALSE;
	BlendStateDesc.IndependentBlendEnable = FALSE;
	BlendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_pDevice->CreateBlendState(&BlendStateDesc, m_pBlendState.GetAddressOf());
	if (FAILED(hr)) return hr;

	//深度テストを無効にする
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = FALSE;//
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	hr = m_pDevice->CreateDepthStencilState(&dsDesc, m_pDSState.GetAddressOf());
	if (FAILED(hr)) return hr;

	//ワイヤーフレーム
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));

	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;    // Set to wireframe mode
	rasterDesc.CullMode = D3D11_CULL_BACK;         // Optional: set culling mode
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthClipEnable = TRUE;

	hr = m_pDevice->CreateRasterizerState(&rasterDesc, m_pWireframeRasterState.GetAddressOf());
	if (FAILED(hr)) return hr;

	//共通のpixelTexture読み込み
	CreateOnePixelTexture(m_pTextureView.GetAddressOf());

	//深度テスト設定
	m_pDeviceContext->OMSetDepthStencilState(m_pDSState.Get(), 1);

	//そのブレンディングをコンテキストに設定
	//float blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
	m_pDeviceContext->OMSetBlendState(m_pBlendState.Get(), NULL, 0xffffffff);

	//インプットレイアウト設定
	m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());

	//サンプラーをピクセルシェーダーに渡す
	m_pDeviceContext->PSSetSamplers(0, 1, m_pSampler.GetAddressOf());

	//定数バッファを頂点シェーダーにセットする
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pVSObjectConstantBuffer.GetAddressOf());

	//定数バッファを頂点シェーダーにセットする
	m_pDeviceContext->VSSetConstantBuffers(1, 1, m_pVSCameraConstantBuffer.GetAddressOf());

	//RenderManagerでDrawするときに後方で設定を戻しているので初めに一回設定しておく
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return S_OK;
}

void DirectX11::D3D_Release()
{
	if (m_pDeviceContext) m_pDeviceContext->ClearState();
}

void DirectX11::D3D_StartRender()
{
	//SpriteBatchで設定が変わるので戻すため
	//===========================================================================================
	//インプットレイアウト設定
	m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());

	//定数バッファを頂点シェーダーにセットする
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pVSObjectConstantBuffer.GetAddressOf());
	//===========================================================================================

	// 画面塗りつぶし色
	static const float clearColor[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	int colorIndex = 0;
	for (auto& targetView : m_pRenderTargetViewList)
	{
		// 描画先キャンバスを塗りつぶす
		m_pDeviceContext->ClearRenderTargetView(targetView.second.Get(), clearColor);
		//colorIndex = (colorIndex + 1) % 4;
	}

	// 深度バッファをリセットする
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//今は一つしかシェーダーを使っていないので設定する必要はない
	//=======================================================================
	//頂点シェーダ設定
	m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), NULL, 0);
	//ピクセルシェーダ設定
	m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), NULL, 0);
	//=======================================================================
}

void DirectX11::D3D_FinishRender()
{
	for (auto swapChain : m_pSwapChainList)
	{
		// ダブルバッファの切り替えを行い画面を更新する
		swapChain.second->Present(0, 0);
	}
}

void DirectX11::CreateOnePixelTexture(ID3D11ShaderResourceView** _resourceView)
{
	// 白色の1ピクセルデータ
	const UINT texWidth = 1;
	const UINT texHeight = 1;
	const UINT pixelSize = 4; // RGBA8
	unsigned char whitePixel[pixelSize] = { 255, 255, 255, 255 }; // R, G, B, A

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = texWidth;
	texDesc.Height = texHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = whitePixel;
	initData.SysMemPitch = texWidth * pixelSize;

	ID3D11Texture2D* pTexture = nullptr;
	HRESULT hr = m_pDevice->CreateTexture2D(&texDesc, &initData, &pTexture);
	if (SUCCEEDED(hr)) {
		// シェーダーリソースビューの作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		hr = m_pDevice->CreateShaderResourceView(pTexture, &srvDesc, _resourceView);
		if (SUCCEEDED(hr)) {
			// ここで pSRV をシェーダーに渡すなどして使用可能
		}
		pTexture->Release();
	}
}

HRESULT DirectX11::CreateWindowSwapchain(HWND hWnd)
{
	// Describe the swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	::ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	// Create the swap chain using the factory
	IDXGISwapChain* swapChain = nullptr;
	HRESULT hr = m_pDxgiFactory->CreateSwapChain(m_pDevice.Get(), &swapChainDesc, &swapChain);
	if (FAILED(hr)) {
		return hr;
	}

	// レンダーターゲットビュー作成
	ID3D11Texture2D* renderTarget;
	ID3D11RenderTargetView* renderTargetView;
	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
	if (FAILED(hr)) return hr;
	hr = m_pDevice->CreateRenderTargetView(renderTarget, NULL, &renderTargetView);
	renderTarget->Release();
	if (FAILED(hr)) return hr;

	m_pSwapChainList.insert(std::make_pair(hWnd, swapChain));
	m_pRenderTargetViewList.insert(std::make_pair(hWnd, renderTargetView));

	return S_OK;
}

void DirectX11::GetTextureDpi(ID3D11ShaderResourceView* _view, UINT& _width, UINT& _height)
{
	// Assume `textureView` is a pointer to an ID3D11ShaderResourceView instance
	ID3D11Resource* pResource = nullptr;
	_view->GetResource(&pResource);

	ID3D11Texture2D* pTexture2D = nullptr;
	HRESULT hr = pResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pTexture2D));
	if (SUCCEEDED(hr) && pTexture2D)
	{
		D3D11_TEXTURE2D_DESC desc;
		pTexture2D->GetDesc(&desc);

		_width = desc.Width;
		_height = desc.Height;

		// Now you have the width and height of the texture
		pTexture2D->Release();
	}

	pResource->Release();
}

