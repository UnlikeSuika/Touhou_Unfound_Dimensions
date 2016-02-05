#ifndef INPUTCLASSH
#define INPUTCLASSH

#define DIRECTINPUT_VERSION 0x0800

#include <Windows.h>

class InputClass{
public:
	InputClass();
	InputClass(const InputClass& other);
	~InputClass();

	bool Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	void Shutdown();
	bool Frame(HWND hwnd);
	void KeyDown(unsigned int input);
	void KeyUp(unsigned int input);
	bool IsKeyDown(unsigned int key);
	bool IsKeyJustPressed(unsigned int key);
	bool IsKeyJustReleased(unsigned int key);
	void GetMouseLocation(int& mouseX, int& mouseY);
	void UpdatePrevKeyboardState();
private:
	bool m_keyboardState[256];
	bool m_prevKeyboardState[256];
	int m_screenWidth, m_screenHeight;
	int m_mouseX, m_mouseY;

	void ProcessInput(HWND hwnd);
};

#endif