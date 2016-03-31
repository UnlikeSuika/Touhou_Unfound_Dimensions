#ifndef SYSTEMCLASSH
#define SYSTEMCLASSH

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include "InputClass.h"
#include "GraphicsClass.h"
#include "TimeClass.h"

const int MAX_CHAR_SELECT_BUTTONS = 8;   //maximum number of character options in character select screen
const int MAX_ON_SCREEN_BULLETS = 200;   //maximum number of in-game bullets
const int MAX_ON_SCREEN_LASERS = 10;     //maximum number of in-game lasers
const float FRICTION = 0.1;              //factor by which speeds of in-game moving objects constantly decrease

// This class initializes the system objects and
// runs the game every frame.
class SystemClass{
private:

	/*********************************************************
	
	TYPE DEFINITIONS

	**********************************************************/

	//enumeration for different game modes
	typedef enum GameMode{
		MAIN_MENU = 0x00000001,
		VERSUS_MODE = 0x00000002,
		TUTORIAL_MODE = 0x00000003,
		CHARACTER_SELECT_MODE = 0x00000004
	}GameMode;

	//enumeration for playable characters
	typedef enum PlayableCharacter{
		UNSELECTED = 0x00,
		REIMU = 0x01,
		MARISA = 0x02
	};

	//enumeration of choices made by the player during Move Phase or Act Phase of the game
	typedef enum PhaseChoice{
		PENDING_CHOICE = 0x00,
		PASS = 0x01,
		SHOOT = 0x02,
		SPELL = 0x03,
		MOVE = 0x04
	};

	//represents a visible button
	struct ButtonType{
		RECT buttonRect; //rectangular portion represented by button
		int bitmapID;    //ID for button's bitmap
	};

	//represents the player's available spellcards
	struct SpellCardType{
		char cardName[MAX_CHARACTER_COUNT];//title of spellcard
		int mpCost;                        //amount of user's MP that will be required to use the spell
	};

	//represents an in-game bullet
	struct BulletType{
		XMFLOAT2 position; //bullet's current position
		XMFLOAT2 moveSpeed;//bullet's current velocity
		float moveAngle;   //direction(angle) of bullet's current velocity
		int damage;        //damage upon colliding on another object
		int* pBitmapID;    //pointer to bullet's in-game sprite bitmap
	};

	//represents a single particle of an in-game laser
	struct LaserParticleType{
		XMFLOAT2 posI;         //the start position of laser particle
		XMFLOAT2 posF;         //the end position of laser particle (where laser lands)
		XMFLOAT2 midPt;        //the midpoint of particle's start and end position
		RECT dimensions;       //x and y dimensions by which the particle will be stretched
		int* pBitmapID;        //pointer to ID of sprite bitmap
	};

	//represents an in-game laser
	struct LaserType{
		float angle;                 //direction(angle) of laser
		int damage;                  //damage upon colliding on another object
		LaserParticleType* particles;//heap array of laser entities
		int numParticles;            //number of laser entities
		int timerID;                 //duration for which laser is rendered
	};

	//represents a player
	//    Note: maybe this should be defined in separate class
	struct PlayerType{
		PlayableCharacter character;//character chosen by the player
		int *pCharacterAvatarID;    //pointer to ID of avatar bitmap displayed in char select screen
		int flashTimerID;           //ID of a timer for flashing the character avatar upon selecting  
		int *pInGameSpriteID;       //pointer to ID of in-game sprite bitmap
		int spriteClockID;          //pointer to ID of in-game sprite motion clock
		int inGameSpriteWidth;      //width of in-game sprite bitmap
		int inGameSpriteHeight;     //height of in-game sprite bitmap

		SpellCardType* spellCard;   //heap array of available spellcards for the chosen character
		unsigned int numSpellCards; //total number of available spellcards
		unsigned int maxHp;         //character's maximum HP
		unsigned int hp;            //character's current HP
		unsigned int mp;            //character's current MP
		float hitboxRadius;         //character's circular collision detection radius
		                            //	  Note: only Reimu will have a small hitbox
		                            //    radius though, unless I change my mind

		XMFLOAT2 position;          //character's current position
		XMFLOAT2 moveSpeed;         //character's current velocity vector
		float moveAngle;            //direction(angle) of character's current velocity
	};

	//represents a map
	struct MapType{
		bool isWall[800][600]; //array of the map's collision detection for every on-screen pixel
		int mapBitmapID;       //ID of map's bitmap
	};
	
	//represents a single versus match
	//    Note: maybe I should create a separate class for this
	struct GameType{
		MapType map;                             //in-game map        

		PlayerType player[2];                    //array of players playing in this match
		                                         //    Note: currently restricted to 1vs1 match, so
		                                         //    the array is defined to possess two elements.
		                                         //    On 2vs2 match, I'll have to make a separate
		                                         //    struct for a team
		int numPlayers;                          //number of player in the match
		int playerTurn;                          //current player turn
		int victoryPlayer;                       //which player won this match
		int reimuStationaryBitmapID[4];          //array of IDs of Reimu's stationary bitmaps
		int marisaStationaryBitmapID[4];         //array of IDs of Marisa's stationary bitmaps

