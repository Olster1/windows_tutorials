#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <stdint.h> //for the type uint8_t for our text input buffer
#include <assert.h>


enum PlatformKeyType {
    PLATFORM_KEY_NULL,
    PLATFORM_KEY_UP,
    PLATFORM_KEY_DOWN,
    PLATFORM_KEY_RIGHT,
    PLATFORM_KEY_LEFT,
    PLATFORM_KEY_X,
    PLATFORM_KEY_Z,

    PLATFORM_MOUSE_LEFT_BUTTON,
    PLATFORM_MOUSE_RIGHT_BUTTON,
    
    // NOTE: Everything before here
    PLATFORM_KEY_TOTAL_COUNT
};

struct PlatformKeyState {
    bool isDown;
    int pressedCount;
    int releasedCount;
};

#define PLATFORM_MAX_TEXT_BUFFER_SIZE_IN_BYTES 256

struct PlatformInputState {

    PlatformKeyState keyStates[PLATFORM_KEY_TOTAL_COUNT]; 

    //NOTE: Mouse data
    float mouseX;
    float mouseY;
    float mouseScrollX;
    float mouseScrollY;

    //NOTE: Text Input
    uint8_t textInput_utf8[PLATFORM_MAX_TEXT_BUFFER_SIZE_IN_BYTES];
    int textInput_bytesUsed;
};

static PlatformInputState global_platformInput;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    //quit our program
    if(msg == WM_CLOSE || msg == WM_DESTROY) {
        PostQuitMessage(0);

    } if(msg == WM_CHAR) {
        WPARAM utf16_character = wparam;

        //NOTE: Convert the utf16 character to utf8

        //NOTE: Get the size of the utf8 character in bytes
        int bufferSize_inBytes = WideCharToMultiByte(
          CP_UTF8,
          0,
          (LPCWCH )&utf16_character,
          1, //NOTE: character not null terminated, so specify it's only one character long
          (LPSTR)global_platformInput.textInput_utf8, 
          0,
          0, 
          0
        );

        // //NOTE: See if we can still fit the character in our buffer. We don't do <= to the max buffer size since we want to keep one character to create a null terminated string.
        if((global_platformInput.textInput_bytesUsed + bufferSize_inBytes) < PLATFORM_MAX_TEXT_BUFFER_SIZE_IN_BYTES) {
                
            //NOTE: Add the utf8 value of the character to our buffer
            int bytesWritten = WideCharToMultiByte(
              CP_UTF8,
              0,
              (LPCWCH )&utf16_character,
              1,
              (LPSTR)(global_platformInput.textInput_utf8 + global_platformInput.textInput_bytesUsed), 
              bufferSize_inBytes,
              0, 
              0
            );

            //NOTE: Increment the buffer size
            global_platformInput.textInput_bytesUsed += bufferSize_inBytes;

            //NOTE: Make the string null terminated
            assert(bufferSize_inBytes < PLATFORM_MAX_TEXT_BUFFER_SIZE_IN_BYTES);
            global_platformInput.textInput_utf8[global_platformInput.textInput_bytesUsed] = '\0';
        }

    } else if(msg == WM_LBUTTONDOWN) {
        if(!global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].isDown) {
            global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].pressedCount++;
        }
        
        global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].isDown = true;

    } else if(msg == WM_LBUTTONUP) {
        global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].releasedCount++;
        global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].isDown = false;

    } else if(msg == WM_RBUTTONDOWN) {
        if(!global_platformInput.keyStates[PLATFORM_MOUSE_RIGHT_BUTTON].isDown) {
            global_platformInput.keyStates[PLATFORM_MOUSE_RIGHT_BUTTON].pressedCount++;
        }
        
        global_platformInput.keyStates[PLATFORM_MOUSE_RIGHT_BUTTON].isDown = true;
    } else if(msg == WM_RBUTTONUP) {
        global_platformInput.keyStates[PLATFORM_MOUSE_RIGHT_BUTTON].releasedCount++;
        global_platformInput.keyStates[PLATFORM_MOUSE_RIGHT_BUTTON].isDown = false;

    } else if(msg == WM_MOUSEWHEEL) {
        //NOTE: We use the HIWORD macro defined in windows.h to get the high 16 bits
        short wheel_delta = HIWORD(wparam);
        global_platformInput.mouseScrollY = (float)wheel_delta;

    } else if(msg == WM_MOUSEHWHEEL) {
        //NOTE: We use the HIWORD macro defined in windows.h to get the high 16 bits
        short wheel_delta = HIWORD(wparam);
        global_platformInput.mouseScrollX = (float)wheel_delta;

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

            global_platformInput.keyStates[keyType].pressedCount += wasPressed;
            global_platformInput.keyStates[keyType].releasedCount += wasReleased;

            global_platformInput.keyStates[keyType].isDown = keyIsDown;
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

        //NOTE: Clear the input text buffer to empty
        global_platformInput.textInput_bytesUsed = 0;
        global_platformInput.textInput_utf8[0] = '\0';

        
        //NOTE: Clear the key pressed and released count before processing our messages
        for(int i = 0; i < PLATFORM_KEY_TOTAL_COUNT; ++i) {
            global_platformInput.keyStates[i].pressedCount = 0;
            global_platformInput.keyStates[i].releasedCount = 0;
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

        //NOTE: Get mouse position
        {  
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(hwnd, &mouse);
            global_platformInput.mouseX = (float)(mouse.x);
            global_platformInput.mouseY = (float)(mouse.y);
        }

        OutputDebugStringA((LPCSTR)global_platformInput.textInput_utf8);
        

        //NOTE: Sleep for a bit
        Sleep(200);

    }
    

    return 0;

}