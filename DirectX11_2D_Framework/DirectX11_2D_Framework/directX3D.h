#pragma once

// Direct3D����̊ȗ����}�N��
#define SAFE_RELEASE(p) { if( nullptr != p ) { p->Release(); p = nullptr; } }

using namespace DirectX;

using Microsoft::WRL::ComPtr;

// �\���̂̒�`
// ���_�f�[�^��\���\����
struct Vertex
{
	// ���_�̈ʒu���W
	float x, y, z;

	//�F
	float r, g, b, a;

	//�e�N�X�`�����W�iUV���W)
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

	// �֐��̃v���g�^�C�v�錾
	static HRESULT D3D_Create(HWND hwnd);
	static void D3D_Release();
	static void D3D_StartRender();
	static void D3D_FinishRender();

	//�v���g�^�C�v�錾
	// ���_�V�F�[�_�[�I�u�W�F�N�g�𐶐��A�����ɒ��_���C�A�E�g������
	static HRESULT CreateVertexShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel,
		D3D11_INPUT_ELEMENT_DESC* layout, unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout);
	// �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�𐶐�
	static HRESULT CreatePixelShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11PixelShader** ppPixelShader);
public:
	inline const ID3D11Device* GetDevice() { return m_pDevice.Get(); }
private:
	// ��ID3D11�Ŏn�܂�|�C���^�^�̕ϐ��́A�������K�v������
	static ComPtr<ID3D11Device> m_pDevice; // �f�o�C�X��DirectX�̊e��@�\�����
	// �R���e�L�X�g���`��֘A���i��@�\
	static ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	// �X���b�v�`�F�C�����_�u���o�b�t�@�@�\
	static ComPtr<IDXGISwapChain> m_pSwapChain;
	// �����_�[�^�[�Q�b�g���`����\���@�\
	static ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
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
	static ComPtr<ID3D11Buffer> m_pVSConstantBuffer;
	//�u�����h�X�e�[�g�ϐ�
	static ComPtr<ID3D11BlendState> m_pBlendState;
};
