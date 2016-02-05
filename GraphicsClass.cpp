#include "GraphicsClass.h"

GraphicsClass::GraphicsClass(){
	m_D3D = 0;
	m_Camera = 0;
	m_TextureShader = 0;
	m_Bitmap = 0;
	m_Sentence = 0;
	m_FadeBitmap = 0;
}

GraphicsClass::GraphicsClass(const GraphicsClass& other){}

GraphicsClass::~GraphicsClass(){}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd){
	bool result;
	XMMATRIX baseViewMatrix;

	m_D3D = new D3DClass;
	if (!m_D3D){
		return false;
	}
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, false, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result){
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	m_Camera = new CameraClass;
	if (!m_Camera){
		return false;
	}
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(baseViewMatrix);

	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader){
		return false;
	}
	result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	m_Bitmap = new BitmapType[MAX_BITMAP_COUNT];
	if (!m_Bitmap){
		return false;
	}

	m_Sentence = new SentenceClass[MAX_SENTENCE_COUNT];
	if (!m_Sentence){
		return false;
	}

	result = InitializeFadingEffect(hwnd, screenWidth, screenHeight);
	if (!result){
		return false;
	}

	m_bitmapCount = 0;
	m_bitmapIDCount = 1;
	m_sentenceCount = 0;
	
	return true;
}

void GraphicsClass::Shutdown(){
	if (m_FadeBitmap){
		if (m_FadeBitmap->bitmap){
			m_FadeBitmap->bitmap->Shutdown();
			delete m_FadeBitmap->bitmap;
			m_FadeBitmap->bitmap = 0;
		}
		delete m_FadeBitmap;
		m_FadeBitmap = 0;
	}
	if (m_Bitmap){
		for (int i = 0; i < m_bitmapCount; i++){
			if (m_Bitmap[i].bitmap){
				m_Bitmap[i].bitmap->Shutdown();
				delete m_Bitmap[i].bitmap;
				m_Bitmap[i].bitmap = 0;
			}
		}
		delete[] m_Bitmap;
		m_Bitmap = 0;
	}
	if (m_Sentence){
		for (int i = 0; i < m_sentenceCount; i++){
			m_Sentence[i].Shutdown();
		}
		delete[] m_Sentence;
		m_Sentence = 0;
	}
	if (m_TextureShader){
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}
	if (m_Camera){
		delete m_Camera;
		m_Camera = 0;
	}
	if (m_D3D){
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}
}

void GraphicsClass::BeginRendering(){
	bool result;
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix, projectionMatrix;
	m_D3D->BeginScene(0.1f, 0.3f, 0.3f, 1.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_D3D->GetOrthoMatrix(orthoMatrix);
	m_D3D->TurnZBufferOff();
	m_D3D->TurnOnAlphaBlending();
	m_worldMatrix = XMMatrixTranspose(worldMatrix);
	m_viewMatrix = XMMatrixTranspose(viewMatrix);
	m_orthoMatrix = XMMatrixTranspose(orthoMatrix);
}

bool GraphicsClass::EndRendering(int screenWidth, int screenHeight){
	bool result;
	if (fading){
		result = m_FadeBitmap->bitmap->Render(m_D3D->GetDeviceContext(), m_FadeBitmap->posX, m_FadeBitmap->posY, screenWidth, screenHeight);
		if (!result){
			return false;
		}
		result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_FadeBitmap->bitmap->GetIndexCount(), m_worldMatrix, m_viewMatrix, m_orthoMatrix, m_FadeBitmap->bitmap->GetTexture(), m_FadeBitmap->blend, NULL_COLOR);
		if (!result){
			return false;
		}
	}
	m_D3D->TurnOffAlphaBlending();
	m_D3D->TurnZBufferOn();
	m_D3D->EndScene();

	return true;
}

