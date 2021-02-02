#include <cmath>
#include <windows.h>
#include <wingdi.h>
#include <windowsx.h> 
#include <commctrl.h>
#include <iostream>
#include <fstream>
#include "Globals.h"
#include "Timer.h"
#include "linterpolation.h"

#define ID_TRACKBAR 1000

HWND hwndButton1;
HWND hwndButton2;
HWND hwndTrackHr;

const char* className = "MyCLASSNAME!";

BOOL do_mouse_move = FALSE;

void wmCreate(HWND hWnd, WPARAM wp, LPARAM lp, int& trackPos)
{
	RECT Rect;
	GetClientRect(hWnd, &Rect);

	int ScrollBarWidth = 25;
	int ScrollBarHeight = 25;

	hwndButton1 = CreateWindow(
		"BUTTON",      
		"RECORD",     
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
		4 * globals::CLIENT_SIZE_X / 9,        
		4 * globals::CLIENT_SIZE_Y / 9,        
		globals::CLIENT_SIZE_X / 9,       
		globals::CLIENT_SIZE_Y / 9,      
		hWnd,                    
		NULL,                     
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL);

	hwndButton2 = CreateWindow(
		"BUTTON",    
		"PLAY",      
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  
		4 * globals::CLIENT_SIZE_X / 9,         
		3 * globals::CLIENT_SIZE_Y / 9,        
		globals::CLIENT_SIZE_X / 9,       
		globals::CLIENT_SIZE_Y / 9,       
		hWnd,                    
		NULL,                    
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL);

	hwndTrackHr = CreateWindowEx(
		0,                                      
		TRACKBAR_CLASS,                         
		"TrackBar",
		WS_CHILD |
		WS_VISIBLE |
		TBS_AUTOTICKS |
		TBS_ENABLESELRANGE | TBS_HORZ,					// style 
		3 * globals::CLIENT_SIZE_X / 9,					// x position 
		5 * globals::CLIENT_SIZE_Y / 9,					// y position 
		3 * globals::CLIENT_SIZE_X / 9,
		globals::CLIENT_SIZE_Y / 20,					// size 
		hWnd,											// parent window 
		(HMENU)ID_TRACKBAR,								// control identifier 
		(HINSTANCE)GetWindowLong(hWnd, GWLP_HINSTANCE),
		NULL											// no WM_CREATE parameter 
	);

	trackPos = 100;
	SendMessage(hwndTrackHr, TBM_SETRANGEMAX, 20, 500);
	SendMessage(hwndTrackHr, TBM_SETPOS, TRUE, trackPos);
}

void play(HWND hWnd, WPARAM wp, LPARAM lp, int trackPos)
{
	std::ifstream fIn;
	fIn.open("positionArray.txt");

	std::vector<dot> dots;

	int previousTime = 0;
	int x, y, time;
	while (!fIn.eof()) {
		fIn >> x >> y >> time;
		for (int i = 0; i < (time - previousTime) / (1000 / globals::FPS); i++) {////
			dots.emplace_back(x, y, time - ((time - previousTime) / (1000 / globals::FPS) - i) * (1000 / globals::FPS));
			previousTime = time;
		}
	}

	fIn.close();

	int i = 1;

	linterpolation inParam;
	Timer cycle;
	cycle.SetInterval(inParam, dots, (double)trackPos / 100);

}

void wmCommand(HWND hWnd, WPARAM wp, LPARAM lp, std::ofstream& fout, int& timer, int& elapsedUpdate, int trackPos)
{
	// see which button was clicked
	if ((HWND)lp == hwndButton2)
	{
		play(hWnd, wp, lp, trackPos);
	}
	else if ((HWND)lp == hwndButton1) {
		if (!do_mouse_move)
		{
			timer = clock();
			elapsedUpdate = clock();
			fout.open("positionArray.txt");
			Button_SetText(hwndButton1, "STOP");
			do_mouse_move = true;
		}
		else if (do_mouse_move)
		{
			Button_SetText(hwndButton1, "RECORD");
			do_mouse_move = false;
			fout.close();
		}
	}
}

void wmMouseMove(HWND hWnd, WPARAM wp, LPARAM lp, std::ofstream& fout, int& timer, int& elapsedUpdate)
{

	int elapsedTime = (int)clock() - timer;
	int elapsedUpdateTime = clock() - elapsedUpdate;
	if (!do_mouse_move)
		return;

	else if (elapsedUpdateTime > 1000 / globals::RECFPS) {

		POINT point;
		HWND desktowpHWND = GetDesktopWindow();

		ScreenToClient(hWnd, &point);
		GetCursorPos(&point);

		fout << point.x << ' ' << point.y << ' ' << elapsedTime << std::endl;

		elapsedUpdate = clock();
	}
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM  lParam)
{
	static std::ofstream fout;
	static int elapsedUpdate;
	static int timer = -1;
	static int speed = 1;
	static int trackPos;

	switch (Msg)
	{

	case WM_CREATE:   
		wmCreate(hWnd, wParam, lParam, trackPos);
		break;
	case WM_COMMAND:   
		wmCommand(hWnd, wParam, lParam, fout, timer, elapsedUpdate, trackPos);   
		break;
	case WM_MOUSEMOVE: 
		wmMouseMove(hWnd, wParam, lParam, fout, timer, elapsedUpdate);  
		break;

	case WM_HSCROLL:

	{
		switch ((int)LOWORD(wParam))
		{
		case TB_THUMBPOSITION:
		case TB_THUMBTRACK:
			trackPos = HIWORD(wParam);
			break;
		}

		return 0;

	}

	break;

	case WM_CLOSE:     
		DestroyWindow(hWnd);
		if (do_mouse_move == true) fout.close();        
			break;

	case WM_DESTROY:   
		PostQuitMessage(0);                              
		break;

	default: return (DefWindowProc(hWnd, Msg, wParam, lParam));

	}

	return 0;

}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//the (int main) for the WINAPI programs.
	WNDCLASSEXA wc = { 0 };
	HWND hWnd;
	MSG Msg = { 0 };


	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); 
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc; 
	wc.lpszClassName = className;
	wc.lpszMenuName = "MENU!";
	wc.style = CS_HREDRAW | CS_VREDRAW; 


	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "No window.", "Uh oh...", MB_ICONERROR | MB_OK);
		return -1;
	}

	//Create the window
	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		className,
		"Mouse recording",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		globals::CLIENT_SIZE_X,
		globals::CLIENT_SIZE_Y,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (hWnd == NULL)
	{
		MessageBox(NULL, "No window.", "Uh oh...", MB_ICONERROR | MB_OK);
		return -1; 
	}

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	while (GetMessage(&Msg, hWnd, 0, 0) > 0) 
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;

}