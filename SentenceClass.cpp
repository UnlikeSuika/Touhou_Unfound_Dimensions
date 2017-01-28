#include "SentenceClass.h"

int SentenceClass::sentenceIDCount = 0;

SentenceClass::SentenceClass(){
	m_Font = NULL;
}

SentenceClass::SentenceClass(const SentenceClass& other){
	m_Font = NULL;
}

SentenceClass::SentenceClass(FontClass* font) {
	m_Font = font;
}

SentenceClass::~SentenceClass(){}

void SentenceClass::Initialize(FontClass* font, char* text, int posX, int posY, XMFLOAT4 textColor, ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight){

	m_Font = font;

	posX = posX + m_Font->GetCharacterWidth(text[0]) / 2;
	posY = posY + m_Font->GetCharacterHeight(text[0]) / 2;

	UpdateSentence(text, posX, posY, textColor);

	sentenceIDCount++;
	m_sentenceID = sentenceIDCount;

}

void SentenceClass::UpdateSentence(char* text, int posX, int posY, XMFLOAT4 textColor){

	m_Sentence.length = (int)strlen(text);
	m_Sentence.textColor = textColor;

	m_Sentence.posX = posX;
	m_Sentence.posY = posY;

	int currentXPos = posX;
	int currentYPos = posY;

	for (int i = 0; i < MAX_STR_LEN; i++){
		m_Sentence.character[i].pBitmap = NULL;
	}

	for (int i = 0; i < m_Sentence.length; i++){
		if (text[i] == '\0') {
			break;
		}
		else if (text[i] == ' '){
			m_Sentence.character[i].pBitmap = NULL;
			currentXPos += 5;
		}
		else if (text[i] == '\n'){
			m_Sentence.character[i].pBitmap = NULL;
			currentXPos = posX;
			currentYPos += 12;
		}
		else{
			int width = m_Font->GetCharacterWidth(text[i]);
			m_Sentence.character[i].ctrX = currentXPos + width / 2;
			m_Sentence.character[i].ctrY = currentYPos;
			m_Sentence.character[i].pBitmap = m_Font->GetCharacterBitmap(text[i]);
			currentXPos += width + 2;
		}
	}

	strcpy(m_Sentence.text, text);

}

void SentenceClass::Shutdown(){

}

bool SentenceClass::Render(HWND hwnd, TextureShaderClass* textureShader, ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& orthoMatrix){
	bool result;
	for (int i = 0; i < m_Sentence.length; i++){
		if (m_Sentence.text[i] == '\0') {
			break;
		}
		else if (m_Sentence.character[i].pBitmap){
			XMFLOAT4 processedTextColor = { m_Sentence.textColor.x, m_Sentence.textColor.y, m_Sentence.textColor.z, 1.0f };
			result = m_Sentence.character[i].pBitmap->Render(deviceContext, m_Sentence.character[i].ctrX, m_Sentence.character[i].ctrY, m_Sentence.character[i].pBitmap->GetWidth(), m_Sentence.character[i].pBitmap->GetHeight());
			if (!result){
				return false;
			}
			result = textureShader->Render(hwnd, deviceContext, m_Sentence.character[i].pBitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_Sentence.character[i].pBitmap->GetTexture(), m_Sentence.textColor.w, processedTextColor);
			if (!result){
				return false;
			}
		}
	}
	return true;
}

int SentenceClass::GetSentenceID(){
	return m_sentenceID;
}

int SentenceClass::GetSentencePosX(){
	return m_Sentence.posX;
}

int SentenceClass::GetSentencePosY(){
	return m_Sentence.posY;
}

char* SentenceClass::GetSentenceCharArray(){
	return m_Sentence.text;
}

void SentenceClass::SwapSentence(SentenceClass& other){
	if (m_Font){
		m_Font->Shutdown();
		m_Font = 0;
	}
	m_Font = other.m_Font;
	other.m_Font = 0;

	m_sentenceID = other.m_sentenceID;
	UpdateSentence(other.m_Sentence.text, other.m_Sentence.posX, other.m_Sentence.posY, other.m_Sentence.textColor);
}