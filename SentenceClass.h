#ifndef _SENTENCE_CLASS_H_
#define _SENTENCE_CLASS_H_

#define _CRT_SECURE_NO_WARNINGS

#include "FontClass.h"
#include "TextureShaderClass.h"

class SentenceClass{
public:
	static int sentenceIDCount;

	SentenceClass();
	SentenceClass(const SentenceClass& other);
	SentenceClass(FontClass* font);
	~SentenceClass();

	void Initialize(FontClass* font, char* text, int posX, int posY, XMFLOAT4 textColor, ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight);
	void Shutdown();
	void UpdateSentence(char* text, int posX, int posY, XMFLOAT4 textColor);
	bool Render(HWND hwnd, TextureShaderClass* textureShader, ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& orthoMatrix);
	int GetSentenceID();
	int GetSentencePosX();
	int GetSentencePosY();
	char* GetSentenceCharArray();
	void SwapSentence(SentenceClass& other);
private:
	struct CharacterType{
		BitmapClass* pBitmap;
		int ctrX, ctrY;
	};
	struct SentenceType{
		CharacterType character[MAX_STR_LEN];
		int length;
		char text[MAX_STR_LEN];
		XMFLOAT4 textColor;
		int posX, posY;
	};

	SentenceType m_Sentence;
	FontClass* m_Font;

	int m_sentenceID;
};



#endif