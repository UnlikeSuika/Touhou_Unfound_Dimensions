#ifndef _TEXTURE_SHADER_CLASS_H_
#define _TEXTURE_SHADER_CLASS_H_

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <Windows.h>
#include <DirectXMath.h>
#include <fstream>
#include "HResultErrorMessage.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d11.lib")

using namespace std;
using namespace DirectX;

const XMFLOAT4 NULL_COLOR = { 0.0f, 0.0f, 0.0f, 0.0f };
const XMFLOAT4 SOLID_RED = { 1.0f, 0.0f, 0.0f, 1.0f };
const XMFLOAT4 SOLID_GREEN = { 0.0f, 1.0f, 0.0f, 1.0f };
const XMFLOAT4 SOLID_BLUE = { 0.0f, 0.0f, 1.0f, 1.0f };
const XMFLOAT4 SOLID_WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
const XMFLOAT4 SOLID_BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };
const XMFLOAT4 SOLID_YELLOW = { 1.0f, 1.0f, 0.0f, 1.0f };
const XMFLOAT4 SOLID_CYAN = { 0.0f, 1.0f, 1.0f, 1.0f };
const XMFLOAT4 SOLID_MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f };

class TextureShaderClass{
public:
	TextureShaderClass();
	TextureShaderClass(const TextureShaderClass& other);
	~TextureShaderClass();
	bool virtual Initialize(ID3D11Device* device, HWND hwnd);
	void virtual Shutdown();
	bool virtual Render(HWND hwnd, ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, float blend, XMFLOAT4 hueColor, bool xFlip = false, bool yFlip = false);
private:
	struct MatrixBufferType{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct PixelShaderBufferType{
		XMFLOAT4 hueColor;
		float alphaBlendAmount;  //4-byte
		bool xFlip;   //1 byte
		bool yFlip;   //1 byte
		bool padding[2]; //2 byte
		XMFLOAT2 padding2;   //8-bytes
	};

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_pixelShaderCBuffer;

	bool virtual InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
	void virtual ShutdownShader();
	void virtual OutputShaderErrorMessage(ID3DBlob* errorMessage, HWND hwnd, WCHAR* shaderFilename);
	bool virtual SetShaderParameters(HWND hwnd, ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, float blend, XMFLOAT4 hueColor, bool xFlip, bool yFlip);
	void virtual RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);
};

#endif