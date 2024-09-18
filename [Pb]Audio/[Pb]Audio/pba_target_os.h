//
//  cr_target_os.h
//  crPlatform
//
//  Created by Joe Moulton on 1/21/19.
//  Copyright © 2019 Abstract Embedded. All rights reserved.
//

#ifndef PBA_TARGET_OS
#define PBA_TARGET_OS


//******************************************************//
#pragma mark -- Core Render Supported Target Macros --
//******************************************************//

#if defined(_WIN32)         // MICROSOFT WINDOWS
#define PBA_TARGET_WIN32     1
#elif defined( __APPLE__ )  // APPLE DARWIN
#import <TargetConditionals.h>
#if   defined(TARGET_OS_IOS) && TARGET_OS_IOS
#define PBA_TARGET_IOS       1
#elif  defined(TARGET_OS_TV) && TARGET_OS_TV
#define PBA_TARGET_TVOS      1
#elif defined(TARGET_OS_OSX) && TARGET_OS_OSX
#define PBA_TARGET_OSX       1
#endif                      // APPLE DARWIN
#endif                      // MICROSOFT WINDOWS


//******************************************************//
#pragma mark -- Std C Extensions --
//******************************************************//

#ifndef __cplusplus

// Don't define _Bool/bool in C++ sans GNU
#ifdef _Bool
#define bool _Bool
#elif !defined(bool)
#define bool int
#endif

#define  true 1
#define false 0

#elif defined(__GNUC__) && !defined(__STRICT_ANSI__)

// Define _Bool/bool as a GNU extension
#define _Bool bool
#define  bool bool

#define false false
#define  true true

#endif

// For Win32 it is imperative that stdio gets included before platform headers
// or printf with throw a "redefinition; different linkage error"
#include <stdio.h>
#include <stdint.h> //uint32_t

#ifdef  PBA_TARGET_WIN32
#define PBA_EXTERN
#else
#define PBA_EXTERN extern
#endif

#if !defined(PBA_INLINE)
# if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define PBA_INLINE static inline
# elif defined(__cplusplus)
#  define PBA_INLINE static inline
# elif defined(__GNUC__)
#  define PBA_INLINE static __inline__
# elif defined(__WIN32__)
#  define PBA_INLINE static __inline
# else
#  define PBA_INLINE static
# endif
#endif


//**************************************************************************************************************************//
#pragma mark -- MICROSOFT Windows NT --
//**************************************************************************************************************************//

#ifdef PBA_TARGET_WIN32                                     // CoreRender MS WINDOWS Target

#define NOMINMAX //Why?
                                                           // Win8.0 is Minimum version required for DXGI DirectComposition IDCompositionDevice::IUnknown
//#define NTDDI_VERSION NTDDI_WINBLUE                      // Win8.1 is Minimum version required for Per Monitor DPI Awareness
//#define NTDDI_VERSION NTDDI_WINBLUE                      // Win8.0 is Minimum version required for DXGI DirectComposition IDCompositionDesktopDevice::IDCompositionDevice2::IUnknown
//#define WINVER       _WIN32_WINNT_WINBLUE                // allow the use of Windows Version specific features
//#define _WIN32_WINNT _WIN32_WINNT_WINBLUE                // allow the use of Windows Version specific features

// standard definitions
#define STRICT                                             // enable strict type-checking of Windows handles
#define WIN32_LEAN_AND_MEAN                                // allow the exclusion of uncommon features
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES         1  // use the new secure functions in the CRT
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT   1  // use the new secure functions in the CRT

#include <windows.h>   // fundamental Windows header file
#define GLDECL WINAPI
#define DXDECL WINAPI
#define VKDECL WINAPI

#define _USE_MATH_DEFINES    //holy crap!!! must define this on ms windows to get M_PI definition!
#include <math.h>

#define THREADPROC WINAPI   //What does this do again?

#ifdef _DEBUG
#include <vld.h>
#endif

//Win32 c based dependencies
#include <tchar.h>              // generic text character mapping
#include <string.h>             // includes string manipulation routines
#include <stdlib.h>             // includes standard libraries
#include <stdint.h>             // includes uint64_t, etc types

#include <stdio.h>              // includes standard input/output routines
#include <process.h>            // threading routines for the CRT
//#include <tlhelp32.h>

