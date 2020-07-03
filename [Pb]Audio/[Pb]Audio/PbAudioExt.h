#ifndef PB_AUDIO_EXT_H
#define PB_AUDIO_EXT_H


#ifdef __cplusplus
extern "C" {
#endif


//Win32 c based dependencies
#include <tchar.h>              // generic text character mapping
#include <string.h>             // includes string manipulation routines
#include <stdlib.h>           // includes standard libraries
#include <stdio.h>              // includes standard input/output routines
#include <process.h>            // threading routines for the CRT

// standard definitions
//#define STRICT                                                  // enable strict type-checking of Windows handles
#define WIN32_LEAN_AND_MEAN                                     // allow the exclusion of uncommon features
//#define WINVER                                          _WIN32_WINNT_WIN7  // allow the use of Windows XP specific features
//#define _WIN32_WINNT                                    _WIN32_WINNT_WIN7  // allow the use of Windows XP specific features
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES         1       // use the new secure functions in the CRT
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT   1       // use the new secure functions in the CRT

#include <windows.h>            // fundamental Windows header file
//#define PBADECL WINAPI
//#define THREADPROC WINAPI   //What does this do again?

#define _USE_MATH_DEFINES	//holy crap!!! must define this on ms windows to get M_PI definition!
#include <math.h>



//#include <Audioclient.h>

//#define PBA_WSAPI_FUNC_LIST
    /* ret, name, dllname, params */
	//PBAXE(HRESULT, PBADeviceActivate, Activate, IMMDevice * device, REFIID refIID, DWORD dwClsCtx, PROPVARIANT * pActivationParams, void ** ppInterface)
	/* end */

//#define PBAXE(ret, name, dllname, ...) typedef ret name##proc(__VA_ARGS__); extern name##proc * ##name;
//	PBA_WSAPI_FUNC_LIST 
//#undef PBAXE

// =============================================================================

//#ifdef CR_GRAPHICS_EXTENSIONS_IMPLEMENTATION
//#define PBAXE(ret, name, dllname, ...) name##proc * name;
//PBA_WSAPI_FUNC_LIST
//#undef PBAXE
    
//PB_AUDIO_API PB_AUDIO_INLINE void pbaudio_wasapi_ext_init();


#ifdef __cplusplus
}
#endif

#endif
