
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

//// Function to display Save File dialog and get the file path
//std::string GetSaveFilePath() {
//    OPENFILENAME ofn;       // Common dialog box structure
//	char szFile[260];
//    ZeroMemory(&ofn, sizeof(ofn));
//    ofn.lStructSize = sizeof(ofn);
//    ofn.hwndOwner = NULL;   // Owner window (NULL if no owner)
//    ofn.lpstrFile = szFile; // Buffer to store the file path
//    ofn.lpstrFile[0] = '\0';  // Initialize with an empty string
//    ofn.nMaxFile = sizeof(szFile); // Max path length
//    ofn.lpstrFilter = "Binary Files\0*.bin\0All Files\0*.*\0";
//    ofn.nFilterIndex = 1;
//    ofn.lpstrDefExt = "bin"; // Default file extension
//    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
//
//	if (GetSaveFileName(&ofn))
//	{
//		std::string str = szFile;
//		return str;
//	}
//
//	return " ";
//}

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

HRESULT ConvertPngToCArray(std::vector<uint8_t>& _data,const std::string& _filePath)
{
	// Open the PNG file in binary mode
	std::ifstream inputFile(_filePath, std::ios::binary);
	if (!inputFile) {
		std::cerr << "Error opening file: " << _filePath << std::endl;
		return S_FALSE;
	}

	// Read file content into a vector of bytes
	std::vector<uint8_t> data((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
	inputFile.close();

	_data = std::move(data);

	return S_OK;
}

void AnimationBulider_Scene::Load()
{
	/*const wchar_t* filePath = ConvertCharToWChar(OpenFileDialog().c_str());
	auto object = Instantiate("test");
	object->SetLayer(LAYER_UI);
	object->AddComponent<Renderer>(filePath);

	delete filePath;*/

	auto object = Instantiate("test");
	object->SetLayer(LAYER_UI);

	imgLoadButton = Instantiate("imgLoadButton");
	imgLoadButton->transform.position = { 250.0f,0.0f };
	imgLoadButton->transform.scale = { 10.0f,3.0f };
	imgLoadButton->SetLayer(LAYER_UI);
	auto rend = imgLoadButton->AddComponent<Renderer>();
	rend->SetColor({ 0.2f,0.2f,0.2f,0.7f });
	auto button = imgLoadButton->AddComponent<Button>();
	button->SetEvent(std::move([&](){
		std::string filePath = OpenFileDialog();
		if (filePath != " ")
		{
			auto object = Instantiate("test");
			object->transform.scale = { 10.0f,10.0f };
			object->SetLayer(LAYER_UI);
			object->AddComponent<Renderer>()->SetTexture(filePath);
		}		
		}));
	auto text = imgLoadButton->AddComponent<SFText>();
	text->SetString("‰æ‘œ‚ð“Ç‚Ýž‚Þ");
	text->offset.y = 8.0f;

	/*img = Instantiate("img");
	img->SetLayer(LAYER_UI);
	img->AddComponent<Renderer>();*/
}

