#ifndef D3DCLASSH
#define D3DCLASSH

#include <d3dcommon.h>
#include <d3d11_1.h>
#include <DirectXMath.h>

using namespace DirectX;

#pragma comment(lib,"d3d11.lib")

class D3DClass{
public:
	D3DClass();
	D3DClass(const D3DClass& other);
	~D3DClass();

	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear);
	void Shutdown();
	void BeginScene(float red, float green, float blue, float alpha);
	void EndScene();
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
	void GetProjectionMatrix(XMMATRIX& projectionMatrix);
	void GetWorldMatrix(XMMATRIX& worldMatrix);
	void GetOrthoMatrix(XMMATRIX& orthoMatrix);
	void TurnZBufferOn();
	void TurnZBufferOff();
	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();
private:
	bool m_vsync_enabled;
	IDXGISwapChain* m_swapChain;
	IDXGISwapChain1* m_swapChain1;
	ID3D11Device* m_device;
	ID3D11Device1* m_device1;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11DeviceContext1* m_deviceContext1;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_worldMatrix;
	XMMATRIX m_orthoMatrix;
	ID3D11DepthStencilState* m_depthDisabledStencilState;
	ID3D11BlendState* m_alphaEnableBlendingState;
	ID3D11BlendState* m_alphaDisableBlendingState;
	D3D_DRIVER_TYPE m_driverType;
	D3D_FEATURE_LEVEL m_featureLevel;
};

#endif