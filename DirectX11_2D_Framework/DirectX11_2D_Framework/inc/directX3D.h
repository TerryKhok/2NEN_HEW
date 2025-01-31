#pragma once

//directXTK�֌W
#include "../directxtk/include/directxtk/DDSTextureLoader.h"
#include "../directxtk/include/directxtk/WICTextureLoader.h"
#include "../directxtk/include/directxtk/SpriteFont.h"
#include "../directxtk/include/directxtk/SimpleMath.h"
//#include "directxtk/include/directxtk/SpriteBatch.h"
//#include "directxtk/include/directxtk/VertexTypes.h"
//#include "directxtk/include/directxtk/Effects.h"

// Direct3D����̊ȗ����}�N��
#define SAFE_RELEASE(p) { if( nullptr != p ) { p->Release(); p = nullptr; } }

using namespace DirectX;

using Microsoft::WRL::ComPtr;


// �\���̂̒�`
// ���_�f�[�^��\���\����
struct Vertex final
{
	//�������m�ۋ֎~
	void* operator new(size_t) = delete;

	// ���_�̈ʒu���W
	float x, y, z;

	//�F
	float r, g, b, a;

	//�e�N�X�`�����W�iUV���W)
	float u, v;
};

struct VSObjectConstantBuffer final
{
	//�������m�ۋ֎~
	void* operator new(size_t) = delete;

	DirectX::XMMATRIX world = {};
	//DirectX::XMMATRIX tex;
	DirectX::XMFLOAT2 uvScale = XMFLOAT2(1.0f, 1.0f);
	DirectX::XMFLOAT2 uvOffset = XMFLOAT2(0.0f, 0.0f);
	DirectX::XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct VSCameraConstantBuffer final
{
	//�������m�ۋ֎~
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
	// ���_�V�F�[�_�[�I�u�W�F�N�g�𐶐��A�����ɒ��_���C�A�E�g������
	static HRESULT CreateVertexShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel,
		D3D11_INPUT_ELEMENT_DESC* layout, unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout);
	// ���_�V�F�[�_�[�I�u�W�F�N�g�𐶐��A�����ɒ��_���C�A�E�g������
	static HRESULT CreateVertexShader(const BYTE* byteCode, SIZE_T size, D3D11_INPUT_ELEMENT_DESC* layout,
		unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout);
	// �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�𐶐�
	static HRESULT CreatePixelShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11PixelShader** ppPixelShader);
	// �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�𐶐�
	static HRESULT CreatePixelShader(const BYTE* byteCode, SIZE_T size, ID3D11PixelShader** ppPixelShader);

	// �֐��̃v���g�^�C�v�錾
	static HRESULT D3D_Create(HWND mainHwnd);
	static void D3D_Release();
	static void D3D_StartRender();
	static void D3D_FinishRender();

	//1pixel�̃e�N�X�`���쐬
	static void CreateOnePixelTexture(ID3D11ShaderResourceView** _resourceView);
	//�E�B���h�E�ɑΉ������V�����X���b�v�`�F�C���쐬
	static HRESULT CreateWindowSwapChain(HWND hWnd);
	//�e�N�X�`���̉𑜓x���擾����
	static void GetTextureDpi(ID3D11ShaderResourceView* _view, UINT& _width, UINT& _height);

private:
	// ��ID3D11�Ŏn�܂�|�C���^�^�̕ϐ��́A�������K�v������
	static ComPtr<ID3D11Device> m_pDevice; // �f�o�C�X��DirectX�̊e��@�\�����
	//�X���b�v�`�F�[�������𐶐����邽��(�����p�C���X�^���X)
	static ComPtr<IDXGIFactory> m_pDxgiFactory;
	// �R���e�L�X�g���`��֘A���i��@�\
	static ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	// �X���b�v�`�F�C�����_�u���o�b�t�@�@�\
	static std::unordered_map<HWND, ComPtr<IDXGISwapChain>> m_pSwapChainList;
	// �����_�[�^�[�Q�b�g���`����\���@�\
	static std::unordered_map<HWND, std::pair<ComPtr<ID3D11RenderTargetView>, std::vector<LAYER>>> m_pRenderTargetViewList;

	static std::unordered_map < HWND, bool> m_waveHandleList;

	//�f�v�X�X�e�[�g
	static ComPtr<ID3D11DepthStencilState> m_pDSState;
	// �f�v�X�o�b�t�@
	static ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
	// �C���v�b�g���C�A�E�g
	static ComPtr<ID3D11InputLayout> m_pInputLayout;
	// ���_�V�F�[�_�[�I�u�W�F�N�g
	static ComPtr<ID3D11VertexShader> m_pVertexShader;
	// �s�N�Z���V�F�[�_�[�I�u�W�F�N�g
	static ComPtr<ID3D11PixelShader> m_pPixelShader;
	//�T���v���[�p�ϐ�
	static ComPtr<ID3D11SamplerState> m_pSampler;
	//�萔�o�b�t�@�ϐ�
	static ComPtr<ID3D11Buffer> m_pVSObjectConstantBuffer;
	//�萔�o�b�t�@�ϐ�
	static ComPtr<ID3D11Buffer> m_pVSCameraConstantBuffer;
	//�u�����h�X�e�[�g�ϐ�
	static ComPtr<ID3D11BlendState> m_pBlendState;
	//���C���[�t���[���X�e�[�g�ϐ�
	static ComPtr<ID3D11RasterizerState> m_pWireframeRasterState;
	//whitePixel�e�N�X�`���r���[
	static ComPtr<ID3D11ShaderResourceView> m_pTextureView;
	//��ʓh��Ԃ��J���[
	static float clearColor[4];


	// �s�N�Z���V�F�[�_�[�I�u�W�F�N�g
	static ComPtr<ID3D11PixelShader> m_pWavePixelShader;
	struct TimeBuffer {
		float time;
		float strength;
		float noiseScale;
		float persistence;
	};
	static TimeBuffer waveData;
	//�萔�o�b�t�@�ϐ�
	static ComPtr<ID3D11Buffer> m_pPSWaveConstantBuffer;
};
