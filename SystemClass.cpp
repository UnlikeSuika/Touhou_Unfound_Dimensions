#include "SystemClass.h"

//SystemClass object initializer.
//Use SystemClass::Initialize to initialize the system.
SystemClass::SystemClass(){
	m_Input = 0;
	m_Graphics = 0;
	m_Clock = 0;
}

//SystemClass object initializer via SystemClass& argument.
//Do not use this initializer.
SystemClass::SystemClass(const SystemClass& other){}

//SystemClass destructor. Do not use this deleter.
//Use SystemClass::Shutdown instead.
SystemClass::~SystemClass(){}

//Initializes SystemClass variables
bool SystemClass::Initialize(){
	bool result;
	int screenWidth = 0;
	int screenHeight = 0;

	//Create a window for the application
	InitializeWindows(screenWidth, screenHeight);
	m_screenHeight = screenHeight;
	m_screenWidth = screenWidth;

	//Initialize InputClass object
	m_Input = new InputClass;
	if (!m_Input){
		MessageBox(m_hwnd, L"Could not create input object.", L"Error", MB_OK);
		return false;
	}
	m_Input->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight);

	//Initialize GraphicsClass object
	m_Graphics = new GraphicsClass;
	if (!m_Graphics){
		MessageBox(m_hwnd, L"Could not create graphics object.", L"Error", MB_OK);
		return false;
	}
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if (!result){
		MessageBox(m_hwnd, L"Could not initialize graphics object.", L"Error", MB_OK);
		return false;
	}

	//Initialize TimeClass object
	m_Clock = new TimeClass;
	if (!m_Clock){
		MessageBox(m_hwnd, L"Could not create time object.", L"Error", MB_OK);
		return false;
	}
	m_Clock->SetFrameCount(0);

	//Initialize the game with main menu screen.
	gameMode = MAIN_MENU;
	
	//Initialize the main menu screen background
	RECT screenRect = { 0, 0, m_screenWidth, m_screenHeight };
	result = m_Graphics->AddBitmap("/Data/title.tga", screenRect, m_screenWidth, m_screenHeight, mainMenuBackgroundID);
	if (!result){
		return false;
	}

	//Place the main menu screen background at the middle of the screen
	m_Graphics->UpdateBitmap(mainMenuBackgroundID, m_screenWidth / 2, m_screenHeight / 2);

	//Initialize game start button
	gameStartButton.buttonRect.left = m_screenWidth / 2 - 150;
	gameStartButton.buttonRect.right = m_screenWidth / 2 + 150;
	gameStartButton.buttonRect.top = m_screenHeight / 2 - 50;
	gameStartButton.buttonRect.bottom = m_screenHeight / 2 + 50;
	result = m_Graphics->AddBitmap("/Data/gameStartButton.tga", gameStartButton.buttonRect, m_screenWidth, m_screenHeight, gameStartButton.bitmapID);
	if (!result){
		return false;
	}

	//No game modes except main menu are initialized
	isCharSelectInit = false;
	isCharSelectStarted = false;
	isVersusModeInit = false;

	//No characters selected yet
	player1 = UNSELECTED;
	player2 = UNSELECTED;

	//Not fading in or out
	fadingIn = false;
	fadingOut = false;

	return true;
}

