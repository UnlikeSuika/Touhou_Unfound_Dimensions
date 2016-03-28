/*

To-do list or reminders

- Marisa's basic attacks: laser
	- extend particle bitmaps from where it starts (Marisa) to where it ends (where it hits a wall, character, etc)

- start working on individual spellcards
- the player should win when the HP of the opponent is decreased to 0 or below

- more elaborate sprite graphics
- tutorial mode
- options mode
	- add full-screen feature later?

- more comments and code proofreads

- add three+ player features

*/

#include "SystemClass.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdShow){
	SystemClass* System = new SystemClass; //main SystemClass object
	if (!System){
		MessageBox(NULL, L"Could not create system object.", L"Error", MB_OK);
		return 0;
	}
	if (System->Initialize()){
		System->Run();     //System will run every frame until WM_QUIT message is received
	}
	System->Shutdown();  
	delete System;
	System = NULL;
	return 0;
}