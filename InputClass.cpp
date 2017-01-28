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
	m_lClickPos = { -1,-1 };
	m_rClickPos = { -1,-1 };
}

void InputClass::Shutdown(){
}

void InputClass::Frame(HWND hwnd){
	GetCursorPos(&m_mousePt);
	ScreenToClient(hwnd, &m_mousePt);

	//if left mouse button is just pressed this frame, set the click position to screen coord. of cursor
	if (IsKeyJustPressed(VK_LBUTTON)) {
		m_lClickPos = m_mousePt;
	}

	//if left mouse button is released, reset the click position to (-1, -1)
	if (!IsKeyDown(VK_LBUTTON) && !IsKeyJustReleased(VK_LBUTTON)) {
		m_lClickPos = { -1,-1 };
	}

	//if right mouse button is just pressed this frame, set the click position to screen coord. of cursor
	if (IsKeyJustPressed(VK_RBUTTON)) {
		m_rClickPos = m_mousePt;
	}

	//if right mouse button is released, reset the click position to (-1, -1)
	if (!IsKeyDown(VK_RBUTTON) && !IsKeyJustReleased(VK_RBUTTON)) {
		m_rClickPos = { -1,-1 };
	}
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

POINT InputClass::GetMouseLocation() {
	return m_mousePt;
}

POINT InputClass::GetMouseLeftClickLocation() {
	return m_lClickPos;
}

POINT InputClass::GetMouseRightClickLocation() {
	return m_rClickPos;
}

void InputClass::UpdatePrevKeyboardState(){
	for (int i = 0; i < 256; i++){
		m_prevKeyboardState[i] = m_keyboardState[i];
	}
}
