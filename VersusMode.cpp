#include "VersusMode.h"

//returns the distance between (x1, y1) and (x2, y2)
float Distance(float x1, float y1, float x2, float y2) {
	return (float)sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
}

//returns the distance between p1 and p2
float Distance(POINT p1, POINT p2) {
	return (float)sqrt(pow(p1.y - p2.y, 2) + pow(p1.x - p2.x, 2));
}

//returns the distance between p1 and p2
float Distance(XMFLOAT2 p1, POINT p2) {
	return (float)sqrt(pow(p1.y - (float)p2.y, 2) + pow(p1.x - (float)p2.x, 2));
}

//returns the distance between p1 and p2
float Distance(POINT p1, XMFLOAT2 p2) {
	return (float)sqrt(pow((float)p1.y - p2.y, 2) + pow((float)p1.x - p2.x, 2));
}

//returns the distance between p1 and p2
float Distance(XMFLOAT2 p1, XMFLOAT2 p2) {
	return (float)sqrt(pow(p1.y - p2.y, 2) + pow(p1.x - p2.x, 2));
}

//returns true if screen coordinates of the given point is contained within the given rectangle
bool RectContains(RECT rect, POINT pt) {
	if (pt.x > rect.left&&pt.x < rect.right&&pt.y > rect.top&&pt.y < rect.bottom) {
		return true;
	}
	else {
		return false;
	}
}

//returns true if a button is left-clicked
bool ButtonLeftClicked(RECT rect, InputClass* input) {
	if (input) {
		return (input->IsKeyJustReleased(VK_LBUTTON) && RectContains(rect, input->GetMouseLeftClickLocation()) && RectContains(rect, input->GetMouseLocation()));
	}
	else {
		return false;
	}
}

VersusMode::VersusMode() {
	m_Graphics = NULL;
	m_Clock = NULL;
	m_screenWidth = 0;
	m_screenHeight = 0;
}

VersusMode::VersusMode(GraphicsClass* graphics, TimeClass* clock, int screenWidth, int screenHeight) {
	m_Graphics = graphics;
	m_Clock = clock;
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}

VersusMode::~VersusMode() {}

