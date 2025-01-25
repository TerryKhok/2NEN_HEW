#pragma once

//Scale = 1.0f �̂Ƃ��̑傫��
//================================================================
constexpr float DEFAULT_OBJECT_SIZE = 10.0f;
constexpr float HALF_OBJECT_SIZE = DEFAULT_OBJECT_SIZE / 2.0f;
constexpr float QUARTER_OBJECT_SIZE = DEFAULT_OBJECT_SIZE / 4.0f;
//================================================================

[[deprecated("������new�����Ă���̂Ń��������[�N���N���₷��..wstring_to_string���g�p����")]]
const char* ConvertWCharToChar(const wchar_t* wstr);

std::string wstring_to_string(const std::wstring& wstr);

std::wstring string_to_wstring(std::string oString);

std::string OpenFileDialog();

const wchar_t* GetFileExtension(const wchar_t* path);