//For Win32 Media Manager File Loading
static const GUID _IID_MF_LOW_LATENCY = { 0x9c27891a, 0xed7a, 0x40e1, {0x88, 0xe8, 0xb2, 0x27, 0x27, 0xa0, 0x24, 0xee} };

//******************************************************//
#pragma mark -- DPI Awareness --
//******************************************************//
// You can and should use an application manifest to define DPI awareness
// per platform target on process startup instead of this API!
//
//#if NTDDI_VERSION >= NTDDI_WINBLUE                   // Windows >= 8.1 Per Monitor DPI Scaling
#include <hidsdi.h>                                    // Solves 'ULONG' followed by 'float' is illegal [WinSDK 10.0.22621.0]
#include <ShellScalingAPI.h>                           // Additional header required to call GetDpiForMonitor
#pragma comment(lib, "shcore")
//#endif

//******************************************************//
#pragma mark -- DXGI Platform Primitives --
//******************************************************//
#include <dxgitype.h>

//******************************************************//
#pragma mark -- Win32 Environment Variables  --
//******************************************************//

#define RESOURCE_HOME "HOMEDRIVE"
#define RESOURCE_DIR  "/3rdGen/AssetLibrary"
#define ENGINE_DIR    "/3rdGen/CoreRender"
#define INCLUDE_DIR   ENGINE_DIR "/include"
#define CACHE_DIR     ENGINE_DIR "/cache"

//**************************************************************************************************************************//
#pragma mark -- APPLE Darwin XNU --
//**************************************************************************************************************************//

#elif defined(__APPLE__)

#import <TargetConditionals.h>
#include <dispatch/dispatch.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
#include <objc/runtime.h> //objective-c runtime
#include <objc/message.h> //obj-c runtime messages

#include <CoreText/CoreText.h>

#endif


//******************************************************//
#pragma mark -- APPLE OSX --
//******************************************************//

#if defined(TARGET_OS_OSX) && TARGET_OS_OSX          // CoreRender APPLE OSX Target
//Private CoreGraphics API
//#include "cg_private.h"                              //OSX Core Graphics Window and CoreGL Platform API
//Public Core Graphics API
//#include <CoreGraphics/CoreGraphics.h>
//#include "crgc_gl_ext.h"

//******************************************************//
#pragma mark -- OSX Environment Variables  --
//******************************************************//

#define RESOURCE_HOME "HOME"
#define RESOURCE_DIR  "/Development/3rdGen/AssetLibrary"
#define ENGINE_DIR    "/Development/3rdGen/CoreRender"
#define INCLUDE_DIR   ENGINE_DIR "/include"
#define CACHE_DIR     ENGINE_DIR "/cache"

//******************************************************//
#pragma mark -- APPLE iOS/tvOS --
//******************************************************//

#elif defined(TARGET_OS_IOS) && TARGET_OS_IOS        // CoreRender APPLE iOS Target

//Public Core Graphics API
#include <CoreGraphics/CoreGraphics.h>

#define GLES_SILENCE_DEPRECATION

//#import <OpenGLES/OpenGLES.h>
//#import <OpenGLES/EAGL.h>
//#import <OpenGLES/EAGLIOSurface.h>
#include <CoreVideo/CVOpenGLESTexture.h>
#include <CoreVideo/CVOpenGLESTextureCache.h>

//For convenience, include the OpenGL headers that will be needed
//for a client to create this context
//OpenGL ES 2
#if !TARGET_OS_MACCATALYST
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

//OpenGL ES 2
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

//OpenGLES 3
//Warning:  An Objective-C EAGLContext can be created without including these headers
//so while you may have an es3 context, you will not have any support for gles3 functions if these aren't included
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

//******************************************************//
#pragma mark -- iOS Environment Variables  --
//******************************************************//

#define RESOURCE_HOME "HOME"
#define RESOURCE_DIR  "/Documents"
#define ENGINE_DIR    "/Documents"
#define INCLUDE_DIR   "/Documents"
#define CACHE_DIR     "/Documents/cache"

#endif

#endif


