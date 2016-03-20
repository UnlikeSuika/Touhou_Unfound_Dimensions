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

	//for each of the char. sel. buttons
	for (int i = 0; i < MAX_CHAR_SELECT_BUTTONS; i++){
	
		//render character select button sprites
		m_Graphics->RenderBitmap(charSelectButton[i].bitmapID);

		//if the cursor is hovering on one of the char. sel. buttons
		if (Contains(charSelectButton[i].buttonRect, mousePt)){
			
			//highlight the button
			m_Graphics->UpdateBitmap(selectedCharButtonID, charSelectButton[i].buttonRect);
			m_Graphics->RenderBitmap(selectedCharButtonID);

			//if the user is currently selecting a character for player 1
			if (versusMatch.player[0].character == UNSELECTED){
	
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
				if (m_Input->IsKeyJustReleased(VK_LBUTTON) && Contains(charSelectButton[i].buttonRect, lClickPos)){
					if (i == 0){
						versusMatch.player[0].character = REIMU;
						versusMatch.player[0].pCharacterAvatarID = &reimuAvatarID;
					}
					else if (i == 1){
						versusMatch.player[0].character = MARISA;
						versusMatch.player[0].pCharacterAvatarID = &marisaAvatarID;
					}

					//start the timer for flashing the character avatar upon selecting
					m_Clock->AddTimer(versusMatch.player[0].flashTimerID, 15);
				}
			}

			//if the user is currently selecting a character for player 2
			else if (versusMatch.player[1].character == UNSELECTED){
	
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
				if (m_Input->IsKeyJustReleased(VK_LBUTTON) && Contains(charSelectButton[i].buttonRect, lClickPos)){
					if (i == 0){
						versusMatch.player[1].character = REIMU;
						versusMatch.player[1].pCharacterAvatarID = &reimuAvatarID;
					}
					else if (i == 1){
						versusMatch.player[1].character = MARISA;
						versusMatch.player[1].pCharacterAvatarID = &marisaAvatarID;
					}

					//start the timer for flashing the character avatar upon selecting
					m_Clock->AddTimer(versusMatch.player[1].flashTimerID, 15);
				}
			}
		}
	}

	//!!!
	//This program will later be expanded to allow more than two players.
	//Currently, there is only 1 vs 1 match available.
	versusMatch.numPlayers = 2;

	//the body of this "for" loop will render the avatars of selected characters
	for (int i = 0; i < versusMatch.numPlayers; i++){
		if (versusMatch.player[i].character != UNSELECTED){
			XMFLOAT4 flashHue;
			 
			//if the flash timer is still running, the degree to which the avatar shines white
			//depends on how much time is left for the timer to reach zero
			if (m_Clock->IsTimerRunning(versusMatch.player[i].flashTimerID)){
				flashHue = SOLID_WHITE;
				flashHue.w = (float)m_Clock->TimeLeft(versusMatch.player[i].flashTimerID) / 15.0f;
			}
			else{
				flashHue = NULL_COLOR;
			}

			//update avatar positions and white hue, and horizontally flip the 
			//character avatar for player 2
			if (i == 0){
				m_Graphics->DisableBitmapXFlip(*versusMatch.player[i].pCharacterAvatarID);
				m_Graphics->UpdateBitmap(*versusMatch.player[i].pCharacterAvatarID, 200, 250, 0.0f, 1.0f, flashHue);
			}
			else if (i == 1){
				m_Graphics->EnableBitmapXFlip(*versusMatch.player[i].pCharacterAvatarID);
				m_Graphics->UpdateBitmap(*versusMatch.player[i].pCharacterAvatarID, 600, 250, 0.0f, 1.0f, flashHue);
			}

			//render the avatar
			m_Graphics->RenderBitmap(*versusMatch.player[i].pCharacterAvatarID);
			m_Graphics->DisableBitmapXFlip(*versusMatch.player[i].pCharacterAvatarID);
		}
	}

	//if both characters are selected, move onto versus mode
	if (versusMatch.player[0].character != UNSELECTED && versusMatch.player[1].character != UNSELECTED){
		fadingOut = true;
		fadingIn = false;
		nextGameMode = VERSUS_MODE;
	}

	return true;
}

//versus mode
void SystemClass::OnVersusMode(){
	
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
			return;
		}
	}

#ifdef _DEBUG

	//in debug mode, R key resets and reloads the versus mode
	if (m_Input->IsKeyJustPressed(0x52)){ 
		ShutdownVersusMode();
		InitializeVersusMode();
		return;
	}
