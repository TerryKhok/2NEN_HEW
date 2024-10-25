#include "../../DirectX11_2D_Framework/scene/AnimationBuilder.h"


// Open File Dialog to select PNG
 std::string OpenFileDialog() {
	OPENFILENAME ofn;
	char szFile[260];
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.lpstrFilter = "PNG Files (*.png)\0*.png\0";//All Files (*.*)\0*.*\0;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrTitle = "Select a PNG file";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		std::string str = szFile;
		return str;
	}

	return " ";
}

 //–ß‚è’l‚ðdelete‚·‚é
 const wchar_t* ConvertCharToWChar(const char* str)
 {
	 // Calculate the length of the required wide string buffer
	 int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
	 if (len == 0) {
		 std::cerr << "Conversion error occurred!" << std::endl;
		 return nullptr;
	 }

	 // Allocate memory for the wide string
	 wchar_t* wideStr = new wchar_t[len];

	 // Perform the actual conversion
	 MultiByteToWideChar(CP_UTF8, 0, str, -1, wideStr, len);

	 return wideStr;

 }

void AnimationBulider_Scene::Load()
{
	const wchar_t* filePath = ConvertCharToWChar(OpenFileDialog().c_str());
	auto object = Instantiate("test");
	object->SetLayer(LAYER_UI);
	object->AddComponent<Renderer>(filePath);

	delete filePath;
}
