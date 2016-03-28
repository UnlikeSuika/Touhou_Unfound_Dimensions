/*

To-do list or reminders

*****next TODO*****

- start working on individual spellcards
- the player should win when the HP of the opponent is decreased to 0 or below
    - change unsigned character health to signed

- more elaborate sprite graphics
    - animation for moving and shooting
	- in title screen, move that "start" button to somewhere much less distracting


*****distant future*****

- tutorial mode
    - introductory video

- options mode
	- add full-screen feature later?

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