bool VersusMode::Initialize() {
	bool result;

	//reset player turn
	playerTurn = 0;

	//reset current phase
	movePhase = true;
	actionPhase = false;

	//reset shooting state and shoot frame
	shooting = false;
	shootFrame = -1;

	//reset player won
	victoryPlayer = -1;

	//reset player choice to PENDING_CHOICE
	choice = PENDING_CHOICE;

	//reset number of bullets to zero
	numBullets = 0;

	//set temporary variables to NULL
	tempAngle = NULL;
	tempPos = NULL;
	tempSpeed = NULL;
	tempBullet = NULL;
	tempBulletNum = NULL;
	tempTimerID = NULL;
	tempTimerIDNum = NULL;

	//add timer for announcement of current phase
	m_Clock->AddTimer(phaseAnnounceTimerID, 240);

	//add bitmap of the background map
	RECT screenRect = { 0, 0, m_screenWidth, m_screenHeight };
	result = m_Graphics->AddBitmap("/Data/test_map.tga", screenRect, m_screenWidth, m_screenHeight, map.mapBitmapID);
	if (!result) {
		return false;
	}

	//add bitmaps for the phase announcement
	RECT announceRect = { 0, 0, 398, 37 };
	result = m_Graphics->AddBitmap("/Data/in_game_move_phase_announce.tga", announceRect,
		m_screenWidth, m_screenHeight, movePhaseAnnounceBitmapID);
	if (!result) {
		return false;
	}
	announceRect = { 0, 0, 338, 37 };
	result = m_Graphics->AddBitmap("/Data/in_game_act_phase_announce.tga", announceRect,
		m_screenWidth, m_screenHeight, actPhaseAnnounceBitmapID);
	if (!result) {
		return false;
	}

	//add bitmap for Pass choice button
	passButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap("/Data/in_game_pass_button.tga", passButton.buttonRect,
		m_screenWidth, m_screenHeight, passButton.bitmapID);
	if (!result) {
		return false;
	}

	//add bitmap for Move choice button
	moveButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap("/Data/in_game_move_button.tga", moveButton.buttonRect,
		m_screenWidth, m_screenHeight, moveButton.bitmapID);
	if (!result) {
		return false;
	}

	//add bitmap for Shoot choice button
	shootButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap("/Data/in_game_shoot_button.tga", shootButton.buttonRect,
		m_screenWidth, m_screenHeight, shootButton.bitmapID);
	if (!result) {
		return false;
	}

	//add bitmap for Spell choice button
	spellButton.buttonRect = { 0, 0, 150, 50 };
	result = m_Graphics->AddBitmap("/Data/in_game_spell_button.tga", spellButton.buttonRect,
		m_screenWidth, m_screenHeight, spellButton.bitmapID);
	if (!result) {
		return false;
	}

	//add bitmap of aiming circle
	RECT aimCircleRect = { 0, 0, 100, 100 };
	result = m_Graphics->AddBitmap("/Data/in_game_aim.tga", aimCircleRect, m_screenWidth, m_screenHeight,
		aimCircleBitmapID);
	if (!result) {
		return false;
	}

	//add bitmap of player indicator
	RECT playerIndicatorRect = { 0, 0, 63, 60 };
	result = m_Graphics->AddBitmap("/Data/in_game_player_indicator.tga", playerIndicatorRect, m_screenWidth, m_screenHeight, playerIndicatorBitmapID);
	if (!result) {
		return false;
	}

	result = m_Clock->AddClock(playerIndicatorClockID);
	if (!result) {
		return false;
	}

	//add bitmaps for all types of bullets
	RECT bulletRect = { 0, 0, 14, 12 };
	result = m_Graphics->AddBitmap("/Data/bullet/type_01_color_01.tga", bulletRect, m_screenWidth,
		m_screenHeight, type01color01bulletID);
	if (!result) {
		return false;
	}

	//add bitmap for the character status window
	RECT statsRect = { 0, 0, 100, 80 };
	result = m_Graphics->AddBitmap("/Data/in_game_stats_window.tga", statsRect, m_screenWidth,
		m_screenHeight, statsWindowBitmapID);
	if (!result) {
		return false;
	}

	//add bitmaps for Reimu's in-game sprites
	for (int i = 0; i < 4; i++) {
		RECT charRect = { 0, 0, 40, 72 };
		char path[MAX_CHARACTER_COUNT];
		std::string pathStr = "/Data/in_game_reimu_stationary_0" + std::to_string(i + 1) + ".tga";
		strcpy(path, pathStr.c_str());
		result = m_Graphics->AddBitmap(path, charRect, m_screenWidth, m_screenHeight, reimuStationaryBitmapID[i]);
		if (!result) {
			return false;
		}
	}

	//add bitmap for Marisa's in-game sprite
	for (int i = 0; i < 4; i++) {
		RECT charRect = { 0, 0, 54, 63 };
		char path[MAX_CHARACTER_COUNT];
		std::string pathStr = "/Data/in_game_marisa_stationary_0" + std::to_string(i + 1) + ".tga";
		strcpy(path, pathStr.c_str());
		result = m_Graphics->AddBitmap(path, charRect, m_screenWidth, m_screenHeight, marisaStationaryBitmapID[i]);
		if (!result) {
			return false;
		}
	}

	//reset number of lasers to zero
	numLasers = 0;

	//add bitmap for laser particles
	RECT particleRect = { 0, 0, 1, 1 };
	for (int i = 0; i < 18; i++) {
		char path[MAX_CHARACTER_COUNT];
		std::string pathStr;
		if (i < 9) {
			pathStr = "/Data/bullet/laser_color_01_particle_0" + std::to_string(i + 1) + ".tga";
		}
		else {
			pathStr = "/Data/bullet/laser_color_01_particle_" + std::to_string(i + 1) + ".tga";
		}
		strcpy(path, pathStr.c_str());
		result = m_Graphics->AddBitmap(path, particleRect, m_screenWidth, m_screenHeight, color01laserParticleID[i]);
		if (!result) {
			return false;
		}
	}

	//initialize button for spell titles
	for (int i = 0; i < 5; i++) {
		spellNameButton[i].buttonRect = { 0, 0, 500, 30 };
		result = m_Graphics->AddBitmap("/Data/spell_name_box.tga", spellNameButton[i].buttonRect, m_screenWidth, m_screenHeight, spellNameButton[i].bitmapID);
		if (!result) {
			return false;
		}
	}

	//add bitmap for spell description box
	RECT spellDescRect = { 0, 0, 280, 161 };
	result = m_Graphics->AddBitmap("/Data/spell_desc_box.tga", spellDescRect, m_screenWidth, m_screenHeight, spellDescBitmapID);
	if (!result) {
		return false;
	}

	//add bitmaps for Reimu's "Fantasy Seal"
	for (int i = 0; i < 3; i++) {
		char path[MAX_CHARACTER_COUNT];
		RECT bitmapRect;

		std::string pathStr = "/Data/spell/reimu/spell_01_color_0" + std::to_string(i + 1) + "_bullet.tga";
		strcpy(path, pathStr.c_str());
		bitmapRect = { 0, 0, 256, 256 };
		result = m_Graphics->AddBitmap(path, bitmapRect, m_screenWidth, m_screenHeight, reimuSpell01Bullet[i]);
		if (!result) {
			return false;
		}

		pathStr = "/Data/spell/reimu/spell_01_color_0" + std::to_string(i + 1) + "_bulletbg.tga";
		strcpy(path, pathStr.c_str());
		bitmapRect = { 0, 0, 128, 128 };
		result = m_Graphics->AddBitmap(path, bitmapRect, m_screenWidth, m_screenHeight, reimuSpell01BulletBg[i]);
		if (!result) {
			return false;
		}
	}

	//add bitmaps for physical impact 1
	for (int i = 0; i < 19; i++) {
		char path[MAX_CHARACTER_COUNT];
		std::string pathStr;
		RECT bitmapRect = { 0, 0, 288, 288 };

		if (i + 1 < 10) {
			pathStr = "/Data/general/hit_01_0" + std::to_string(i + 1) + ".tga";
		}
		else {
			pathStr = "/Data/general/hit_01_" + std::to_string(i + 1) + ".tga";
		}
		strcpy(path, pathStr.c_str());

		result = m_Graphics->AddBitmap(path, bitmapRect, m_screenWidth, m_screenHeight, hit01BitmapID[i]);
		if (!result) {
			return false;
		}
	}

	//add sentence object for showing spell card list
	result = m_Graphics->AddSentence(" ", 0, 0, m_screenWidth, m_screenHeight, SOLID_BLACK, spellNameSentID);
	if (!result) {
		return false;
	}

	//add sentence object for showing spell card description
	result = m_Graphics->AddSentence(" ", 0, 0, m_screenWidth, m_screenHeight, SOLID_BLACK, spellDescSentID);
	if (!result) {
		return false;
	}

	//add sentence object for showing current HP
	result = m_Graphics->AddSentence(" ", 0, 0, m_screenWidth, m_screenHeight, SOLID_BLACK, hpDispSentID);
	if (!result) {
		return false;
	}

	//add sentence object for showing current MP
	result = m_Graphics->AddSentence(" ", 0, 0, m_screenWidth, m_screenHeight, SOLID_BLACK, mpDispSentID);
	if (!result) {
		return false;
	}

	//spell card is currently not selected
	isSpellSelected = false;

	//set the map's wall collision detection
	map.numRectWall = 4;
	map.rectWall = new RECT[4];
	map.rectWall[0] = { 0, 0, 800, 30 };
	map.rectWall[1] = { 0, 570, 800, 600 };
	map.rectWall[2] = { 0, 30, 30, 570 };
	map.rectWall[3] = { 770, 30, 800, 570 };

	numPlayers = 2;
	for (int i = 0; i < numPlayers; i++) {

		//set spell card lists to NULL
		player[i].spellCard = 0;

		//add clock for character sprite motion
		m_Clock->AddClock(player[i].spriteClockID);

		RECT spriteRect;

		switch (player[i].character) {

			//initialize the character stats corresponding to Reimu
		case REIMU:
			player[i].maxHp = 40;
			player[i].numSpellCards = 2;
			player[i].inGameSpriteWidth = 40;
			player[i].inGameSpriteHeight = 72;
			player[i].hitboxRadius = 10.0f;
			spriteRect = { 0, 0, player[i].inGameSpriteWidth, player[i].inGameSpriteHeight };
			player[i].spellCard = new SpellCardType[player[i].numSpellCards];
			strcpy(player[i].spellCard[0].cardName, "Spirit Sign \"Fantasy Seal\"");
			player[i].spellCard[0].mpCost = 6;
			strcpy(player[i].spellCard[0].desc, "Shoots six homing orbs at the opponent,\neach of which inflicts 3 damage.");
			strcpy(player[i].spellCard[1].cardName, "Dream Sign \"Evil-Sealing Circle\"");
			player[i].spellCard[1].mpCost = 8;
			strcpy(player[i].spellCard[1].desc, "Surrounds herself with a circular barrier\nthat inflicts 5 damage and stuns opponent\nfor one turn.");

			break;

			//initialize the character stats corresponding to Marisa
		case MARISA:
			player[i].maxHp = 43;
			player[i].inGameSpriteWidth = 54;
			player[i].inGameSpriteHeight = 63;
			player[i].hitboxRadius = 15.0f;
			player[i].numSpellCards = 2;
			player[i].spellCard = new SpellCardType[player[i].numSpellCards];
			strcpy(player[i].spellCard[0].cardName, "Magic Sign \"Stardust Reverie\"");
			player[i].spellCard[0].mpCost = 12;
			strcpy(player[i].spellCard[0].desc, "Rides her broom to charge at the opponent,\ninflicting 5 damage and causing the\nopponent to knock back.");
			strcpy(player[i].spellCard[1].cardName, "Love Sign \"Master Spark\"");
			player[i].spellCard[1].mpCost = 15;
			strcpy(player[i].spellCard[1].desc, "Shoots some lovely laser, inflicting \n8 damage.");

			break;
		}

		//set current HP to max HP
		player[i].hp = player[i].maxHp;

		//set current MP to zero
		player[i].mp = 0;

		//characters are stationary
		player[i].moveSpeed = { 0.0f, 0.0f };

		//first player starts at the left side of the map
		if (i == 0) {
			player[i].position = { 100.0f, 300.0f };
		}

		//second player starts at the right side of the map
		else if (i == 1) {
			player[i].position = { 700.0f, 300.0f };
		}
	}

	return true;
}

