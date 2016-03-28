#include "InputClass.h"

InputClass::InputClass(){}

InputClass::InputClass(const InputClass& other){}

InputClass::~InputClass(){}

void InputClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight){
	for (int i = 0; i < 256; i++){
		m_keyboardState[i] = false;
		m_prevKeyboardState[i] = false;
	}

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}

void InputClass::Shutdown(){
}

void InputClass::Frame(HWND hwnd){
	POINT mousePt;
	GetCursorPos(&mousePt);
	ScreenToClient(hwnd, &mousePt);

	m_mouseX = (int)mousePt.x;
	m_mouseY = (int)mousePt.y;
}

void InputClass::KeyDown(unsigned int input){
	m_keyboardState[input] = true;
}

void InputClass::KeyUp(unsigned int input){
	m_keyboardState[input] = false;
}

bool InputClass::IsKeyDown(unsigned int key){
	return m_keyboardState[key];
}

bool InputClass::IsKeyJustPressed(unsigned int key){
	if (m_keyboardState[key] && !m_prevKeyboardState[key]){
		return true;
	}
	return false;
}

bool InputClass::IsKeyJustReleased(unsigned int key){
	if (!m_keyboardState[key] && m_prevKeyboardState[key]){
		return true;
	}
	return false;
}

void InputClass::GetMouseLocation(int& mouseX, int& mouseY){
	mouseX = m_mouseX;
	mouseY = m_mouseY;
}

void InputClass::UpdatePrevKeyboardState(){
	for (int i = 0; i < 256; i++){
		m_prevKeyboardState[i] = m_keyboardState[i];
	}
}
