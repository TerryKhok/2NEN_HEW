

std::unordered_map<const wchar_t*, ComPtr<ID3D11ShaderResourceView>> TextureAssets::m_textureLib;
HRESULT(*TextureAssets::pLoadTexture)(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName) = Load;

const char* ConvertWCharToChar(const wchar_t* wstr) {
	size_t requiredSize = 0;
	// First, calculate the size of the converted string, including the null terminator
	wcstombs_s(&requiredSize, nullptr, 0, wstr, _TRUNCATE);

	// Allocate buffer to hold the multi-byte string
	char* buffer = new char[requiredSize];

	// Perform the conversion
	wcstombs_s(nullptr, buffer, requiredSize, wstr, _TRUNCATE);

	return buffer;  // Remember to free the memory later
}

HRESULT TextureAssets::Load(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName)
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
		const char* cstr = ConvertWCharToChar(_texName);
		std::string log = cstr;
		log = "テクスチャ読み込み失敗 :" + log;
		MessageBoxA(NULL, log.c_str(), "エラー", MB_ICONERROR | MB_OK);
		delete[] cstr;  // Free the allocated memory
	}

	m_textureLib[_texName] = _textureView;

	return hr;
}

HRESULT TextureAssets::Void(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName)
{
	return S_FALSE;
}

void TextureAssets::LoadEnd()
{
	pLoadTexture = TextureAssets::Void;
}