#endif

	//if timer for versus mode fading into next mode reaches one, change current
	//mode to the next mode
	if (fadingOut && m_Clock->TimeLeft(fadeTimerID) == 1){
		gameMode = nextGameMode;
	}

	//render background map
	m_Graphics->UpdateBitmap(versusMatch.map.mapBitmapID, 400, 300);
	m_Graphics->RenderBitmap(versusMatch.map.mapBitmapID);

	//render character sprites
	for (int i = 0; i < versusMatch.numPlayers; i++){
		float renderBitmapPosX = versusMatch.player[i].position.x;
		float renderBitmapPosY = versusMatch.player[i].position.y;
		m_Graphics->UpdateBitmap(versusMatch.player[i].inGameSpriteID, round(renderBitmapPosX), round(renderBitmapPosY));
		m_Graphics->RenderBitmap(versusMatch.player[i].inGameSpriteID);
	}

	//render bullet sprites
	for (int i = 0; i < versusMatch.numBullets; i++){
		m_Graphics->UpdateBitmap(*versusMatch.bullet[i].pBitmapID, versusMatch.bullet[i].position.x, versusMatch.bullet[i].position.y, versusMatch.bullet[i].moveAngle);
		m_Graphics->RenderBitmap(*versusMatch.bullet[i].pBitmapID);
	}

	//render lasers
	for (int i = 0; i < versusMatch.numLasers; i++){
		LaserType& laser = versusMatch.laser[i];
		int height = laser.dimensions.bottom - laser.dimensions.top;
		int width = laser.dimensions.right - laser.dimensions.left;
		RECT dim;
		dim.top = laser.midPt.y - height / 2;
		dim.bottom = laser.midPt.y + height / 2;
		dim.left = laser.midPt.x - width / 2;
		dim.right = laser.midPt.x + width / 2;

		m_Graphics->UpdateBitmap(*versusMatch.laser[i].pBitmapID, dim);
		m_Graphics->RenderBitmap(*versusMatch.laser[i].pBitmapID);
	}

	//if it is currently Move Phase and player hasn't made a choice
	if (versusMatch.movePhase && versusMatch.choice==PENDING_CHOICE){
	
		//update and render Move choice button
		versusMatch.moveButton.buttonRect = { 0, 550, 150, 600 };
		m_Graphics->UpdateBitmap(versusMatch.moveButton.bitmapID, versusMatch.moveButton.buttonRect);
		m_Graphics->RenderBitmap(versusMatch.moveButton.bitmapID);

		//update and render Pass choice button
		versusMatch.passButton.buttonRect = { 150, 550, 300, 600 };
		m_Graphics->UpdateBitmap(versusMatch.passButton.bitmapID, versusMatch.passButton.buttonRect);
		m_Graphics->RenderBitmap(versusMatch.passButton.bitmapID);

		//if left mouse button is clicked and just released
		if (m_Input->IsKeyJustReleased(VK_LBUTTON)){
		
			//if the user left-clicked the Move choice button
			if (Contains(versusMatch.moveButton.buttonRect,mousePt) && Contains(versusMatch.moveButton.buttonRect, lClickPos)){
				versusMatch.choice = MOVE;
			}

			//if the user left-clicked the Pass choice button
			else if (Contains(versusMatch.passButton.buttonRect,mousePt) && Contains(versusMatch.passButton.buttonRect, lClickPos)){
				versusMatch.choice = PASS;
			}
		}

#ifdef _DEBUG
	
		//in debug mode, M key is shortkey for Move choice and P key is shortkey for Pass choice
		else if (m_Input->IsKeyJustPressed(0x4D)){
			versusMatch.choice = MOVE;
		}
		else if (m_Input->IsKeyJustPressed(0x50)){
			versusMatch.choice = PASS;
		}
#endif
	}

	//if it is currently Act Phase and player hasn't made a choice
	else if (versusMatch.actionPhase && versusMatch.choice == PENDING_CHOICE){
	
		//update and render Shoot choice button
		versusMatch.shootButton.buttonRect = { 0, 550, 150, 600 };
		m_Graphics->UpdateBitmap(versusMatch.shootButton.bitmapID, versusMatch.shootButton.buttonRect);
		m_Graphics->RenderBitmap(versusMatch.shootButton.bitmapID);

		//update and render Spell choice button
		versusMatch.spellButton.buttonRect = { 150, 550, 300, 600 };
		m_Graphics->UpdateBitmap(versusMatch.spellButton.bitmapID, versusMatch.spellButton.buttonRect);
		m_Graphics->RenderBitmap(versusMatch.spellButton.bitmapID);

		//update and render Pass choice button
		versusMatch.passButton.buttonRect = { 300, 550, 450, 600 };
		m_Graphics->UpdateBitmap(versusMatch.passButton.bitmapID, versusMatch.passButton.buttonRect);
		m_Graphics->RenderBitmap(versusMatch.passButton.bitmapID);

		//if left mouse button is clicked and just released
		if (m_Input->IsKeyJustReleased(VK_LBUTTON)){
			
			//if the user left-clicked the Shoot choice button
			if (Contains(versusMatch.shootButton.buttonRect, mousePt) && Contains(versusMatch.shootButton.buttonRect, lClickPos)){
				versusMatch.choice = SHOOT;
			}

			//if the user left-clicked the Pass choice button
			else if (Contains(versusMatch.passButton.buttonRect, mousePt) && Contains(versusMatch.passButton.buttonRect, lClickPos)){
				versusMatch.choice = PASS;
			}

			//if the user left-clicked the Spell choice button
			else if (Contains(versusMatch.spellButton.buttonRect, mousePt) && Contains(versusMatch.spellButton.buttonRect, lClickPos)){
				versusMatch.choice = SPELL;
			}
		}
	}

	//if it is currently Move Phase and player has made a choice
	if (versusMatch.movePhase && versusMatch.choice != PENDING_CHOICE){  
	
		XMFLOAT2& pos = versusMatch.player[versusMatch.playerTurn].position;
		XMFLOAT2& speedVec = versusMatch.player[versusMatch.playerTurn].moveSpeed;
		float& angle = versusMatch.player[versusMatch.playerTurn].moveAngle;  
		
		//Move Phase choice handling
		switch (versusMatch.choice){
		
		//Move choice
		case MOVE:

			//the user will move the character
			versusMatch.shooting = true;

			//if the player hasn't moved the character yet
			if (versusMatch.shooting && speedVec.x == 0 && speedVec.y == 0){
				Shoot(pos, speedVec, angle);
			}

			//if the player has moved the character and character is still moving
			else if (versusMatch.shooting){
				
				//move the character in the initial speed/direction of the mouse
				//until it gradually slows down to zero speed or it collides into a wall
				//after which shooting = false
				Moving(pos, speedVec, angle, versusMatch.player[versusMatch.playerTurn].hitboxRadius);

				//if the character has stopped moving
				if (!versusMatch.shooting){

					//change the current phase to Act Phase
					versusMatch.movePhase = false;
					versusMatch.actionPhase = true;
					
					//announce Act Phase
					m_Clock->SetTimer(versusMatch.phaseAnnounceTimerID, 240);

					//reset the player's choice to PENDING_CHOICE
					versusMatch.choice = PENDING_CHOICE;
				}
			}
			break;

		//Pass choice
		case PASS:
			
			//change the current phase to Act Phase
			versusMatch.movePhase = false;
			versusMatch.actionPhase = true;

			//announce Act Phase
			m_Clock->SetTimer(versusMatch.phaseAnnounceTimerID, 240);

			//reset the player's choice to PENDING_CHOICE
			versusMatch.choice = PENDING_CHOICE;

			break;
		}
	}

	//if it is currently Act Phase and player has made a choice
	else if (versusMatch.actionPhase && versusMatch.choice != PENDING_CHOICE){

		//Act Phase choice handling
		switch (versusMatch.choice){

			//Shoot choice
		case SHOOT:

			//different shooting properties for each character
			switch (versusMatch.player[versusMatch.playerTurn].character){

				//Reimu will shoot a single 2-damage amulet
			case REIMU:

				//if temporary position, velocity, and angle variables are not yet initialized
				if (!versusMatch.shooting){

					//initialize the temporary variables
					InitializeTempPosSpeedAngle(versusMatch.player[versusMatch.playerTurn].position.x,
						versusMatch.player[versusMatch.playerTurn].position.y);

					//the player will shoot the bullet
					versusMatch.shooting = true;
				}

				//if the player is shooting the bullet or the bullet is in motion
				if (versusMatch.shooting){

					//temporary variables are used so that the bullet can be created and rendered
					//after position, speed, and angle are recorded 
					if (versusMatch.tempPos || versusMatch.tempSpeed || versusMatch.tempAngle){

						//if the speed, position, and angle have not yet been recorded
						if (versusMatch.tempSpeed->x == 0.0f && versusMatch.tempSpeed->y == 0.0f){
							Shoot(*versusMatch.tempPos, *versusMatch.tempSpeed, *versusMatch.tempAngle);
						}

						//if the speed, position, and angle have been recorded
						else{

							//bullet gives 2 damage upon contact with the opponent character
							versusMatch.bullet[versusMatch.numBullets].damage = 2;

							//set the bullet sprite
							versusMatch.bullet[versusMatch.numBullets].pBitmapID = &versusMatch.type01color01bulletID;

							//the new bullet will have the position as recorded in temporary variable
							if (versusMatch.tempPos){
								versusMatch.bullet[versusMatch.numBullets].position = *versusMatch.tempPos;
								delete versusMatch.tempPos;
								versusMatch.tempPos = 0;
							}

							//the new bullet will have the velocity as recorded in temporary variable
							if (versusMatch.tempSpeed){
								versusMatch.bullet[versusMatch.numBullets].moveSpeed = *versusMatch.tempSpeed;
								delete versusMatch.tempSpeed;
								versusMatch.tempSpeed = 0;
							}

							//the new bullet will have the angle as recorded in temporary variable
							if (versusMatch.tempAngle){
								versusMatch.bullet[versusMatch.numBullets].moveAngle = *versusMatch.tempAngle;
								delete versusMatch.tempAngle;
								versusMatch.tempAngle = 0;
							}

							//increase the bullet count by one
							versusMatch.numBullets++;
						}
					}

					//if the position, velocity, and angle has been transferred from temporary
					//variables to the new bullet
					if (!versusMatch.tempPos && !versusMatch.tempSpeed && !versusMatch.tempAngle){

						XMFLOAT2& bulletPos = versusMatch.bullet[versusMatch.numBullets - 1].position;
						XMFLOAT2& bulletSpeed = versusMatch.bullet[versusMatch.numBullets - 1].moveSpeed;
						float& bulletAngle = versusMatch.bullet[versusMatch.numBullets - 1].moveAngle;

						//the bullet is being shot
						Moving(bulletPos, bulletSpeed, bulletAngle, 6.0f);

						//if the bullet stopped moving
						if (!versusMatch.shooting){

							int collidedChar;

							//detect whether bullet has collided with opponent character
							if (CollisionWithCharacter(bulletPos, 6.0f, collidedChar)){
								versusMatch.player[collidedChar].hp -= versusMatch.bullet[versusMatch.numBullets - 1].damage;
							}

							//bullet is erased
							versusMatch.numBullets--;


							//move on to next player's turn
							versusMatch.playerTurn++;
							if (versusMatch.playerTurn >= versusMatch.numPlayers){
								versusMatch.playerTurn = 0;
							}

							//move on to Move Phase
							versusMatch.movePhase = true;
							versusMatch.actionPhase = false;

							//announce Move Phase
							m_Clock->SetTimer(versusMatch.phaseAnnounceTimerID, 240);

							//reset the player's choice to PENDING_CHOICE
							versusMatch.choice = PENDING_CHOICE;
						}
					}
				}

				break;

				//Marisa will shoot a 3-damage laser
			case MARISA:

				//if temporary position, velocity and angle variables are not yet initialized
				if (!versusMatch.shooting){

					//Initialize the temporary variables
					InitializeTempPosSpeedAngle(versusMatch.player[versusMatch.playerTurn].position.x,
						versusMatch.player[versusMatch.playerTurn].position.y);

					//the player will shoot the laser
					versusMatch.shooting = true;
				}

				//if the player is shooting the laser or the laser is still activated
				if (versusMatch.shooting){

					int collidedChar = -1;

					//temporary variables are used so that the laser can be created and rendered
					//after the position and angle are recorded
					if (versusMatch.tempPos || versusMatch.tempSpeed || versusMatch.tempAngle){

						//if the speed, position, and angle have not yet been recorded
						if (versusMatch.tempSpeed->x == 0 && versusMatch.tempSpeed->y == 0){
							Shoot(*versusMatch.tempPos, *versusMatch.tempSpeed, *versusMatch.tempAngle);
						}

						//if the speed, position, and angle have been recorded
						else{


							//TODO
							//laser

							LaserType& laser = versusMatch.laser[versusMatch.numLasers];

							//laser gives 3 damage upon contact with the opponent character
							laser.damage = 3;

							//setting laser particles to laserColor01 particles
							laser.pBitmapID = &versusMatch.color01LaserID;

							XMFLOAT2 posCtrI = *versusMatch.tempPos;
							float slope = versusMatch.tempSpeed->y / versusMatch.tempSpeed->x;
							float yInt = versusMatch.tempPos->y - slope*versusMatch.tempPos->x;


							XMFLOAT2& posI = laser.posI;
							XMFLOAT2& posF = laser.posF;
							XMFLOAT2& midPt = laser.midPt;
							RECT& dim = laser.dimensions;
							posI = posCtrI;
							posF = posI;

							while (!(CollisionWithCharacter(posF, 1.0f, collidedChar)) && !(CollisionWithWall(posF, 1.0f))){
								posF.x += 1.0f;
								posF.y += slope;
							}
							midPt.x = (posI.x + posF.x) / 2.0f;
							midPt.y = (posI.y + posF.y) / 2.0f;
							dim.left = 0;
							dim.top = 0;
							dim.bottom = round(versusMatch.player[versusMatch.playerTurn].hitboxRadius);
							dim.right = round(Distance(posI, posF));


							delete versusMatch.tempPos;
							versusMatch.tempPos = 0;

							//temporary speed variable is deleted
							delete versusMatch.tempSpeed;
							versusMatch.tempSpeed = 0;

							//the laser will have the angle as recorded in temporary variable
							laser.angle = *versusMatch.tempAngle;
							delete versusMatch.tempAngle;
							versusMatch.tempAngle = 0;

							m_Clock->AddTimer(versusMatch.laser[versusMatch.numLasers].timerID, 180);

							//increase the laser count by one
							versusMatch.numLasers++;
						}
					}
					LaserType& laser = versusMatch.laser[versusMatch.numLasers - 1];
					if (!versusMatch.tempPos && !versusMatch.tempSpeed && !versusMatch.tempAngle){
						if (collidedChar != -1){
							versusMatch.player[collidedChar].hp -= laser.damage;
							collidedChar = -1;
						}

						if (!m_Clock->IsTimerRunning(laser.timerID)){
							m_Clock->DeleteTimer(laser.timerID);
							versusMatch.numLasers--;
							versusMatch.playerTurn++;
							if (versusMatch.playerTurn >= versusMatch.numPlayers){
								versusMatch.playerTurn = 0;
							}
							versusMatch.movePhase = true;
							versusMatch.actionPhase = false;
							m_Clock->SetTimer(versusMatch.phaseAnnounceTimerID, 240);
							versusMatch.choice = PENDING_CHOICE;
						}
					}
				}
				break;
			}
			break;

			//Pass choice
		case PASS:

			//move on to Move Phase
			versusMatch.movePhase = true;
			versusMatch.actionPhase = false;

			//switch player turn
			versusMatch.playerTurn++;
			if (versusMatch.playerTurn >= versusMatch.numPlayers){
				versusMatch.playerTurn = 0;
			}

			//announce Move Phase
			m_Clock->SetTimer(versusMatch.phaseAnnounceTimerID, 240);

			//reset the player choice to PENDING_CHOICE
			versusMatch.choice = PENDING_CHOICE;
			break;

		//Spell choice
		case SPELL:
			break;

		//error handling: anything else just skips the turn
		default:

			//change from Act Phase to Move Phase
			versusMatch.movePhase = true;
			versusMatch.actionPhase = false;

			//switch player turn
			versusMatch.playerTurn++;
			if (versusMatch.playerTurn >= versusMatch.numPlayers){
				versusMatch.playerTurn = 0;
			}

			//announce Move Phase
			m_Clock->SetTimer(versusMatch.phaseAnnounceTimerID, 240);

			//reset the player choice to PENDING_CHOICE
			versusMatch.choice = PENDING_CHOICE;
		}
	}
	
	//display the character's status window if player hasn't made a choice yet
	if (versusMatch.choice == PENDING_CHOICE){
		for (int i = 0; i < versusMatch.numPlayers; i++){
			
			//set rectangle containing character sprite
			RECT rc;
			rc.left = (long)round(versusMatch.player[i].position.x - versusMatch.player[i].inGameSpriteWidth / 2.0f);
			rc.right = (long)round(versusMatch.player[i].position.x + versusMatch.player[i].inGameSpriteWidth / 2.0f);
			rc.top = (long)round(versusMatch.player[i].position.y - versusMatch.player[i].inGameSpriteHeight / 2.0f);
			rc.bottom = (long)round(versusMatch.player[i].position.y + versusMatch.player[i].inGameSpriteHeight / 2.0f);

			//if the mouse cursor is hovering over the rectangle
			if (Contains(rc, mousePt)){

				//update and render status window over the mouse position
				m_Graphics->UpdateBitmap(versusMatch.statsWindowBitmapID, mousePt.x + 35, mousePt.y - 40);
				m_Graphics->RenderBitmap(versusMatch.statsWindowBitmapID);
				
				//assemble string that displays current HP
				string hpStr = "HP: ";
				hpStr = hpStr + to_string(versusMatch.player[i].hp) + " / " + to_string(versusMatch.player[i].maxHp);
				char hpDisp[MAX_CHARACTER_COUNT];
				strcpy(hpDisp, hpStr.c_str());

				//update and render current HP
				m_Graphics->UpdateSentence(versusMatch.hpDispSentID, hpDisp, mousePt.x, mousePt.y - 65, SOLID_BLACK);
				m_Graphics->RenderSentence(versusMatch.hpDispSentID);

				//assemble string that displays current MP
				string mpStr = "MP: " + to_string(versusMatch.player[i].mp);
				char mpDisp[MAX_CHARACTER_COUNT];
				strcpy(mpDisp, mpStr.c_str());

				//update and render current MP
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

		//more than 3.5 seconds left
		if (timeLeft>=210){
			
			//the announcement bitmap fades in
			m_Graphics->UpdateBitmap(*versusMatch.pPhaseAnnounceBitmapID, posX, posY, 0.0f, 1.0f - ((float)timeLeft - 210.0f) / 30.0f, NULL_COLOR);
		}

		//3.5 to 1 seconds left
		else if (timeLeft < 210 && timeLeft >= 60){

			//the announcement bitmap stays
			m_Graphics->UpdateBitmap(*versusMatch.pPhaseAnnounceBitmapID, posX, posY);
		}

		//less than 1 second left
		else if (timeLeft < 60){

			//the announcement bitmap fades out
			m_Graphics->UpdateBitmap(*versusMatch.pPhaseAnnounceBitmapID, posX, posY, 0.0f, (float)timeLeft / 60.0f);
		}

		//render the announcement bitmap
		m_Graphics->RenderBitmap(*versusMatch.pPhaseAnnounceBitmapID);
	}
}

//initialize variables for character select mode
bool SystemClass::InitializeCharSelect(){
	bool result;
	RECT rc;

	//currently, the number of players is 2
	versusMatch.numPlayers = 2;

	//set the dimensions for character select buttons
	for (int i = 0; i < MAX_CHAR_SELECT_BUTTONS; i++){
		charSelectButton[i].buttonRect.top = 500;
		charSelectButton[i].buttonRect.bottom = 600;
		charSelectButton[i].buttonRect.left = 100 * i;
		charSelectButton[i].buttonRect.right = 100 * i + 100;
	}

	//add bitmap for Reimu's character select button
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_reimu.tga", charSelectButton[0].buttonRect,
		m_screenWidth, m_screenHeight, charSelectButton[0].bitmapID);
	if (!result){
		return false;
	}

	//add bitmap for Marisa's character select button
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_marisa.tga", charSelectButton[1].buttonRect,
		m_screenWidth, m_screenHeight, charSelectButton[1].bitmapID);
	if (!result){
		return false;
	}

	//add bitmaps for rest of the char. sel. buttons (unselectable)
	for (int i = 2; i < MAX_CHAR_SELECT_BUTTONS; i++){
		result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_locked.tga", charSelectButton[i].buttonRect,
			m_screenWidth, m_screenHeight, charSelectButton[i].bitmapID);
		if (!result){
			return false;
		}
	}

	//this bitmap will be rendered over the char. sel. button
	//over which the mouse cursor is hovering. this bitmap is used to
	//highlight such select button.
	rc = { 0, 100, 0, 100 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_selected.tga", rc, m_screenWidth,
		m_screenHeight, selectedCharButtonID);
	if (!result){
		return false;
	}

	//background bitmap for char. sel. mode
	rc = { 0, 0, m_screenWidth, m_screenHeight };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_background.tga", rc, m_screenWidth,
		m_screenHeight, charSelectModeBackgroundID);
	if (!result){
		return false;
	}

	rc = { 0, 0, 400, 500 };

	//avatar bitmap for Reimu
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_reimu_avatar.tga", rc, m_screenWidth,
		m_screenHeight, reimuAvatarID);
	if (!result){
		return false;
	}

	//avatar bitmap for Marisa
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/character_select_screen_marisa_avatar.tga", rc, m_screenWidth,
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
	bool result;

	//reset player turn
	versusMatch.playerTurn = 0;

	//reset current phase
	versusMatch.movePhase = true;
	versusMatch.actionPhase = false;

	//reset shooting state and shoot frame
	versusMatch.shooting = false;
	versusMatch.shootFrame = -1;

	//reset player won
	versusMatch.victoryPlayer = -1;

	//reset player choice to PENDING_CHOICE
	versusMatch.choice = PENDING_CHOICE;

	//reset number of bullets to zero
	versusMatch.numBullets = 0;

	//set temporary heap variables to NULL
	versusMatch.tempAngle = 0;
	versusMatch.tempPos = 0;
	versusMatch.tempSpeed = 0;

	//add timer for announcement of current phase
	m_Clock->AddTimer(versusMatch.phaseAnnounceTimerID, 240);

	//add bitmap of the background map
	RECT screenRect = { 0, 0, m_screenWidth, m_screenHeight };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/test_map.tga", screenRect, m_screenWidth, m_screenHeight, versusMatch.map.mapBitmapID);
	if (!result){
		return false;
	}

	//add bitmaps for the phase announcement
	RECT announceRect = { 0, 0, 398, 37 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_move_phase_announce.tga", announceRect,
		m_screenWidth, m_screenHeight, versusMatch.movePhaseAnnounceBitmapID);
	if (!result){
		return false;
	}
	announceRect = { 0, 0, 338, 37 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_act_phase_announce.tga", announceRect,
		m_screenWidth, m_screenHeight, versusMatch.actPhaseAnnounceBitmapID);
	if (!result){
		return false;
	}

	//add bitmaps for choice buttons
	versusMatch.passButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_pass_button.tga", versusMatch.passButton.buttonRect,
		m_screenWidth, m_screenHeight, versusMatch.passButton.bitmapID);
	if (!result){
		return false;
	}
	versusMatch.moveButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_move_button.tga", versusMatch.moveButton.buttonRect,
		m_screenWidth, m_screenHeight, versusMatch.moveButton.bitmapID);
	if (!result){
		return false;
	}
	versusMatch.shootButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_shoot_button.tga", versusMatch.shootButton.buttonRect,
		m_screenWidth, m_screenHeight, versusMatch.shootButton.bitmapID);
	if (!result){
		return false;
	}
	versusMatch.spellButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_spell_button.tga", versusMatch.spellButton.buttonRect,
		m_screenWidth, m_screenHeight, versusMatch.spellButton.bitmapID);
	if (!result){
		return false;
	}

	//add bitmap of aiming circle
	RECT aimCircleRect = { 0, 0, 100, 100 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_aim.tga", aimCircleRect, m_screenWidth, m_screenHeight,
		versusMatch.aimCircleBitmapID);
	if (!result){
		return false;
	}

	//add bitmaps for all types of bullets
	RECT bulletRect = { 0, 0, 14, 12 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/bullet/type_01_color_01.tga", bulletRect, m_screenWidth,
		m_screenHeight, versusMatch.type01color01bulletID);
	if (!result){
		return false;
	}

	//add bitmap for the character status window
	RECT statsRect = { 0, 0, 100, 80 };
	result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_stats_window.tga", statsRect, m_screenWidth,
		m_screenHeight, versusMatch.statsWindowBitmapID);
	if (!result){
		return false;
	}

	//reset number of lasers to zero
	versusMatch.numLasers = 0;

	//add bitmap for p
	RECT particleRect = { 0, 0, 1, 1 };
	char path[MAX_CHARACTER_COUNT];
	string pathStr = "/Data/bullet/laser_color_01_particle_03.tga";
	strcpy(path, pathStr.c_str());
	result = m_Graphics->AddBitmap(m_hwnd, path, particleRect, m_screenWidth, m_screenHeight, versusMatch.color01LaserID);
	if (!result){
		return false;
	}

	//add sentence object for showing current HP
	result = m_Graphics->AddSentence(m_hwnd, " ", 0, 0, m_screenWidth, m_screenHeight, SOLID_BLACK, versusMatch.hpDispSentID);
	if (!result){
		return false;
	}

	//add sentence object for showing current MP
	result = m_Graphics->AddSentence(m_hwnd, " ", 0, 0, m_screenWidth, m_screenHeight, SOLID_BLACK, versusMatch.mpDispSentID);
	if (!result){
		return false;
	}

	//set the map's wall collision detection
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
		 
		//set spell card lists to NULL
		versusMatch.player[i].spellCard = 0;

		RECT spriteRect;
		
		switch (versusMatch.player[i].character){
		
		//initialize the character stats corresponding to Reimu
		case REIMU:
			versusMatch.player[i].maxHp = 40;
			versusMatch.player[i].numSpellCards = 2;
			versusMatch.player[i].inGameSpriteWidth = 20;
			versusMatch.player[i].inGameSpriteHeight = 20;
			versusMatch.player[i].hitboxRadius = 10.0f;
			spriteRect = { 0, 0, versusMatch.player[i].inGameSpriteWidth, versusMatch.player[i].inGameSpriteHeight };
			result = m_Graphics->AddBitmap(m_hwnd, "/Data/in_game_reimu.tga", spriteRect, m_screenWidth, m_screenHeight,
				versusMatch.player[i].inGameSpriteID);
			if (!result){
				return false;
			}
			versusMatch.player[i].spellCard = new SpellCardType[versusMatch.player[i].numSpellCards];
			strcpy(versusMatch.player[i].spellCard[0].cardName, "Spirit Sign \"Fantasy Seal\"");
			versusMatch.player[i].spellCard[0].mpCost = 6;
			strcpy(versusMatch.player[i].spellCard[1].cardName, "Dream Sign \"Evil-Sealing Circle\"");
			versusMatch.player[i].spellCard[1].mpCost = 8;

			break;

			//initialize the character stats corresponding to Marisa
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

		//set current HP to max HP
		versusMatch.player[i].hp = versusMatch.player[i].maxHp;

		//set current MP to zero
		versusMatch.player[i].mp = 0;

		//characters are stationary
		versusMatch.player[i].moveSpeed = { 0.0f, 0.0f };

		//first player starts at the left side of the map
		if (i == 0){
			versusMatch.player[i].position = { 100.0f, 300.0f };
		}

		//second player starts at the right side of the map
		else if (i == 1){
			versusMatch.player[i].position = { 700.0f, 300.0f };
		}
	}

	//versus mode is initialized
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
	m_Graphics->DeleteBitmap(versusMatch.color01LaserID);

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