//Shuts down SystemClass variables
void SystemClass::Shutdown(){

	//Shut down GraphicsClass object if present
	if (m_Graphics){
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	//Shut down InputClass object if present
	if (m_Input){
		m_Input->Shutdown();
		delete m_Input;
		m_Input = 0;
	}

	//Shut down TimeClass object if present
	if (m_Clock){
		delete m_Clock;
		m_Clock = 0;
	}

	//Shut down the application window
	ShutdownWindows();
}

//Receive, translate and dispatch system messages every frame
void SystemClass::Run(){
	MSG msg;
	bool done = false;
	ZeroMemory(&msg, sizeof(MSG));

	while (!done){

		//if system message is received
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//if system message is WM_QUIT, quit the game
		if (msg.message == WM_QUIT){
			done = true;
		}

		//otherwise, run the game during the given frame
		else{
			if (!Frame()){
				done = true;
			}
		}
	}
}

//Process the application every frame
bool SystemClass::Frame(){
	bool result;

	//Increase the frame count
	m_Clock->FrameIncrement();

	//Update mouse position every frame
	m_Input->Frame(m_hwnd);

	//Start rendering the buffer
	m_Graphics->BeginRendering();

	//Set effects of screen fading
	SetFadingEffects();
	
	//Handle game modes
	switch (gameMode){
	case MAIN_MENU:
		OnMainMenu();
		break;

	case CHARACTER_SELECT_MODE:
		result = OnCharacterSelectMode();
		if (!result){
			MessageBox(m_hwnd, L"Character select mode crashed.", L"Error", MB_OK);
			return false;
		}
		break;

	case VERSUS_MODE:
		result = OnVersusMode();
		if (!result){
			MessageBox(m_hwnd, L"Versus mode crashed.", L"Error", MB_OK);
			return false;
		}
		break;

	default:
		MessageBox(m_hwnd, L"Unavailable mode. How did you even get here?", L"Error", MB_OK);
		return false;
	}
	
	//Display what is currently in the buffer
	m_Graphics->EndRendering(m_screenWidth, m_screenHeight);

	//Update the previous key states to current key states
	m_Input->UpdatePrevKeyboardState();

	return true;
}

//Creates window for the application
void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight){
	ApplicationHandle = this;
	m_hinstance = GetModuleHandle(NULL);
	m_applicationName = L"UnlikeSuika's Programming Practice";

	//Setting window parameters
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	//Register window
	RegisterClassEx(&wc);

	//Set window size and styles
	screenWidth = 800;
	screenHeight = 600;
	RECT wr = { 0, 0, screenWidth, screenHeight };
	DWORD winStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	AdjustWindowRectEx(&wr, winStyle, FALSE, WS_EX_APPWINDOW);

	//Set window position
	int posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
	int posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

	//Create window
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, winStyle,
		posX, posY, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, m_hinstance, NULL);

	//Display window on foreground
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	//Display default cursor
	ShowCursor(true);
}

//Shuts down the application window
void SystemClass::ShutdownWindows(){

	//Display default cursor (just in case it was set not to be displayed)
	ShowCursor(true);

	//Destroy the application window object
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	//Unregister the window
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;
	ApplicationHandle = NULL;
}

//Set the effect for screen fading into or out of game mode
void SystemClass::SetFadingEffects(){

	//if fading out of current mode
	if (fadingOut && (!fadingIn)){
	
		//if timer for fading out is already running
		if (m_Clock->IsTimerRunning(fadeTimerID)){
			m_Graphics->SetFadingEffect(1.0f*(1 - (float)m_Clock->TimeLeft(fadeTimerID) / 30));
	
			//if the timer has reached zero
			if (m_Clock->TimeLeft(fadeTimerID) == 0){
				fadingOut = false;
				fadingIn = true;
			}
		}
	
		//if timer for fading out has not yet started
		else{
			m_Graphics->StartFadingEffect();
			m_Clock->AddTimer(fadeTimerID, 30);
		}
	}

	// if fading into the next game mode
	else if ((!fadingOut) && fadingIn){
	
		//if the timer for fading in is running
		if (m_Clock->IsTimerRunning(fadeTimerID)){
			m_Graphics->SetFadingEffect(1.0f*(float)m_Clock->TimeLeft(fadeTimerID) / 30);
	
			//if the timer has reached zero
			if (m_Clock->TimeLeft(fadeTimerID) == 0){
				fadingIn = false;
				m_Graphics->StopFadingEffect();
				m_Clock->DeleteTimer(fadeTimerID);
			}
		}
	
		//if timer for fading in has not yet started
		else{
			m_Clock->SetTimer(fadeTimerID, 30);
		}
	}
}

