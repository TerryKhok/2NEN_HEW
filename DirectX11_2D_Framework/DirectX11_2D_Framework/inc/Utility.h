#pragma once

[[deprecated("������new�����Ă���̂Ń��������[�N���N���₷��..wstring_to_string���g�p����")]]
const char* ConvertWCharToChar(const wchar_t* wstr);

std::string wstring_to_string(const std::wstring& wstr);

std::wstring string_to_wstring(std::string oString);

std::string OpenFileDialog();

const wchar_t* GetFileExtension(const wchar_t* path);

