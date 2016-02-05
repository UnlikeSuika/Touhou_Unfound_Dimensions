#include "SentenceClass.h"

int SentenceClass::sentenceIDCount = 0;

SentenceClass::SentenceClass(){
	m_Font = 0;
}

SentenceClass::SentenceClass(const SentenceClass& other){}

SentenceClass::~SentenceClass(){}

bool SentenceClass::Initialize(char* text, int posX, int posY, XMFLOAT4 textColor, ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight){
	bool result;

	m_Font = new FontClass;
	result = m_Font->Initialize(device, deviceContext, hwnd, screenWidth, screenHeight);
	if (!result){
		return false;
	}

	posX = posX + m_Font->GetCharacterWidth(text[0]) / 2;
	posY = posY + m_Font->GetCharacterHeight(text[0]) / 2;

	UpdateSentence(text, posX, posY, textColor);

	sentenceIDCount++;
	m_sentenceID = sentenceIDCount;

	return true;
}

void SentenceClass::UpdateSentence(char* text, int posX, int posY, XMFLOAT4 textColor){

	m_Sentence.length = strlen(text);
	m_Sentence.textColor = textColor;

	m_Sentence.posX = posX;
	m_Sentence.posY = posY;

	int currentXPos = posX;
	int currentYPos = posY;

	for (int i = 0; i < MAX_CHARACTER_COUNT; i++){
		m_Sentence.character[i].pBitmap = 0;
	}

	for (int i = 0; i < m_Sentence.length; i++){
		m_Sentence.character[i].posX = currentXPos;
		m_Sentence.character[i].posY = currentYPos;
		if (text[i] == ' '){
			m_Sentence.character[i].pBitmap = NULL;
			currentXPos += 3;
		}
		else if (text[i] == '\n'){
			m_Sentence.character[i].pBitmap = NULL;
			currentYPos += m_Font->GetCharacterHeight(text[i]);
		}
		else{
			m_Sentence.character[i].pBitmap = m_Font->GetCharacterBitmap(text[i]);
			currentXPos += m_Font->GetCharacterWidth(text[i]) + 1;
		}
	}

	strcpy(m_Sentence.text, text);

}

void SentenceClass::Shutdown(){

	if (m_Font){
		m_Font->Shutdown();
		delete m_Font;
		m_Font = 0;
	}
}

bool SentenceClass::Render(TextureShaderClass* textureShader, ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& orthoMatrix){
	bool result;
	for (int i = 0; i < m_Sentence.length; i++){
		if (m_Sentence.character[i].pBitmap){
			XMFLOAT4 processedTextColor = { m_Sentence.textColor.x, m_Sentence.textColor.y, m_Sentence.textColor.z, 1.0f };
			result = m_Sentence.character[i].pBitmap->Render(deviceContext, m_Sentence.character[i].posX, m_Sentence.character[i].posY, m_Sentence.character[i].pBitmap->GetWidth(), m_Sentence.character[i].pBitmap->GetHeight());
			if (!result){
				return false;
			}
			result = textureShader->Render(deviceContext, m_Sentence.character[i].pBitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_Sentence.character[i].pBitmap->GetTexture(), m_Sentence.textColor.w, processedTextColor);
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