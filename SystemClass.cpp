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

//SystemClass deleter. Do not use this deleter.
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
		return false;
	}
	m_Input->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight);

	//Initialize GraphicsClass object
	m_Graphics = new GraphicsClass;
	if (!m_Graphics){
		return false;
	}
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if (!result){
		return false;
	}

	//Initialize TimeClass object
	m_Clock = new TimeClass;
	if (!m_Clock){
		return false;
	}
	m_Clock->SetFrameCount(0);

	//Initialize the game with main menu screen.
	gameMode = MAIN_MENU;
	//Initialize game start button
	gameStartButton.buttonRect.left = m_screenWidth / 2 - 150;
	gameStartButton.buttonRect.right = m_screenWidth / 2 + 150;
	gameStartButton.buttonRect.top = m_screenHeight / 2 - 50;
	gameStartButton.buttonRect.bottom = m_screenHeight / 2 + 50;
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/gameStartButton.tga", gameStartButton.buttonRect,
		m_screenWidth, m_screenHeight, gameStartButton.bitmapID);
	if (!result){
		return false;
	}

	//No game modes except main menu are initialized
	isCharSelectInit = false;
	isCharSelectStarted = false;
	isVersusModeInit = false;

	//No characters selected yet
	for (int i = 0; i < 2; i++){
		versusMatch.player[i].character = UNSELECTED;
	}

	//Set click positions to default (-1)
	lClickPos.x = -1;
	lClickPos.y = -1;
	rClickPos.x = -1;
	rClickPos.y = -1;

	//Not fading in or out at the start of the application
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
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT){
			done = true;
		}
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
	int mouseX, mouseY;

	//Increase the frame count
	m_Clock->FrameIncrement();

	//Update mouse position every frame
	m_Input->Frame(m_hwnd);
	m_Input->GetMouseLocation(mouseX, mouseY);
	mousePt.x = mouseX;
	mousePt.y = mouseY;
	//Set the left and right mouse click positions where applicable
	SetInitialClickPositions();

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
			return false;
		}
		break;

	case VERSUS_MODE:
		OnVersusMode();
		break;
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