//processes a currently moving object
void SystemClass::Moving(XMFLOAT2& pos, XMFLOAT2& speedVec, float& angle, float radius){
	
	//update x position
	pos.x += speedVec.x; 

	//update y position
	pos.y += speedVec.y;

	//a null integer variable for receiving information for collision with character
	int nullInt = 0;

	//if the object collided with a wall or a character
	if (CollisionWithWall(pos, radius) || CollisionWithCharacter(pos, radius, nullInt)){
		
		//slope for calculation a linear line (y = mx + b)
		//that represents the object's movement
		float slope;

		//object's position in the previous frame
		XMFLOAT2 posI = { pos.x - speedVec.x, pos.y - speedVec.y };

		//if the object is moving in a vertical line
		if (speedVec.x == 0){

			//if the object is heading downwards
			if (speedVec.y > 0){

				//test whether the object collides 0.1 pixel, 0.2 pixel, etc
				//below the initial position
				for (float y = 0.0f; posI.y + y <= pos.y; y += 0.1f){
					XMFLOAT2 testPos = { posI.x, posI.y + y };

					//if the object collides at testPos, set the object's position to testPos
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(pos, radius, nullInt)){
						pos = testPos;
						break;
					}
				}
			}
			//if the object is heading upwards
			else if (speedVec.y < 0){

				//test whether the object collides 0.1 pixel, 0.2 pixel, etc
				//above the initial position
				for (float y = 0.0f; posI.y + y >= pos.y; y -= 0.1f){
					XMFLOAT2 testPos = { posI.x, posI.y + y };

					//if the object collides at testPos, set the object's position to testPos
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(testPos, radius, nullInt)){
						pos = testPos;
						break;
					}
				}
			}
		}

		//if the object is not moving in a vertical line
		else{

			//slope (m) in y = mx + b
			slope = speedVec.y / speedVec.x;

			//y-intercept (b) in y = mx + b
			float b = pos.y - slope*pos.x;

			//if the object is moving to the right
			if (speedVec.x > 0){
				
				//test where the object collides by incrementing the x-position 
				//by 0.1 pixels and updating y-position to mx + b
				for (float x = 0.0f; posI.x + x <= pos.x; x += 0.1f){
					XMFLOAT2 testPos = { posI.x + x, slope*(posI.x + x) + b };
					
					//if the object collides at testPos, set the object's position to testPos
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(testPos,radius,nullInt)){
						pos = testPos;
						break;
					}
				}
			}

			//if the object is moving to the left
			else if (speedVec.x < 0){
				
				//test where the object collides by decrementing the x-position 
				//by 0.1 pixels and updating y-position to mx + b
				for (float x = 0.0f; posI.x + x >= pos.x; x -= 0.1f){
					XMFLOAT2 testPos = { posI.x + x, slope*(posI.x + x) + b };
					
					//if the object collides at testPos, set the object's position to testPos
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(testPos,radius,nullInt)){
						pos = testPos;
						break;
					}
				}
			}
		}

		//the object has stopped moving
		speedVec.x = 0;
		speedVec.y = 0;
	}

	//if the object is still moving without colliding with wall or character
	else{

		//if the object is moving to the right
		if (speedVec.x > 0){

			//slow down x-speed
			speedVec.x -= FRICTION*cos(angle);
			if (speedVec.x <= 0){
				speedVec.x = 0;
			}
		}

		//if the object is moving to the left
		else if (speedVec.x < 0){

			//slow down x-speed
			speedVec.x -= FRICTION*cos(angle);
			if (speedVec.x >= 0){
				speedVec.x = 0;
			}
		}

		//if the object is moving down
		if (speedVec.y > 0){

			//slow down y-speed
			speedVec.y -= FRICTION*sin(angle);
			if (speedVec.y <= 0){
				speedVec.y = 0;
			}
		}

		//if the object is moving up
		else if (speedVec.y < 0){
			
			//slow down y-speed
			speedVec.y -= FRICTION*sin(angle);
			if (speedVec.y >= 0){
				speedVec.y = 0;
			}
		}
	}

	//if the object is no longer moving
	if (speedVec.x == 0 && speedVec.y == 0){
		
		//the object is no longer moving
		versusMatch.shooting = false;
	}
}

//initializes the temporary variables for recording
//position, speed, and angle
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
