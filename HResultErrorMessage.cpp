#include "HResultErrorMessage.h"

void OutputErrorMessage(HWND hwnd, HRESULT hr, LPCWSTR prefix){
	wstring prefixWStr(prefix);

	wstringstream wstream;
	wstream << hex << hr;
	wstring hrStr(wstream.str());
	
	wstring errStr = prefixWStr + hrStr;

	MessageBox(hwnd, errStr.c_str(), L"Error", MB_OK);
}