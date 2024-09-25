#pragma once

class TextureAssets
{
	friend class RenderNode;
	friend class RenderManager;

public:
	~TextureAssets()
	{
		m_textureLib.clear();
	}
private:
	TextureAssets() {}
	
	static HRESULT LoadTexture(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName);
private:
	static std::unordered_map<const wchar_t*, ComPtr<ID3D11ShaderResourceView>> m_textureLib;
};
