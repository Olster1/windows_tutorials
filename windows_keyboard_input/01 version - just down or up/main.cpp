#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>

enum PlatformKeyType {
    PLATFORM_KEY_UP,
    PLATFORM_KEY_DOWN,
    PLATFORM_KEY_RIGHT,
    PLATFORM_KEY_LEFT,
    
    // NOTE: Everything before here
    PLATFORM_KEY_TOTAL_COUNT
};

static bool global_keyDownStates[PLATFORM_KEY_TOTAL_COUNT];

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    if(msg == WM_KEYDOWN || msg == WM_KEYUP) {

        bool keyDown = (msg == WM_KEYDOWN);
        
        //NOTE: handle our keyboard input
        if(wparam == VK_UP) {        
            global_keyDownStates[PLATFORM_KEY_UP] = keyDown;

        } else if(wparam == VK_DOWN) {
            global_keyDownStates[PLATFORM_KEY_DOWN] = keyDown;

        } else if(wparam == VK_LEFT) {
           global_keyDownStates[PLATFORM_KEY_LEFT] = keyDown;

        } else if(wparam == VK_RIGHT) {
          global_keyDownStates[PLATFORM_KEY_RIGHT] = keyDown;
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
    	MSG message = {};
        while(PeekMessageW(&message, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        //NOTE: Use our global array to access the key down states 
        if(global_keyDownStates[PLATFORM_KEY_UP]) {
            //NOTE: Move Character forward
            OutputDebugStringA("Key Up Down\n");
        }

        if(global_keyDownStates[PLATFORM_KEY_DOWN]) {
            //NOTE: Move Character down
            OutputDebugStringA("Key Down Down\n");
        }

        if(global_keyDownStates[PLATFORM_KEY_LEFT]) {
            //NOTE: Move Character left
            OutputDebugStringA("Key Left Down\n");
        }

        if(global_keyDownStates[PLATFORM_KEY_RIGHT]) {
            //NOTE: Move Character right
            OutputDebugStringA("Key Right Down\n");
        }
    }
    

    return 0;

}