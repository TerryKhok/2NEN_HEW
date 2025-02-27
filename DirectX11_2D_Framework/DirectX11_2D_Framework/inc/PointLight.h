#pragma once

//==================================================================
// 非同期ロード中にポイントライトをDeleteできる処理はまだない
//==================================================================

//アライメントの影響のため4の倍数で
#define MAX_LIGHT_NUM 64

struct POINT_LIGHT
{
	DirectX::XMFLOAT3 color;
	float padding;
	DirectX::XMFLOAT2 pos;
	float intensity;
	float radius;
};

struct ComputeShaderConstants
{	
	POINT_LIGHT Lights[MAX_LIGHT_NUM];
	unsigned int LightCount;
	float padding[3];
};

struct ComputeCameraBuffer
{
	Vector2 pos;
	Vector2 zoom;
};


class PointLight : public Component
{
	friend class DirectX11;
	friend class SceneManager;

	void Start() override;
	void Update() override;
	void SetActive(bool _active) override;
	void Delete() override;
	void DrawImGui(ImGuiApp::HandleUI& _handleUi) override;

	void Serialize(SERIALIZE_OUTPUT& ar) override;
	void Deserialize(SERIALIZE_INPUT& ar) override;

	//ライトマッピング用コンピュートシェーダー
	static ComPtr<ID3D11ComputeShader> m_lightComputeShader;
	//ライトマッピング用ピクセルシェーダーオブジェクト
	static ComPtr<ID3D11PixelShader> m_lightPixelShader;
	//定数バッファ変数
	static ComPtr<ID3D11Buffer> m_pCSLightMappingBuffer;
	//コンピュートシェーダーのカメラバッファ
	static ComPtr<ID3D11Buffer> m_pCSCameraBuffer;
	static ComPtr<ID3D11UnorderedAccessView> lightMapUAV;
	static ComPtr<ID3D11ShaderResourceView> lightMapSRV;
	static ComputeShaderConstants m_computeBuffer;
	static ComputeCameraBuffer m_cameraBuffer;
	static std::vector<PointLight*> m_useLights;
	static std::vector<PointLight*> m_newLights;

	static thread_local void(PointLight::*pAddFunc)();
public:
	void SetColor(DirectX::XMFLOAT3 _color);
	void SetIntensity(float _intensity);
	void SetLightParamater(POINT_LIGHT& _light);
	DirectX::XMFLOAT3 GetColor() const { return m_color; }
	float GetIntensity() const { return m_intensity; }
private:
	static HRESULT Init(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	static void SetAndUpdateLight(ID3D11DeviceContext* _pDeviceContext);

	static void StartAsync();
	static void LinkLight();

	void AddPointLight();
	void AddPointLightAsync();
public:
	static void SetLightPSShader(ID3D11DeviceContext* _pDeviceContext);

	float m_radius = 100.0f;
private:
	int m_index = -1;
	DirectX::XMFLOAT3 m_color = {1.0,1.0,1.0};
	float m_intensity = 1.0f;
};
