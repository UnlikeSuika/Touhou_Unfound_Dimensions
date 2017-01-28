#include "GraphicsClass.h"

//GraphicsClass object initializer.
//Use GraphicsClass::Initialize to initialize the system.
GraphicsClass::GraphicsClass(){
	m_D3D = NULL;
	m_Camera = NULL;
	m_TextureShader = NULL;
	m_FadeBitmap = NULL;
}

//GraphicsClass object initializer via GraphicsClass& argument.
//Do not use this initializer.
GraphicsClass::GraphicsClass(const GraphicsClass& other){}

//GraphicsClass destructor. Do not use this deleter.
//Use GraphicsClass::Shutdown instead.
GraphicsClass::~GraphicsClass(){}

//Initializes GraphicsClass variables
bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd){
	bool result;

	//create D3DClass object
	m_D3D = new D3DClass;
	if (!m_D3D){
		MessageBox(hwnd, L"Could not create Direct3D object.", L"Error", MB_OK);
		return false;
	}

	//initialize D3DClass object
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, false, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result){
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	//create CameraClass object
	m_Camera = new CameraClass;
	if (!m_Camera){
		MessageBox(hwnd, L"Could not create camera object.", L"Error", MB_OK);
		return false;
	}

	//set camera position to (0, 0, -10)
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);

	//set up view matrix
	m_Camera->Render();

	//get view matrix
	XMMATRIX baseViewMatrix;
	m_Camera->GetViewMatrix(baseViewMatrix);

	//create TextureShaderClass object
	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader){
		MessageBox(hwnd, L"Could not create texture shader object.", L"Error", MB_OK);
		return false;
	}

	//initialize TextureShaderClass object
	result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result){
		MessageBox(hwnd, L"Could not initialize texture shader object.", L"Error", MB_OK);
		return false;
	}

	//initialize fading effect
	result = InitializeFadingEffect(screenWidth, screenHeight);
	if (!result){
		MessageBox(hwnd, L"Could not initialize fading effect.", L"Error", MB_OK);
		return false;
	}

	m_Font = new FontClass;
	result = m_Font->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), m_hwnd, screenWidth, screenHeight);
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the font.", L"Error", MB_OK);
		return false;
	}

	m_hwnd = hwnd;

	m_bitmapCount = 0;
	m_bitmapIDCount = 1;
	m_sentenceCount = 0;
	
	return true;
}

//Shuts down GraphicsClass variables
void GraphicsClass::Shutdown() {
	if (m_FadeBitmap) {
		if (m_FadeBitmap->bitmap) {
			m_FadeBitmap->bitmap->Shutdown();
			delete m_FadeBitmap->bitmap;
			m_FadeBitmap->bitmap = NULL;
		}
		delete m_FadeBitmap;
		m_FadeBitmap = NULL;
	}

	for (int i = 0; i < m_bitmapCount; i++) {
		if (m_Bitmap[i].bitmap) {
			m_Bitmap[i].bitmap->Shutdown();
			delete m_Bitmap[i].bitmap;
			m_Bitmap[i].bitmap = NULL;
		}
	}
	m_bitmapCount = 0;


	for (int i = 0; i < m_sentenceCount; i++) {
		m_Sentence[i].Shutdown();
	}
	m_sentenceCount = 0;

	if (m_Font) {
		m_Font->Shutdown();
		m_Font = NULL;
	}

	if (m_TextureShader){
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = NULL;
	}
	if (m_Camera){
		delete m_Camera;
		m_Camera = NULL;
	}
	if (m_D3D){
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = NULL;
	}
}

//Marks the start of rendering. Before this function call,
//no bitmaps can be rendered within the given frame.
void GraphicsClass::BeginRendering(){
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

//Marks the end of rendering. After this function call,
//no bitmaps can be rendered within the given frame.
bool GraphicsClass::EndRendering(int screenWidth, int screenHeight){
	bool result;
	if (fading){
		result = m_FadeBitmap->bitmap->Render(m_D3D->GetDeviceContext(), m_FadeBitmap->posX, m_FadeBitmap->posY, screenWidth, screenHeight);
		if (!result){
			return false;
		}
		result = m_TextureShader->Render(m_hwnd, m_D3D->GetDeviceContext(), m_FadeBitmap->bitmap->GetIndexCount(), m_worldMatrix, m_viewMatrix, m_orthoMatrix, m_FadeBitmap->bitmap->GetTexture(), m_FadeBitmap->blend, NULL_COLOR);
		if (!result){
			return false;
		}
	}
	m_D3D->TurnOffAlphaBlending();
	m_D3D->TurnZBufferOn();
	m_D3D->EndScene();

	return true;
}

//Add a bitmap from given TGA path and with given location (bitmapRect), angle, alpha value, and a hue colour
bool GraphicsClass::AddBitmap(char* path, RECT bitmapRect, int screenWidth, int screenHeight, int& bitmapID, float angle, float blend, XMFLOAT4 hueColor){
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

	result = m_Bitmap[m_bitmapCount].bitmap->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), m_hwnd, screenWidth, screenHeight, tgaDirectory, bitmapWidth, bitmapHeight);
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

