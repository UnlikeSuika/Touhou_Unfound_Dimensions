#ifndef _VERSUS_MODE_H_
#define _VERSUS_MODE_H_

#include "GraphicsClass.h"
#include "TimeClass.h"
#include "InputClass.h"

#ifndef MAX_ON_SCREEN_BULLETS
#define MAX_ON_SCREEN_BULLETS 200  //maximum number of in-game bullets
#endif

#ifndef MAX_ON_SCREEN_LASERS
#define MAX_ON_SCREEN_LASERS 10    //maximum number of in-game lasers
#endif

#ifndef FRICTION
#define FRICTION 0.1f              //factor by which speeds of in-game moving objects constantly decrease
#endif

/*********************************************************

GLOBAL HELPER FUNCTIONS

**********************************************************/

//returns the distance between (x1, y1) and (x2, y2)
float Distance(float x1, float y1, float x2, float y2);

//returns the distance between p1 and p2
float Distance(POINT p1, POINT p2);
float Distance(XMFLOAT2 p1, POINT p2);
float Distance(POINT p1, XMFLOAT2 p2);
float Distance(XMFLOAT2 p1, XMFLOAT2 p2);

bool RectContains(RECT rect, POINT pt);
bool ButtonLeftClicked(RECT rect, InputClass* input);

/*********************************************************

TYPE DEFINITIONS

**********************************************************/

//enumeration for different game modes
typedef enum GameMode {
	MAIN_MENU = 0x00000001,
	VERSUS_MODE = 0x00000002,
	TUTORIAL_MODE = 0x00000003,
	CHARACTER_SELECT_MODE = 0x00000004
}GameMode;

//enumeration for playable characters
typedef enum PlayableCharacter {
	UNSELECTED = 0x00,
	REIMU = 0x01,
	MARISA = 0x02
}PlayableCharacter;

//enumeration of actions chosen by the player during Move Phase or Act Phase of the game
typedef enum Action {
	PENDING_CHOICE = 0x00,
	PASS = 0x01,
	SHOOT = 0x02,
	SPELL = 0x03,
	MOVE = 0x04
}Action;

//enumeration of flags for bullet's states
typedef enum BulletFlag {
	BULLET_ACTIVE = 0x00,
	BULLET_EXPLODING = 0x01
}BulletFlag;

//represents a visible button
typedef struct ButtonType {
	RECT buttonRect; //rectangular portion represented by button
	int bitmapID;    //ID for button's bitmap
}ButtonType;

//represents the player's available spellcards
typedef struct SpellCardType {
	char cardName[MAX_STR_LEN];//title of spellcard
	int mpCost;                        //amount of user's MP that will be required to use the spell
	char desc[MAX_STR_LEN];    //spell card description
}SpellCardType;

//represents an in-game bullet
typedef struct BulletType {
	XMFLOAT2 position; //bullet's current position
	XMFLOAT2 moveSpeed;//bullet's current velocity
	float moveAngle;   //direction(angle) of bullet's current velocity
	int damage;        //damage upon colliding on another object
	int bitmapID;    //pointer to bullet's in-game sprite bitmap
	BulletFlag state;  //flag for bullet states
}BulletType;

//represents a single particle of an in-game laser
typedef struct LaserParticleType {
	XMFLOAT2 posI;         //the start position of laser particle
	XMFLOAT2 posF;         //the end position of laser particle (where laser lands)
	XMFLOAT2 midPt;        //the midpoint of particle's start and end position
	RECT dimensions;       //x and y dimensions by which the particle will be stretched
	int* pBitmapID;        //pointer to ID of sprite bitmap
}LaserParticleType;

//represents an in-game laser
typedef struct LaserType {
	float angle;                 //direction(angle) of laser
	int damage;                  //damage upon colliding on another object
	LaserParticleType* particles;//heap array of laser entities
	int numParticles;            //number of laser entities
	int timerID;                 //duration for which laser is rendered
}LaserType;

