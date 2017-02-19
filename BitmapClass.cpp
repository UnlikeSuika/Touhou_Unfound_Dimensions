#include "BitmapClass.h"

BitmapClass::BitmapClass(){
	m_vertexBuffer = NULL;
	m_indexBuffer = NULL;
	m_Texture = NULL;
}

BitmapClass::BitmapClass(const BitmapClass& other){}

BitmapClass::~BitmapClass(){}

bool BitmapClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight, char* textureFilename, int bitmapWidth, int bitmapHeight){
	bool result;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_previousPosX = -1;
	m_previousPosY = -1;
	m_previousAngle = -1000.0f;

	result = InitializeBuffers(device);
	if (!result){
		return false;
	}
	result = LoadTexture(device, deviceContext, textureFilename, hwnd);
	if (!result){
		return false;
	}

	if (bitmapHeight == 0){
		m_bitmapHeight = m_Texture->GetHeight();
	}
	else{
		m_bitmapHeight = bitmapHeight;
	}

	if (bitmapWidth == 0){
		m_bitmapWidth = m_Texture->GetWidth();
	}
	else{
		m_bitmapWidth = bitmapWidth;
	}

	return true;
}

void BitmapClass::Shutdown(){
	ReleaseTexture();
	ShutdownBuffers();
}

bool BitmapClass::Render(ID3D11DeviceContext* deviceContext, int positionX, int positionY, int bitmapWidth, int bitmapHeight, float angle){
	bool result = UpdateBuffers(deviceContext, positionX, positionY, bitmapWidth, bitmapHeight, -angle);
	if (!result){
		return false;
	}
	RenderBuffers(deviceContext);
	return true;
}

int BitmapClass::GetIndexCount(){
	return m_indexCount;
}

ID3D11ShaderResourceView* BitmapClass::GetTexture(){
	return m_Texture->GetTexture();
}

bool BitmapClass::InitializeBuffers(ID3D11Device* device){
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT hr;

	m_vertexCount = 6;
	m_indexCount = m_vertexCount;
	vertices = new VertexType[m_vertexCount];
	if (!vertices){
		return false;
	}
	indices = new unsigned long[m_indexCount];
	if (!indices){
		return false;
	}

	memset(vertices, 0, (sizeof(VertexType)*m_vertexCount));

	for (int i = 0; i < m_indexCount; i++){
		indices[i] = i;
	}

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType)*m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(hr)){
		return false;
	}

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long)*m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(hr)){
		return false;
	}

	delete[] vertices;
	delete[] indices;
	vertices = 0;
	indices = 0;
	return true;
}

void BitmapClass::ShutdownBuffers(){
	if (m_indexBuffer){
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}
	if (m_vertexBuffer){
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}

bool BitmapClass::UpdateBuffers(ID3D11DeviceContext* deviceContext, int positionX, int positionY, int bitmapWidth, int bitmapHeight, float angle){
	float left, right, top, bottom;
	VertexType* vertices;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;
	HRESULT hr;

	if ((positionX == m_previousPosX) && (positionY == m_previousPosY) && (angle == m_previousAngle) && (bitmapWidth == m_bitmapWidth) && (bitmapHeight == m_bitmapHeight)){
		return true;
	}

	m_previousPosX = positionX;
	m_previousPosY = positionY;
	m_previousAngle = angle;
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;

	left = (float)((m_screenWidth / 2)*-1) + (float)positionX - (float)m_bitmapWidth / 2.0f;
	right = left + (float)m_bitmapWidth;
	top = (float)(m_screenHeight / 2) - (float)positionY + (float)m_bitmapHeight / 2.0f;
	bottom = top - (float)m_bitmapHeight;

	float midPtX = (left + right) / 2.0f;
	float midPtY = (bottom + top) / 2.0f;
	float radius = (float)sqrt(pow(top - bottom, 2) + pow(right - left, 2))/2.0f;
	float refAngle = atan2(top - bottom, right - left);

	XMFLOAT3 topRight = XMFLOAT3(midPtX + radius*cos(refAngle + angle), midPtY + radius*sin(refAngle + angle), 0.0f);
	XMFLOAT3 topLeft = XMFLOAT3(midPtX + radius*cos(XM_PI - refAngle + angle),
		midPtY + radius*sin(XM_PI - refAngle + angle), 0.0f);
	XMFLOAT3 bottomLeft = XMFLOAT3(midPtX + radius*cos(XM_PI + refAngle + angle),
		midPtY + radius*sin(XM_PI + refAngle + angle), 0.0f);
	XMFLOAT3 bottomRight = XMFLOAT3(midPtX + radius*cos(XM_2PI - refAngle + angle),
		midPtY + radius*sin(XM_2PI - refAngle + angle), 0.0f);

	vertices = new VertexType[m_vertexCount];
	if (!vertices){
		return false;
	}

	vertices[0].position = topLeft;
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);
	vertices[1].position = bottomRight;
	vertices[1].texture = XMFLOAT2(1.0f, 1.0f);
	vertices[2].position = bottomLeft;
	vertices[2].texture = XMFLOAT2(0.0f, 1.0f);
	vertices[3].position = topLeft;
	vertices[3].texture = XMFLOAT2(0.0f, 0.0f);
	vertices[4].position = topRight;
	vertices[4].texture = XMFLOAT2(1.0f, 0.0f);
	vertices[5].position = bottomRight;
	vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

	hr = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)){
		return false;
	}

	verticesPtr = (VertexType*)mappedResource.pData;
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType)*m_vertexCount));

	deviceContext->Unmap(m_vertexBuffer, 0);
	delete[] vertices;
	vertices = 0;

	return true;
}

void BitmapClass::RenderBuffers(ID3D11DeviceContext* deviceContext){
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool BitmapClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename, HWND hwnd){
	bool result;
	m_Texture = new TextureClass;
	if (!m_Texture){
		return false;
	}
	result = m_Texture->Initialize(device, deviceContext, filename, hwnd);
	if (!result){
		return false;
	}
	return true;
}

void BitmapClass::ReleaseTexture(){
	if (m_Texture){
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}
}

int BitmapClass::GetWidth(){
	return m_bitmapWidth;;
}

int BitmapClass::GetHeight(){
	return m_bitmapHeight;
}