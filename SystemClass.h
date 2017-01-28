#ifndef _SYSTEM_CLASS_H_
#define _SYSTEM_CLASS_H_

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include "InputClass.h"
#include "GraphicsClass.h"
#include "TimeClass.h"
#include "VersusMode.h"

#ifndef MAX_CHAR_SELECT_BUTTONS
#define MAX_CHAR_SELECT_BUTTONS 8  //maximum number of character options in character select screen
#endif

// This class initializes the system objects and
// manages what will be executed in the game every frame.
class SystemClass{
private:

	/*********************************************************

	PRIVATE FUNCTIONS

	**********************************************************/

	//Windows-related functions
	bool Frame(); 
	void InitializeWindows(int& screenWidth, int& screenHeight); 
	void ShutdownWindows(); 
	
	//functions related to fading effect
	void SetFadingEffects();

	//mode-related functions
	void OnMainMenu(); 
	bool OnCharacterSelectMode(); 
	bool OnVersusMode(); 
	bool InitializeCharSelect(); 
	bool InitializeVersusMode(); 
	void ShutdownVersusMode(); 

	/*********************************************************

	PRIVATE VARIABLES

	**********************************************************/

	//objects and variables related to app system
	LPCWSTR m_applicationName;        //name of the application
	HINSTANCE m_hinstance;            //main instance
	int m_screenWidth;                //screen width of the window
    int m_screenHeight;               //screen height of the window
	HWND m_hwnd;                      //handle to the window
	InputClass* m_Input;              //InputClass object
	GraphicsClass* m_Graphics;        //GraphicsClass object
	TimeClass* m_Clock;               //TimeClass object

	//variables related to game modes
	GameMode gameMode;                                   //current game mode
	GameMode nextGameMode;                               //which game mode the system is transitioning into

	//variables related to title screen
	ButtonType gameStartButton;                          //button for starting a game in main menu
	ButtonType charSelectButton[MAX_CHAR_SELECT_BUTTONS];//array of character select choice buttons in character select screen

	//variables related to fading effects
	bool fadingIn;           //whether the screen is fading into another game mode
	bool fadingOut;          //whether the screen is fading out of the current game mode

	//variables related to character select mode
	bool isCharSelectInit;   //whether character select has been initialized
	bool isCharSelectStarted;//whether character select mode has been initiated

	//variables related to versus mode
	bool isVersusModeInit;   //whether the versus match has been initialized
	VersusMode* versusMatch;    //object of the versus match

	//IDs of bitmaps or timers
	int mainMenuBackgroundID;      //ID of bitmap of main menu background
	int cursorSpriteID;            //ID of bitmap of cursor
	int selectedCharButtonID;      //ID of bitmap that highlights character select button upon hovering the mouse over
	int reimuAvatarID;             //ID of bitmap of Reimu in character select screen
	int marisaAvatarID;            //ID of bitmap of Marisa in character select screen
	int charSelectModeBackgroundID;//ID of background bitmap in character select screen
	int fadeTimerID;               //ID of timer for the fading effect of screen

	PlayableCharacter player1;
	PlayableCharacter player2;
	int player1FlashTimerID;
	int player2FlashTimerID;
	int player1AvatarID;
	int player2AvatarID;

public:

	/*********************************************************

	PUBLIC FUNCTIONS

	**********************************************************/

	//initializers and destructors
	SystemClass();
	SystemClass(const SystemClass& other);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static SystemClass* ApplicationHandle = 0;
#endif