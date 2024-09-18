//
//  [Pb]Audio.h
//  [Pb]Audio
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#ifndef _Pb_Audio_h
#define _Pb_Audio_h


#ifdef __cplusplus
extern "C" {
#endif


//only define bool if not previoulsy defined by CoreRender crWindow global header
//and not using C++
#ifndef __cplusplus
//typedef unsigned char bool;
#ifndef bool
#define bool int
#define true 1
#define false 0
#endif
#endif


/*************************************************************************
 * Compiler- and platform-specific preprocessor work
 *************************************************************************/

/* If we are we on Views, we want a single define for it.
 */
#if !defined(_WIN32) && (defined(__WIN32__) || defined(WIN32) || defined(__MINGW32__))
 #define _WIN32
#endif /* _WIN32 */


#if defined(PB_AUDIO_DLL) && defined(_PB_AUDIO_BUILD_DLL)
 /* CRGC_DLL must be defined by applications that are linking against the DLL
  * version of the CRGC library.  _CRGC_BUILD_DLL is defined by the CRGC
  * configuration header when compiling the DLL version of the library.
  */
 #error "You may not have both PB_AUDIO_DLL and _PB_AUDIO_BUILD_DLL defined"
#endif

/* CR_PRIMITIVES_API is used to declare public API functions for export
 * from the DLL / shared library / dynamic library.
 */
#if defined(_WIN32) && defined(_PB_AUDIO_BUILD_DLL)
 /* We are building crMath as a Win32 DLL */
 #define PB_AUDIO_API __declspec(dllexport)
#elif defined(_WIN32) && defined(PB_AUDIO_DLL)
 /* We are calling crMath as a Win32 DLL */
 #define PB_AUDIO_API __declspec(dllimport)
#elif defined(__GNUC__) && defined(_PB_AUDIO_BUILD_DLL)
 /* We are building CR_PRIMITIVES as a shared / dynamic library */
 #define PB_AUDIO_API __attribute__((visibility("default")))
#else
 /* We are building or calling crMath as a static library */
 #define PB_AUDIO_API
#endif
 
//inline doesn't exist in C89, __inline is MSVC specific
#ifndef PB_AUDIO_INLINE
#ifdef _WIN32
#define PB_AUDIO_INLINE __inline
#else
#define PB_AUDIO_INLINE //__inline__ __attribute__((noinline))
#endif
#endif

//inline doesn't exist in C89, __inline is MSVC specific
#ifndef CR_SIMD_INLINE
#ifdef _WIN32
#define CR_SIMD_INLINE __inline
#else
#define CR_SIMD_INLINE __inline__ __attribute__((always_inline))
#endif
#endif
    
//__decspec doesn't exist in C89, __declspec is MSVC specific
#ifndef PB_AUDIO_DECLSPEC
#ifdef _WIN32
#define PB_AUDIO_DECLSPEC __declspec
#else
#define PB_AUDIO_DECLSPEC
#endif
#endif
    
//align functions are diffent on windows vs iOS, Linux, etc.
#ifndef PB_AUDIO_ALIGN//(X)
#ifdef _WIN32
#define PB_AUDIO_ALIGN(X) (align(X))
#else
#define PB_AUDIO_ALIGN(X) __attribute__ ((aligned(X)))
#endif
#endif

/* We are building or calling CRPrimitives as a static library */
#ifdef _WIN32
#define PB_AUDIO_EXTERN
#else
#define PB_AUDIO_EXTERN extern
#endif

//CONFIGURATION -- THESE ARE THE ONLY MACROS CLIENTS SHOULD EDIT WHEN BUILDING

//typically, this should be defined by the project file, but you can define it here to explicitly enable
//memory alignment sanity checking for release builds
//#define PB_AUDIO_DEBUG


//END CONFIGURATION -- DON'T EDIT ANYTHING ELSE BEYOND THIS POINT

//include std c headers
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#if defined(_WIN32) && defined(_DEBUG)
#include <vld.h>
#endif

//#define PBA_DEBUG

//SIMD datatypes are processor/platform specific includes

#include "[Pb]Audio/pba_target_os.h"
#include "[Pb]Audio/pba_time.h"
#include "[Pb]Audio/pba_dsp_waveforms.h"


//now include our C primitive data types and functionality
#ifdef _WIN32
#include "[Pb]Audio/PbAudioExt.h"
#endif
#include "[Pb]Audio/pba_format.h"
#include "[Pb]Audio/pba_error.h"
#include "[Pb]Audio/PbAudioBufferList.h"
#include "[Pb]Audio/PbAudioBufferStack.h"
#include "[Pb]Audio/PbAudioCallbacks.h"
#include "[Pb]Audio/PbAudioDevice.h"

//Stream
#include "[Pb]Audio/PbAudioStream.h"

//API
#include "[Pb]Audio/PbAudioAPI.h"


//Utils
#include "[Pb]Audio/PbAudioByteUtils.h"

//File/URL Playback
#include "[Pb]Audio/PbAudioFileStream.h"

//Event Queue(s)
#include "[Pb]Audio/pba_event_queue.h"

//Render Pass
#include "[Pb]Audio/PbAudioRenderPass.h"


//Midi
#include "CMidi/CMidi.h"

#ifdef __cplusplus
}
#endif

#endif /* _Pb_Audio_h */