//Update lClickPos and rClickPos when left/right mouse button is pressed/released
void SystemClass::SetInitialClickPositions(){
	if (m_Input->IsKeyJustPressed(VK_LBUTTON)){
		lClickPos.x = mousePt.x;
		lClickPos.y = mousePt.y;
	}
	if (!m_Input->IsKeyDown(VK_LBUTTON) && !m_Input->IsKeyJustReleased(VK_LBUTTON)){
		lClickPos.x = -1;
		lClickPos.y = -1;
	}
	if (m_Input->IsKeyJustPressed(VK_RBUTTON)){
		rClickPos.x = mousePt.x;
		rClickPos.y = mousePt.y;
	}
	if (!m_Input->IsKeyDown(VK_RBUTTON) && !m_Input->IsKeyJustReleased(VK_RBUTTON)){
		rClickPos.x = -1;
		rClickPos.y = -1;
	}
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
	//render Start button
	m_Graphics->RenderBitmap(gameStartButton.bitmapID);
	
	//if fading out of main menu into another game mode
	if (fadingOut&&m_Clock->TimeLeft(fadeTimerID) == 1){
		gameMode = nextGameMode;
	}
	//if not fading out of main menu
	else if (!fadingOut){
		//if Started button is pressed
		if (m_Input->IsKeyJustReleased(VK_LBUTTON) && Contains(gameStartButton.buttonRect, lClickPos) &&
			Contains(gameStartButton.buttonRect, mousePt)){
			
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
		versusMatch.player[0].character = UNSELECTED;
		versusMatch.player[1].character = UNSELECTED;
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
	//if right-clicked the mouse, currently selected character will be cancelled
	if (m_Input->IsKeyJustReleased(VK_RBUTTON) && Contains(rc, rClickPos)){
		if (versusMatch.player[1].character != UNSELECTED){
			versusMatch.player[1].character = UNSELECTED;
			m_Clock->DeleteTimer(versusMatch.player[1].flashTimerID);
		}
		else if (versusMatch.player[0].character != UNSELECTED){
			versusMatch.player[0].character = UNSELECTED;
			m_Clock->DeleteTimer(versusMatch.player[0].flashTimerID);
		}
	}

	for (int i = 0; i < MAX_CHAR_SELECT_BUTTONS; i++){
		m_Graphics->RenderBitmap(charSelect[i].bitmapID);
		if (Contains(charSelect[i].buttonRect, mousePt)){
			m_Graphics->UpdateBitmap(selectedCharButtonID, charSelect[i].buttonRect);
			m_Graphics->RenderBitmap(selectedCharButtonID);
			if (versusMatch.player[0].character == UNSELECTED){
				if (i == 0){
					m_Graphics->UpdateBitmap(reimuAvatarID, 200, 250);
					m_Graphics->RenderBitmap(reimuAvatarID);
				}
				else if (i == 1){
					m_Graphics->UpdateBitmap(marisaAvatarID, 200, 250);
					m_Graphics->RenderBitmap(marisaAvatarID);
				}
				if (m_Input->IsKeyJustReleased(VK_LBUTTON) && Contains(charSelect[i].buttonRect, lClickPos)){
					if (i == 0){
						versusMatch.player[0].character = REIMU;
						versusMatch.player[0].pCharacterAvatarID = &reimuAvatarID;
					}
					else if (i == 1){
						versusMatch.player[0].character = MARISA;
						versusMatch.player[0].pCharacterAvatarID = &marisaAvatarID;
					}
					m_Clock->AddTimer(versusMatch.player[0].flashTimerID, 15);
				}
			}
			else if (versusMatch.player[1].character == UNSELECTED){
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
				if (m_Input->IsKeyJustReleased(VK_LBUTTON) && Contains(charSelect[i].buttonRect, lClickPos)){
					if (i == 0){
						versusMatch.player[1].character = REIMU;
						versusMatch.player[1].pCharacterAvatarID = &reimuAvatarID;
					}
					else if (i == 1){
						versusMatch.player[1].character = MARISA;
						versusMatch.player[1].pCharacterAvatarID = &marisaAvatarID;
					}
					m_Clock->AddTimer(versusMatch.player[1].flashTimerID, 15);
				}
			}
		}
	}
	versusMatch.numPlayers = 2;
	for (int i = 0; i < versusMatch.numPlayers; i++){
		if (versusMatch.player[i].character != UNSELECTED){
			XMFLOAT4 flashHue;
			if (m_Clock->IsTimerRunning(versusMatch.player[i].flashTimerID)){
				flashHue = SOLID_WHITE;
				flashHue.w = (float)m_Clock->TimeLeft(versusMatch.player[i].flashTimerID) / 15.0f;
			}
			else{
				flashHue = NULL_COLOR;
			}
			if (i == 0){
				m_Graphics->DisableBitmapXFlip(*versusMatch.player[i].pCharacterAvatarID);
				m_Graphics->UpdateBitmap(*versusMatch.player[i].pCharacterAvatarID, 200, 250, 0.0f, 1.0f, flashHue);
			}
			else if (i == 1){
				m_Graphics->EnableBitmapXFlip(*versusMatch.player[i].pCharacterAvatarID);
				m_Graphics->UpdateBitmap(*versusMatch.player[i].pCharacterAvatarID, 600, 250, 0.0f, 1.0f, flashHue);
			}
			m_Graphics->RenderBitmap(*versusMatch.player[i].pCharacterAvatarID);
			m_Graphics->DisableBitmapXFlip(*versusMatch.player[i].pCharacterAvatarID);
		}
	}

	if (versusMatch.player[0].character != UNSELECTED && versusMatch.player[1].character != UNSELECTED){
		fadingOut = true;
		fadingIn = false;
		nextGameMode = VERSUS_MODE;
	}

	return true;
}

void SystemClass::OnVersusMode(){
	if (!isVersusModeInit){
		if(!InitializeVersusMode()){ //Initialize versus mode, and if not initialized successfully,
			MessageBox(m_hwnd, L"Could not load the versus mode.", L"Error", MB_OK);
			ShutdownVersusMode();
			fadingOut = true;
			fadingIn = false;
			nextGameMode = MAIN_MENU;
			return;
		}
	}

#ifdef _DEBUG
	if (m_Input->IsKeyJustPressed(0x52)){  //R key resets/reloads versus mode
		ShutdownVersusMode();
		InitializeVersusMode();
		return;
	}
#endif

	if (fadingOut && m_Clock->TimeLeft(fadeTimerID) == 1){
		gameMode = nextGameMode;
	}

	//render map background
	m_Graphics->UpdateBitmap(versusMatch.map.mapBitmapID, 400, 300);
	m_Graphics->RenderBitmap(versusMatch.map.mapBitmapID);

	//render character sprites
	for (int i = 0; i < versusMatch.numPlayers; i++){
		float renderBitmapPosX = versusMatch.player[i].position.x;
		float renderBitmapPosY = versusMatch.player[i].position.y;
		m_Graphics->UpdateBitmap(versusMatch.player[i].inGameSpriteID, round(renderBitmapPosX), round(renderBitmapPosY));
		m_Graphics->RenderBitmap(versusMatch.player[i].inGameSpriteID);
	}
	for (int i = 0; i < versusMatch.numBullets; i++){
		m_Graphics->UpdateBitmap(*versusMatch.bullet[i].pBitmapID, versusMatch.bullet[i].position.x, versusMatch.bullet[i].position.y, versusMatch.bullet[i].moveAngle);
		m_Graphics->RenderBitmap(*versusMatch.bullet[i].pBitmapID);
	}


	if (versusMatch.movePhase && versusMatch.choice==PENDING_CHOICE){
		versusMatch.moveButton.buttonRect = { 0, 550, 150, 600 };
		m_Graphics->UpdateBitmap(versusMatch.moveButton.bitmapID, versusMatch.moveButton.buttonRect);
		m_Graphics->RenderBitmap(versusMatch.moveButton.bitmapID);

		versusMatch.passButton.buttonRect = { 150, 550, 300, 600 };
		m_Graphics->UpdateBitmap(versusMatch.passButton.bitmapID, versusMatch.passButton.buttonRect);
		m_Graphics->RenderBitmap(versusMatch.passButton.bitmapID);
		if (m_Input->IsKeyJustReleased(VK_LBUTTON)){
			if (Contains(versusMatch.moveButton.buttonRect,mousePt) && Contains(versusMatch.moveButton.buttonRect, lClickPos)){
				versusMatch.choice = MOVE;
			}
			else if (Contains(versusMatch.passButton.buttonRect,mousePt) && Contains(versusMatch.passButton.buttonRect, lClickPos)){
				
				versusMatch.choice = PASS;
			}
		}

#ifdef _DEBUG
		else if (m_Input->IsKeyJustPressed(0x4D)){
			versusMatch.choice = MOVE;
		}
		else if (m_Input->IsKeyJustPressed(0x50)){
			versusMatch.choice = PASS;
		}
#endif
	}
	else if (versusMatch.actionPhase && versusMatch.choice == PENDING_CHOICE){
		versusMatch.shootButton.buttonRect = { 0, 550, 150, 600 };
		m_Graphics->UpdateBitmap(versusMatch.shootButton.bitmapID, versusMatch.shootButton.buttonRect);
		m_Graphics->RenderBitmap(versusMatch.shootButton.bitmapID);

		versusMatch.spellButton.buttonRect = { 150, 550, 300, 600 };
		m_Graphics->UpdateBitmap(versusMatch.spellButton.bitmapID, versusMatch.spellButton.buttonRect);
		m_Graphics->RenderBitmap(versusMatch.spellButton.bitmapID);

		versusMatch.passButton.buttonRect = { 300, 550, 450, 600 };
		m_Graphics->UpdateBitmap(versusMatch.passButton.bitmapID, versusMatch.passButton.buttonRect);
		m_Graphics->RenderBitmap(versusMatch.passButton.bitmapID);

		if (m_Input->IsKeyJustReleased(VK_LBUTTON)){
			if (Contains(versusMatch.shootButton.buttonRect, mousePt) && Contains(versusMatch.shootButton.buttonRect, lClickPos)){
				versusMatch.choice = SHOOT;
			}
			else if (Contains(versusMatch.passButton.buttonRect, mousePt) && Contains(versusMatch.passButton.buttonRect, lClickPos)){
				versusMatch.choice = PASS;
			}
			else if (Contains(versusMatch.spellButton.buttonRect, mousePt) && Contains(versusMatch.spellButton.buttonRect, lClickPos)){
				versusMatch.choice = SPELL;
			}
		}
	}

	if (versusMatch.movePhase && versusMatch.choice != PENDING_CHOICE){  //if the user makes a choice in move phase
		XMFLOAT2& pos = versusMatch.player[versusMatch.playerTurn].position;  //x,y position of character
		XMFLOAT2& speedVec = versusMatch.player[versusMatch.playerTurn].moveSpeed;  //x,y component of character's velocity
		float& angle = versusMatch.player[versusMatch.playerTurn].moveAngle;  //angle of character's velocity
		switch (versusMatch.choice){
		case MOVE:
			versusMatch.shooting = true;
			if (versusMatch.shooting && speedVec.x == 0 && speedVec.y == 0){
				Shoot(pos, speedVec, angle);
			}
			else if (versusMatch.shooting){
				Moving(pos, speedVec, angle,versusMatch.player[versusMatch.playerTurn].hitboxRadius);
				if (!versusMatch.shooting){
					versusMatch.movePhase = false;
					versusMatch.actionPhase = true;
					m_Clock->SetTimer(versusMatch.phaseAnnounceTimerID, 240);
					versusMatch.choice = PENDING_CHOICE;
				}
			}
			break;
		case PASS:
			versusMatch.movePhase = false;
			versusMatch.actionPhase = true;
			m_Clock->SetTimer(versusMatch.phaseAnnounceTimerID, 240);
			versusMatch.choice = PENDING_CHOICE;
			break;
		}
	}
	else if (versusMatch.actionPhase && versusMatch.choice != PENDING_CHOICE){
		switch (versusMatch.choice){
		case SHOOT:
			switch (versusMatch.player[versusMatch.playerTurn].character){
			case REIMU:
				if (!versusMatch.shooting){
					InitializeTempPosSpeedAngle(versusMatch.player[versusMatch.playerTurn].position.x, versusMatch.player[versusMatch.playerTurn].position.y);
					versusMatch.shooting = true;
				}
				if (versusMatch.shooting){
					if (versusMatch.tempPos || versusMatch.tempSpeed || versusMatch.tempAngle){
						if (versusMatch.tempSpeed->x == 0.0f && versusMatch.tempSpeed->y == 0.0f){
							Shoot(*versusMatch.tempPos, *versusMatch.tempSpeed, *versusMatch.tempAngle);
						}
						else{
							versusMatch.bullet[versusMatch.numBullets].damage = 2;
							versusMatch.bullet[versusMatch.numBullets].pBitmapID = &versusMatch.type01color01bulletID;
							if (versusMatch.tempPos){
								versusMatch.bullet[versusMatch.numBullets].position = *versusMatch.tempPos;
								delete versusMatch.tempPos;
								versusMatch.tempPos = 0;
							}
							if (versusMatch.tempSpeed){
								versusMatch.bullet[versusMatch.numBullets].moveSpeed = *versusMatch.tempSpeed;
								delete versusMatch.tempSpeed;
								versusMatch.tempSpeed = 0;
							}
							if (versusMatch.tempAngle){
								versusMatch.bullet[versusMatch.numBullets].moveAngle = *versusMatch.tempAngle;
								delete versusMatch.tempAngle;
								versusMatch.tempAngle = 0;
							}
							versusMatch.numBullets++;
						}
					}
					if(!versusMatch.tempPos && !versusMatch.tempSpeed && !versusMatch.tempAngle){
						XMFLOAT2& bulletPos = versusMatch.bullet[versusMatch.numBullets - 1].position;
						XMFLOAT2& bulletSpeed = versusMatch.bullet[versusMatch.numBullets - 1].moveSpeed;
						float& bulletAngle = versusMatch.bullet[versusMatch.numBullets - 1].moveAngle;
						Moving(bulletPos, bulletSpeed, bulletAngle, 6.0f);
						if (!versusMatch.shooting){
							int collidedChar;
							if (CollisionWithCharacter(bulletPos, 6.0f, collidedChar)){
								versusMatch.player[collidedChar].hp -= versusMatch.bullet[versusMatch.numBullets - 1].damage;
							}
							versusMatch.numBullets--;
							versusMatch.movePhase = true;
							versusMatch.actionPhase = false;
							versusMatch.playerTurn++;
							if (versusMatch.playerTurn >= versusMatch.numPlayers){
								versusMatch.playerTurn = 0;
							}
							m_Clock->SetTimer(versusMatch.phaseAnnounceTimerID, 240);
							versusMatch.choice = PENDING_CHOICE;
						}
					}
				}
				
				break;
			case MARISA:
				if (!versusMatch.shooting){
					InitializeTempPosSpeedAngle(versusMatch.player[versusMatch.playerTurn].position.x, versusMatch.player[versusMatch.playerTurn].position.y);
					versusMatch.shooting = true;
				}
				if (versusMatch.shooting){
					if (versusMatch.tempPos || versusMatch.tempSpeed || versusMatch.tempAngle){
						if (versusMatch.tempSpeed->x == 0 && versusMatch.tempSpeed->y == 0){
							Shoot(*versusMatch.tempPos, *versusMatch.tempSpeed, *versusMatch.tempAngle);
						}
						else{
							if (versusMatch.tempAngle){
								versusMatch.laser[versusMatch.numLasers].angle = *versusMatch.tempAngle;
								delete versusMatch.tempAngle;
								versusMatch.tempAngle = 0;
							}
							if (versusMatch.tempPos){
								for (int i = 0; i < 9; i++){
									
									versusMatch.laser[versusMatch.numLasers].particle[i]
								}
								// !!!
								// purposefully left a syntax error here so that
								// I know where to come back to
							}
							if (versusMatch.tempSpeed){
								delete versusMatch.tempSpeed;
								versusMatch.tempSpeed = 0;
							}
							//set parameters for the laser
						}
					}
				}
				break;
			}
			break;
		case PASS:
			versusMatch.movePhase = true;
			versusMatch.actionPhase = false;
			versusMatch.playerTurn++;
			if (versusMatch.playerTurn >= versusMatch.numPlayers){
				versusMatch.playerTurn = 0;
			}
			m_Clock->SetTimer(versusMatch.phaseAnnounceTimerID, 240);
			versusMatch.choice = PENDING_CHOICE;
			break;
		case SPELL:
			break;
		}
	}
	
	//character status bar
	if (versusMatch.choice == PENDING_CHOICE){
		for (int i = 0; i < versusMatch.numPlayers; i++){
			RECT rc;
			rc.left = (long)round(versusMatch.player[i].position.x - versusMatch.player[i].inGameSpriteWidth / 2.0f);
			rc.right = (long)round(versusMatch.player[i].position.x + versusMatch.player[i].inGameSpriteWidth / 2.0f);
			rc.top = (long)round(versusMatch.player[i].position.y - versusMatch.player[i].inGameSpriteHeight / 2.0f);
			rc.bottom = (long)round(versusMatch.player[i].position.y + versusMatch.player[i].inGameSpriteHeight / 2.0f);
			if (Contains(rc, mousePt)){
				m_Graphics->UpdateBitmap(versusMatch.statsWindowBitmapID, mousePt.x + 35, mousePt.y - 40);
				m_Graphics->RenderBitmap(versusMatch.statsWindowBitmapID);
				
				string hpStr = "HP: ";
				hpStr = hpStr + to_string(versusMatch.player[i].hp) + " / " + to_string(versusMatch.player[i].maxHp);
				char hpDisp[MAX_CHARACTER_COUNT];
				strcpy(hpDisp, hpStr.c_str());
				m_Graphics->UpdateSentence(versusMatch.hpDispSentID, hpDisp, mousePt.x, mousePt.y - 65, SOLID_BLACK);
				m_Graphics->RenderSentence(versusMatch.hpDispSentID);

				string mpStr = "MP: " + to_string(versusMatch.player[i].mp);
				char mpDisp[MAX_CHARACTER_COUNT];
				strcpy(mpDisp, mpStr.c_str());
				m_Graphics->UpdateSentence(versusMatch.mpDispSentID, mpDisp, mousePt.x, mousePt.y-30, SOLID_BLACK);
				m_Graphics->RenderSentence(versusMatch.mpDispSentID);

				break;
			}
		}
	}

	//update pPhaseAnnounceBitmapID
	if (versusMatch.movePhase){
		versusMatch.pPhaseAnnounceBitmapID = &versusMatch.movePhaseAnnounceBitmapID;
	}
	else if (versusMatch.actionPhase){
		versusMatch.pPhaseAnnounceBitmapID = &versusMatch.actPhaseAnnounceBitmapID;
	}

	//render phase announcement based on how much time is left
	if (m_Clock->IsTimerRunning(versusMatch.phaseAnnounceTimerID)){
		long timeLeft = m_Clock->TimeLeft(versusMatch.phaseAnnounceTimerID);
		int posX = m_Graphics->GetBitmapWidth(*versusMatch.pPhaseAnnounceBitmapID) / 2;
		int posY = m_Graphics->GetBitmapHeight(*versusMatch.pPhaseAnnounceBitmapID) / 2;
		if (timeLeft>=210){
			m_Graphics->UpdateBitmap(*versusMatch.pPhaseAnnounceBitmapID, posX, posY, 0.0f, 1.0f - ((float)timeLeft - 210.0f) / 30.0f, NULL_COLOR);
		}
		else if (timeLeft < 210 && timeLeft >= 60){
			m_Graphics->UpdateBitmap(*versusMatch.pPhaseAnnounceBitmapID, posX, posY);
		}
		else if (timeLeft < 60){
			m_Graphics->UpdateBitmap(*versusMatch.pPhaseAnnounceBitmapID, posX, posY, 0.0f, (float)timeLeft / 60.0f);
		}
		m_Graphics->RenderBitmap(*versusMatch.pPhaseAnnounceBitmapID);
	}
}

bool SystemClass::InitializeCharSelect(){
	bool result;

	versusMatch.numPlayers = 2;

	for (int i = 0; i < MAX_CHAR_SELECT_BUTTONS; i++){
		charSelect[i].buttonRect.top = 500;
		charSelect[i].buttonRect.bottom = 600;
		charSelect[i].buttonRect.left = 100 * i;
		charSelect[i].buttonRect.right = 100 * i + 100;
	}

	result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_reimu.tga", charSelect[0].buttonRect, m_screenWidth, m_screenHeight, charSelect[0].bitmapID);
	if (!result){
		return false;
	}
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_marisa.tga", charSelect[1].buttonRect, m_screenWidth, m_screenHeight, charSelect[1].bitmapID);
	if (!result){
		return false;
	}
	for (int i = 2; i < MAX_CHAR_SELECT_BUTTONS; i++){
		result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_locked.tga", charSelect[i].buttonRect, m_screenWidth, m_screenHeight, charSelect[i].bitmapID);
		if (!result){
			return false;
		}
	}

	RECT rc;
	rc.top = 0;
	rc.bottom = 100;
	rc.left = 0;
	rc.right = 100;
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_selected.tga", rc, m_screenWidth, m_screenHeight, selectedCharButtonID);
	if (!result){
		return false;
	}

	rc = { 0, 0, m_screenWidth, m_screenHeight };
	m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_background.tga", rc, m_screenWidth, m_screenHeight, charSelectModeBackgroundID);

	rc = { 0, 0, 400, 500 };
	m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_reimu_avatar.tga", rc, m_screenWidth, m_screenHeight, reimuAvatarID);
	m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_marisa_avatar.tga", rc, m_screenWidth, m_screenHeight, marisaAvatarID);

	isCharSelectInit = true;
	return true;
}

bool SystemClass::InitializeVersusMode(){
	bool result;

	versusMatch.playerTurn = 0;
	versusMatch.movePhase = true;
	versusMatch.actionPhase = false;
	versusMatch.shooting = false;
	versusMatch.shootFrame = -1;
	versusMatch.victoryPlayer = -1;
	versusMatch.choice = PENDING_CHOICE;
	versusMatch.numBullets = 0;
	versusMatch.tempAngle = 0;
	versusMatch.tempPos = 0;
	versusMatch.tempSpeed = 0;
	m_Clock->AddTimer(versusMatch.phaseAnnounceTimerID, 240);
	RECT screenRect = { 0, 0, m_screenWidth, m_screenHeight };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/test_map.tga", screenRect, m_screenWidth, m_screenHeight, versusMatch.map.mapBitmapID);
	if (!result){
		return false;
	}
	RECT announceRect = { 0, 0, 398, 37 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_move_phase_announce.tga", announceRect, m_screenWidth, m_screenHeight, versusMatch.movePhaseAnnounceBitmapID);
	if (!result){
		return false;
	}
	announceRect = { 0, 0, 338, 37 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_act_phase_announce.tga", announceRect, m_screenWidth, m_screenHeight, versusMatch.actPhaseAnnounceBitmapID);
	if (!result){
		return false;
	}
	versusMatch.passButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_pass_button.tga", versusMatch.passButton.buttonRect, m_screenWidth, m_screenHeight, versusMatch.passButton.bitmapID);
	if (!result){
		return false;
	}
	versusMatch.moveButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_move_button.tga", versusMatch.moveButton.buttonRect, m_screenWidth, m_screenHeight, versusMatch.moveButton.bitmapID);
	if (!result){
		return false;
	}
	versusMatch.shootButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_shoot_button.tga", versusMatch.shootButton.buttonRect, m_screenWidth, m_screenHeight, versusMatch.shootButton.bitmapID);
	if (!result){
		return false;
	}
	versusMatch.spellButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_spell_button.tga", versusMatch.spellButton.buttonRect, m_screenWidth, m_screenHeight, versusMatch.spellButton.bitmapID);
	if (!result){
		return false;
	}
	RECT aimCircleRect = { 0, 0, 100, 100 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_aim.tga", aimCircleRect, m_screenWidth, m_screenHeight, versusMatch.aimCircleBitmapID);
	if (!result){
		return false;
	}
	RECT bulletRect = { 0, 0, 14, 12 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/bullet/type_01_color_01.tga", bulletRect, m_screenWidth, m_screenHeight, versusMatch.type01color01bulletID);
	if (!result){
		return false;
	}
	RECT statsRect = { 0, 0, 100, 80 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_stats_window.tga", statsRect, m_screenWidth, m_screenHeight, versusMatch.statsWindowBitmapID);
	if (!result){
		return false;
	}
	
	for (int i = 0; i < MAX_ON_SCREEN_LASERS; i++){
		versusMatch.laser[i].particle = 0;
	}
	versusMatch.numLasers = 0;

	RECT particleRect = { 0, 0, 1, 1 };
	for (int i = 0; i < 9; i++){
		char path[MAX_CHARACTER_COUNT];
		string pathStr = "/Data/bullet/laser_color_01_particle_0" + to_string(i + 1) + ".tga";
		strcpy(path, pathStr.c_str());
		result = m_Graphics->AddBitmap(m_hwnd, path, particleRect, m_screenWidth, m_screenHeight, versusMatch.laserColor01particleID[i]);
		if (!result){
			return false;
		}
	}

	result = m_Graphics->AddSentence(m_hwnd, " ", 0, 0, m_screenWidth, m_screenHeight, SOLID_BLACK, versusMatch.hpDispSentID);
	if (!result){
		return false;
	}
	result = m_Graphics->AddSentence(m_hwnd, " ", 0, 0, m_screenWidth, m_screenHeight, SOLID_BLACK, versusMatch.mpDispSentID);
	if (!result){
		return false;
	}

	for (int y = 0; y < 600; y++){
		for (int x = 0; x < 800; x++){
			versusMatch.map.isWall[x][y] = false;
		}
	}
	for (int x = 0; x < 800; x++){
		for (int y = 0; y < 30; y++){
			versusMatch.map.isWall[x][y] = true;
		}
		for (int y = 570; y < 600; y++){
			versusMatch.map.isWall[x][y] = true;
		}
	}
	for (int y = 30; y < 570; y++){
		for (int x = 0; x < 30; x++){
			versusMatch.map.isWall[x][y] = true;
		}
		for (int x = 770; x < 800; x++){
			versusMatch.map.isWall[x][y] = true;
		}
	}

	for (int i = 0; i < versusMatch.numPlayers; i++){
		versusMatch.player[i].spellCard = 0;
		RECT spriteRect;
		switch (versusMatch.player[i].character){
		case REIMU:
			versusMatch.player[i].maxHp = 40;
			versusMatch.player[i].numSpellCards = 2;
			versusMatch.player[i].inGameSpriteWidth = 20;
			versusMatch.player[i].inGameSpriteHeight = 20;
			versusMatch.player[i].hitboxRadius = 10.0f;
			spriteRect = { 0, 0, versusMatch.player[i].inGameSpriteWidth, versusMatch.player[i].inGameSpriteHeight };
			result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_reimu.tga", spriteRect, m_screenWidth, m_screenHeight, versusMatch.player[i].inGameSpriteID);
			if (!result){
				return false;
			}
			versusMatch.player[i].spellCard = new SpellCardType[versusMatch.player[i].numSpellCards];
			strcpy(versusMatch.player[i].spellCard[0].cardName, "Spirit Sign \"Fantasy Seal\"");
			versusMatch.player[i].spellCard[0].mpCost = 6;
			strcpy(versusMatch.player[i].spellCard[1].cardName, "Dream Sign \"Evil-Sealing Circle\"");
			versusMatch.player[i].spellCard[1].mpCost = 8;

			break;

		case MARISA:
			versusMatch.player[i].maxHp = 43;
			versusMatch.player[i].inGameSpriteWidth = 30;
			versusMatch.player[i].inGameSpriteHeight = 30;
			versusMatch.player[i].hitboxRadius = 15.0f;
			spriteRect = { 0, 0, versusMatch.player[i].inGameSpriteWidth, versusMatch.player[i].inGameSpriteHeight };
			result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_marisa.tga", spriteRect, m_screenWidth, m_screenHeight, versusMatch.player[i].inGameSpriteID);
			if (!result){
				return false;
			}
			versusMatch.player[i].numSpellCards = 2;
			versusMatch.player[i].spellCard = new SpellCardType[versusMatch.player[i].numSpellCards];
			strcpy(versusMatch.player[i].spellCard[0].cardName, "Magic Sign \"Stardust Reverie\"");
			versusMatch.player[i].spellCard[0].mpCost = 12;
			strcpy(versusMatch.player[i].spellCard[1].cardName, "Love Sign \"Master Spark\"");
			versusMatch.player[i].spellCard[1].mpCost = 15;

			break;
		}
		versusMatch.player[i].hp = versusMatch.player[i].maxHp;
		versusMatch.player[i].mp = 0;
		versusMatch.player[i].moveSpeed = { 0.0f, 0.0f };

		if (i == 0){
			versusMatch.player[i].position = { 100.0f, 300.0f };
		}
		else if (i == 1){
			versusMatch.player[i].position = { 700.0f, 300.0f };
		}
	}

	isVersusModeInit = true;
	return true;
}

void SystemClass::ShutdownVersusMode(){
	for (int i = 0; i < versusMatch.numPlayers; i++){
		m_Graphics->DeleteBitmap(versusMatch.player[i].inGameSpriteID);
		if (versusMatch.player[i].spellCard){
			delete[] versusMatch.player[i].spellCard;
			versusMatch.player[i].spellCard = 0;
		}
	}
	if (versusMatch.tempAngle){
		delete versusMatch.tempAngle;
		versusMatch.tempAngle = 0;
	}
	if (versusMatch.tempPos){
		delete versusMatch.tempPos;
		versusMatch.tempPos = 0;
	}
	if (versusMatch.tempSpeed){
		delete versusMatch.tempSpeed;
		versusMatch.tempSpeed = 0;
	}
	m_Graphics->DeleteBitmap(versusMatch.map.mapBitmapID);
	m_Graphics->DeleteBitmap(versusMatch.movePhaseAnnounceBitmapID);
	m_Graphics->DeleteBitmap(versusMatch.actPhaseAnnounceBitmapID);
	m_Graphics->DeleteBitmap(versusMatch.passButton.bitmapID);
	m_Graphics->DeleteBitmap(versusMatch.moveButton.bitmapID);
	m_Graphics->DeleteBitmap(versusMatch.shootButton.bitmapID);
	m_Graphics->DeleteBitmap(versusMatch.spellButton.bitmapID);
	m_Graphics->DeleteBitmap(versusMatch.aimCircleBitmapID);
	m_Graphics->DeleteBitmap(versusMatch.statsWindowBitmapID);
	for (int i = 0; i < 9; i++){
		m_Graphics->DeleteBitmap(versusMatch.laserColor01particleID[i]);
	}

	m_Graphics->DeleteSentence(versusMatch.hpDispSentID);
	m_Graphics->DeleteSentence(versusMatch.mpDispSentID);

	m_Clock->DeleteTimer(versusMatch.phaseAnnounceTimerID);

	isVersusModeInit = false;
}

bool SystemClass::Contains(RECT rect, POINT pt){
	if (pt.x > rect.left&&pt.x < rect.right&&pt.y > rect.top&&pt.y < rect.bottom){
		return true;
	}
	else{
		return false;
	}
}

float SystemClass::Distance(float x1, float y1, float x2, float y2){
	return (float)sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
}

float SystemClass::Distance(POINT p1, POINT p2){
	return (float)sqrt(pow(p1.y - p2.y, 2) + pow(p1.x - p2.x, 2));
}

float SystemClass::Distance(XMFLOAT2 p1, POINT p2){
	return (float)sqrt(pow(p1.y - (float)p2.y, 2) + pow(p1.x - (float)p2.x, 2));
}

float SystemClass::Distance(POINT p1, XMFLOAT2 p2){
	return (float)sqrt(pow((float)p1.y - p2.y, 2) + pow((float)p1.x - p2.x, 2));
}

float SystemClass::Distance(XMFLOAT2 p1, XMFLOAT2 p2){
	return (float)sqrt(pow(p1.y - p2.y, 2) + pow(p1.x - p2.x, 2));
}

bool SystemClass::CollisionWithWall(XMFLOAT2 pos, float radius){
	RECT hitboxRect = { (LONG)round(pos.x - radius), (LONG)round(pos.y - radius), (LONG)round(pos.x + radius), (LONG)round(pos.y + radius) };
	for (int x = hitboxRect.left; x <= hitboxRect.right; x++){
		for (int y = hitboxRect.top; y <= hitboxRect.bottom; y++){
			XMFLOAT2 testCoord = { (float)x, (float)y };
			if (Distance(testCoord, pos) <= radius && versusMatch.map.isWall[x - 1][y - 1]){
				return true;
			}
		}
	}
	return false;
}

bool SystemClass::CollisionWithCharacter(XMFLOAT2 pos, float radius, int& collidedChar){
	RECT hitboxRect = { (LONG)round(pos.x - radius), (LONG)round(pos.y - radius), (LONG)round(pos.x + radius), (LONG)round(pos.y + radius) };
	for (int x = hitboxRect.left; x <= hitboxRect.right; x++){
		for (int y = hitboxRect.top; y <= hitboxRect.bottom; y++){
			XMFLOAT2 testCoord = { (float)x, (float)y };
			for (int i = 0; i < versusMatch.numPlayers; i++){
				if (i != versusMatch.playerTurn){
					if (Distance(testCoord, pos) <= radius && Distance(testCoord, versusMatch.player[i].position) <= versusMatch.player[i].hitboxRadius){
						collidedChar = i;
						return true;
					}
				}
			}
		}
	}
	return false;
}

void SystemClass::Shoot(XMFLOAT2& pos, XMFLOAT2& speedVec, float& angle){
	if (versusMatch.shootFrame == -1 && m_Input->IsKeyDown(VK_LBUTTON) && Distance(lClickPos, versusMatch.player[versusMatch.playerTurn].position) <= (float)versusMatch.player[versusMatch.playerTurn].hitboxRadius && (mousePt.x != lClickPos.x || mousePt.y != lClickPos.y)){ //if the player clicked on the character sprite
		versusMatch.shootFrame = m_Clock->GetFrameCount() - 1;
	}
	//Character's velocity will be recorded once player releases the left mouse button,
	//or the mouse gets certain distance away from the character
	else if (versusMatch.shootFrame != -1 && (m_Input->IsKeyJustReleased(VK_LBUTTON) || Distance(mousePt, lClickPos) > 50.0f)){
		speedVec.x = (float)(mousePt.x - lClickPos.x) / (float)(m_Clock->GetFrameCount() - versusMatch.shootFrame); //record x component
		speedVec.y = (float)(mousePt.y - lClickPos.y) / (float)(m_Clock->GetFrameCount() - versusMatch.shootFrame); //record y component
		angle = atan2(speedVec.y, speedVec.x);   //record angle
		versusMatch.shootFrame = -1;
	}

	if (m_Input->IsKeyDown(VK_LBUTTON) && Distance(lClickPos, versusMatch.player[versusMatch.playerTurn].position) <= (float)versusMatch.player[versusMatch.playerTurn].hitboxRadius){
		m_Graphics->UpdateBitmap(versusMatch.aimCircleBitmapID, lClickPos.x, lClickPos.y);
		m_Graphics->RenderBitmap(versusMatch.aimCircleBitmapID);
	}

}

void SystemClass::Moving(XMFLOAT2& pos, XMFLOAT2& speedVec, float& angle, float radius){
	pos.x += speedVec.x;  //Update x position
	pos.y += speedVec.y;  //update y position
	int nullInt;
	if (CollisionWithWall(pos, radius) || CollisionWithCharacter(pos,radius,nullInt)){
		float slope;
		XMFLOAT2 posI = { pos.x - speedVec.x, pos.y - speedVec.y };
		if (speedVec.x == 0){
			if (speedVec.y > 0){
				for (float y = 0.0f; posI.y + y <= pos.y; y += 0.1f){
					XMFLOAT2 testPos = { posI.x, posI.y + y };
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(pos, radius, nullInt)){
						pos = testPos;
						break;
					}
				}
			}
			else if (speedVec.y < 0){
				for (float y = 0.0f; posI.y + y >= pos.y; y -= 0.1f){
					XMFLOAT2 testPos = { posI.x, posI.y + y };
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(testPos, radius, nullInt)){
						pos = testPos;
						break;
					}
				}
			}
		}
		else{
			slope = speedVec.y / speedVec.x;
			float b = pos.y - slope*pos.x;
			if (speedVec.x > 0){
				for (float x = 0.0f; posI.x + x <= pos.x; x += 0.1f){
					XMFLOAT2 testPos = { posI.x + x, slope*(posI.x + x) + b };
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(testPos,radius,nullInt)){
						pos = testPos;
						break;
					}
				}
			}
			else if (speedVec.x < 0){
				for (float x = 0.0f; posI.x + x >= pos.x; x -= 0.1f){
					XMFLOAT2 testPos = { posI.x + x, slope*(posI.x + x) + b };
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(testPos,radius,nullInt)){
						pos = testPos;
						break;
					}
				}
			}
		}
		speedVec.x = 0;
		speedVec.y = 0;
	}
	else{
		//Below portion is for slowing down the character's velocity
		if (speedVec.x > 0){
			speedVec.x -= FRICTION*cos(angle);
			if (speedVec.x <= 0){
				speedVec.x = 0;
			}
		}
		else if (speedVec.x < 0){
			speedVec.x -= FRICTION*cos(angle);
			if (speedVec.x >= 0){
				speedVec.x = 0;
			}
		}

		if (speedVec.y > 0){
			speedVec.y -= FRICTION*sin(angle);
			if (speedVec.y <= 0){
				speedVec.y = 0;
			}
		}
		else if (speedVec.y < 0){
			speedVec.y -= FRICTION*sin(angle);
			if (speedVec.y >= 0){
				speedVec.y = 0;
			}
		}
	}
	if (speedVec.x == 0 && speedVec.y == 0){
		versusMatch.shooting = false;
	}
}

void SystemClass::InitializeTempPosSpeedAngle(float x, float y){
	versusMatch.tempAngle = new float;
	versusMatch.tempPos = new XMFLOAT2;
	versusMatch.tempSpeed = new XMFLOAT2;
	versusMatch.tempPos->x = x;
	versusMatch.tempPos->y = y;
	versusMatch.tempSpeed->x = 0.0f;
	versusMatch.tempSpeed->y = 0.0f;
	*versusMatch.tempAngle = 0.0f;
}

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
