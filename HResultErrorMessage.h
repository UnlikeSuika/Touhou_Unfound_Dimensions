#ifndef _HRESULT_ERROR_MESSAGE_H_
#define _HRESULT_ERROR_MESSAGE_H_

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>
#include <string>
#include <sstream>

using namespace std;

void OutputErrorMessage(HWND hwnd, HRESULT hr, LPCWSTR prefix);

#endif