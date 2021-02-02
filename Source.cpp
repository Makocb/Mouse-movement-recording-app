#include <vector>
#include <cmath>

#include <windows.h>
#include <wingdi.h>
#include <windowsx.h> // for changing the button text

#include <iostream>
#include <fstream>


HWND hwndButton1;
HWND hwndButton2;

const int WINDOW_POS_X = 0;
const int WINDOW_POS_Y = 0;
const int CLIENT_SIZE_X = 1000;
const int CLIENT_SIZE_Y = 1000;
const int MSperFrame = 20;
const COLORREF COLOR_BACK = RGB(0, 0, 80);
const COLORREF COLOR_FORE = RGB(200, 200, 50);

const int RECFPS =60;
const int FPS = 60;

const char* className = "MyCLASSNAME!";

BOOL do_mouse_move = FALSE;

struct dot 
{
    int x, y, time;
    dot(int a, int b, int t) 
    {
        x = a;
        y = b;
        time = t;
    }
};

void wmCreate(HWND hWnd, WPARAM wp, LPARAM lp)
{
        hwndButton1 = CreateWindow(
            "BUTTON",      // Predefined class; Unicode assumed 
            "RECORD",      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
            4 * CLIENT_SIZE_X / 9,         // x position 
            4 * CLIENT_SIZE_Y / 9,         // y position 
            CLIENT_SIZE_X / 9,        // Button width
            CLIENT_SIZE_Y / 9,        // Button height
            hWnd,                     // Parent window
            NULL,                     // No menu.
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);

        hwndButton2 = CreateWindow(
            "BUTTON",    // Predefined class; Unicode assumed 
            "PLAY",      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
            4 * CLIENT_SIZE_X / 9,         // x position 
            3 * CLIENT_SIZE_Y / 9,         // y position 
            CLIENT_SIZE_X / 9,        // Button width
            CLIENT_SIZE_Y / 9,        // Button height
            hWnd,                     // Parent window
            NULL,                     // No menu.
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);
}

void play(HWND hWnd, WPARAM wp, LPARAM lp)
{
    std::ifstream fIn;
    fIn.open("positionArray.txt");

    std::vector<dot> dots;

    int x, y, time;
    while(!fIn.eof()) {
        fIn >> x >> y >> time;
        dots.emplace_back(x, y, time);
    }

    fIn.close();

    int i = 1;
    
    auto interpolation = [](dot d1, dot d2, int num,int k)
    {
        SetCursorPos((num * d1.x + (k*FPS / RECFPS - num)*d2.x) / (k*FPS / RECFPS), (num * d1.y + (k*FPS / RECFPS-num)*d2.y) / (k*FPS / RECFPS));
        Sleep(1);
    };

    int lastUpdate = clock();
    int timer = clock();
    int elapsedTime = 0;
    int elapsedTimer = 0;

    while(i< dots.size()-1)
    {
        elapsedTime = clock() - lastUpdate;

        if ((dots[i].time > elapsedTimer)&&(elapsedTime >=1000/FPS))
        {
            if (((dots[i].time > elapsedTimer)) && ((dots[i - 1].time < elapsedTimer)))
            {
                int k = 2;
                for (int j = 1; j < k *(FPS / RECFPS)-1; j++)
                    interpolation(dots[i - 1], dots[i], j, k);
            }
            
            SetCursorPos(dots[i].x, dots[i].y);

            lastUpdate = clock();
        }
        else if (dots[i].time <= elapsedTimer) {
            while((dots[i].time <= elapsedTimer)&&(i < dots.size()-1))
            i++;
        }
        elapsedTimer = clock() - timer;
    }
}

void wmCommand(HWND hWnd, WPARAM wp, LPARAM lp, std::ofstream& fout, int& timer, int& elapsedUpdate)
{
        // see which button was clicked
        if ((HWND)lp == hwndButton2)
        {
            play(hWnd, wp, lp);
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
  
    int elapsedTime = (int)clock()- timer ;
    int elapsedUpdateTime = clock() - elapsedUpdate;
    if (!do_mouse_move) 
        return;
    
    else if (elapsedUpdateTime > 1000/ RECFPS) {
        
        POINT point;
        HWND desktowpHWND = GetDesktopWindow();
        
        ScreenToClient(hWnd, &point);
        GetCursorPos(&point);
        
        //for (int i = 1; i < (int)(1+elapsedTime/40); i++)
        fout << point.x << ' ' << point.y <<' '<<elapsedTime<<std::endl;
        
        elapsedUpdate = clock();
    }
}

LRESULT CALLBACK WndProc(HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam)
{   
    static std::ofstream fout;
    static int elapsedUpdate;
    static int timer = -1;
    switch (Msg)
    {

    case WM_CREATE:    wmCreate(hWnd, wParam, lParam);                  break;
    case WM_COMMAND:   wmCommand(hWnd, wParam, lParam, fout, timer, elapsedUpdate);    break;
    case WM_MOUSEMOVE: wmMouseMove(hWnd, wParam, lParam, fout, timer, elapsedUpdate);  break;
    case WM_PAINT:                                                      break;
   
    case WM_CLOSE:     DestroyWindow(hWnd);               
                       if (do_mouse_move == true) fout.close();         break;

    case WM_DESTROY:   PostQuitMessage(0);                              break;

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

    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;//CreateSolidBrush(RGB(/*RGB: Red, Green, Blue.*/ 200, 200, 200));

    wc.hCursor = LoadCursor(NULL, IDC_ARROW); //The cursor

    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); //The icon

    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    wc.hInstance = hInstance;

    wc.lpfnWndProc = WndProc; //The window procedure!

    wc.lpszClassName = className;

    wc.lpszMenuName = "MENU!";

    wc.style = CS_HREDRAW | CS_VREDRAW; //The window style.


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
        CLIENT_SIZE_X,
        CLIENT_SIZE_Y,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (hWnd == NULL)
    {
        MessageBox(NULL, "No window.", "Uh oh...", MB_ICONERROR | MB_OK);
        return -1; //Cancel window creation.
    }

    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);

    while (GetMessage(&Msg, hWnd, 0, 0) > 0) //Get the message.
    {
        TranslateMessage(&Msg); //Translate it
        DispatchMessage(&Msg);
    }

    return Msg.wParam;

}