#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <stdio.h>

enum PlatformKeyType {
    PLATFORM_KEY_NULL,
    PLATFORM_KEY_UP,
    PLATFORM_KEY_DOWN,
    PLATFORM_KEY_RIGHT,
    PLATFORM_KEY_LEFT,
    PLATFORM_KEY_X,
    PLATFORM_KEY_Z,
    
    // NOTE: Everything before here
    PLATFORM_KEY_TOTAL_COUNT
};

struct PlatformKeyState {
    bool isDown;
    int pressedCount;
    int releasedCount;
};

static PlatformKeyState global_keyDownStates[PLATFORM_KEY_TOTAL_COUNT];

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    //quit our program
    if(msg == WM_CLOSE || msg == WM_DESTROY) {
        PostQuitMessage(0);

    } else if(msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) {

        bool keyWasDown = ((lparam & (1 << 30)) == 0);
        bool keyIsDown =   !(lparam & (1 << 31));

        WPARAM vk_code = wparam;        

        PlatformKeyType keyType = PLATFORM_KEY_NULL; 

        //NOTE: match our internal key names to the vk code
        if(vk_code == VK_UP) { 
            keyType = PLATFORM_KEY_UP;
        } else if(vk_code == VK_DOWN) {
            keyType = PLATFORM_KEY_DOWN;
        } else if(vk_code == VK_LEFT) {
            keyType = PLATFORM_KEY_LEFT;
        } else if(vk_code == VK_RIGHT) {
            keyType = PLATFORM_KEY_RIGHT;
        } else if(vk_code == 'Z') {
            keyType = PLATFORM_KEY_Z;
        } else if(vk_code == 'X') {
            keyType = PLATFORM_KEY_X;
        }


        //NOTE: Key pressed, is down and release events  
        if(keyType != PLATFORM_KEY_NULL) {
            int wasPressed = (keyIsDown && !keyWasDown) ? 1 : 0;
            int wasReleased = (!keyIsDown) ? 1 : 0;

            global_keyDownStates[keyType].pressedCount += wasPressed;
            global_keyDownStates[keyType].releasedCount += wasReleased;

            global_keyDownStates[keyType].isDown = keyIsDown;
        }

    } else {
        result = DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return result;
} 

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
    // Open a window
    HWND hwnd;
    {	
    	//First register the type of window we are going to create
        WNDCLASSEXW winClass = {};
        winClass.cbSize = sizeof(WNDCLASSEXW);
        winClass.style = CS_HREDRAW | CS_VREDRAW;
        winClass.lpfnWndProc = &WndProc;
        winClass.hInstance = hInstance;
        winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
        winClass.hCursor = LoadCursorW(0, IDC_ARROW);
        winClass.lpszClassName = L"Keyboard Input";
        winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);

        if(!RegisterClassExW(&winClass)) {
            MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }

        //Now create the actual window
        RECT initialRect = { 0, 0, 960, 540 };
        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
        LONG initialWidth = initialRect.right - initialRect.left;
        LONG initialHeight = initialRect.bottom - initialRect.top;

        hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                                winClass.lpszClassName,
                                L"Keyboard Input",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                initialWidth, 
                                initialHeight,
                                0, 0, hInstance, 0);

        if(!hwnd) {
            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }
    }

    bool running = true;

    while(running) {
        
        //NOTE: Clear the key pressed and released count before processing our messages
        for(int i = 0; i < PLATFORM_KEY_TOTAL_COUNT; ++i) {
            global_keyDownStates[i].pressedCount = 0;
            global_keyDownStates[i].releasedCount = 0;
        }

    	MSG message = {};
        while(PeekMessageW(&message, 0, 0, 0, PM_REMOVE)) {
            //NOTE: Handle any quit messages
            if(message.message == WM_QUIT) {
                running = false;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        //NOTE: Cast the spell the number of times the user pressed the key in the last frame
        for(int i = 0; i < global_keyDownStates[PLATFORM_KEY_Z].pressedCount; ++i) {
            char buffer[256];
            sprintf(buffer, "Cast Spell %d\n", i);
            OutputDebugStringA(buffer);
        }

        if(global_keyDownStates[PLATFORM_KEY_Z].releasedCount > 0) {
            char buffer[256];
            sprintf(buffer, "Release Count %d\n", global_keyDownStates[PLATFORM_KEY_Z].releasedCount);
            OutputDebugStringA(buffer);
        }



        //NOTE: Sleep for a bit to demonstrate multiple key presses 
        Sleep(200); //200 milliseconds or 5fps is actually long enough to register more than one key press in a frame.

    }
    

    return 0;

}