//main menu mode
void SystemClass::OnMainMenu(){

	//render background
	m_Graphics->RenderBitmap(mainMenuBackgroundID);

	//render Start button
	m_Graphics->RenderBitmap(gameStartButton.bitmapID);
	
	//if fading out of main menu into another game mode
	if (fadingOut&&m_Clock->TimeLeft(fadeTimerID) == 1){
		gameMode = nextGameMode;
	}

	//if not fading out of main menu
	else if (!fadingOut){
	
		//if Started button is pressed
		if (ButtonLeftClicked(gameStartButton.buttonRect, m_Input)){
			
			//fade into character select screen
			fadingOut = true;
			fadingIn = false;
			nextGameMode = CHARACTER_SELECT_MODE;
		}
	}
}

//character select mode
bool SystemClass::OnCharacterSelectMode(){
	
	//initialize character select mode if not already done so
	if (!isCharSelectInit){
		if (!InitializeCharSelect()){
			return false;
		}
	}

	//reset selected characters to unselected upon starting char. sel. mode
	if (!isCharSelectStarted){
		player1 = UNSELECTED;
		player2 = UNSELECTED;
		isCharSelectStarted = true;
	}

	//if esc key is pressed, the mode changes to main menu
	if (m_Input->IsKeyJustPressed(VK_ESCAPE)){
		fadingOut = true;
		fadingIn = false;
		nextGameMode = MAIN_MENU;
	}

	//if timer for char.sel. mode fading into next mode reaches one, change current
	//mode to next one
	if (fadingOut && m_Clock->TimeLeft(fadeTimerID) == 1){
		isCharSelectStarted = false;
		gameMode = nextGameMode;
	}

	//render background bitmap
	m_Graphics->RenderBitmap(charSelectModeBackgroundID);

	RECT rc = { 0, 0, m_screenWidth, m_screenHeight };

	//if right-clicked the mouse, currently selected character will be cancelled,
	//as long as the screen is not fading into different game mode
	if (!fadingOut){
		if (m_Input->IsKeyJustReleased(VK_RBUTTON) && RectContains(rc, m_Input->GetMouseRightClickLocation())){
			if (player2 != UNSELECTED){
				player2 = UNSELECTED;
				m_Clock->DeleteTimer(player2FlashTimerID);
			}
			else if (player1 != UNSELECTED){
				player1 = UNSELECTED;
				m_Clock->DeleteTimer(player1FlashTimerID);
			}
		}
	}

	//for each of the char. sel. buttons
	for (int i = 0; i < MAX_CHAR_SELECT_BUTTONS; i++){
	
		//render character select button sprites
		m_Graphics->RenderBitmap(charSelectButton[i].bitmapID);

		//if the cursor is hovering on one of the char. sel. buttons
		if (RectContains(charSelectButton[i].buttonRect, m_Input->GetMouseLocation())){
			
			//highlight the button
			m_Graphics->UpdateBitmap(selectedCharButtonID, charSelectButton[i].buttonRect);
			m_Graphics->RenderBitmap(selectedCharButtonID);

			//if the user is currently selecting a character for player 1
			if (player1 == UNSELECTED){
	
				//display the character avatars on the left side of the versus matchup
				if (i == 0){
					m_Graphics->UpdateBitmap(reimuAvatarID, 200, 250);
					m_Graphics->RenderBitmap(reimuAvatarID);
				}
				else if (i == 1){
					m_Graphics->UpdateBitmap(marisaAvatarID, 200, 250);
					m_Graphics->RenderBitmap(marisaAvatarID);
				}

				//if user clicked and released the left mouse button on the character select button,
				//the character indicated by the char. sel. button will be selected
				if (ButtonLeftClicked(charSelectButton[i].buttonRect, m_Input)){
					if (i == 0){
						player1 = REIMU;
						player1AvatarID = reimuAvatarID;
					}
					else if (i == 1){
						player1 = MARISA;
						player1AvatarID = marisaAvatarID;
					}

					//start the timer for flashing the character avatar upon selecting
					m_Clock->AddTimer(player1FlashTimerID, 15);
				}
			}

			//if the user is currently selecting a character for player 2
			else if (player2 == UNSELECTED){
	
				//display a horizontally flipped avatar on the right side of the versus matchup
				if (i == 0){ 
					m_Graphics->UpdateBitmap(reimuAvatarID, 600, 250);
					m_Graphics->EnableBitmapXFlip(reimuAvatarID);
					m_Graphics->RenderBitmap(reimuAvatarID);
					m_Graphics->DisableBitmapXFlip(reimuAvatarID);
				}
				else if (i == 1){
					m_Graphics->UpdateBitmap(marisaAvatarID, 600, 250);
					m_Graphics->EnableBitmapXFlip(marisaAvatarID);
					m_Graphics->RenderBitmap(marisaAvatarID);
					m_Graphics->DisableBitmapXFlip(marisaAvatarID);
				}

				//if user clicked and released the left mouse button on the character select button,
				//the character indicated by the char. sel. button will be selected
				if (ButtonLeftClicked(charSelectButton[i].buttonRect, m_Input)){
					if (i == 0){
						player2 = REIMU;
						player2AvatarID = reimuAvatarID;
					}
					else if (i == 1){
						player2 = MARISA;
						player2AvatarID = marisaAvatarID;
					}

					//start the timer for flashing the character avatar upon selecting
					m_Clock->AddTimer(player2FlashTimerID, 15);
				}
			}
		}
	}

	if (player1 != UNSELECTED) {
		XMFLOAT4 flashHue;

		//if the flash timer is still running, the degree to which the avatar shines white
		//depends on how much time is left for the timer to reach zero
		if (m_Clock->IsTimerRunning(player1FlashTimerID)) {
			flashHue = SOLID_WHITE;
			flashHue.w = (float)m_Clock->TimeLeft(player1FlashTimerID) / 15.0f;
		}
		else {
			flashHue = NULL_COLOR;
		}

		//update avatar positions and white hue
		m_Graphics->DisableBitmapXFlip(player1AvatarID);
		m_Graphics->UpdateBitmap(player1AvatarID, 200, 250, 0.0f, 1.0f, flashHue);

		//render the avatar
		m_Graphics->RenderBitmap(player1AvatarID);
	}

	if (player2 != UNSELECTED) {
		XMFLOAT4 flashHue;

		//if the flash timer is still running, the degree to which the avatar shines white
		//depends on how much time is left for the timer to reach zero
		if (m_Clock->IsTimerRunning(player2FlashTimerID)) {
			flashHue = SOLID_WHITE;
			flashHue.w = (float)m_Clock->TimeLeft(player2FlashTimerID) / 15.0f;
		}
		else {
			flashHue = NULL_COLOR;
		}

		//update avatar positions and white hue, and horizontally flip the 
		//character avatar for player 2
		m_Graphics->EnableBitmapXFlip(player2AvatarID);
		m_Graphics->UpdateBitmap(player2AvatarID, 600, 250, 0.0f, 1.0f, flashHue);

		//render the avatar
		m_Graphics->RenderBitmap(player2AvatarID);
		m_Graphics->DisableBitmapXFlip(player2AvatarID);
	}

	//if both characters are selected, move onto versus mode
	if (player1 != UNSELECTED && player2 != UNSELECTED){
		fadingOut = true;
		fadingIn = false;
		nextGameMode = VERSUS_MODE;
	}

	return true;
}

