#include <windows.h>
#include <shlwapi.h>
#include <assert.h>

#include <stdint.h> //for the integer types were using 
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
//////////////////////////////////////////

static u8 *getExePath_as_utf8() {
	u8 *resourcePath_as_utf8 = 0;

#if _WIN32
	WCHAR resourcesPath[MAX_PATH]; //NOTE: Use WCHAR type to be unicode compatible 
	int sizeOfPath = GetModuleFileNameW(NULL, resourcesPath, ARRAYSIZE(resourcesPath));

	//NOTE: Assert if function failed
	if(sizeOfPath == 0) {
	    assert(!"get module file name failed");
	}

	PathRemoveFileSpecW(resourcesPath);
	PathAppendW(resourcesPath, L"resources\\");


	///////////////////////************* Convert the string now to utf8 ************////////////////////

	int bufferSize_inBytes = WideCharToMultiByte(
	  CP_UTF8,
	  0,
	  resourcesPath,
	  -1,
	  (LPSTR)resourcePath_as_utf8, 
	  0,
	  0, 
	  0
	);


	resourcePath_as_utf8 = (u8 *)malloc(bufferSize_inBytes);

	u32 bytesWritten = WideCharToMultiByte(
	  CP_UTF8,
	  0,
	  resourcesPath,
	  -1,
	  (LPSTR)resourcePath_as_utf8, 
	  bufferSize_inBytes,
	  0, 
	  0
	);

	assert(bytesWritten == bufferSize_inBytes);

#endif

	return resourcePath_as_utf8;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
	u8 *string = getExePath_as_utf8();
	OutputDebugStringA((LPCSTR)string);
}