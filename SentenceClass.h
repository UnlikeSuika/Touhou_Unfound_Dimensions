#ifndef _SENTENCE_CLASS_H_
#define _SENTENCE_CLASS_H_

#define _CRT_SECURE_NO_WARNINGS

#include "FontClass.h"
#include "TextureShaderClass.h"

#ifndef _MCC_
#define _MCC_
const int MAX_CHARACTER_COUNT = 256;
#endif

class SentenceClass{
public:
	static int sentenceIDCount;

	SentenceClass();
	SentenceClass(const SentenceClass& other);
	~SentenceClass();

	bool Initialize(char* text, int posX, int posY, XMFLOAT4 textColor, ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight);
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
		CharacterType character[MAX_CHARACTER_COUNT];
		int length;
		char text[MAX_CHARACTER_COUNT];
		XMFLOAT4 textColor;
		int posX, posY;
	};

	SentenceType m_Sentence;
	FontClass* m_Font;

	int m_sentenceID;
};



#endif