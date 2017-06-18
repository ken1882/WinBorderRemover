#include <iostream>
#include <windows.h>
#include <map>
#include <cstring>
#include <conio.h>
#include <iomanip>

std::string Version = "1.1.0";

// Constant global variables initialization
const unsigned short FPS = 60;          // update/frame per second
const unsigned short StatePause   = 0;  /*----------------------------*/
const unsigned short StateChange  = 1;  /* Application status control */
                                        /*----------------------------*/

// Global Variables initialization
bool running = true;                        // Terminated flag
unsigned short last_state    = 0xffff;      // Previous application stage
unsigned short app_state     = StatePause;  // Initialize to main stage
unsigned short keystate[256] = {0};         // Keyboard event sensor
std::map<HWND, LONG> ori_winstyle;          // Window Style with title bar
std::map<HWND, RECT> ori_winsize;           // Window Size
std::map<HWND, bool> removed;               // Border removed flag

tagPOINT MousePos;
tagPOINT LastMousePos;

// Win32 Window Handler
HWND hwnd;      // Current focused window
HWND hthis;     // Application handler
HWND last_hwnd; // Last focused window

// Keyboard event detection
void update_key(){
    memset(keystate, 0, sizeof(keystate));
    for(int i=0;i<0xff;i++){
        keystate[i] = GetAsyncKeyState(i);
        if(keystate[VK_ESCAPE] && (hwnd == hthis || keystate[VK_SHIFT])){
            running = false;
            break;
        }
    }
}

// Output current phase message
void output_info(std::string info){
    if(last_state != app_state){
        std::cout << info;
        last_state = app_state;
        memset(keystate, 0, sizeof(keystate));
        Sleep(300);
    }
}

void detect_window_size(){
    if(removed[hwnd])return ;
    RECT rect;
    GetWindowRect(hwnd, &rect);
    ori_winstyle[hwnd] = GetWindowLongPtr(hwnd, GWL_STYLE);
    ori_winsize[hwnd]  = rect;
}

void remove_border(bool reversed, HWND _hwnd = hwnd){
    LONG new_style = ori_winstyle[_hwnd];
    auto rect = ori_winsize[_hwnd];
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    if(!reversed){
        new_style &= ~(WS_CAPTION |WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
        new_style &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
        width  -= 6;
        height -= 29;
    }
    ShowWindow(_hwnd, SW_HIDE);
    SetWindowLongPtr(_hwnd, GWL_STYLE, new_style);
    memset(keystate, 0, sizeof(keystate));
    std::cout << "Succeed!\n";
    Sleep(300);
    //ShowWindow(_hwnd, SW_SHOWDEFAULT);
    SetWindowPos(_hwnd, NULL, 0,0, width, height, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

void update_dragging(){
    GetCursorPos(&MousePos);
    if(keystate[VK_CONTROL] && removed[hwnd]){
        int deltaX = MousePos.x - LastMousePos.x, deltaY = MousePos.y - LastMousePos.y;
        RECT rect;
        GetWindowRect(hwnd, &rect);
        SetWindowPos(hwnd, NULL, rect.left + deltaX, rect.top + deltaY, 0, 0, SWP_NOREDRAW | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    }
    LastMousePos = MousePos;
}

void change_winstyle(){
    output_info("\nPress F8 to exit change phase, F9 to remove border, F10 to restore.\n");
    if(hwnd != last_hwnd){
        last_hwnd = hwnd;
        char buffer[0xff] = {0};
        GetWindowText(hwnd, buffer, 0xff);
        std::cout << "You are editing the border of \"" << buffer << '\"' << '\n';
    }

    if(keystate[VK_F9]){
        if(!removed[hwnd]){
            detect_window_size();
            remove_border(false);
            removed[hwnd] = true;
        }
        else std::cout << "Border already removed!\n";
    }
    else if(keystate[VK_F10]){
        if(removed[hwnd]){
            remove_border(true);
            removed[hwnd] = false;
        }
        else std::cout << "Border already restored!\n";
    }
    update_dragging();
}

// Main action update
void update_action(){
    output_info("\nPress F8 to enter change phase, Shift + ESC to exit.\n");
    if(keystate[VK_F8]){
        app_state ^= 1;
        detect_window_size();
        Sleep(300);
    }
}

// Main update
void update(){
    update_key();
    update_action();

    switch(app_state){
    case StateChange:
        change_winstyle();
        break;
    default:
        app_state = 0;
    }
}

void restore_all_window(){
    for(auto i = ori_winstyle.begin(); i != ori_winstyle.end();i++){
        remove_border(true, i -> first);
    }
}

// Main entry process
int main(int argc, char* argv[]){
    hthis = GetForegroundWindow();
    const int uwait = 1000/FPS;
    std::cout << "\nVersion: " << Version << '\n';
    std::cout << "*---------------------------------> Information <---------------------------------*\n";
    std::cout << "* This application can remove other application's Windows default window border.  *\n";
    std::cout << "* Leave this application work in the background, once you closed this, all window *\n";
    std::cout << "* you edited will automatically restore.                                          *\n";
    std::cout << "*---------------------------------------------------------------------------------*\n";
    std::cout << "*                                 > User Guide  <                                 *\n";
    std::cout << "*---------------------------------------------------------------------------------*\n";
    std::cout << "* Just switch to the window you'd like to remove the border, then press F8 to     *\n";
    std::cout << "* start editing, meanwhile, you'll see the title of the window you selected in    *\n";
    std::cout << "* this application.                                                               *\n";
    std::cout << "* Then, press F9 to remove its border, F10 to restore, press CTRL + Mouse to drag *\n";
    std::cout << "* the window while the border is removed.                                         *\n";
    std::cout << "* Once you done, to prevent F9/F10 is also used in the app, press F8 to exit to   *\n";
    std::cout << "* previous status of this app.                                                    *\n";
    std::cout << "*---------------------------------------------------------------------------------*\n";
    while(running){
        hwnd = GetForegroundWindow();
        update();
        Sleep(uwait);
    }

    std::cout << "Restoring all windows..." << std::endl;
    restore_all_window();
    system("pause");
    return 0;
}
