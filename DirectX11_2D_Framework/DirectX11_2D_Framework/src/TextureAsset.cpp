#define STB_IMAGE_IMPLEMENTATION
#include "../../DirectX11_2D_Framework/inc/stb_image.h"

ComPtr<IWICImagingFactory> TextureAssets::m_pWICFactory = nullptr;
std::unordered_map<const wchar_t*, ComPtr<ID3D11ShaderResourceView>> TextureAssets::m_textureLib;
HRESULT(*TextureAssets::pLoadTexture)(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName) = WICLoad;


HRESULT TextureAssets::Init()
{
	HRESULT hr = CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICFactory));
	return hr;
}

void TextureAssets::Uninit()
{
	CoUninitialize();
}

HRESULT TextureAssets::WICLoad(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName)
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
		std::string log = wstring_to_string(_texName);
		log = "テクスチャ読み込み失敗 :" + log;
		MessageBoxA(NULL, log.c_str(), "エラー", MB_ICONERROR | MB_OK);
	}

	m_textureLib[_texName] = _textureView;

	return hr;
}

HRESULT TextureAssets::Void(ComPtr<ID3D11ShaderResourceView>& _textureView, const wchar_t* _texName)
{
	return S_FALSE;
}

// Function to display Save File dialog and get the file path
std::string GetSaveFilePath() {
	OPENFILENAME ofn;       // Common dialog box structure
	char szFile[260];
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;   // Owner window (NULL if no owner)
	ofn.lpstrFile = szFile; // Buffer to store the file path
	ofn.lpstrFile[0] = '\0';  // Initialize with an empty string
	ofn.nMaxFile = sizeof(szFile); // Max path length
	ofn.lpstrFilter = "Binary Files\0*.bin\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = "bin"; // Default file extension
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn))
	{
		std::string str = szFile;
		return str;
	}

	return " ";
}


HRESULT TextureAssets::StbiLoad(ComPtr<ID3D11ShaderResourceView>& _textureView, std::string _filePath)
{
	int width, height;
	return StbiLoad(_textureView, _filePath, &width, &height);
}

HRESULT TextureAssets::StbiLoad(ComPtr<ID3D11ShaderResourceView>& _textureView, std::string _filePath, int* _width, int* _height)
{
	// Load the PNG and decode it to raw pixel data (RGBA)
	int channels;
	unsigned char* decodedData = stbi_load(_filePath.c_str(), _width, _height, &channels, 4);  // Force RGBA format
	if (!decodedData) {
		LOG_WARNING("Failed to load image: %s", _filePath.c_str());
		return S_FALSE;
	}

	// Set up the texture description
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = *_width;
	textureDesc.Height = *_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // RGBA format
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// Define the initial texture data
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = decodedData;
	initData.SysMemPitch = *_width * 4;  // 4 bytes per pixel for RGBA

	// Create the texture
	ComPtr<ID3D11Texture2D> texture;
	HRESULT hr = DirectX11::m_pDevice->CreateTexture2D(&textureDesc, &initData, &texture);
	if (FAILED(hr)) {
		LOG_WARNING("Failed to create texture from decoded data.");
		stbi_image_free(decodedData);  // Free the image data
		return S_FALSE;
	}

	// Create a shader resource view for the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

	hr = DirectX11::m_pDevice->CreateShaderResourceView(texture.Get(), &srvDesc, _textureView.GetAddressOf());
	if (FAILED(hr)) {
		LOG_WARNING("Failed to create shader resource view.");
	}

	//std::string outputFilePath = GetSaveFilePath();
	//if (outputFilePath == "")
	//{
	//	// Free the decoded image data
	//	stbi_image_free(decodedData);

	//	return S_OK;
	//}

	// Open the output file
	//std::ofstream outputFile(outputFilePath.c_str());
	//if (!outputFile) {
	//	std::cerr << "Error opening output file.\n";
	//	stbi_image_free(decodedData);
	//	return S_OK;
	//}

	//// Write the C array header
	//outputFile << "const uint8_t imageData[] = {\n";
	//size_t dataSize = width * height * 4;  // 4 bytes per pixel for RGBA
	//for (size_t i = 0; i < dataSize; ++i) {
	//	if (i % 12 == 0) outputFile << "    ";  // 12 values per line
	//	outputFile << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(decodedData[i]);
	//	if (i < dataSize - 1) outputFile << ", ";
	//	if (i % 12 == 11) outputFile << "\n";
	//}
	//outputFile << "\n};\n";

	// Free the decoded image data
	stbi_image_free(decodedData);

	//outputFile.close();

	// Clean up
	//stbi_image_free(decodedData);  // Free the decoded image data now that it's in the texture

	return S_OK;
}

void TextureAssets::LoadEnd()
{
	pLoadTexture = TextureAssets::Void;
}
