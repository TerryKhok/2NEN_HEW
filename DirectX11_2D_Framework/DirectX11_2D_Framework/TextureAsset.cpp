
std::unordered_map<const wchar_t*, ComPtr<ID3D11ShaderResourceView>> TextureAssets::m_textureLib;

HRESULT TextureAssets::LoadTexture(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName)
{
	auto iter = m_textureLib.find(_texName);
	if (iter != m_textureLib.end())
	{
		_textureView = iter->second;
		return S_OK;
	}

	//texture追加の処理をする

	HRESULT  hr;
	hr = DirectX::CreateWICTextureFromFile(
		DirectX11::m_pDevice.Get(), _texName, NULL, _textureView.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "テクスチャ読み込み失敗", "エラー", MB_ICONERROR | MB_OK);
	}

	m_textureLib[_texName] = _textureView;

	return hr;
}
