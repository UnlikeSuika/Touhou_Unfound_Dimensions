#include "TextureShaderClass.h"

TextureShaderClass::TextureShaderClass(){
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sampleState = 0;
	m_pixelShaderCBuffer = 0;
}

TextureShaderClass::TextureShaderClass(const TextureShaderClass& other){
}

TextureShaderClass::~TextureShaderClass(){
}

bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd){
	WCHAR currentDir[256];
	DWORD const buffer_chars = sizeof(currentDir) / sizeof(currentDir[0]);
	DWORD a = GetCurrentDirectory(buffer_chars, currentDir);

	std::wstring dirVSWStr = std::wstring(currentDir) + L"\\Data\\texture.vs";

	std::wstring dirPSWStr = std::wstring(currentDir) + L"\\Data\\texture.ps";
	
	return InitializeShader(device, hwnd, (WCHAR*)dirVSWStr.c_str(), (WCHAR*)dirPSWStr.c_str());
}

void TextureShaderClass::Shutdown(){
	ShutdownShader();
}

bool TextureShaderClass::Render(HWND hwnd, ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, float blend, XMFLOAT4 hueColor, bool xFlip, bool yFlip){
	bool result = SetShaderParameters(hwnd, deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, blend, hueColor, xFlip, yFlip);
	if (!result){
		return false;
	}
	RenderShader(deviceContext, indexCount);
	return true;
}

bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename){
	HRESULT hr;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC transparentBufferDesc;

	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	hr = D3DCompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(hr)){
		if (errorMessage){
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		else{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}
		return false;
	}

	hr = D3DCompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(hr)){
		if (errorMessage){
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		else{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}
		return false;
	}

	hr = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(hr)){
		OutputErrorMessage(hwnd, hr, L"Could not create vertex shader with error: ");
		return false;
	}

	hr = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(hr)){
		OutputErrorMessage(hwnd, hr, L"Could not create pixel shader with error: ");
		return false;
	}

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;
	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);
	
	hr = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(hr)){
		OutputErrorMessage(hwnd, hr, L"Could not create input layout with error: ");
		return false;
	}

	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	ZeroMemory(&matrixBufferDesc, sizeof(matrixBufferDesc));
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(hr)){
		OutputErrorMessage(hwnd, hr, L"Could not create matrix buffer with error: ");
		return false;
	}

	ZeroMemory(&transparentBufferDesc, sizeof(transparentBufferDesc));
	transparentBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	transparentBufferDesc.ByteWidth = sizeof(PixelShaderBufferType);
	transparentBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transparentBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	transparentBufferDesc.MiscFlags = 0;
	transparentBufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&transparentBufferDesc, NULL, &m_pixelShaderCBuffer);
	if (FAILED(hr)){
		OutputErrorMessage(hwnd, hr, L"Could not create transparent buffer with error: ");
		return false;
	}

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 1;
	samplerDesc.BorderColor[2] = 2;
	samplerDesc.BorderColor[3] = 3;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(hr)){
		OutputErrorMessage(hwnd, hr, L"Could not create sampler state with error: ");
		return false;
	}

	return true;
}

void TextureShaderClass::ShutdownShader(){
	if (m_pixelShaderCBuffer){
		m_pixelShaderCBuffer->Release();
		m_pixelShaderCBuffer = 0;
	}
	if (m_sampleState){
		m_sampleState->Release();
		m_sampleState = 0;
	}
	if (m_matrixBuffer){
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}
	if (m_layout){
		m_layout->Release();
		m_layout = 0;
	}
	if (m_pixelShader){
		m_pixelShader->Release();
		m_pixelShader = 0;
	}
	if (m_vertexShader){
		m_vertexShader->Release();
		m_vertexShader = 0;
	}
}

void TextureShaderClass::OutputShaderErrorMessage(ID3DBlob* errorMessage, HWND hwnd, WCHAR* shaderFilename){
	char* compileErrors;
	unsigned long bufferSize;
	std::ofstream fout;
	compileErrors = (char*)(errorMessage->GetBufferPointer());
	bufferSize = errorMessage->GetBufferSize();
	fout.open("shader-error.txt");
	for (unsigned long i = 0; i < bufferSize; i++){
		fout << compileErrors[i];
	}
	fout.close();
	errorMessage->Release();
	errorMessage = 0;
	MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);
}

bool TextureShaderClass::SetShaderParameters(HWND hwnd, ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, float blend, XMFLOAT4 hueColor, bool xFlip, bool yFlip){
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	PixelShaderBufferType* dataPtr2;
	unsigned int bufferNumber;

	hr = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)){
		OutputErrorMessage(hwnd, hr, L"Could not map matrix buffer with error: ");
		return false;
	}

	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	deviceContext->Unmap(m_matrixBuffer, 0);

	bufferNumber = 0;

	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	deviceContext->PSSetShaderResources(0, 1, &texture);

	hr = deviceContext->Map(m_pixelShaderCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)){
		OutputErrorMessage(hwnd, hr, L"Could not map pixel shader C buffer with error: ");
		return false;
	}
	
	dataPtr2 = (PixelShaderBufferType*)mappedResource.pData;
	dataPtr2->hueColor = hueColor;
	dataPtr2->alphaBlendAmount = blend;
	dataPtr2->xFlip = xFlip;
	dataPtr2->yFlip = yFlip;
	deviceContext->Unmap(m_pixelShaderCBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_pixelShaderCBuffer);

	return true;
}

void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount){
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	deviceContext->DrawIndexed(indexCount, 0, 0);
}