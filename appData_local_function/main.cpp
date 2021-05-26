#define UNICODE
#include <assert.h>
#include <Shlobj.h> //for SHGetKnownFolderPath


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


static u8 *getSaveFileLocation() {
	u8 *result = 0;
#if _WIN32
	PWSTR  win32_wideString_utf16 = 0; 

	//NOTE(ollie): Get the folder name
	if(SHGetKnownFolderPath(
	  FOLDERID_SavedGames,
	  KF_FLAG_CREATE,
	  0,
	  (PWSTR *)&win32_wideString_utf16
	) != S_OK) {
		assert(false);
		return result;
	}

	///////////////////////************* Convert the string now to utf8 ************////////////////////

	int bufferSize_inBytes = WideCharToMultiByte(
	  CP_UTF8,
	  0,
	  win32_wideString_utf16,
	  -1,
	  (LPSTR)result, 
	  0,
	  0, 
	  0
	);


	result = (u8 *)malloc(bufferSize_inBytes);

	u32 bytesWritten = WideCharToMultiByte(
	  CP_UTF8,
	  0,
	  win32_wideString_utf16,
	  -1,
	  (LPSTR)result, 
	  bufferSize_inBytes,
	  0, 
	  0
	);

	assert(bytesWritten == bufferSize_inBytes);

	//NOTE(ollie): Free the string
	CoTaskMemFree(win32_wideString_utf16);
#endif

	return result;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
	u8 *string = getSaveFileLocation();
	OutputDebugStringA((LPCSTR)string);
}