

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

std::string wstring_to_string(const std::wstring& wstr) {
    std::size_t len = wstr.size() * 4; // maximum possible length
    char* buffer = new char[len];
    std::size_t convertedChars = 0;

    // _wcstombs_s is the safer version
    errno_t err = wcstombs_s(&convertedChars, buffer, len, wstr.c_str(), len - 1);
    std::string str(buffer);
    delete[] buffer;

    if (err != 0) {
        // Handle error
        return "";
    }

    return str;
}

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
