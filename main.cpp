#include "SystemClass.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdShow){

	SystemClass* System = new SystemClass; //main SystemClass object
	if (!System){
		MessageBox(NULL, L"Could not create system object.", L"Error", MB_OK);
		return 1;
	}
	if (System->Initialize()){
		System->Run();     //System will run every frame until WM_QUIT message is received
	}
	System->Shutdown();  
	delete System;

	System = NULL;
	return 0;
}