//represents a player
//    Note: maybe this should be defined in separate class
typedef struct PlayerType {
	PlayableCharacter character;//character chosen by the player
	int inGameSpriteID;       //pointer to ID of in-game sprite bitmap
	int spriteClockID;          //pointer to ID of in-game sprite motion clock
	int inGameSpriteWidth;      //width of in-game sprite bitmap
	int inGameSpriteHeight;     //height of in-game sprite bitmap

	SpellCardType* spellCard;   //heap array of available spellcards for the chosen character
	unsigned int numSpellCards; //total number of available spellcards
	unsigned int maxHp;         //character's maximum HP
	int hp;            //character's current HP
	int mp;            //character's current MP
	float hitboxRadius;         //character's circular collision detection radius
								//	  Note: only Reimu will have a small hitbox
								//    radius though, unless I change my mind

	XMFLOAT2 position;          //character's current position
	XMFLOAT2 moveSpeed;         //character's current velocity vector
	float moveAngle;            //direction(angle) of character's current velocity
}PlayerType;

//represents a map
typedef struct MapType {
	RECT* rectWall;        //heap array of rectangular wall detection
	int numRectWall;       //number of rectangular walls
	int mapBitmapID;       //ID of map's bitmap
}MapType;

//represents a single versus match
//    Note: maybe I should create a separate class for this
class VersusMode {
private:
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
	int playerIndicatorBitmapID;             //ID of bitmap that displays currently active player
	int playerIndicatorClockID;              //ID of clock for displaying active player
	Action choice;                           //choice the player made during Move Phase or Act Phase
	ButtonType passButton;                   //Pass choice button
	ButtonType moveButton;                   //Move choice button
	ButtonType shootButton;                  //Shoot choice button
	ButtonType spellButton;                  //Spell choice button

	int spellNameSentID;                     //ID of sentence object that displays spell card names
	int spellDescSentID;                     //ID of sentence object that displays spell card description
	bool isSpellSelected;                    //whether spell card is selected
	int spellSelected;                       //index of the spell card selected
	ButtonType spellNameButton[5];           //buttons for showing list of individual spell names
	int spellDescBitmapID;                   //ID of bitmap for spell description box

	int reimuSpell01Bullet[3];               //array of IDs of bitmaps for orbs of "Fantasy Seal"
	int reimuSpell01BulletBg[3];             //array of IDs of bitmaps for backgrounds of "Fantasy Seal"

	int hit01BitmapID[19];                   //array of IDs of bitmaps for physical impact 1

	int statsWindowBitmapID;                 //ID of bitmap that displays character's status window
	int hpDispSentID;                        //ID of sentence object that displays character's current HP
	int mpDispSentID;                        //ID of sentence object that displays character's current MP

	XMFLOAT2* tempPos;                       //(heap)temporary variable for recording position
	XMFLOAT2* tempSpeed;                     //(heap)temporary variable for recording velocity
	float* tempAngle;                        //(heap)temporary variable for recording angle
	BulletType* tempBullet;                  //heap array of temporary bullets
	int tempBulletNum;                       //number of temporary bullets
	int* tempTimerID;                        //heap array of IDs of temporary timers
	int tempTimerIDNum;                      //number of IDs of temporary timers

	GraphicsClass* m_Graphics;
	TimeClass* m_Clock;
	InputClass* m_Input;
	int m_screenWidth;
	int m_screenHeight;

	//movement-related functions
	bool CollisionWithWall(XMFLOAT2 pos, float radius);
	bool CollisionWithCharacter(XMFLOAT2 pos, float radius, int& collidedChar);
	void Shoot(XMFLOAT2& pos, XMFLOAT2& speedVec, float& angle, float radius);
	bool Moving(XMFLOAT2& pos, XMFLOAT2& speedVec, float& angle, float radius);
	void InitializeTempPosSpeedAngle(float x, float y);

	//helper functions
	bool IsStationary(XMFLOAT2 speed);
	bool AllStationary(XMFLOAT2* speedList, int size);
	void NextPhase();

public:
	VersusMode();
	VersusMode(GraphicsClass* graphics, TimeClass* clock, InputClass* input, int screenWidth, int screenHeight, PlayableCharacter player1, PlayableCharacter player2);
	~VersusMode();

	bool Initialize();
	void Shutdown();
	bool Frame();
};

#endif