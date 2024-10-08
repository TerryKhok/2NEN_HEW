#include "Debug.h"

PointerRegistryManager PointerRegistryManager::manager;//std::unordered_map<GameObject*, std::vector<LoggingPointer<GameObject>*>> LoggingPointerManager::m_pointerList;

const char* relativePath(const char* fullPath) {
#ifdef _WIN32
	const char separator = '\\'; // Windows uses backslashes
#else
	const char separator = '/';  // Unix-like systems use slashes
#endif
	const char* relative = strrchr(fullPath, separator);
	return relative ? relative + 1 : fullPath; // Return the file name if found, else return the full path
}

void setConsoleTextColor(unsigned int color)
{
#ifdef DEBUG_TRUE
	static const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
#endif
}