/*
//Supported Core Render Platform IDENTIFIERS
typedef enum cr_target_platform_id
{
    PBA_TARGET_ID_UNSUPPORTED   = -1,
    PBA_TARGET_ID_WIN32         = 0,
    PBA_TARGET_ID_IOS           = 1,
    PBA_TARGET_ID_TVOS          = 2,
    PBA_TARGET_ID_OSX           = 3,
    //PBA_TARGET_RHEL      = 2,
    //PBA_TARGET_ANDROID   = 5,
    //PBA_TARGET_FUSCHIA   = 6,
    //PBA_TARGET_BSD       = 7,
    CR_NUM_TARGET_PLATFORM     = 4
}PBA_TARGET_PLATFORM_ID;

static PBA_TARGET_PLATFORM_ID PBA_TARGET_OS()
{
#if defined(PBA_TARGET_WIN32)
    return PBA_TARGET_ID_WIN32;
#elif defined(PBA_TARGET_OSX)
    return PBA_TARGET_ID_OSX;
#elif defined(PBA_TARGET_IOS)
    return PBA_TARGET_ID_IOS;
#elif defined(PBA_TARGET_TVOS)
    return PBA_TARGET_ID_TVOS;
#else
    return PBA_TARGET_ID_UNSUPPORTED;
#endif
}
 */


//******************************************************//
#pragma mark -- Platform Primitive Types --
//******************************************************//

#if __APPLE__

//*** 2D Primitives ***//
typedef CGFloat                 OSFloat;
typedef CGPoint                 OSPoint;

typedef CGSize                  OSSize;
typedef CGRect                  OSRect;

//*** Color ***//
//#define OSColorSize           4
#define OSColorSize             32                      //Not used
typedef CGColorRef              OSColorRef;             //CoreGraphics Color Object Reference

//*** Font ***//
typedef CTFontDescriptorRef     OSLogicalFontRef;
typedef CTFontRef               OSFontRef;

#else

//Extend LOGFONTW to hold a floating point font size
typedef struct tagLOGFONTWF
{
    LONG  lfHeight;
    LONG  lfWidth;
    LONG  lfEscapement;
    LONG  lfOrientation;
    LONG  lfWeight;
    BYTE  lfItalic;
    BYTE  lfUnderline;
    BYTE  lfStrikeOut;
    BYTE  lfCharSet;
    BYTE  lfOutPrecision;
    BYTE  lfClipPrecision;
    BYTE  lfQuality;
    BYTE  lfPitchAndFamily;
    WCHAR lfFaceName[LF_FACESIZE];
    FLOAT fontSize;
} LOGFONTWF, * PLOGFONTWF, * NPLOGFONTWF, * LPLOGFONTWF;


//*** 2D Primitives ***//
typedef FLOAT                   OSFloat;
typedef POINT                   OSPoint;

//typedef D2D_SIZE_F              OSSize;
//typedef D2D_SIZE_F              CGRect;

//*** Color ***//
//#define OSColorSize           4
#define OSColorSize             16                  //Determines compile time color float vs byte
//typedef COLORREF              OSColorType;
typedef DXGI_RGBA               OSColorType;
typedef HBRUSH                  OSColorRef;         //GDI Color Object Reference

//*** Font ***//
typedef LOGFONTWF               OSLogicalFontRef;
typedef HFONT                   OSFontRef;

#endif


//******************************************************//
#pragma mark -- Platform Window + Layer Abstractions --
//******************************************************//

#ifndef CALLBACK
#define CALLBACK
#endif

#ifdef PBA_TARGET_WIN32
//this is just a helper struct for passing these two vars portably
typedef struct HGLRCHDC { HGLRC hglrc; HDC hdc; } HGLRCHDC;
typedef HANDLE    CRKernelQueueType;                    //From cr_event_queue.h
typedef HWND      CRWindowRef;                          //MS Platform Window
typedef HWND      CRLayerRef;                           //MS Platform Layer (is just a child HWND)
typedef void (CALLBACK* WNDDRAW)(HDC, RECT);            //MS Platform Draw Callback (HWND WM_ERASEBKGND GDI/GDI+ Hook)
#elif defined(PBA_TARGET_OSX)
typedef int       CRKernelQueueType;                    //kqueues are just file descriptors
//typedef CGSWindow CRWindowRef;                          //OSX Platform Window
typedef id        CRLayerRef;                           //OSX Platform Layer (is an NSView's CALayer)
typedef void (CALLBACK* WNDDRAW)(CGContextRef, CGRect); //OSX Cocoa NSView Layer drawRect: CoreGraphics Hook
#else
typedef int       CRKernelQueueType;                    //kqueues are just file descriptors
typedef int       CRWindowRef;                          //iOS Window Definition
typedef id        CRLayerRef;                           //iOS Platform Layer (is a  UIView's CALayer)
typedef void (CALLBACK* WNDDRAW)(CGContextRef, CGRect); //iOS Cocoa UIView Layer drawRect: CoreGraphics Hook
#endif

