#pragma once

class TextureAssets final
{
	friend class Window;
	friend class SceneManager;
	friend class RenderNode;
	friend class RenderManager;
	friend class AnimationClip;
	friend class ImGuiApp;
	
private:
	TextureAssets() = delete;
	~TextureAssets()
	{
		m_textureLib.clear();
	}

	static HRESULT Init();
	static void UnInit();
	
	static HRESULT DDSWICLoad(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texPath);
	static HRESULT DDSWICLoadNext(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texPath);
	static HRESULT Void(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texPath);

	static HRESULT StbiLoad(ComPtr<ID3D11ShaderResourceView>& _textureView, std::string _filePath);
	static HRESULT StbiLoad(ComPtr<ID3D11ShaderResourceView>& _textureView, std::string _filePath, int* _width, int* _height);

	static void ChangeNextTextureLib();
	static void LinkNextTextureLib();

	static void LoadEnd();
private:
	//テクスチャ読み込み関数ポインタ
	static thread_local HRESULT(* pLoadTexture)(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texPath);
private:
	static ComPtr<IWICImagingFactory> m_pWICFactory;
	static std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_textureLib;
	static std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_nextTextureLib;
};
