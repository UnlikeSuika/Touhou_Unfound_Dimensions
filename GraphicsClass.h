#ifndef GRAPHICSCLASSH
#define GRAPHICSCLASSH

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <direct.h>
#include <math.h>
#include "D3DClass.h"
#include "CameraClass.h"
#include "TextureShaderClass.h"
#include "BitmapClass.h"
#include "SentenceClass.h"

//const bool FULL_SCREEN = true;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

const int MAX_BITMAP_COUNT = 100;
const int MAX_SENTENCE_COUNT = 30;

class GraphicsClass{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass& other);
	~GraphicsClass();

	bool Initialize(int screenWidth, int screenHeight, HWND hwnd);
	void Shutdown();
	void BeginRendering();
	bool EndRendering(int screenWidth, int screenHeight);

	bool AddBitmap(HWND hwnd, char* path, RECT bitmapRect, int screenWidth, int screenHeight, int& bitmapID, float angle = 0.0f, float blend = 1.0f, XMFLOAT4 hueColor = NULL_COLOR);
	void UpdateBitmap(int bitmapID, int posX, int posY, float angle = 0.0f, float blend = 1.0f, XMFLOAT4 hueColor = NULL_COLOR);
	void UpdateBitmap(int bitmapID, RECT rc, float angle = 0.0f, float blend = 1.0f, XMFLOAT4 hueColor = NULL_COLOR);
	void DeleteBitmap(int bitmapID);
	bool RenderBitmap(int bitmapID);
	void EnableBitmapXFlip(int bitmapID);
	void DisableBitmapXFlip(int bitmapID);
	void EnableBitmapYFlip(int bitmapID);
	void DisableBitmapYFlip(int bitmapID);
	int GetBitmapWidth(int bitmapID);
	int GetBitmapHeight(int bitmapID);

	bool AddSentence(HWND hwnd, char* text, int posX, int posY, int screenWidth, int screenHeight, XMFLOAT4 textColor, int& id);
	void UpdateSentence(int sentenceID, char* text, int posX, int posY, XMFLOAT4 textColor);
	bool RenderSentence(int sentenceID);
	void DeleteSentence(int& sentenceID);
	//void TurnOnSentence(int sentenceID);
	//void TurnOffSentence(int sentenceID);

	bool InitializeFadingEffect(HWND hwnd, int screenWidth, int screenHeight);
	void StartFadingEffect();
	void SetFadingEffect(float blend);
	void StopFadingEffect();
private:
	struct BitmapType{
		BitmapClass* bitmap;
		int bitmapWidth, bitmapHeight;
		int posX, posY;
		float blend;
		XMFLOAT4 hueColor;
		bool xFlip, yFlip;
		float rotation;
		int ID;
	};
	
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	TextureShaderClass* m_TextureShader;
	BitmapType* m_Bitmap;
	SentenceClass* m_Sentence;
	int m_bitmapCount, m_bitmapIDCount;
	int m_sentenceCount;
	BitmapType* m_FadeBitmap;
	bool fading;
	XMMATRIX m_viewMatrix, m_projectionMatrix, m_worldMatrix, m_orthoMatrix;
};

#endif