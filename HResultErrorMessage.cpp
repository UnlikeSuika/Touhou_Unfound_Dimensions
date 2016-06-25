#include "HResultErrorMessage.h"

void OutputErrorMessage(HWND hwnd, HRESULT hr, LPCWSTR prefix){
	std::wstringstream wstream;
	wstream << std::hex << hr;
	
	std::wstring errMsg = std::wstring(prefix) + wstream.str();

	MessageBox(hwnd, errMsg.c_str(), L"Error", MB_OK);
}

void OutputErrorMessageWithFilename(HWND hwnd, HRESULT hr, char* filename, LPCWSTR prefix){
	size_t origSize = strlen(filename) + 1;
	const size_t newSize = 200;
	size_t convertedChars = 0;
	wchar_t wcstr[newSize];
	mbstowcs_s(&convertedChars, wcstr, origSize, filename, _TRUNCATE);

	std::wstringstream wstream;
	wstream << std::hex << hr;

	std::wstring errMsg = prefix + std::wstring(wcstr) + L"\nHResult code: " + wstream.str();

	MessageBox(hwnd, errMsg.c_str(), L"Error", MB_OK);
}