bool GraphicsClass::AddBitmap(HWND hwnd, char* path, RECT bitmapRect, int screenWidth, int screenHeight, int& bitmapID, float angle, float blend, XMFLOAT4 hueColor){
	bool result;
	int posX = (bitmapRect.left + bitmapRect.right) / 2;
	int posY = (bitmapRect.top + bitmapRect.bottom) / 2;
	int bitmapWidth = abs(bitmapRect.right - bitmapRect.left);
	int bitmapHeight = abs(bitmapRect.bottom - bitmapRect.top);

	if (m_bitmapCount >= MAX_BITMAP_COUNT){
		return false;
	}

	const int maxLength = 256;
	char currentDirectory[maxLength];
	_getcwd(currentDirectory, maxLength);
	char tgaDirectory[maxLength];
	strcpy(tgaDirectory, currentDirectory);
	strcat(tgaDirectory, path);

	m_Bitmap[m_bitmapCount].bitmap = new BitmapClass;
	if (!m_Bitmap[m_bitmapCount].bitmap){
		return false;
	}

	result = m_Bitmap[m_bitmapCount].bitmap->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, tgaDirectory, bitmapWidth, bitmapHeight);
	if (!result){
		delete m_Bitmap[m_bitmapCount].bitmap;
		m_Bitmap[m_bitmapCount].bitmap = 0;
		return false;
	}

	m_Bitmap[m_bitmapCount].bitmapWidth = bitmapWidth;
	m_Bitmap[m_bitmapCount].bitmapHeight = bitmapHeight;
	m_Bitmap[m_bitmapCount].posX = posX;
	m_Bitmap[m_bitmapCount].posY = posY;
	m_Bitmap[m_bitmapCount].ID = m_bitmapIDCount;
	m_Bitmap[m_bitmapCount].blend = blend;
	m_Bitmap[m_bitmapCount].hueColor = hueColor;
	m_Bitmap[m_bitmapCount].xFlip = false;
	m_Bitmap[m_bitmapCount].yFlip = false;
	m_Bitmap[m_bitmapCount].rotation = angle;

	bitmapID = m_Bitmap[m_bitmapCount].ID;

	m_bitmapCount++;
	m_bitmapIDCount++;

	return true;
}

void GraphicsClass::UpdateBitmap(int bitmapID, int posX, int posY, float angle, float blend, XMFLOAT4 hueColor){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			m_Bitmap[i].posX = posX;
			m_Bitmap[i].posY = posY;
			m_Bitmap[i].rotation = angle;
			m_Bitmap[i].blend = blend;
			m_Bitmap[i].hueColor = hueColor;
			break;
		}
	}
}

void GraphicsClass::UpdateBitmap(int bitmapID, RECT rc, float angle, float blend, XMFLOAT4 hueColor){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			m_Bitmap[i].posX = (rc.right + rc.left) / 2;
			m_Bitmap[i].posY = (rc.bottom + rc.top) / 2;
			m_Bitmap[i].bitmapWidth = rc.right - rc.left;
			m_Bitmap[i].bitmapHeight = rc.bottom - rc.top;
			m_Bitmap[i].rotation = angle;
			m_Bitmap[i].blend = blend;
			m_Bitmap[i].hueColor = hueColor;
			break;
		}
	}
}

void GraphicsClass::DeleteBitmap(int bitmapID){
	int index = -1;

	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			index = i;
		}
	}
	if (index == -1){
		return;
	}

	if (m_Bitmap[index].bitmap){
		m_Bitmap[index].bitmap->Shutdown();
		m_Bitmap[index].bitmap = 0;
	}

	for (int i = index; i < m_bitmapCount-1; i++){
		m_Bitmap[i].bitmap = m_Bitmap[i + 1].bitmap;
		m_Bitmap[i].bitmapHeight = m_Bitmap[i + 1].bitmapHeight;
		m_Bitmap[i].bitmapWidth = m_Bitmap[i + 1].bitmapWidth;
		m_Bitmap[i].posX = m_Bitmap[i + 1].posX;
		m_Bitmap[i].posY = m_Bitmap[i + 1].posY;
		m_Bitmap[i].ID = m_Bitmap[i + 1].ID;
	}

	m_Bitmap[m_bitmapCount].bitmap = 0;
	m_Bitmap[m_bitmapCount].ID = 0;

	m_bitmapCount--;
}

bool GraphicsClass::RenderBitmap(int bitmapID){
	bool result;
	int index = -1;

	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			index = i;
		}
	}
	if (index == -1){
		return false;
	}

	result = m_Bitmap[index].bitmap->Render(m_D3D->GetDeviceContext(), m_Bitmap[index].posX, m_Bitmap[index].posY, m_Bitmap[index].bitmapWidth, m_Bitmap[index].bitmapHeight, m_Bitmap[index].rotation);
	if (!result){
		return false;
	}
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Bitmap[index].bitmap->GetIndexCount(), m_worldMatrix, m_viewMatrix, m_orthoMatrix, m_Bitmap[index].bitmap->GetTexture(), m_Bitmap[index].blend, m_Bitmap[index].hueColor, m_Bitmap[index].xFlip, m_Bitmap[index].yFlip);
	if (!result){
		return false;
	}

	return true;
}

