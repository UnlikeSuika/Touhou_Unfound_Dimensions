#include "TextureClass.h"

TextureClass::TextureClass(){
	m_targaData = 0;
	m_texture = 0;
	m_textureView = 0;
}

TextureClass::TextureClass(const TextureClass& other){}

TextureClass::~TextureClass(){}

bool TextureClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename, HWND hwnd){
	bool result;
	int height, width;
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hr;
	unsigned int rowPitch;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	result = LoadTarga(filename, height, width, hwnd);
	if (!result){
		return false;
	}

	m_width = width;
	m_height = height;

	textureDesc.Height = height;
	textureDesc.Width = width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	hr = device->CreateTexture2D(&textureDesc, NULL, &m_texture);

	if (FAILED(hr)){
		size_t origSize = strlen(filename) + 1;
		const size_t newSize = 200;
		size_t convertedChars = 0;
		wchar_t wcstr[newSize];
		mbstowcs_s(&convertedChars, wcstr, origSize, filename, _TRUNCATE);
		wstring str(wcstr);
		wstring msg(L"Could not create 2D texture: ");
		wstring concStr = msg + str;
		LPCWSTR outputStr = concStr.c_str();
		MessageBox(hwnd, outputStr, L"Error", MB_OK);
		return false;
	}

	rowPitch = (width * 4)*sizeof(unsigned char);

	deviceContext->UpdateSubresource(m_texture, 0, NULL, m_targaData, rowPitch, 0);
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	hr = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hr)){
		size_t origSize = strlen(filename) + 1;
		const size_t newSize = 200;
		size_t convertedChars = 0;
		wchar_t wcstr[newSize];
		mbstowcs_s(&convertedChars, wcstr, origSize, filename, _TRUNCATE);
		wstring str(wcstr);
		wstring msg(L"Could not create shader resource view for the following TGA file: ");
		wstring concStr = msg + str;
		LPCWSTR outputStr = concStr.c_str();
		MessageBox(hwnd, outputStr, L"Error", MB_OK);
		return false;
	}

	deviceContext->GenerateMips(m_textureView);

	delete[] m_targaData;
	m_targaData = 0;

	return true;
}

void TextureClass::Shutdown(){
	if (m_textureView){
		m_textureView->Release();
		m_textureView = 0;
	}
	if (m_texture){
		m_texture->Release();
		m_texture = 0;
	}
	if (m_targaData){
		delete[] m_targaData;
		m_targaData = 0;
	}
}

ID3D11ShaderResourceView* TextureClass::GetTexture(){
	return m_textureView;
}

bool TextureClass::LoadTarga(char* filename, int& height, int& width, HWND hwnd){
	int error, bpp, imageSize, index;
	FILE* filePtr;
	unsigned int count;
	TargaHeader targaFileHeader;
	unsigned char* targaImage;

	error = fopen_s(&filePtr, filename, "rb");
	if (error){
		size_t origSize = strlen(filename) + 1;
		const size_t newSize = 200;
		size_t convertedChars = 0;
		wchar_t wcstr[newSize];
		mbstowcs_s(&convertedChars, wcstr, origSize, filename, _TRUNCATE);
		wstring str(wcstr);
		wstring msg(L"Could not load TGA file: ");
		wstring concStr = msg + str;
		LPCWSTR outputStr = concStr.c_str();
		MessageBox(hwnd, outputStr, L"Error", MB_OK);
		return false;
	}

	count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
	if (count != 1){
		return false;
	}

	height = (int)targaFileHeader.height;
	width = (int)targaFileHeader.width;
	bpp = (int)targaFileHeader.bpp;

	if (bpp != 32){
		size_t origSize = strlen(filename) + 1;
		const size_t newSize = 200;
		size_t convertedChars = 0;
		wchar_t wcstr[newSize];
		mbstowcs_s(&convertedChars, wcstr, origSize, filename, _TRUNCATE);
		wstring str(wcstr);
		wstring msg(L"The following TGA file is not 32bit: ");
		wstring concStr = msg + str;
		LPCWSTR outputStr = concStr.c_str();
		MessageBox(hwnd, outputStr, L"Error", MB_OK);
		return false;
	}

	imageSize = width*height * 4;
	targaImage = new unsigned char[imageSize];
	if (!targaImage){
		MessageBox(hwnd, L"Out of memory. Could not allocate space for targa image variable.", L"Error", MB_OK);
		return false;
	}

	count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
	if (count != imageSize){
		size_t origSize = strlen(filename) + 1;
		const size_t newSize = 200;
		size_t convertedChars = 0;
		wchar_t wcstr[newSize];
		mbstowcs_s(&convertedChars, wcstr, origSize, filename, _TRUNCATE);
		wstring str(wcstr);
		wstring msg(L"The image size for the following TGA file does not match the pixels read: ");
		wstring concStr = msg + str;
		LPCWSTR outputStr = concStr.c_str();
		MessageBox(hwnd, outputStr, L"Error", MB_OK);
		return false;
	}

	error = fclose(filePtr);
	if (error){
		size_t origSize = strlen(filename) + 1;
		const size_t newSize = 200;
		size_t convertedChars = 0;
		wchar_t wcstr[newSize];
		mbstowcs_s(&convertedChars, wcstr, origSize, filename, _TRUNCATE);
		wstring str(wcstr);
		wstring msg(L"Could not close the following TGA file: ");
		wstring concStr = msg + str;
		LPCWSTR outputStr = concStr.c_str();
		MessageBox(hwnd, outputStr, L"Error", MB_OK);
		return false;
	}

	m_targaData = new unsigned char[imageSize];
	if (!m_targaData){
		size_t origSize = strlen(filename) + 1;
		const size_t newSize = 200;
		size_t convertedChars = 0;
		wchar_t wcstr[newSize];
		mbstowcs_s(&convertedChars, wcstr, origSize, filename, _TRUNCATE);
		wstring str(wcstr);
		wstring msg(L"Out of memory. Could not create targa data variable for the following TGA file: ");
		wstring concStr = msg + str;
		LPCWSTR outputStr = concStr.c_str();
		MessageBox(hwnd, outputStr, L"Error", MB_OK);
		return false;
	}

	index = 0;
	int k = (width*height * 4) - (width * 4);

	for (int j = 0; j < height; j++){
		for (int i = 0; i < width; i++){
			m_targaData[index + 0] = targaImage[k + 2];
			m_targaData[index + 1] = targaImage[k + 1];
			m_targaData[index + 2] = targaImage[k + 0];
			m_targaData[index + 3] = targaImage[k + 3];
			k += 4;
			index += 4;
		}
		k -= (width * 8);
	}

	delete[] targaImage;
	targaImage = 0;

	return true;
}

int TextureClass::GetWidth(){
	return m_width;
}

int TextureClass::GetHeight(){
	return m_height;
}