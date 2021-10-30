#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <stdint.h> //for the type uint8_t for our text input buffer
#include <assert.h>
#include <stdio.h>


enum PlatformKeyType {
    PLATFORM_KEY_NULL,
    PLATFORM_KEY_UP,
    PLATFORM_KEY_DOWN,
    PLATFORM_KEY_RIGHT,
    PLATFORM_KEY_LEFT,
    PLATFORM_KEY_X,
    PLATFORM_KEY_Z,

    PLATFORM_KEY_BACKSPACE,

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
#define PLATFORM_MAX_KEY_INPUT_BUFFER 16

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

    PlatformKeyType keyInputCommandBuffer[PLATFORM_MAX_KEY_INPUT_BUFFER];
    int keyInputCommand_count;

    WCHAR low_surrogate;
};

static PlatformInputState global_platformInput;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    //quit our program
    if(msg == WM_CLOSE || msg == WM_DESTROY) {
        PostQuitMessage(0);

    } if(msg == WM_CHAR) {
        
        //NOTE: Dont add backspace to the buffer
        if(wparam != VK_BACK) {

            WCHAR utf16_character = (WCHAR)wparam;

            int characterCount = 0;
            WCHAR characters[2];


            //NOTE: Build the utf-16 string
            if (IS_LOW_SURROGATE(utf16_character))
            {
                if (global_platformInput.low_surrogate != 0)
                {
                    // received two low surrogates in a row, just ignore the first one
                }
                global_platformInput.low_surrogate = utf16_character;
            }
            else if (IS_HIGH_SURROGATE(utf16_character))
            {
                if (global_platformInput.low_surrogate == 0)
                {
                    // received hight surrogate without low one first, just ignore it
                    
                }
                else if (!IS_SURROGATE_PAIR(utf16_character, global_platformInput.low_surrogate))
                {
                    // invalid surrogate pair, ignore the two pairs
                    global_platformInput.low_surrogate = 0;
                } 
                else 
                {
                    //NOTE: We got a surrogate pair. The string we convert to utf8 will be 2 characters long - 32bits not 16bits
                    characterCount = 2;
                    characters[0] = global_platformInput.low_surrogate;
                    characters[1] = utf16_character;

                }
            }
            else
            {
                if (global_platformInput.low_surrogate != 0)
                {
                    // expected high surrogate after low one, but received normal char
                    // accept normal char message (ignore low surrogate)
                }

                //NOTE: always add non-pair characters. The string will be one character long - 16bits
                characterCount = 1;
                characters[0] = utf16_character;

                global_platformInput.low_surrogate = 0;
            }

            if(characterCount > 0) {
            
                //NOTE: Convert the utf16 character to utf8

                //NOTE: Get the size of the utf8 character in bytes
                int bufferSize_inBytes = WideCharToMultiByte(
                  CP_UTF8,
                  0,
                  (LPCWCH)characters,
                  characterCount,
                  (LPSTR)global_platformInput.textInput_utf8, 
                  0,
                  0, 
                  0
                );

                //NOTE: See if we can still fit the character in our buffer. We don't do <= to the max buffer size since we want to keep one character to create a null terminated string.
                if((global_platformInput.textInput_bytesUsed + bufferSize_inBytes) < PLATFORM_MAX_TEXT_BUFFER_SIZE_IN_BYTES) {
                        
                    //NOTE: Add the utf8 value of the character to our buffer
                    int bytesWritten = WideCharToMultiByte(
                      CP_UTF8,
                      0,
                      (LPCWCH)characters,
                      characterCount,
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

                global_platformInput.low_surrogate = 0;
            }
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

        bool addToCommandBuffer = false;

        //NOTE: match our internal key names to the vk code
        if(vk_code == VK_UP) { 
            keyType = PLATFORM_KEY_UP;
        } else if(vk_code == VK_DOWN) {
            keyType = PLATFORM_KEY_DOWN;
        } else if(vk_code == VK_LEFT) {
            keyType = PLATFORM_KEY_LEFT;

            //NOTE: Also add the message to our command buffer if it was a KEYDOWN message
            addToCommandBuffer = keyIsDown;
        } else if(vk_code == VK_RIGHT) {
            keyType = PLATFORM_KEY_RIGHT;

            //NOTE: Also add the message to our command buffer if it was a KEYDOWN message
            addToCommandBuffer = keyIsDown;
        } else if(vk_code == 'Z') {
            keyType = PLATFORM_KEY_Z;
        } else if(vk_code == 'X') {
            keyType = PLATFORM_KEY_X;
        } else if(vk_code == VK_BACK) {
            keyType = PLATFORM_KEY_BACKSPACE;

            //NOTE: Also add the message to our command buffer if it was a KEYDOWN message
            addToCommandBuffer = keyIsDown;
        }

        //NOTE: Add the command message here 
        if(addToCommandBuffer && global_platformInput.keyInputCommand_count < PLATFORM_MAX_KEY_INPUT_BUFFER) {
            global_platformInput.keyInputCommandBuffer[global_platformInput.keyInputCommand_count++] = keyType;
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

    //NOTE: Create a input buffer to store text input across frames.
    #define MAX_INPUT_BUFFER_SIZE 128
    int textBuffer_count = 0;
    uint8_t textBuffer[MAX_INPUT_BUFFER_SIZE] = {};

    int cursorAt = 0;  //NOTE: Where our cursor position is
    //////////////////////////////////////////////////////////////


    bool running = true;

    while(running) {

        //NOTE: Clear the input text buffer to empty
        global_platformInput.textInput_bytesUsed = 0;
        global_platformInput.textInput_utf8[0] = '\0';

        //NOTE: Clear our input command buffer
        global_platformInput.keyInputCommand_count = 0;

        
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

        //NOTE: Find the smallest size we can add to the buffer without overflowing it
        int bytesToMoveAboveCursor = global_platformInput.textInput_bytesUsed;
        int spaceLeftInBuffer = (MAX_INPUT_BUFFER_SIZE - textBuffer_count - 1); //minus one to put a null terminating character in
        if(bytesToMoveAboveCursor > spaceLeftInBuffer) {
            bytesToMoveAboveCursor = spaceLeftInBuffer;
        }

        //NOTE: Get all characters above cursor and save them in a buffer
        char tempBuffer[MAX_INPUT_BUFFER_SIZE] = {};
        int tempBufferCount = 0;
        for(int i = cursorAt; i < textBuffer_count; i++) {
            tempBuffer[tempBufferCount++] = textBuffer[i];
        }

        //NOTE: Copy new string into the buffer
        for(int i = 0; i < bytesToMoveAboveCursor; ++i) {
            textBuffer[cursorAt + i] = global_platformInput.textInput_utf8[i];
        }
        
        //NOTE: Advance the cursor and the buffer count
        textBuffer_count += bytesToMoveAboveCursor;
        cursorAt += bytesToMoveAboveCursor;

        //NOTE: Replace characters above the cursor that we would have written over
        for(int i = 0; i < tempBufferCount; ++i) {
            textBuffer[cursorAt + i] = tempBuffer[i]; 
        }


        //NOTE: Process our command buffer
        for(int i = 0; i < global_platformInput.keyInputCommand_count; ++i) {
            PlatformKeyType command = global_platformInput.keyInputCommandBuffer[i];
            if(command == PLATFORM_KEY_BACKSPACE) {
                
                //NOTE: can't backspace a character if cursor is in front of text
                if(cursorAt > 0 && textBuffer_count > 0) {
                    //NOTE: Move all characters in front of cursor down
                    int charactersToMoveCount = textBuffer_count - cursorAt;
                    for(int i = 0; i < charactersToMoveCount; ++i) {
                        int indexInFront = cursorAt + i;
                        assert(indexInFront < textBuffer_count); //make sure not buffer overflow
                        textBuffer[cursorAt + i - 1] = textBuffer[indexInFront]; //get the one in front 
                    }

                    cursorAt--;
                    textBuffer_count--;
                }
                
            }

            if(command == PLATFORM_KEY_LEFT) {
                //NOTE: Move cursor left 
                if(cursorAt > 0) {
                    cursorAt--;
                }
            }

            if(command == PLATFORM_KEY_RIGHT) {
                //NOTE: Move cursor right 
                if(cursorAt < textBuffer_count) {
                    cursorAt++;
                }
            }       
        }  

        //NOTE: put in a null terminating character at the end
        assert(textBuffer_count < MAX_INPUT_BUFFER_SIZE);
        textBuffer[textBuffer_count] = '\0';  

        OutputDebugStringA((LPCSTR)textBuffer);

        OutputDebugStringA("\n");

        char cursorAtBuffer[16];
        sprintf(cursorAtBuffer, "cursorAt: %d\n", cursorAt);
        OutputDebugStringA((LPCSTR)cursorAtBuffer);        

        
            

        //NOTE: Sleep for a bit
        Sleep(200);

    }
    

    return 0;

}