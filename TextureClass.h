#ifndef TEXTURECLASSH
#define TEXTURECLASSH

#include <d3d11.h>
#include <stdio.h>
#include <string>

#pragma comment(lib,"d3d11.lib")

using namespace std;

class TextureClass{
public:
	TextureClass();
	TextureClass(const TextureClass& other);
	~TextureClass();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename, HWND hwnd);
	void Shutdown();
	ID3D11ShaderResourceView* GetTexture();
	int GetWidth();
	int GetHeight();
private:
	struct TargaHeader{
		unsigned char data1[12];
		unsigned short width;
		unsigned short height;
		unsigned char bpp;
		unsigned char data2;
	};

	unsigned char* m_targaData;
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_textureView;
	int m_width;
	int m_height;
	
	bool LoadTarga(char* filename, int& height, int& width, HWND hwnd);
};

#endif