void VersusMode::Shutdown() {
	for (int i = 0; i < numPlayers; i++) {
		if (player[i].spellCard) {
			delete[] player[i].spellCard;
			player[i].spellCard = 0;
		}
		m_Clock->DeleteClock(player[i].spriteClockID);
	}
	if (map.rectWall) {
		delete[] map.rectWall;
		map.rectWall = 0;
	}
	if (tempAngle) {
		delete tempAngle;
		tempAngle = 0;
	}
	if (tempPos) {
		delete tempPos;
		tempPos = 0;
	}
	if (tempSpeed) {
		delete tempSpeed;
		tempSpeed = 0;
	}
	if (tempBullet) {
		delete[] tempBullet;
		tempBullet = 0;
	}
	if (tempTimerID) {
		delete[] tempTimerID;
		tempTimerID = 0;
	}
	m_Graphics->DeleteBitmap(map.mapBitmapID);
	m_Graphics->DeleteBitmap(type01color01bulletID);
	m_Graphics->DeleteBitmap(movePhaseAnnounceBitmapID);
	m_Graphics->DeleteBitmap(actPhaseAnnounceBitmapID);
	m_Graphics->DeleteBitmap(playerIndicatorBitmapID);
	m_Graphics->DeleteBitmap(passButton.bitmapID);
	m_Graphics->DeleteBitmap(moveButton.bitmapID);
	m_Graphics->DeleteBitmap(shootButton.bitmapID);
	m_Graphics->DeleteBitmap(spellButton.bitmapID);
	m_Graphics->DeleteBitmap(aimCircleBitmapID);
	m_Graphics->DeleteBitmap(statsWindowBitmapID);
	m_Graphics->DeleteBitmap(spellDescBitmapID);
	for (int i = 0; i < 18; i++) {
		m_Graphics->DeleteBitmap(color01laserParticleID[i]);
	}
	for (int i = 0; i < 4; i++) {
		m_Graphics->DeleteBitmap(reimuStationaryBitmapID[i]);
	}
	for (int i = 0; i < 4; i++) {
		m_Graphics->DeleteBitmap(marisaStationaryBitmapID[i]);
	}
	for (int i = 0; i < 5; i++) {
		m_Graphics->DeleteBitmap(spellNameButton[i].bitmapID);
	}
	for (int i = 0; i < 3; i++) {
		m_Graphics->DeleteBitmap(reimuSpell01Bullet[i]);
		m_Graphics->DeleteBitmap(reimuSpell01BulletBg[i]);
	}
	for (int i = 0; i < 19; i++) {
		m_Graphics->DeleteBitmap(hit01BitmapID[i]);
	}
	m_Graphics->DeleteSentence(spellNameSentID);
	m_Graphics->DeleteSentence(spellDescSentID);
	m_Graphics->DeleteSentence(hpDispSentID);
	m_Graphics->DeleteSentence(mpDispSentID);

	m_Clock->DeleteTimer(phaseAnnounceTimerID);
	m_Clock->DeleteClock(playerIndicatorClockID);
}

//returns true if a wall is present within given radius from pos
bool VersusMode::CollisionWithWall(XMFLOAT2 pos, float radius) {

	//check every rectangular wall
	for (int i = 0; i < map.numRectWall; i++) {

		RECT& wallRect = map.rectWall[i];

		//if X position is left to the wall
		if (pos.x < wallRect.left) {

			//if Y position is below the wall
			if (pos.y > wallRect.bottom) {
				if (Distance(pos.x, pos.y, (float)wallRect.left, (float)wallRect.bottom) >= radius) {
					return true;
				}
			}

			//if Y position is above the wall
			else if (pos.y < wallRect.top) {
				if (Distance(pos.x, pos.y, (float)wallRect.left, (float)wallRect.top) >= radius) {
					return true;
				}
			}

			//if Y position is between top and bottom side of the wall
			else {
				if (pos.x + radius >= wallRect.left) {
					return true;
				}
			}
		}

		//if X position is right to the wall
		else if (pos.x>wallRect.right) {
			//if Y position is below the wall
			if (pos.y > wallRect.bottom) {
				if (Distance(pos.x, pos.y, (float)wallRect.right, (float)wallRect.bottom) >= radius) {
					return true;
				}
			}

			//if Y position is above the wall
			else if (pos.y < wallRect.top) {
				if (Distance(pos.x, pos.y, (float)wallRect.right, (float)wallRect.top) >= radius) {
					return true;
				}
			}

			//if Y position is between top and bottom side of the wall
			else {
				if (pos.x - radius <= wallRect.left) {
					return true;
				}
			}
		}

		//if X position is between left and right side of the wall
		else {
			if (pos.y < wallRect.top) {
				if (pos.y + radius > wallRect.top) {
					return true;
				}
			}
			else if (pos.y > wallRect.bottom) {
				if (pos.y - radius < wallRect.bottom) {
					return true;
				}
			}
			else {
				return true;
			}
		}
	}
	return false;
}

//returns true if given velocity is zero
bool VersusMode::IsStationary(XMFLOAT2 speed) {
	return ((speed.x == 0) && (speed.y == 0));
}

//returns true if all given velocities are zero
bool VersusMode::AllStationary(XMFLOAT2* speedList, int size) {
	for (int i = 0; i < size; i++) {
		if (!(IsStationary(speedList[i]))) {
			return false;
		}
	}
	return true;
}

//moves onto next phase or next player's turn
void VersusMode::NextPhase() {

	//if current phase is Move Phase
	if (movePhase) {
		//move on to Act Phase
		movePhase = false;
		actionPhase = true;

		//announce Move Phase
		m_Clock->SetTimer(phaseAnnounceTimerID, 240);

		//reset the player choice to PENDING_CHOICE
		choice = PENDING_CHOICE;
	}

	//if current phase is Act Phase
	else {
		//move on to Move Phase
		movePhase = true;
		actionPhase = false;

		//add 2 MP to current player
		player[playerTurn].mp += 2;

		//next player turn
		playerTurn++;
		if (playerTurn >= numPlayers) {
			playerTurn = 0;
		}

		//announce Move Phase
		m_Clock->SetTimer(phaseAnnounceTimerID, 240);

		//set player indicator clock to zero
		m_Clock->SetClock(playerIndicatorClockID, 0);

		//reset the player choice to PENDING_CHOICE
		choice = PENDING_CHOICE;
	}
}

//returns true if a character hurtbox is present within given radius from pos
//and sets collidedChar to the currently collided character
bool VersusMode::CollisionWithCharacter(XMFLOAT2 pos, float radius, int& collidedChar) {
	for (int i = 0; i < numPlayers; i++) {
		if (i != playerTurn) {
			if (Distance(pos, player[i].position) <= radius + player[i].hitboxRadius) {
				collidedChar = i;
				return true;
			}
		}
	}
	return false;
}

//prompts the player to shoot from the player, and records the shot speed,
//initial position, and angle to given parameters
void VersusMode::Shoot(XMFLOAT2& pos, XMFLOAT2& speedVec, float& angle, float radius) {
	POINT lClickPos = m_Input->GetMouseLeftClickLocation();
	POINT mousePt = m_Input->GetMouseLocation();
	if (shootFrame == -1 && m_Input->IsKeyDown(VK_LBUTTON) && Distance(lClickPos, pos) <= radius && (mousePt.x != lClickPos.x || mousePt.y != lClickPos.y)) { //if the player clicked on the character sprite
		shootFrame = m_Clock->GetFrameCount() - 1;
	}
	//Character's velocity will be recorded once player releases the left mouse button,
	//or when the mouse gets certain distance away from the character
	else if (shootFrame != -1 && (m_Input->IsKeyJustReleased(VK_LBUTTON) || Distance(mousePt, lClickPos) > 50.0f)) {

		//record x component
		speedVec.x = (float)(mousePt.x - lClickPos.x) / (float)(m_Clock->GetFrameCount() - shootFrame);

		//record y component
		speedVec.y = (float)(mousePt.y - lClickPos.y) / (float)(m_Clock->GetFrameCount() - shootFrame);

		//record angle
		angle = atan2(speedVec.y, speedVec.x);

		//reset the shootFrame
		shootFrame = -1;
	}

	//if player is currently shooting/moving, update and render aim circle
	if (m_Input->IsKeyDown(VK_LBUTTON) && Distance(lClickPos, pos) <= radius &&Distance(lClickPos, mousePt) <= 50.0f) {
		m_Graphics->UpdateBitmap(aimCircleBitmapID, lClickPos.x, lClickPos.y);
		m_Graphics->RenderBitmap(aimCircleBitmapID);
	}

}