//Update a bitmap with given location (posX, posY), angle, alpha value, and a hue colour
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

//Update a bitmap with given location (rc), angle, alpha value, and a hue colour
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

//Delete the bitmap with the given ID
void GraphicsClass::DeleteBitmap(int bitmapID){
	int index = -1;

	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			index = i;
			break;
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

//Render the bitmap with given ID
bool GraphicsClass::RenderBitmap(int bitmapID){
	bool result;
	int index = -1;

	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			index = i;
			break;
		}
	}
	if (index == -1){
		return false;
	}

	result = m_Bitmap[index].bitmap->Render(m_D3D->GetDeviceContext(), m_Bitmap[index].posX, m_Bitmap[index].posY, m_Bitmap[index].bitmapWidth, m_Bitmap[index].bitmapHeight, m_Bitmap[index].rotation);
	if (!result){
		return false;
	}
	result = m_TextureShader->Render(m_hwnd, m_D3D->GetDeviceContext(), m_Bitmap[index].bitmap->GetIndexCount(), m_worldMatrix, m_viewMatrix, m_orthoMatrix, m_Bitmap[index].bitmap->GetTexture(), m_Bitmap[index].blend, m_Bitmap[index].hueColor, m_Bitmap[index].xFlip, m_Bitmap[index].yFlip);
	if (!result){
		return false;
	}

	return true;
}

//Turn on horizontal flip for the bitmap
void GraphicsClass::EnableBitmapXFlip(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			m_Bitmap[i].xFlip = true;
			break;
		}
	}
}

//Turn off horizontal flip for the bitmap
void GraphicsClass::DisableBitmapXFlip(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			m_Bitmap[i].xFlip = false;
			break;
		}
	}
}

//Turn on vertical flip for the bitmap
void GraphicsClass::EnableBitmapYFlip(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			m_Bitmap[i].yFlip = true;
			break;
		}
	}
}

//Turn off vertical flip for the bitmap
void GraphicsClass::DisableBitmapYFlip(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			m_Bitmap[i].yFlip = false;
			break;
		}
	}
}

//Returns width of bitmap in pixels, returns -1 if bitmap not found
int GraphicsClass::GetBitmapWidth(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			return m_Bitmap[i].bitmapWidth;
		}
	}
	return -1;
}

//Returns height of bitmap in pixels, returns -1 if bitmap not found
int GraphicsClass::GetBitmapHeight(int bitmapID){
	for (int i = 0; i < m_bitmapCount; i++){
		if (m_Bitmap[i].ID == bitmapID){
			return m_Bitmap[i].bitmapHeight;
		}
	}
	return -1;
}

//Adds a sentence with given text, position (posX, posY), and text colour
bool GraphicsClass::AddSentence(char* text, int posX, int posY, int screenWidth, int screenHeight, XMFLOAT4 textColor, int& id){

	m_Sentence[m_sentenceCount].Initialize(m_Font, text, posX, posY, textColor, m_D3D->GetDevice(), m_D3D->GetDeviceContext(), m_hwnd, screenWidth, screenHeight);
	
	id = m_Sentence[m_sentenceCount].GetSentenceID();

	m_sentenceCount++;

	return true;
}

//Updates the sentence with given text, position (posX, posY), and text colour
void GraphicsClass::UpdateSentence(int sentenceID, char* text, int posX, int posY, XMFLOAT4 textColor){
	for (int i = 0; i < m_sentenceCount; i++){
		if (sentenceID == m_Sentence[i].GetSentenceID()){
			m_Sentence[i].UpdateSentence(text, posX, posY, textColor);
			break;
		}
	}
}

//Renders the sentence
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

	result = m_Sentence[index].Render(m_hwnd, m_TextureShader, m_D3D->GetDeviceContext(), m_worldMatrix, m_viewMatrix, m_orthoMatrix);
	if (!result){
		return false;
	}

	return true;
}

//Deletes the sentence
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

//Initializes the fading effect
bool GraphicsClass::InitializeFadingEffect(int screenWidth, int screenHeight){
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

	result = m_FadeBitmap->bitmap->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), m_hwnd, screenWidth, screenHeight, tgaDirectory, screenWidth, screenHeight);
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

//Marks the start of fading effect
void GraphicsClass::StartFadingEffect(){
	fading = true;
}

//Set how much the screen should fade (by alpha value)
void GraphicsClass::SetFadingEffect(float blend){
	m_FadeBitmap->blend = blend;
}

//Marks the end of fading effect
void GraphicsClass::StopFadingEffect(){
	fading = false;
}