//versus mode
bool SystemClass::OnVersusMode(){
	
	//initialize versus mode upon first running
	if (!isVersusModeInit){
		if(!InitializeVersusMode()){
	
			//error message
			MessageBox(m_hwnd, L"Could not load the versus mode.", L"Error", MB_OK);
			
			//shut down versus mode
			ShutdownVersusMode();
			
			//move on to main menu
			fadingOut = true;
			fadingIn = false;
			nextGameMode = MAIN_MENU;
			return false;
		}
	}

	//if timer for versus mode fading into next mode reaches one, change current
	//mode to the next mode
	if (fadingOut && m_Clock->TimeLeft(fadeTimerID) == 1) {
		gameMode = nextGameMode;
	}

	return versusMatch->Frame();
}

//initialize variables for character select mode
bool SystemClass::InitializeCharSelect(){
	bool result;
	RECT rc;

	//set the dimensions for character select buttons
	for (int i = 0; i < MAX_CHAR_SELECT_BUTTONS; i++){
		charSelectButton[i].buttonRect.top = 500;
		charSelectButton[i].buttonRect.bottom = 600;
		charSelectButton[i].buttonRect.left = 100 * i;
		charSelectButton[i].buttonRect.right = 100 * i + 100;
	}

	//add bitmap for Reimu's character select button
	result = m_Graphics->AddBitmap("/Data/character_select_screen_reimu.tga", charSelectButton[0].buttonRect,
		m_screenWidth, m_screenHeight, charSelectButton[0].bitmapID);
	if (!result){
		return false;
	}

	//add bitmap for Marisa's character select button
	result = m_Graphics->AddBitmap("/Data/character_select_screen_marisa.tga", charSelectButton[1].buttonRect,
		m_screenWidth, m_screenHeight, charSelectButton[1].bitmapID);
	if (!result){
		return false;
	}

	//add bitmaps for rest of the char. sel. buttons (unselectable)
	for (int i = 2; i < MAX_CHAR_SELECT_BUTTONS; i++){
		result = m_Graphics->AddBitmap("/Data/character_select_screen_locked.tga", charSelectButton[i].buttonRect,
			m_screenWidth, m_screenHeight, charSelectButton[i].bitmapID);
		if (!result){
			return false;
		}
	}

	//this bitmap will be rendered over the char. sel. button
	//over which the mouse cursor is hovering. this bitmap is used to
	//highlight such select button.
	rc = { 0, 100, 0, 100 };
	result = m_Graphics->AddBitmap("/Data/character_select_screen_selected.tga", rc, m_screenWidth,
		m_screenHeight, selectedCharButtonID);
	if (!result){
		return false;
	}

	//background bitmap for char. sel. mode
	rc = { 0, 0, m_screenWidth, m_screenHeight };
	result = m_Graphics->AddBitmap("/Data/character_select_screen_background.tga", rc, m_screenWidth,
		m_screenHeight, charSelectModeBackgroundID);
	if (!result){
		return false;
	}

	rc = { 0, 0, 400, 500 };

	//avatar bitmap for Reimu
	result = m_Graphics->AddBitmap("/Data/character_select_screen_reimu_avatar.tga", rc, m_screenWidth,
		m_screenHeight, reimuAvatarID);
	if (!result){
		return false;
	}

	//avatar bitmap for Marisa
	result = m_Graphics->AddBitmap("/Data/character_select_screen_marisa_avatar.tga", rc, m_screenWidth,
		m_screenHeight, marisaAvatarID);
	if (!result){
		return false;
	}

	//char. sel. mode is initialized
	isCharSelectInit = true;

	return true;
}

