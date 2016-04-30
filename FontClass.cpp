#include "FontClass.h"

FontClass::FontClass(){
}

FontClass::FontClass(const FontClass& other){}

FontClass::~FontClass(){}

bool FontClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight){
	bool result;

	if (!m_Font){
		MessageBox(hwnd, L"Out of memory. Could not initialize font object.", L"Error", MB_OK);
		return false;
	}

	const int maxLength = 256;
	char currentDirectory[maxLength];
	_getcwd(currentDirectory, maxLength);

	for (int i = 0; i < MAX_CHARACTERS_IN_FONT; i++){
		char tgaDirectory[maxLength];
		strcpy(tgaDirectory, currentDirectory);
		strcat(tgaDirectory, "\\Data\\font\\");
		string tgaNoStr = to_string(i + 33);
		const char *tgaNoCStr = tgaNoStr.c_str();
		strcat(tgaDirectory, tgaNoCStr);
		strcat(tgaDirectory, ".tga");

		m_Font[i].bitmap = new BitmapClass;
		if (!m_Font[i].bitmap){
			return false;
		}
		result = m_Font[i].bitmap->Initialize(device, deviceContext, hwnd, screenWidth, screenHeight, tgaDirectory);
		if (!result){
			return false;
		}
		m_Font[i].width = m_Font[i].bitmap->GetWidth();
		m_Font[i].height = m_Font[i].bitmap->GetHeight();
	}
	
	return true;
}

void FontClass::Shutdown(){
	for (int i = 0; i < 94; i++){
		m_Font[i].bitmap->Shutdown();
		delete m_Font[i].bitmap;
		m_Font[i].bitmap = 0;
	}
}

BitmapClass* FontClass::GetCharacterBitmap(char c){
	return m_Font[(int)c - 33].bitmap;
}

int FontClass::GetCharacterWidth(char c){
	return m_Font[(int)c - 33].width;
}

int FontClass::GetCharacterHeight(char c){
	return m_Font[(int)c - 33].height;
}