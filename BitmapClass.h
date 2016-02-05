#ifndef BITMAPCLASSH
#define BITMAPCLASSH

#include <D3D11.h>
#include <directxmath.h>
#include "TextureClass.h"

using namespace DirectX;

class BitmapClass{
public:
	BitmapClass();
	BitmapClass(const BitmapClass& other);
	~BitmapClass();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight, char* textureFilename, int bitmapWidth=0, int bitmapHeight=0);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, int positionX, int positionY, int bitmapWidth, int bitmapHeight, float angle = 0.0f);
	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();
	int GetWidth();
	int GetHeight();
private:
	struct VertexType{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	int m_vertexCount;
	int m_indexCount;
	TextureClass* m_Texture;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_previousPosX, m_previousPosY;
	float m_previousAngle;

	bool InitializeBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	bool UpdateBuffers(ID3D11DeviceContext* deviceContext, int positionX, int positionY, int bitmapWidth, int bitmapHeight, float angle);
	void RenderBuffers(ID3D11DeviceContext* deviceContext);
	bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename, HWND hwnd);
	void ReleaseTexture();
};

#endif