//moves the object in the current velocity, decreasing the speed by
//friction. returns true if the object is still moving, false otherwise
bool VersusMode::Moving(XMFLOAT2& pos, XMFLOAT2& speedVec, float& angle, float radius) {

	//update x position
	pos.x += speedVec.x;

	//update y position
	pos.y += speedVec.y;

	//a null integer variable for receiving information for collision with character
	int nullInt = 0;

	//if the object collided with a wall or a character
	if (CollisionWithWall(pos, radius) || CollisionWithCharacter(pos, radius, nullInt)) {

		//slope for calculation a linear line (y = mx + b)
		//that represents the object's movement
		float slope;

		//object's position in the previous frame
		XMFLOAT2 posI = { pos.x - speedVec.x, pos.y - speedVec.y };

		//if the object is moving in a vertical line
		if (speedVec.x == 0) {

			//if the object is heading downwards
			if (speedVec.y > 0) {

				//test whether the object collides 0.1 pixel, 0.2 pixel, etc
				//below the initial position
				for (float y = 0.0f; posI.y + y <= pos.y; y += 0.1f) {
					XMFLOAT2 testPos = { posI.x, posI.y + y };

					//if the object collides at testPos, set the object's position to testPos
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(pos, radius, nullInt)) {
						pos = testPos;
						break;
					}
				}
			}
			//if the object is heading upwards
			else if (speedVec.y < 0) {

				//test whether the object collides 0.1 pixel, 0.2 pixel, etc
				//above the initial position
				for (float y = 0.0f; posI.y + y >= pos.y; y -= 0.1f) {
					XMFLOAT2 testPos = { posI.x, posI.y + y };

					//if the object collides at testPos, set the object's position to testPos
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(testPos, radius, nullInt)) {
						pos = testPos;
						break;
					}
				}
			}
		}

		//if the object is not moving in a vertical line
		else {

			//slope (m) in y = mx + b
			slope = speedVec.y / speedVec.x;

			//y-intercept (b) in y = mx + b
			float b = pos.y - slope*pos.x;

			//if the object is moving to the right
			if (speedVec.x > 0) {

				//test where the object collides by incrementing the x-position 
				//by 0.1 pixels and updating y-position to mx + b
				for (float x = 0.0f; posI.x + x <= pos.x; x += 0.1f) {
					XMFLOAT2 testPos = { posI.x + x, slope*(posI.x + x) + b };

					//if the object collides at testPos, set the object's position to testPos
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(testPos, radius, nullInt)) {
						pos = testPos;
						break;
					}
				}
			}

			//if the object is moving to the left
			else if (speedVec.x < 0) {

				//test where the object collides by decrementing the x-position 
				//by 0.1 pixels and updating y-position to mx + b
				for (float x = 0.0f; posI.x + x >= pos.x; x -= 0.1f) {
					XMFLOAT2 testPos = { posI.x + x, slope*(posI.x + x) + b };

					//if the object collides at testPos, set the object's position to testPos
					if (CollisionWithWall(testPos, radius) || CollisionWithCharacter(testPos, radius, nullInt)) {
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
	else {

		//if the object is moving to the right
		if (speedVec.x > 0) {

			//slow down x-speed
			speedVec.x -= FRICTION*cos(angle);
			if (speedVec.x <= 0) {
				speedVec.x = 0;
			}
		}

		//if the object is moving to the left
		else if (speedVec.x < 0) {

			//slow down x-speed
			speedVec.x -= FRICTION*cos(angle);
			if (speedVec.x >= 0) {
				speedVec.x = 0;
			}
		}

		//if the object is moving down
		if (speedVec.y > 0) {

			//slow down y-speed
			speedVec.y -= FRICTION*sin(angle);
			if (speedVec.y <= 0) {
				speedVec.y = 0;
			}
		}

		//if the object is moving up
		else if (speedVec.y < 0) {

			//slow down y-speed
			speedVec.y -= FRICTION*sin(angle);
			if (speedVec.y >= 0) {
				speedVec.y = 0;
			}
		}
	}

	//if the object is no longer moving
	if (speedVec.x == 0 && speedVec.y == 0) {

		//the object is no longer moving
		return false;
	}

	return true;
}

//initializes the temporary heap variables for recording
//position, speed, and angle
void VersusMode::InitializeTempPosSpeedAngle(float x, float y) {
	tempAngle = new float;
	tempPos = new XMFLOAT2;
	tempSpeed = new XMFLOAT2;
	tempPos->x = x;
	tempPos->y = y;
	tempSpeed->x = 0.0f;
	tempSpeed->y = 0.0f;
	*tempAngle = 0.0f;
}

bool VersusMode::Frame() {
#ifdef _DEBUG
	//debug mode

	//R key resets and reloads the versus mode
	if (m_Input->IsKeyJustPressed(0x52)) {
		Shutdown();
		return Initialize();
	}

	//0 key adds 5 MP to current character's turn
	if (m_Input->IsKeyJustPressed(0x30)) {
		player[playerTurn].mp += 5;
	}
#endif

	//render background map
	m_Graphics->UpdateBitmap(map.mapBitmapID, 400, 300);
	m_Graphics->RenderBitmap(map.mapBitmapID);

	//render character sprites
	for (int i = 0; i < numPlayers; i++) {

		//X position to render bitmap
		float renderBitmapPosX = player[i].position.x;

		//Y position to render bitmap
		float renderBitmapPosY = player[i].position.y;

		//current time in character's sprite clock
		int time = m_Clock->CurrentClockTime(player[i].spriteClockID);

		switch (player[i].character) {
		case REIMU:

			//idle animation
			if (time % 120 <= 3) {
				player[i].inGameSpriteID = reimuStationaryBitmapID[2];
			}
			else if (time % 12 < 3) {
				player[i].inGameSpriteID = reimuStationaryBitmapID[0];
			}
			else if (time % 12 > 3 && time % 12 <= 6) {
				player[i].inGameSpriteID = reimuStationaryBitmapID[1];
			}
			else {
				player[i].inGameSpriteID = reimuStationaryBitmapID[3];
			}

			//render Reimu floating up and down
			renderBitmapPosY = round(3.0f*sin((float)time / 30.0f) + renderBitmapPosY);

			break;

		case MARISA:

			//idle animation
			if (time % 32 < 8) {
				player[i].inGameSpriteID = marisaStationaryBitmapID[0];
			}
			else if (time % 32 > 8 && time % 32 <= 16) {
				player[i].inGameSpriteID = marisaStationaryBitmapID[1];
			}
			else if (time % 32 > 16 && time % 32 <= 24) {
				player[i].inGameSpriteID = marisaStationaryBitmapID[2];
			}
			else {
				player[i].inGameSpriteID = marisaStationaryBitmapID[3];
			}

			//render Marisa floating up and down
			renderBitmapPosY = round(4.0f*sin((float)time / 15.0f) + renderBitmapPosY);
		}

		//update bitmap position and render the sprite
		m_Graphics->UpdateBitmap(player[i].inGameSpriteID, (int)round(renderBitmapPosX), (int)round(renderBitmapPosY));
		m_Graphics->RenderBitmap(player[i].inGameSpriteID);
	}

	//render player indicator bitmap
	m_Graphics->UpdateBitmap(playerIndicatorBitmapID, (int)round(player[playerTurn].position.x), (int)round(player[playerTurn].position.y), (float)m_Clock->CurrentClockTime(playerIndicatorClockID)*XM_PI / 60.0f);
	m_Graphics->RenderBitmap(playerIndicatorBitmapID);

	//render bullet sprites
	for (int i = 0; i < numBullets; i++) {
		m_Graphics->UpdateBitmap(bullet[i].bitmapID, (int)round(bullet[i].position.x), (int)round(bullet[i].position.y), bullet[i].moveAngle);
		m_Graphics->RenderBitmap(bullet[i].bitmapID);
	}

	//render lasers
	for (int i = 0; i < numLasers; i++) {

		for (int j = 0; j < laser[i].numParticles; j++) {

			LaserParticleType& particle = laser[i].particles[j];

			//set laser sprite's dimensions
			m_Graphics->UpdateBitmap(*particle.pBitmapID, particle.dimensions);

			//set laser sprite's centre position
			m_Graphics->UpdateBitmap(*particle.pBitmapID, (int)round(particle.midPt.x), (int)round(particle.midPt.y), laser[i].angle);

			//render laser sprite
			m_Graphics->RenderBitmap(*particle.pBitmapID);
		}
	}

	//if it is currently Move Phase and player hasn't made a choice
	if (movePhase && choice == PENDING_CHOICE) {

		//update and render Move choice button
		moveButton.buttonRect = { 0, 550, 150, 600 };
		m_Graphics->UpdateBitmap(moveButton.bitmapID, moveButton.buttonRect);
		m_Graphics->RenderBitmap(moveButton.bitmapID);

		//update and render Pass choice button
		passButton.buttonRect = { 150, 550, 300, 600 };
		m_Graphics->UpdateBitmap(passButton.bitmapID, passButton.buttonRect);
		m_Graphics->RenderBitmap(passButton.bitmapID);

		//if left mouse button is clicked and just released
		if (m_Input->IsKeyJustReleased(VK_LBUTTON)) {

			//if the user left-clicked the Move choice button
			if (RectContains(moveButton.buttonRect, m_Input->GetMouseLocation()) && RectContains(moveButton.buttonRect, m_Input->GetMouseLeftClickLocation())) {
				choice = MOVE;
			}

			//if the user left-clicked the Pass choice button
			else if (RectContains(passButton.buttonRect, m_Input->GetMouseLocation()) && RectContains(passButton.buttonRect, m_Input->GetMouseLeftClickLocation())) {
				choice = PASS;
			}
		}

#ifdef _DEBUG

		//in debug mode, M key is shortkey for Move choice and P key is shortkey for Pass choice
		else if (m_Input->IsKeyJustPressed(0x4D)) {
			choice = MOVE;
		}
		else if (m_Input->IsKeyJustPressed(0x50)) {
			choice = PASS;
		}
#endif
	}

	//if it is currently Act Phase and player hasn't made a choice
	else if (actionPhase && choice == PENDING_CHOICE) {

		//update and render Shoot choice button
		shootButton.buttonRect = { 0, 550, 150, 600 };
		m_Graphics->UpdateBitmap(shootButton.bitmapID, shootButton.buttonRect);
		m_Graphics->RenderBitmap(shootButton.bitmapID);

		//update and render Spell choice button
		spellButton.buttonRect = { 150, 550, 300, 600 };
		m_Graphics->UpdateBitmap(spellButton.bitmapID, spellButton.buttonRect);
		m_Graphics->RenderBitmap(spellButton.bitmapID);

		//update and render Pass choice button
		passButton.buttonRect = { 300, 550, 450, 600 };
		m_Graphics->UpdateBitmap(passButton.bitmapID, passButton.buttonRect);
		m_Graphics->RenderBitmap(passButton.bitmapID);

		//if left mouse button is clicked and just released
		if (m_Input->IsKeyJustReleased(VK_LBUTTON)) {

			//if the user left-clicked the Shoot choice button
			if (RectContains(shootButton.buttonRect, m_Input->GetMouseLocation()) && RectContains(shootButton.buttonRect, m_Input->GetMouseLeftClickLocation())) {
				choice = SHOOT;
			}

			//if the user left-clicked the Pass choice button
			else if (RectContains(passButton.buttonRect, m_Input->GetMouseLocation()) && RectContains(passButton.buttonRect, m_Input->GetMouseLeftClickLocation())) {
				choice = PASS;
			}

			//if the user left-clicked the Spell choice button
			else if (RectContains(spellButton.buttonRect, m_Input->GetMouseLocation()) && RectContains(spellButton.buttonRect, m_Input->GetMouseLeftClickLocation())) {
				choice = SPELL;
			}
		}
#ifdef _DEBUG
		//in debug mode, P key is shortkey for Pass choice, S key is for Shoot, L key is for Spell
		else if (m_Input->IsKeyJustPressed(0x50)) {
			choice = PASS;
		}
		else if (m_Input->IsKeyJustPressed(0x53)) {
			choice = SHOOT;
		}
		else if (m_Input->IsKeyJustPressed(0x4C)) {
			choice = SPELL;
		}
#endif
	}

	//if it is currently Move Phase and player has made a choice
	if (movePhase && choice != PENDING_CHOICE) {

		XMFLOAT2& pos = player[playerTurn].position;
		XMFLOAT2& speedVec = player[playerTurn].moveSpeed;
		float& angle = player[playerTurn].moveAngle;

		//Move Phase choice handling
		switch (choice) {

			//Move choice
		case MOVE:

			//the user will move the character
			shooting = true;

			//if the player hasn't moved the character yet
			if (shooting && speedVec.x == 0 && speedVec.y == 0) {
				Shoot(pos, speedVec, angle, player[playerTurn].hitboxRadius);
			}

			//if the player has moved the character and character is still moving
			else if (shooting) {

				//if the character has stopped moving
				if (!Moving(pos, speedVec, angle, player[playerTurn].hitboxRadius)) {

					//character is stationary
					shooting = false;

					//change the current phase to Act Phase
					NextPhase();
				}
			}
			break;

			//Pass choice
		case PASS:

			//change the current phase to Act Phase
			NextPhase();

			break;

			//error-handling: any other choice just passes the turn
		default:
			//change the current phase to Act Phase
			NextPhase();
		}
	}

	//if it is currently Act Phase and player has made a choice
	else if (actionPhase && choice != PENDING_CHOICE) {

		//Act Phase choice handling
		switch (choice) {

			//Shoot choice
		case SHOOT:

			//different shooting properties for each character
			switch (player[playerTurn].character) {

				//Reimu will shoot a single 2-damage amulet
			case REIMU:

				//if temporary position, velocity, and angle variables are not yet initialized
				if (!shooting) {

					//initialize the temporary variables
					InitializeTempPosSpeedAngle(player[playerTurn].position.x,
						player[playerTurn].position.y);

					//the player will shoot the bullet
					shooting = true;
				}

				//if the player is shooting the bullet or the bullet is in motion
				if (shooting) {

					//temporary variables are used so that the bullet can be created and rendered
					//after position, speed, and angle are recorded 
					if (tempPos || tempSpeed || tempAngle) {

						//if the speed, position, and angle have not yet been recorded
						if (tempSpeed->x == 0.0f && tempSpeed->y == 0.0f) {
							Shoot(*tempPos, *tempSpeed, *tempAngle, player[playerTurn].hitboxRadius);
						}

						//if the speed, position, and angle have been recorded
						else {

							//bullet gives 2 damage upon contact with the opponent character
							bullet[numBullets].damage = 2;

							//set the bullet sprite
							bullet[numBullets].bitmapID = type01color01bulletID;

							//the new bullet will have the position as recorded in temporary variable
							if (tempPos) {
								bullet[numBullets].position = *tempPos;
								delete tempPos;
								tempPos = 0;
							}

							//the new bullet will have the velocity as recorded in temporary variable
							if (tempSpeed) {
								bullet[numBullets].moveSpeed = *tempSpeed;
								delete tempSpeed;
								tempSpeed = 0;
							}

							//the new bullet will have the angle as recorded in temporary variable
							if (tempAngle) {
								bullet[numBullets].moveAngle = *tempAngle;
								delete tempAngle;
								tempAngle = 0;
							}

							//increase the bullet count by one
							numBullets++;
						}
					}

					//if the position, velocity, and angle has been transferred from temporary
					//variables to the new bullet
					if (!tempPos && !tempSpeed && !tempAngle) {

						XMFLOAT2& bulletPos = bullet[numBullets - 1].position;
						XMFLOAT2& bulletSpeed = bullet[numBullets - 1].moveSpeed;
						float& bulletAngle = bullet[numBullets - 1].moveAngle;

						//if the bullet stopped moving
						if (!Moving(bulletPos, bulletSpeed, bulletAngle, 6.0f)) {

							int collidedChar;

							//bullet is not being shot
							shooting = false;

							//detect whether bullet has collided with opponent character
							if (CollisionWithCharacter(bulletPos, 6.0f, collidedChar)) {
								player[collidedChar].hp -= bullet[numBullets - 1].damage;
							}

							//bullet is erased
							numBullets--;

							//move on to next player's turn
							NextPhase();
						}
					}
				}

				break;

				//Marisa will shoot a 3-damage laser
			case MARISA:

				//if temporary position, velocity and angle variables are not yet initialized
				if (!shooting) {

					//Initialize the temporary variables
					InitializeTempPosSpeedAngle(player[playerTurn].position.x,
						player[playerTurn].position.y);

					//the player will shoot the laser
					shooting = true;
				}

				//if the player is shooting the laser or the laser is still activated
				if (shooting) {

					int collidedChar = -1;

					//temporary variables are used so that the laser can be created and rendered
					//after the position and angle are recorded
					if (tempPos || tempSpeed || tempAngle) {

						//if the speed, position, and angle have not yet been recorded
						if (tempSpeed->x == 0 && tempSpeed->y == 0) {
							Shoot(*tempPos, *tempSpeed, *tempAngle, player[playerTurn].hitboxRadius);
						}

						//if the speed, position, and angle have been recorded
						else {

							//laser gives 3 damage upon contact with the opponent character
							laser[numLasers].damage = 3;

							//set number of laser entities
							laser[numLasers].numParticles = 18;

							//setting laser particles to laserColor01 particles
							laser[numLasers].particles = new LaserParticleType[laser[numLasers].numParticles];

							//centre position of the entire set of lasers
							XMFLOAT2 posCtrI = *tempPos;

							//slope, yInt represent m, b in y = mx + b, the linear line of the laser's trajectory
							float slope = tempSpeed->y / tempSpeed->x;  //TODO: deal with division by 0
							float yInt = tempPos->y - slope*tempPos->x;

							//slope, yInt represent m, b in y = mx + b, the linear line perpendicular to above line
							float perpSlope = -1.0f / slope;
							float perpYInt = tempPos->y - perpSlope*tempPos->x;

							//for each laser entity
							for (int i = 0; i < laser[numLasers].numParticles; i++) {

								//set pointer to bitmap ID
								laser[numLasers].particles[i].pBitmapID = &color01laserParticleID[i];

								//entity's initial position
								XMFLOAT2& posI = laser[numLasers].particles[i].posI;

								//entity's final position (where the laser lands)
								XMFLOAT2& posF = laser[numLasers].particles[i].posF;

								//midpoint between entity's initial and final positions
								XMFLOAT2& midPt = laser[numLasers].particles[i].midPt;

								//rectangular dimensions of entity sprite
								RECT& dim = laser[numLasers].particles[i].dimensions;

								//x-component of maximum displacement of entity's position from laser's center
								float maxXDisp = sqrt(pow((float)player[playerTurn].hitboxRadius, 2.0f) / (1.0f + pow(perpSlope, 2)));

								//set initial position
								posI.x = posCtrI.x + maxXDisp*(float)(i - laser[numLasers].numParticles / 2.0f) / (laser[numLasers].numParticles / 2.0f);
								posI.y = posI.x*perpSlope + perpYInt;

								//initialize final position to posI
								posF = posI;

								//if laser has not made a collision with opponent yet
								if (collidedChar == -1) {

									//if player shot laser to the right
									if (tempSpeed->x > 0) {

										//increment posF for checking wall/character collision
										while (!(CollisionWithCharacter(posF, 1.0f, collidedChar)) && !(CollisionWithWall(posF, 1.0f))) {
											posF.x += 1.0f;
											posF.y += slope;
										}
									}

									//if player shot laser to the left
									else if (tempSpeed->x < 0) {

										//decrement posF for checking wall/chararcter collision
										while (!(CollisionWithCharacter(posF, 1.0f, collidedChar)) && !(CollisionWithWall(posF, 1.0f))) {
											posF.x -= 1.0f;
											posF.y -= slope;
										}
									}

									//if player shot vertically
									else {

										//if player shot the laser upwards
										if (tempSpeed->y > 0) {

											//increment the y-position of 
											while (!(CollisionWithCharacter(posF, 1.0f, collidedChar)) && !(CollisionWithWall(posF, 1.0f))) {
												posF.y += 1.0f;
											}
										}

										//if player shot the laser downwards
										else {
											while (!(CollisionWithCharacter(posF, 1.0f, collidedChar)) && !(CollisionWithWall(posF, 1.0f))) {
												posF.y -= 1.0f;
											}
										}
									}
								}

								//if the laser has made collision with another character, do not change the collidedChar
								//TODO: when laser hits more than one character, an array of players hit will have to be recorded
								else {
									int nullChar = -1;
									if (tempSpeed->x > 0) {
										while (!(CollisionWithCharacter(posF, 1.0f, nullChar)) && !(CollisionWithWall(posF, 1.0f))) {
											posF.x += 1.0f;
											posF.y += slope;
										}
									}
									else if (tempSpeed->x < 0) {
										while (!(CollisionWithCharacter(posF, 1.0f, nullChar)) && !(CollisionWithWall(posF, 1.0f))) {
											posF.x -= 1.0f;
											posF.y -= slope;
										}
									}
									else {
										if (tempSpeed->y > 0) {
											while (!(CollisionWithCharacter(posF, 1.0f, nullChar)) && !(CollisionWithWall(posF, 1.0f))) {
												posF.y += 1.0f;
											}
										}
										else {
											while (!(CollisionWithCharacter(posF, 1.0f, nullChar)) && !(CollisionWithWall(posF, 1.0f))) {
												posF.y -= 1.0f;
											}
										}
									}
								}

								//set midpoint between initial and final position of laser entity
								midPt.x = (posI.x + posF.x) / 2.0f;
								midPt.y = (posI.y + posF.y) / 2.0f;

								//stretch the laser entity sprite to fit from initial to final position of laser entity
								dim.left = 0;
								dim.top = 0;
								dim.bottom = (long)round(2.0f*player[playerTurn].hitboxRadius / (float)laser[numLasers].numParticles + 2.0f);
								dim.right = (long)round(Distance(posI, posF));
							}

							//temporary position variable is deleted
							delete tempPos;
							tempPos = 0;

							//temporary speed variable is deleted
							delete tempSpeed;
							tempSpeed = 0;

							//the laser will have the angle as recorded in temporary variable
							laser[numLasers].angle = *tempAngle;
							delete tempAngle;
							tempAngle = 0;

							//the laser will be on-screen for 3 seconds
							m_Clock->AddTimer(laser[numLasers].timerID, 180);

							//increase the laser count by one
							numLasers++;
						}
					}


					if (!tempPos && !tempSpeed && !tempAngle) {

						//if the opponent character is hit, subtract HP by laser damage
						//TODO: what if more than one opponent character gets hit by the laser?
						if (collidedChar != -1) {
							player[collidedChar].hp -= laser[numLasers - 1].damage;
							collidedChar = -1;
						}

						//if the laser reaches time limit (of 3 seconds)
						if (!m_Clock->IsTimerRunning(laser[numLasers - 1].timerID)) {

							//delete timer
							m_Clock->DeleteTimer(laser[numLasers - 1].timerID);

							//delete heap particles and set pointer to NULL
							delete[] laser[numLasers].particles;
							laser[numLasers].particles = 0;

							//decrease number of on-screen laser by 1
							numLasers--;

							//Marisa is no longer shooting
							shooting = false;

							//move on to next player's turn
							NextPhase();
						}
					}
				}
				break;
			}
			break;

			//Pass choice
		case PASS:

			//move on to Move Phase
			NextPhase();
			break;

			//Spell choice
		case SPELL:

			//if a spell card is not yet selected
			if (!isSpellSelected) {

				//if right mouse button is pressed, go back to previous choices
				if (m_Input->IsKeyJustPressed(VK_RBUTTON)) {
					choice = PENDING_CHOICE;
				}

				//index of spell card which the mouse hovers over
				int hover = -1;

				for (int i = 0; i < 5; i++) {
					ButtonType& button = spellNameButton[i];

					//update bitmap position
					button.buttonRect = { 20, 435 + 30 * i, 520, 465 + 30 * i };
					m_Graphics->UpdateBitmap(button.bitmapID, button.buttonRect);

					//render bitmap
					m_Graphics->RenderBitmap(button.bitmapID);

					if ((unsigned)i < player[playerTurn].numSpellCards) {

						//update position of spell card name sentence
						m_Graphics->UpdateSentence(spellNameSentID, player[playerTurn].spellCard[i].cardName, 30, 450 + 30 * i, SOLID_BLACK);

						//render spell card name sentence
						m_Graphics->RenderSentence(spellNameSentID);

						//if mouse is hovering over the button, set hover to current spell card index
						if (RectContains(button.buttonRect, m_Input->GetMouseLocation())) {
							hover = i;
						}

						//if left clicked the button and player has sufficient MP
						if (ButtonLeftClicked(button.buttonRect, m_Input) && player[playerTurn].mp >= player[playerTurn].spellCard[i].mpCost) {
							player[playerTurn].mp -= player[playerTurn].spellCard[i].mpCost;
							spellSelected = i;
							isSpellSelected = true;
						}
					}
				}

				//if mouse is hovering over a button, display spell description box
				if (hover != -1) {
					m_Graphics->UpdateBitmap(spellDescBitmapID, 630, 510);
					m_Graphics->RenderBitmap(spellDescBitmapID);

					char mpCostCStr[MAX_CHARACTER_COUNT];
					std::string mpCostStr = "MP Cost: " + std::to_string(player[playerTurn].spellCard[hover].mpCost);
					strcpy(mpCostCStr, mpCostStr.c_str());
					m_Graphics->UpdateSentence(spellDescSentID, mpCostCStr, 507, 442, SOLID_BLACK);
					m_Graphics->RenderSentence(spellDescSentID);

					m_Graphics->UpdateSentence(spellDescSentID, player[playerTurn].spellCard[hover].desc, 507, 474, SOLID_BLACK);
					m_Graphics->RenderSentence(spellDescSentID);
				}
			}

			//if spell card is selected
			else {
				switch (player[playerTurn].character) {
				case REIMU:
					switch (spellSelected) {

						//Spirit Sign "Fantasy Seal"
					case 0:

						//orbs have not been created
						if (!tempBullet) {

							tempBulletNum = 6;
							tempBullet = new BulletType[tempBulletNum];
							tempTimerIDNum = 6;
							tempTimerID = new int[tempTimerIDNum];

							for (int i = 0; i < tempBulletNum; i++) {

								//each orb gives 3 damage
								tempBullet[i].damage = 3;

								//each orb will start at character's position
								tempBullet[i].position = player[playerTurn].position;

								//each orb will spread out from the player character's position to surround the character
								tempBullet[i].moveAngle = (-1.0f)*XM_PI / 2.0f + (float)i*XM_2PI / 6.0f;
								tempBullet[i].moveSpeed.x = 5.0f*cos(tempBullet[i].moveAngle);
								tempBullet[i].moveSpeed.y = 5.0f*sin(tempBullet[i].moveAngle);

								//orb will be set to R B G R B G colours in order
								tempBullet[i].bitmapID = reimuSpell01Bullet[i % 3];

								//orbs are active
								tempBullet[i].state = BULLET_ACTIVE;
							}
						}

						//orbs have been created
						else {

							BulletType* bullet = tempBullet;

							if (!shooting) {

								//array of velocities of the orbs
								XMFLOAT2 spdList[6];
								for (int i = 0; i < 6; i++) {
									spdList[i] = bullet[i].moveSpeed;
								}

								//if orbs are still moving
								if (!AllStationary(spdList, 6)) {
									for (int i = 0; i < 6; i++) {

										//process moving orb
										Moving(bullet[i].position, bullet[i].moveSpeed, bullet[i].moveAngle, 32.0f);
									}
								}
								//if all orbs have been spawned and are stationary
								else {
									shooting = true;

									//initialize temporary position, speed, angle
									if (!tempPos) {
										tempPos = new XMFLOAT2;
										tempSpeed = new XMFLOAT2;
										tempAngle = new float;
										tempSpeed->x = 0.0f;
										tempSpeed->y = 0.0f;
										*tempAngle = 0.0f;
									}
								}
							}
							else {

								int bulletIndex = -1;

								if (m_Input->IsKeyDown(VK_LBUTTON) || m_Input->IsKeyJustReleased(VK_LBUTTON)) {
									for (int i = 0; i < tempBulletNum; i++) {

										POINT lClickPos = m_Input->GetMouseLeftClickLocation();
										//if left clicked the current orb
										if (Distance(lClickPos, bullet[i].position) <= 32.0f && bullet[i].moveSpeed.x == 0.0f && bullet[i].moveSpeed.y == 0.0f) {

											//update the temporary position to left click position
											tempPos->x = (float)lClickPos.x;
											tempPos->y = (float)lClickPos.y;
											bulletIndex = i;
											break;
										}
									}
								}

								//if shooting the orb
								if (bulletIndex != -1) {

									//if temporary speed is zero
									if (tempSpeed->x == 0.0f && tempSpeed->y == 0.0f && bullet[bulletIndex].state == BULLET_ACTIVE) {
										Shoot(*tempPos, *tempSpeed, *tempAngle, 32.0f);
									}

									//if temporary speed is not zero, set velocity and angle of the orb to temporary values
									if (tempSpeed->x != 0.0f || tempSpeed->y != 0.0f) {
										bullet[bulletIndex].moveAngle = *tempAngle;
										bullet[bulletIndex].moveSpeed.x = 10.0f*cos(*tempAngle);
										bullet[bulletIndex].moveSpeed.y = 10.0f*sin(*tempAngle);

										//reset temporary speed to zero
										tempSpeed->x = 0.0f;
										tempSpeed->y = 0.0f;
									}
								}

								for (int i = 0; i < tempBulletNum; i++) {
									//update positions of the orb
									bullet[i].position.x += bullet[i].moveSpeed.x;
									bullet[i].position.y += bullet[i].moveSpeed.y;

									//index of the character that made collision
									int collidedChar = -1;

									//if the orb is moving
									if (bullet[i].moveSpeed.x != 0.0f || bullet[i].moveSpeed.y != 0.0f) {

										//if the orb has made collision with the opponent
										if (CollisionWithCharacter(bullet[i].position, 10.0f, collidedChar)) {
											bullet[i].position = player[collidedChar].position;
											bullet[i].moveSpeed.x = 0.0f;
											bullet[i].moveSpeed.y = 0.0f;
											player[collidedChar].hp -= bullet[i].damage;
										}

										//if the orb has made collision with the wall
										else if (CollisionWithWall(bullet[i].position, 10.0f)) {
											bullet[i].moveSpeed.x = 0.0f;
											bullet[i].moveSpeed.y = 0.0f;
										}

										//if the orb is within 150 pixels from the opponent
										else if (CollisionWithCharacter(bullet[i].position, 150.0f, collidedChar)) {
											float angleToOpponent = atan2(player[collidedChar].position.y - bullet[i].position.y, player[collidedChar].position.x - bullet[i].position.x);
											float angleIncrement = XM_PI / 60.0f;
											float angleDiff = bullet[i].moveAngle - angleToOpponent;

											//tilt the move angle towards the opponent
											if (angleDiff > XM_PI) {
												bullet[i].moveAngle += angleIncrement;
											}
											else if (angleDiff < -1.0f*XM_PI) {
												bullet[i].moveAngle -= angleIncrement;
											}
											else if (angleDiff >= angleIncrement) {
												bullet[i].moveAngle -= angleIncrement;
											}
											else if (angleDiff < -1.0f*angleIncrement) {
												bullet[i].moveAngle += angleIncrement;
											}
											else {
												bullet[i].moveAngle = angleToOpponent;
											}

											//update orb's velocity
											bullet[i].moveSpeed.x = 10.0f*cos(bullet[i].moveAngle);
											bullet[i].moveSpeed.y = 10.0f*sin(bullet[i].moveAngle);
										}
									}
									//if the orb is not moving
									else {
										int collidedChar = -1;
										if (CollisionWithCharacter(bullet[i].position, 10.0f, collidedChar) || CollisionWithWall(bullet[i].position, 10.0f)) {
											int& timer = tempTimerID[i];

											if (!m_Clock->IsTimerRunning(timer)) {
												m_Clock->AddTimer(timer, 18);
												bullet[i].state = BULLET_EXPLODING;
											}
											if (m_Clock->TimeLeft(timer) == 0) {

												m_Clock->DeleteTimer(timer);

												if (tempBulletNum == 1) {
													delete[] bullet;
													tempBullet = 0;

													delete[] tempTimerID;
													tempTimerID = 0;

													shooting = false;
													isSpellSelected = false;

													NextPhase();
												}
												else {
													for (int j = i; j < tempBulletNum - 1; j++) {
														bullet[j].moveAngle = bullet[j + 1].moveAngle;
														bullet[j].moveSpeed = bullet[j + 1].moveSpeed;
														bullet[j].bitmapID = bullet[j + 1].bitmapID;
														bullet[j].position = bullet[j + 1].position;
														bullet[j].state = bullet[j + 1].state;

														tempTimerID[j] = tempTimerID[j + 1];
													}
												}

												tempBulletNum--;
												tempTimerIDNum--;
											}
										}
									}
								}
							}


							for (int i = 0; i < tempBulletNum; i++) {
								BulletType& bullet = tempBullet[i];

								//the orb spins 720 degrees per 60 frames
								float angle = (float)m_Clock->GetFrameCount()*XM_PI / 15.0f;

								//if the timer for physical impact is running
								if (m_Clock->IsTimerRunning(tempTimerID[i])) {

									int timeLeft = (int)m_Clock->TimeLeft(tempTimerID[i]);

									//update and render orb bitmaps if time left >= 8
									if (timeLeft >= 8) {
										for (int j = 0; j < 3; j++) {
											if (bullet.bitmapID == reimuSpell01Bullet[j]) {
												m_Graphics->UpdateBitmap(reimuSpell01BulletBg[j], (int)round(bullet.position.x), (int)round(bullet.position.y), angle);
												m_Graphics->RenderBitmap(reimuSpell01BulletBg[j]);
												break;
											}
										}
										m_Graphics->UpdateBitmap(bullet.bitmapID, (int)round(bullet.position.x), (int)round(bullet.position.y), angle);
										m_Graphics->RenderBitmap(bullet.bitmapID);
									}

									m_Graphics->UpdateBitmap(hit01BitmapID[18 - timeLeft], (int)round(bullet.position.x), (int)round(bullet.position.y));
									m_Graphics->RenderBitmap(hit01BitmapID[18 - timeLeft]);
								}
								else {
									for (int j = 0; j < 3; j++) {
										if (bullet.bitmapID == reimuSpell01Bullet[j]) {
											m_Graphics->UpdateBitmap(reimuSpell01BulletBg[j], (int)round(bullet.position.x), (int)round(bullet.position.y), angle);
											m_Graphics->RenderBitmap(reimuSpell01BulletBg[j]);
											break;
										}
									}
									m_Graphics->UpdateBitmap(bullet.bitmapID, (int)round(bullet.position.x), (int)round(bullet.position.y), angle);
									m_Graphics->RenderBitmap(bullet.bitmapID);
								}
							}
						}

						break;

						//Dream Sign "Evil-Sealing Circle"
					case 1:

						isSpellSelected = false;
						NextPhase();
						break;

					default:

						isSpellSelected = false;
						NextPhase();
						break;
					}
					break;

				case MARISA:

					isSpellSelected = false;
					NextPhase();
					break;
				}
			}

			break;

			//error handling: anything else just skips the turn
		default:
			NextPhase();
		}
	}

	//display the character's status window if player hasn't made a move or
	//has not yet chosen a spell card while selecting spell cards
	if (choice == PENDING_CHOICE || (choice == SPELL && !isSpellSelected)) {
		for (int i = 0; i < numPlayers; i++) {

			//set rectangle containing character sprite
			RECT rc;
			rc.left = (long)round(player[i].position.x - player[i].inGameSpriteWidth / 2.0f);
			rc.right = (long)round(player[i].position.x + player[i].inGameSpriteWidth / 2.0f);
			rc.top = (long)round(player[i].position.y - player[i].inGameSpriteHeight / 2.0f);
			rc.bottom = (long)round(player[i].position.y + player[i].inGameSpriteHeight / 2.0f);

			POINT mousePt = m_Input->GetMouseLocation();
			//if the mouse cursor is hovering over the rectangle
			if (RectContains(rc, mousePt)) {

				//update and render status window over the mouse position
				m_Graphics->UpdateBitmap(statsWindowBitmapID, mousePt.x + 35, mousePt.y - 40);
				m_Graphics->RenderBitmap(statsWindowBitmapID);

				//assemble string that displays current HP
				std::string hpStr = "HP: " + std::to_string(player[i].hp) + " / " + std::to_string(player[i].maxHp);
				char hpDisp[MAX_CHARACTER_COUNT];
				strcpy(hpDisp, hpStr.c_str());

				//update and render current HP
				m_Graphics->UpdateSentence(hpDispSentID, hpDisp, mousePt.x, mousePt.y - 65, SOLID_BLACK);
				m_Graphics->RenderSentence(hpDispSentID);

				//assemble string that displays current MP
				std::string mpStr = "MP: " + std::to_string(player[i].mp);
				char mpDisp[MAX_CHARACTER_COUNT];
				strcpy(mpDisp, mpStr.c_str());

				//update and render current MP
				m_Graphics->UpdateSentence(mpDispSentID, mpDisp, mousePt.x, mousePt.y - 30, SOLID_BLACK);
				m_Graphics->RenderSentence(mpDispSentID);

				break;
			}
		}
	}

	//update pPhaseAnnounceBitmapID
	if (movePhase) {
		pPhaseAnnounceBitmapID = &movePhaseAnnounceBitmapID;
	}
	else if (actionPhase) {
		pPhaseAnnounceBitmapID = &actPhaseAnnounceBitmapID;
	}

	//render phase announcement based on how much time is left
	if (m_Clock->IsTimerRunning(phaseAnnounceTimerID)) {
		long timeLeft = m_Clock->TimeLeft(phaseAnnounceTimerID);
		int posX = m_Graphics->GetBitmapWidth(*pPhaseAnnounceBitmapID) / 2;
		int posY = m_Graphics->GetBitmapHeight(*pPhaseAnnounceBitmapID) / 2;

		//more than 3.5 seconds left
		if (timeLeft >= 210) {

			//the announcement bitmap fades in
			m_Graphics->UpdateBitmap(*pPhaseAnnounceBitmapID, posX, posY, 0.0f, 1.0f - ((float)timeLeft - 210.0f) / 30.0f, NULL_COLOR);
		}

		//3.5 to 1 seconds left
		else if (timeLeft < 210 && timeLeft >= 60) {

			//the announcement bitmap stays
			m_Graphics->UpdateBitmap(*pPhaseAnnounceBitmapID, posX, posY);
		}

		//less than 1 second left
		else if (timeLeft < 60) {

			//the announcement bitmap fades out
			m_Graphics->UpdateBitmap(*pPhaseAnnounceBitmapID, posX, posY, 0.0f, (float)timeLeft / 60.0f);
		}

		//render the announcement bitmap
		m_Graphics->RenderBitmap(*pPhaseAnnounceBitmapID);
	}

	return true;
}