//forward declarations
//struct crgc_view;
//struct cr_vbo_mesh;
//struct CRenderTarget;

//typedef uint64_t (CALLBACK* CRGCDRAW) \
//(uint64_t currentFrame, uint64_t refreshTicks, uint64_t scheduledFrameTime, void* user_data, struct CRenderTarget* renderTarget, struct cr_vbo_mesh* vQuad);

//typedef uint64_t (CALLBACK* CRGCSWAP) \
//(struct crgc_view*, uint32_t, uint64_t, uint64_t, uint64_t);

//******************************************************//
#pragma mark -- Core Render Graphics Context Enums --
//******************************************************//

typedef enum PBA_BUFFER_USAGE
{
    //#define GL_STREAM_DRAW    0x88E0
    //#define GL_STREAM_READ    0x88E1
    //#define GL_STREAM_COPY    0x88E2
    //#define GL_STATIC_DRAW    0x88E4
    //#define GL_STATIC_READ    0x88E5
    //#define GL_STATIC_COPY    0x88E6
    //#define GL_DYNAMIC_DRAW   0x88E8
    //#define GL_DYNAMIC_READ   0x88E9
    //#define GL_DYNAMIC_COPY   0x88EA

    PBA_STREAM_DRAW  = -4,
    PBA_STREAM_READ  = -3,
    PBA_STREAM_COPY  = -2,
  //PBA_STREAM_NONE  = -1,
    PBA_STATIC_DRAW  =  0,
    PBA_STATIC_READ  =  1,
    PBA_STATIC_COPY  =  2,
  //PBA_STATIC_NONE  =  3,
    PBA_DYNAMIC_DRAW =  4,
    PBA_DYNAMIC_READ =  5,
    PBA_DYNAMIC_COPY =  6,
  //PBA_DYNAMIC_NONE =  7,

}PBA_BUFFER_USAGE;


//***************************************************//
#pragma mark -- Core Render Graphics Context Typedefs --
//***************************************************//

//*** crgc_size ***/
typedef struct crgc_size
{
    uint32_t    width;
    uint32_t    height;
}crgc_size;

//*** crgc_rect ***/
typedef struct crgc_rect
{
    //union
    //{
        int x, y, width, height;
        //cr_int4 frame;
        //crgc_size origin, size;
    //};
}crgc_rect;

//Deprecated!
//TO DO:  Remove after updating on all platforms
/*
typedef enum crgc_profile
{
    CURRENT = 0,             //WGL = most current opengl context provided by the graphics device, kCGLOGLPVersion_GL4_Core
    CORE = 1,                //WGL_CONTEXT_CORE_PROFILE_BIT_ARB, kCGLOGLPVersion_GL3_Core
    COMPATIBILITY = 2        //WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, kCGLOGLPVersion_Legacy
}crgcProfile;
*/

//Core Render OpenGL Profile Enumeraitons
typedef enum crgc_profile
{
    CRGL_NONE          = 0,
    CRGL_CURRENT       = 1, //WGL = most current opengl context provided by the graphics device, kCGLOGLPVersion_GL4_Core
    CRGL_CORE          = 2, //WGL_CONTEXT_CORE_PROFILE_BIT_ARB, kCGLOGLPVersion_GL3_Core
    CRGL_COMPATIBILITY = 3, //WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, kCGLOGLPVersion_Legacy
    CRGL_DIRECTX       = 4,
    CRGL_METAL         = 5,
    CRGL_VULKAN        = 6
}crgc_profile;

PBA_INLINE crgc_rect crgc_rect_make(int x, int y, int width, int height){crgc_rect rect = {x, y, width, height};return rect;}
PBA_INLINE crgc_size crgc_size_make(int width, int height){crgc_size size = {width, height};return size;}

//***************************************************//
#pragma mark -- Core Render Graphics Context Externs --
//***************************************************//

// CRGC Defines are shared between all _ext.h implementations
//PBA_EXTERN volatile int PBA_MAX_TEXTURE_SIZE;


//#ifdef __cplusplus
//}
//#endif

#endif /* cr_target_os_h */
