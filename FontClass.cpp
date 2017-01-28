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

	char currentDirectory[MAX_STR_LEN];
	_getcwd(currentDirectory, MAX_STR_LEN);

	for (int i = 0; i < MAX_CHARACTERS_IN_FONT; i++){
		std::string tgaDirStr = currentDirectory + std::string("\\Data\\font\\") + std::to_string(i + 33) + ".tga";
		char tgaDirectory[MAX_STR_LEN];
		strcpy(tgaDirectory, tgaDirStr.c_str());

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