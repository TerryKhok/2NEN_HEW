#pragma once

const char* ConvertWCharToChar(const wchar_t* wstr);

class TextureAssets final
{
	friend class Window;
	friend class RenderNode;
	friend class RenderManager;
	
private:
	TextureAssets() = delete;
	~TextureAssets()
	{
		m_textureLib.clear();
	}
	
	//static HRESULT LoadTexture(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName);
	static HRESULT Load(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName);
	static HRESULT Void(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName);
	static void LoadEnd();
private:
	//テクスチャ読み込み関数ポインタ
	static HRESULT(* pLoadTexture)(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName);
private:
	static std::unordered_map<const wchar_t*, ComPtr<ID3D11ShaderResourceView>> m_textureLib;
	
};
