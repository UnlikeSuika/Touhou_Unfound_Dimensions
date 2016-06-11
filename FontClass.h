#ifndef _FONT_CLASS_H_
#define _FONT_CLASS_H_

#define _CRT_SECURE_NO_WARNINGS

#include "BitmapClass.h"
#include <string>
#include <direct.h>

using namespace std;

const int MAX_CHARACTERS_IN_FONT = 94;

class FontClass{
public:
	FontClass();
	FontClass(const FontClass& other);
	~FontClass();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight);
	void Shutdown();
	BitmapClass* GetCharacterBitmap(char c);
	int GetCharacterWidth(char c);
	int GetCharacterHeight(char c);
	void operator=(FontClass other);
private:
	struct CharacterType{
		BitmapClass* bitmap;
		int width, height;
	};

	CharacterType m_Font[MAX_CHARACTERS_IN_FONT];

};

#endif