void GraphicsClass::EnableBitmapXFlip(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			m_Bitmap[i].xFlip = true;
			break;
		}
	}
}

void GraphicsClass::DisableBitmapXFlip(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			m_Bitmap[i].xFlip = false;
			break;
		}
	}
}

void GraphicsClass::EnableBitmapYFlip(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			m_Bitmap[i].yFlip = true;
			break;
		}
	}
}

void GraphicsClass::DisableBitmapYFlip(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			m_Bitmap[i].yFlip = false;
			break;
		}
	}
}

int GraphicsClass::GetBitmapWidth(int bitmapID){

	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			return m_Bitmap[i].bitmapWidth;
		}
	}
}

int GraphicsClass::GetBitmapHeight(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			return m_Bitmap[i].bitmapHeight;
		}
	}
}

bool GraphicsClass::AddSentence(HWND hwnd, char* text, int posX, int posY, int screenWidth, int screenHeight, XMFLOAT4 textColor, int& id){
	bool result;
	result = m_Sentence[m_sentenceCount].Initialize(text, posX, posY, textColor, m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight);
	if (!result){
		return false;
	}
	id = m_Sentence[m_sentenceCount].GetSentenceID();

	m_sentenceCount++;

	return true;
}

void GraphicsClass::UpdateSentence(int sentenceID, char* text, int posX, int posY, XMFLOAT4 textColor){
	for (int i = 0; i < m_sentenceCount; i++){
		if (sentenceID == m_Sentence[i].GetSentenceID()){
			m_Sentence[i].UpdateSentence(text, posX, posY, textColor);
			break;
		}
	}
}

bool GraphicsClass::RenderSentence(int sentenceID){
	bool result;
	int index = -1;

	for (int i = 0; i < m_sentenceCount; i++){
		if (sentenceID==m_Sentence[i].GetSentenceID()){
			index = i;
			break;
		}
	}
	if (index == -1){
		return false;
	}

	result = m_Sentence[index].Render(m_TextureShader, m_D3D->GetDeviceContext(), m_worldMatrix, m_viewMatrix, m_orthoMatrix);
	if (!result){
		return false;
	}

	return true;
}

void GraphicsClass::DeleteSentence(int& sentenceID){
	int index = -1;
	for (int i = 0; i < m_sentenceCount; i++){
		if (sentenceID == m_Sentence[i].GetSentenceID()){
			index = i;
			break;
		}
	}
	if (index == -1){
		return;
	}
	m_Sentence[index].Shutdown();
	for (int i = index; i < m_sentenceCount - 1; i++){
		m_Sentence[i] = m_Sentence[i + 1];
	}
	sentenceID = -1;
	m_sentenceCount--;
}

bool GraphicsClass::InitializeFadingEffect(HWND hwnd, int screenWidth, int screenHeight){
	bool result;

	m_FadeBitmap = new BitmapType;
	if (!m_FadeBitmap){
		return false;
	}

	m_FadeBitmap->bitmap = new BitmapClass;
	if (!m_FadeBitmap->bitmap){
		return false;
	}

	const int maxLength = 256;
	char tgaDirectory[maxLength];
	_getcwd(tgaDirectory, maxLength);
	strcat(tgaDirectory, "/Data/fade.tga");

	result = m_FadeBitmap->bitmap->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, tgaDirectory, screenWidth, screenHeight);
	if (!result){
		return false;
	}

	m_FadeBitmap->bitmapWidth = screenWidth;
	m_FadeBitmap->bitmapHeight = screenHeight;
	m_FadeBitmap->posX = screenWidth / 2;
	m_FadeBitmap->posY = screenHeight / 2;
	m_FadeBitmap->blend = 0.0f;

	fading = false;

	return true;
}

void GraphicsClass::StartFadingEffect(){
	fading = true;
}

void GraphicsClass::SetFadingEffect(float blend){
	m_FadeBitmap->blend = blend;
}

void GraphicsClass::StopFadingEffect(){
	fading = false;
}