//initialize variables for versus mode
bool SystemClass::InitializeVersusMode(){
	versusMatch = new VersusMode(m_Graphics, m_Clock, m_screenWidth, m_screenHeight);
	if (!versusMatch) {
		isVersusModeInit = false;
		return false;
	}

	isVersusModeInit = versusMatch->Initialize();
	return isVersusModeInit;
}

//shuts down versus mode
void SystemClass::ShutdownVersusMode() {
	if (versusMatch) {
		versusMatch->Shutdown();
	}
	isVersusModeInit = false;
}

//handles messages to the Windows application
LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg){
	case WM_KEYDOWN:
		m_Input->KeyDown((unsigned int)wParam);
		return 0;
	case WM_KEYUP:
		m_Input->KeyUp((unsigned int)wParam);
		return 0;
	case WM_LBUTTONDOWN:
		m_Input->KeyDown(VK_LBUTTON);
		SetCapture(m_hwnd);
		return 0;
	case WM_LBUTTONUP:
		m_Input->KeyUp(VK_LBUTTON);
		ReleaseCapture();
		return 0;
	case WM_RBUTTONDOWN:
		m_Input->KeyDown(VK_RBUTTON);
		return 0;
	case WM_RBUTTONUP:
		m_Input->KeyUp(VK_RBUTTON);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

//receives the translated messages, closes the application
//if WM_DESTROY or WM_CLOSE, handle message otherwise
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg){
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		return ApplicationHandle->MessageHandler(hwnd, uMsg, wParam, lParam);
	}
}