		BulletType bullet[MAX_ON_SCREEN_BULLETS];//array of on-screen bullets
		LaserType laser[MAX_ON_SCREEN_LASERS];   //array of on-screen lasers
		int shootFrame;                          //the frame when the mouse click is first made to shoot/move
		int numBullets;                          //number of on-screen bullets
		int numLasers;                           //number of on-screen lasers
		bool shooting;                           //whether the player is currently shooting a bullet/moving a character
		int aimCircleBitmapID;                   //ID of bitmap of aiming circle
		int type01color01bulletID;               //ID of bitmap of Type01Color01 bullet
		int color01laserParticleID[18];          //array of IDs of Color1 laser bitmaps

		bool movePhase;                          //whether it is currently Move Phase
		bool actionPhase;                        //whether it is currently Act Phase
		
		int phaseAnnounceTimerID;                //ID of timer for the announcement of phase
		int* pPhaseAnnounceBitmapID;             //pointer to ID of bitmap for current phase announcement
		int movePhaseAnnounceBitmapID;           //ID of bitmap for Move Phase announcement
		int actPhaseAnnounceBitmapID;            //ID of bitmap for Act Phase announcement
		PhaseChoice choice;                      //choice the player made during Move Phase or Act Phase
		ButtonType passButton;                   //Pass choice button
		ButtonType moveButton;                   //Move choice button
		ButtonType shootButton;                  //Shoot choice button
		ButtonType spellButton;                  //Spell choice button

		int statsWindowBitmapID;                 //ID of bitmap that displays character's status window
		int hpDispSentID;                        //ID of sentence object that displays character's current HP
		int mpDispSentID;                        //ID of sentence object that displays character's current MP

		XMFLOAT2* tempPos;                       //(heap)temporary variable for recording position
		XMFLOAT2* tempSpeed;                     //(heap)temporary variable for recording velocity
		float* tempAngle;                        //(heap)temporary variable for recording angle
	};

	/*********************************************************

	PRIVATE FUNCTIONS

	**********************************************************/

	//Windows-related functions
	bool Frame(); 
	void InitializeWindows(int& screenWidth, int& screenHeight); 
	void ShutdownWindows(); 

	//mouse-related functions
	void SetInitialClickPositions(); 
	
	//functions related to fading effect
	void SetFadingEffects();

	//mode-related functions
	void OnMainMenu(); 
	bool OnCharacterSelectMode(); 
	bool OnVersusMode(); 
	bool InitializeCharSelect(); 
	bool InitializeVersusMode(); 
	void ShutdownVersusMode(); 
	
	//movement-related functions
	bool CollisionWithWall(XMFLOAT2 pos, float radius);
	bool CollisionWithCharacter(XMFLOAT2 pos, float radius, int& collidedChar);
	void Shoot(XMFLOAT2& pos, XMFLOAT2& speedVec, float& angle);
	void Moving(XMFLOAT2& pos, XMFLOAT2& speedVec, float& angle, float radius);
	void InitializeTempPosSpeedAngle(float x, float y);

	//helper functions
	bool Contains(RECT rect, POINT pt);
	float Distance(float x1, float y1, float x2, float y2);
	float Distance(POINT p1, POINT p2);
	float Distance(XMFLOAT2 p1, POINT p2);
	float Distance(POINT p1, XMFLOAT2 p2);
	float Distance(XMFLOAT2 p1, XMFLOAT2 p2);

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
	
	//mouse-related variables
	POINT mousePt;           //current cursor position relative to client window
	POINT lClickPos;         //cursor position upon left clicking
	POINT rClickPos;         //cursor position upon right clicking

	//variables related to fading effects
	bool fadingIn;           //whether the screen is fading into another game mode
	bool fadingOut;          //whether the screen is fading out of the current game mode

	//variables related to character select mode
	bool isCharSelectInit;   //whether character select has been initialized
	bool isCharSelectStarted;//whether character select mode has been initiated

	//variables related to versus mode
	bool isVersusModeInit;   //whether the versus match has been initialized
	GameType versusMatch;    //object of the versus match

	//IDs of bitmaps or timers
	int mainMenuBackgroundID;     //ID of bitmap of main menu background
	int cursorSpriteID;            //ID of bitmap of cursor
	int selectedCharButtonID;      //ID of bitmap that highlights character select button upon hovering the mouse over
	int reimuAvatarID;             //ID of bitmap of Reimu in character select screen
	int marisaAvatarID;            //ID of bitmap of Marisa in character select screen
	int charSelectModeBackgroundID;//ID of background bitmap in character select screen
	int fadeTimerID;               //ID of timer for the fading effect of screen

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