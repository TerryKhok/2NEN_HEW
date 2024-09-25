#pragma once

// Direct3D解放の簡略化マクロ
#define SAFE_RELEASE(p) { if( nullptr != p ) { p->Release(); p = nullptr; } }

using namespace DirectX;

using Microsoft::WRL::ComPtr;

// 構造体の定義
// 頂点データを表す構造体
struct Vertex
{
	// 頂点の位置座標
	float x, y, z;

	//色
	float r, g, b, a;

	//テクスチャ座標（UV座標)
	float u, v;
};

struct VSConstantBuffer
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX tex;
	DirectX::XMFLOAT4 color;
};


class DirectX11
{
	friend class Window;
	friend class TextureAssets;
	friend class RenderNode;
	friend class RenderManager;

	DirectX11() = delete;

	// 関数のプロトタイプ宣言
	static HRESULT D3D_Create(HWND hwnd);
	static void D3D_Release();
	static void D3D_StartRender();
	static void D3D_FinishRender();

	//プロトタイプ宣言
	// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	static HRESULT CreateVertexShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel,
		D3D11_INPUT_ELEMENT_DESC* layout, unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout);
	// ピクセルシェーダーオブジェクトを生成
	static HRESULT CreatePixelShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11PixelShader** ppPixelShader);
public:
	inline const ID3D11Device* GetDevice() { return m_pDevice.Get(); }
private:
	// ※ID3D11で始まるポインタ型の変数は、解放する必要がある
	static ComPtr<ID3D11Device> m_pDevice; // デバイス＝DirectXの各種機能を作る
	// コンテキスト＝描画関連を司る機能
	static ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	// スワップチェイン＝ダブルバッファ機能
	static ComPtr<IDXGISwapChain> m_pSwapChain;
	// レンダーターゲット＝描画先を表す機能
	static ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
	// デプスバッファ
	static ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
	// インプットレイアウト
	static ComPtr<ID3D11InputLayout> m_pInputLayout;
	// 頂点シェーダーオブジェクト
	static ComPtr<ID3D11VertexShader> m_pVertexShader;
	// ピクセルシェーダーオブジェクト
	static ComPtr<ID3D11PixelShader> m_pPixelShader;
	//サンプラー用変数
	static ComPtr<ID3D11SamplerState> m_pSampler;
	//定数バッファ変数
	static ComPtr<ID3D11Buffer> m_pVSConstantBuffer;
	//ブレンドステート変数
	static ComPtr<ID3D11BlendState> m_pBlendState;
};
