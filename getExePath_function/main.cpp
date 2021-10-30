#define UNICODE
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
	const WCHAR executablePath[MAX_PATH + 1]; //NOTE: Use WCHAR type to be unicode compatible 

	DWORD size_of_executable_path =
	            GetModuleFileNameW(0, executablePath, sizeof(executablePath)); //NOTE: Use the wide version of the function to be unicode compatible

	//NOTE: Assert if function failed
	if(size_of_executable_path == 0) {
	    assert(!"get module file name failed");
	}

	WCHAR resourcesPath[MAX_PATH + 1]; //NOTE: Use WCHAR type to be unicode compatible 

	WCHAR stringToAppend = L"resources\\"; //NOTE: The L converts the string to a wide string (utf-16) 

	memcpy(resourcesPath, executablePath, size_of_executable_path*sizeof(WCHAR)); //NOTE: Copy over the exe string so we don't alter the executable path

	//NOTE: Find the last backslash in the path
	WCHAR *one_past_last_slash = resourcesPath;
	for(int i = 0; size_of_executable_path; ++i) {

	    if(resourcesPath[i] == L'\\') {
	        one_past_last_slash = resourcesPath + i + 1;
	    }
	}

	//NOTE: Loop through the string to append and add it to the buffer
	WCHAR *at = stringToAppend;

	int charIndex = 0;
	while(*at) {
	        
	    one_past_last_slash[charIndex] = *at;

	    assert(((one_past_last_slash + charIndex) - resourcesPath) <= MAX_PATH*sizeof(WCHAR)); //NOTE: Make sure we have overwritten the length of the buffer

	    charIndex++;
	    at++;
	}


	///////////////////////************* Convert the string now to utf8 ************////////////////////

	int bufferSize_inBytes = WideCharToMultiByte(
	  CP_UTF8,
	  0,
	  path,
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
	  path,
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