#ifndef _GRAPHICS_CLASS_H_
#define _GRAPHICS_CLASS_H_

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

#ifndef SCREEN_DEPTH
#define SCREEN_DEPTH 1000.0f   //far distance for depth buffer
#endif

#ifndef SCREEN_NEAR
#define SCREEN_NEAR 0.1f       //near distance for depth buffer
#endif

#ifndef MAX_BITMAP_COUNT
#define MAX_BITMAP_COUNT 300   //maximum number of bitmaps
#endif

#ifndef MAX_SENTENCE_COUNT
#define MAX_SENTENCE_COUNT 30  //maximum number of sentences
#endif

// This class manages the graphical components
// of DirectX.
class GraphicsClass{
public:

	//initializers and destructors. Only use SystemClass() for creating system object.
	GraphicsClass();
	GraphicsClass(const GraphicsClass& other);
	~GraphicsClass();

	bool Initialize(int screenWidth, int screenHeight, HWND hwnd);
	void Shutdown();

	//functions for marking the beginning or the end of rendering
	void BeginRendering();
	bool EndRendering(int screenWidth, int screenHeight);

	//bitmap-related functions
	bool AddBitmap(char* path, RECT bitmapRect, int screenWidth, int screenHeight, int& bitmapID, float angle = 0.0f, float blend = 1.0f, XMFLOAT4 hueColor = NULL_COLOR);
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

	//sentence-related functions
	bool AddSentence(char* text, int posX, int posY, int screenWidth, int screenHeight, XMFLOAT4 textColor, int& id);
	void UpdateSentence(int sentenceID, char* text, int posX, int posY, XMFLOAT4 textColor);
	bool RenderSentence(int sentenceID);
	void DeleteSentence(int& sentenceID);

	//functions related to fading effect
	bool InitializeFadingEffect(int screenWidth, int screenHeight);
	void StartFadingEffect();
	void SetFadingEffect(float blend);
	void StopFadingEffect();

private:

	//represents a bitmap
	struct BitmapType{
		BitmapClass* bitmap; //bitmap object in heap
		int bitmapWidth;     //width of bitmap in pixels
		int bitmapHeight;    //height of bitmap in pixels
		int posX;            //x-position of the centre of the bitmap
		int posY;            //y-position of the centre of the bitmap
		float blend;         //transparency of the bitmap
		XMFLOAT4 hueColor;   //colour blend into the bitmap
		bool xFlip;          //whether bitmap is horizontally flipped
		bool yFlip;          //whether bitmap is vertically flipped
		float rotation;      //clockwise rotation of the bitmap in radians
		int ID;              //ID for the bitmap
	};
	
	//objects and variables related to app system
	HWND m_hwnd;                                   //handle to window
	D3DClass* m_D3D;                               //D3DClass object
	CameraClass* m_Camera;                         //CameraClass object
	TextureShaderClass* m_TextureShader;           //TextureShaderClass object
	BitmapType m_Bitmap[MAX_BITMAP_COUNT];         //heap array of current bitmaps
	SentenceClass m_Sentence[MAX_SENTENCE_COUNT];   //heap array of current sentences
	int m_bitmapCount;                             //number of bitmaps
	int m_bitmapIDCount;                           //bitmap ID to mark for the next new bitmap
	int m_sentenceCount;                           //number of sentences
	BitmapType* m_FadeBitmap;                      //bitmap for fading the screen to black
	bool fading;                                   //whether the screen is currently fading to or from black
	XMMATRIX m_viewMatrix;                         //view matrix
	XMMATRIX m_projectionMatrix;                   //projection matrix
	XMMATRIX m_worldMatrix;                        //world matrix
	XMMATRIX m_orthoMatrix;                        //ortho matrix
};

#endif