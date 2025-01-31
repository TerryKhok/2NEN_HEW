#pragma once

//directXTK関係
#include "../directxtk/include/directxtk/DDSTextureLoader.h"
#include "../directxtk/include/directxtk/WICTextureLoader.h"
#include "../directxtk/include/directxtk/SpriteFont.h"
#include "../directxtk/include/directxtk/SimpleMath.h"
//#include "directxtk/include/directxtk/SpriteBatch.h"
//#include "directxtk/include/directxtk/VertexTypes.h"
//#include "directxtk/include/directxtk/Effects.h"

// Direct3D解放の簡略化マクロ
#define SAFE_RELEASE(p) { if( nullptr != p ) { p->Release(); p = nullptr; } }

using namespace DirectX;

using Microsoft::WRL::ComPtr;


// 構造体の定義
// 頂点データを表す構造体
struct Vertex final
{
	//メモリ確保禁止
	void* operator new(size_t) = delete;

	// 頂点の位置座標
	float x, y, z;

	//色
	float r, g, b, a;

	//テクスチャ座標（UV座標)
	float u, v;
};

struct VSObjectConstantBuffer final
{
	//メモリ確保禁止
	void* operator new(size_t) = delete;

	DirectX::XMMATRIX world = {};
	//DirectX::XMMATRIX tex;
	DirectX::XMFLOAT2 uvScale = XMFLOAT2(1.0f, 1.0f);
	DirectX::XMFLOAT2 uvOffset = XMFLOAT2(0.0f, 0.0f);
	DirectX::XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct VSCameraConstantBuffer final
{
	//メモリ確保禁止
	void* operator new(size_t) = delete;

	DirectX::XMMATRIX view = {};
	DirectX::XMMATRIX projection = {};
};


class DirectX11 final
{
	friend class Window;
	friend class ImGuiApp;
	friend class SubWindow;
	friend class DWTextManager;
	friend class TextureAssets;
	friend class RenderNode;
	friend class UVRenderNode;
	friend class TileRenderNode;
	friend class TileMap;
	friend class RenderManager;
	friend class CameraManager;
	friend class SFTextManager;
	friend class Box2DBodyManager;
	friend class Box2DBoxRenderNode;
	friend class Box2DCircleRenderNode;
	friend class Box2DCapsuleRenderNode;
	friend class Box2DMeshRenderNode;
	friend class Box2DConvexMeshRenderNode;
	friend class Box2DLineRenderNode;

	DirectX11() = delete;

private:
	// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	static HRESULT CreateVertexShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel,
		D3D11_INPUT_ELEMENT_DESC* layout, unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout);
	// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	static HRESULT CreateVertexShader(const BYTE* byteCode, SIZE_T size, D3D11_INPUT_ELEMENT_DESC* layout,
		unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout);
	// ピクセルシェーダーオブジェクトを生成
	static HRESULT CreatePixelShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11PixelShader** ppPixelShader);
	// ピクセルシェーダーオブジェクトを生成
	static HRESULT CreatePixelShader(const BYTE* byteCode, SIZE_T size, ID3D11PixelShader** ppPixelShader);

	// 関数のプロトタイプ宣言
	static HRESULT D3D_Create(HWND mainHwnd);
	static void D3D_Release();
	static void D3D_StartRender();
	static void D3D_FinishRender();

	//1pixelのテクスチャ作成
	static void CreateOnePixelTexture(ID3D11ShaderResourceView** _resourceView);
	//ウィンドウに対応した新しいスワップチェイン作成
	static HRESULT CreateWindowSwapChain(HWND hWnd);
	//テクスチャの解像度を取得する
	static void GetTextureDpi(ID3D11ShaderResourceView* _view, UINT& _width, UINT& _height);

private:
	// ※ID3D11で始まるポインタ型の変数は、解放する必要がある
	static ComPtr<ID3D11Device> m_pDevice; // デバイス＝DirectXの各種機能を作る
	//スワップチェーンだけを生成するため(生成用インスタンス)
	static ComPtr<IDXGIFactory> m_pDxgiFactory;
	// コンテキスト＝描画関連を司る機能
	static ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	// スワップチェイン＝ダブルバッファ機能
	static std::unordered_map<HWND, ComPtr<IDXGISwapChain>> m_pSwapChainList;
	// レンダーターゲット＝描画先を表す機能
	static std::unordered_map<HWND, std::pair<ComPtr<ID3D11RenderTargetView>, std::vector<LAYER>>> m_pRenderTargetViewList;

	static std::unordered_map < HWND, bool> m_waveHandleList;

	//デプスステート
	static ComPtr<ID3D11DepthStencilState> m_pDSState;
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
	static ComPtr<ID3D11Buffer> m_pVSObjectConstantBuffer;
	//定数バッファ変数
	static ComPtr<ID3D11Buffer> m_pVSCameraConstantBuffer;
	//ブレンドステート変数
	static ComPtr<ID3D11BlendState> m_pBlendState;
	//ワイヤーフレームステート変数
	static ComPtr<ID3D11RasterizerState> m_pWireframeRasterState;
	//whitePixelテクスチャビュー
	static ComPtr<ID3D11ShaderResourceView> m_pTextureView;
	//画面塗りつぶしカラー
	static float clearColor[4];


	// ピクセルシェーダーオブジェクト
	static ComPtr<ID3D11PixelShader> m_pWavePixelShader;
	struct TimeBuffer {
		float time;
		float strength;
		float noiseScale;
		float persistence;
	};
	static TimeBuffer waveData;
	//定数バッファ変数
	static ComPtr<ID3D11Buffer> m_pPSWaveConstantBuffer;
};
