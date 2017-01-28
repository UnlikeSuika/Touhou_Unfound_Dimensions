#ifndef INPUTCLASSH
#define INPUTCLASSH

#define DIRECTINPUT_VERSION 0x0800

#include <Windows.h>

class InputClass{
public:
	InputClass();
	InputClass(const InputClass& other);
	~InputClass();

	void Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	void Shutdown();
	void Frame(HWND hwnd);

	void KeyDown(unsigned int input);
	void KeyUp(unsigned int input);
	bool IsKeyDown(unsigned int key);
	bool IsKeyJustPressed(unsigned int key);
	bool IsKeyJustReleased(unsigned int key);
	POINT GetMouseLocation();
	POINT GetMouseLeftClickLocation();
	POINT GetMouseRightClickLocation();
	void UpdatePrevKeyboardState();
private:
	bool m_keyboardState[256];
	bool m_prevKeyboardState[256];
	int m_screenWidth, m_screenHeight;
	POINT m_mousePt;
	POINT m_lClickPos;         //cursor position upon left clicking
	POINT m_rClickPos;         //cursor position upon right clicking
};

#endif