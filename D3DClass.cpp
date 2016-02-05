#include "D3DClass.h"

D3DClass::D3DClass(){
	m_swapChain = 0;
	m_swapChain1 = 0;
	m_device = 0;
	m_device1 = 0;
	m_deviceContext = 0;
	m_deviceContext1 = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;
	m_depthDisabledStencilState = 0;
	m_alphaEnableBlendingState = 0;
	m_alphaDisableBlendingState = 0;
}

D3DClass::D3DClass(const D3DClass& other){
}

D3DClass::~D3DClass(){
}

bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear){
	HRESULT hr;
	float fov, screenAspect;

	m_vsync_enabled = vsync;
	m_featureLevel = D3D_FEATURE_LEVEL_11_0;

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);
	D3D_FEATURE_LEVEL featureLevels[] = {
		//D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (int driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++){
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(NULL, m_driverType, NULL, 0, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &m_device, &m_featureLevel, &m_deviceContext);
		if (hr == E_INVALIDARG){
			hr = D3D11CreateDevice(NULL, m_driverType, NULL, 0, &featureLevels[1], numFeatureLevels - 1, D3D11_SDK_VERSION, &m_device, &m_featureLevel, &m_deviceContext);
		}
		if (SUCCEEDED(hr)){
			break;
		}
	}
	if (FAILED(hr)){
		return false;
	}

	IDXGIFactory1* factory = 0;
	IDXGIDevice* device = 0;
	hr = m_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&device));
	if (SUCCEEDED(hr)){
		IDXGIAdapter* adapter = 0;
		hr = device->GetAdapter(&adapter);
		if (SUCCEEDED(hr)){
			hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory));
			adapter->Release();
		}
		device->Release();
	}
	if (FAILED(hr)){
		return false;
	}

	IDXGIFactory2* factory2 = 0;
	hr = factory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&factory2));
	if (factory2){
		hr = m_device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&m_device1));
		if (SUCCEEDED(hr)){
			(void)m_deviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_deviceContext1));
		}
		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = screenWidth;
		sd.Height = screenHeight;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		hr = factory2->CreateSwapChainForHwnd(m_device, hwnd, &sd, NULL, NULL, &m_swapChain1);
		if (SUCCEEDED(hr)){
			m_swapChain = NULL;
			hr = m_swapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_swapChain));
		}
		factory2->Release();
	}
	else{
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = screenWidth;
		sd.BufferDesc.Height = screenHeight;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		hr = factory->CreateSwapChain(m_device, &sd, &m_swapChain);
	}
	factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	factory->Release();
	if (FAILED(hr)){
		return false;
	}

	ID3D11Texture2D* backBuffer = 0;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	if (FAILED(hr)){
		return false;
	}
	hr = m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView);
	backBuffer->Release();
	if (FAILED(hr)){
		return false;
	}
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, NULL);

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	hr = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(hr)){
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(hr)){
		return false;
	}
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(hr)){
		return false;
	}
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	hr = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(hr)){
		return false;
	}
	m_deviceContext->RSSetState(m_rasterState);

	D3D11_VIEWPORT viewport;
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	m_deviceContext->RSSetViewports(1, &viewport);

	fov = XM_PI / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fov, screenAspect, screenNear, screenDepth);
	m_worldMatrix = XMMatrixIdentity();
	m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);
	
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = m_device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);
	if (FAILED(hr)){
		return false;
	}

	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	//blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
//	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	hr = m_device->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingState);
	if (FAILED(hr)){
		return false;
	}

	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;
	hr = m_device->CreateBlendState(&blendStateDescription, &m_alphaDisableBlendingState);
	if (FAILED(hr)){
		return false;
	}

	return true;
}

void D3DClass::Shutdown(){
	if (m_swapChain){
		m_swapChain->Release();
		m_swapChain = 0;
	}
	if (m_swapChain1){
		m_swapChain1->Release();
		m_swapChain1 = 0;
	}
	if (m_alphaEnableBlendingState){
		m_alphaEnableBlendingState->Release();
		m_alphaEnableBlendingState = 0;
	}
	if (m_alphaDisableBlendingState){
		m_alphaDisableBlendingState->Release();
		m_alphaDisableBlendingState = 0;
	}
	if (m_depthDisabledStencilState){
		m_depthDisabledStencilState->Release();
		m_depthDisabledStencilState = 0;
	}
	if (m_rasterState){
		m_rasterState->Release();
		m_rasterState = 0;
	}
	if (m_depthStencilView){
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}
	if (m_depthStencilState){
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}
	if (m_depthStencilBuffer){
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}
	if (m_renderTargetView){
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}
	if (m_deviceContext){
		m_deviceContext->Release();
		m_deviceContext = 0;
	}
	if (m_deviceContext1){
		m_deviceContext1->Release();
		m_deviceContext1 = 0;
	}
	if (m_device){
		m_device->Release();
		m_device = 0;
	}
	if (m_device1){
		m_device1->Release();
		m_device1 = 0;
	}
}

void D3DClass::TurnOnAlphaBlending(){
	float blendFactor[4];
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	m_deviceContext->OMSetBlendState(m_alphaEnableBlendingState, blendFactor, 0xffffffff);
}

void D3DClass::TurnOffAlphaBlending(){
	float blendFactor[4];
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	m_deviceContext->OMSetBlendState(m_alphaDisableBlendingState, blendFactor, 0xffffffff);
}

void D3DClass::BeginScene(float red, float green, float blue, float alpha){
	float color[4];
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;
	
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3DClass::EndScene(){
	if (m_vsync_enabled){
		m_swapChain->Present(1, 0);
	}
	else{
		m_swapChain->Present(0, 0);
	}
}

ID3D11Device* D3DClass::GetDevice(){
	return m_device;
}

ID3D11DeviceContext* D3DClass::GetDeviceContext(){
	return m_deviceContext;
}

void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix){
	projectionMatrix = m_projectionMatrix;
}

void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix){
	worldMatrix = m_worldMatrix;
}

void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix){
	orthoMatrix = m_orthoMatrix;
}

void D3DClass::TurnZBufferOn(){
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
}

void D3DClass::TurnZBufferOff(){
	m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
}