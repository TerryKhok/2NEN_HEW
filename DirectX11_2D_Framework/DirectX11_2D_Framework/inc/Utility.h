#pragma once

[[deprecated("������new�����Ă���̂Ń��������[�N���N���₷��..wstring_to_string���g�p����")]]
const char* ConvertWCharToChar(const wchar_t* wstr);

std::string wstring_to_string(const std::wstring& wstr);

std::string OpenFileDialog();
