//
//  CMAppInterface.h
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/27/22.
//

#ifndef PBAudioAppInterface_h
#define PBAudioAppInterface_h

//----------------------------------------------------PREPROCESSOR----------------------------------------------------------------------//

//#ifndef __BLOCKS__
//#error must be compiled with -fblocks option enabled
//#endif

//------------------------------------------------------HEADERS------------------------------------------------------------------------//

#pragma mark -- [Pb]Audio Dependencies

#define MIDI_DEBUG
#include <CMidi/CMidi.h>          
#include <[Pb]Audio/[Pb]Audio.h>

#pragma mark -- Core Render Dependencies

//#include <CoreRender/crSystem.h>
//#include <CoreRender/crPlatform.h>
//#include <CoreRender/crMath.h>
//#include <CoreRender/crMesh.h>
//#include <CoreRender/xng.h>

#pragma mark -- CMidi Message Queue

//static const long         kInFlightTriggerEventFrames             = 3;
#define                   MAX_TRIGGER_EVENTS                          32    //# elements in circular message buffer (bounded by min audio buffer size)
static CMTriggerMessage   trigger_events[MAX_TRIGGER_EVENTS]        = {0};  //circular message buffer
static unsigned int       cm_trigger_event_index                    =  0;   //circular message buffer head

#pragma mark -- PBAudio Engine Process Message Queue

//TO DO: This is relatively naive
#define                    kInFlightMessageEventFrames              3
#define                    MAX_MESSAGE_EVENTS                       (CMMessageTypeTimeout * kInFlightMessageEventFrames) //# elements in circular message buffer
static CMUniversalMessage  pba_message_events[MAX_MESSAGE_EVENTS] = { CMMessageTypeUnknownF, {0}};                       //circular message buffer
static unsigned int        pba_message_event_index                  =  0;                                                //circular message buffer head

typedef enum OutputPassID
{
    NullOutputPassID,
    PassThroughOutputPassID,
    TestOutputPassID,
    SamplerOutputPassID,
    MaxOutputPassID
}OutputPassID;

#ifndef _Nullable
#define _Nullable 
#endif

extern PBAStreamOutputPass _Nullable OutputPass[MaxOutputPassID];

#pragma mark -- Additional Platform Dependencies

//CoreRender crPlatform lib includes the necessary platform dependencies for the supported platforms
//However, you may want to use some headers not provided in crPlatform

#ifdef PBA_TARGET_WIN32

//#define _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#define new new(_CLIENT_BLOCK,__FILE__, __LINE__)
//#define _CRTDBG_MAP_ALLOC

//Additional header required to call GetDpiForMonitor, use enumerations and other API calls
//WARNING:  You should use application manifest to set the process DPI awareness mode on startup instead of this API
#if NTDDI_VERSION >= NTDDI_WINBLUE
#include <hidsdi.h>                                     //Solves 'ULONG' followed by 'float' is illegal [WinSDK 10.0.22621.0]
#include <ShellScalingAPI.h>                         //Windows >= 8.1 Per Monitor DPI Scaling
#endif

#include "crtdbg.h"                                     //CRT Debug

#ifdef _DEBUG
#include <vld.h>                                     //Visual Leak Detector
#endif

//#include "ntdef.h"
//#include "ntstatus.h"
#include <tchar.h>                                     // generic text character mapping
#include <string.h>                                     // includes string manipulation routines
//#include <stdlib.h>                                 // includes standard libraries
#include <stdio.h>                                     // includes standard input/output routines
#include <process.h>                                 // threading routines for the CRT
#include <tlhelp32.h>

#elif defined(__APPLE__)

#include <TargetConditionals.h>
#include <CoreFoundation/CoreFoundation.h>           //Core Foundation
#include <objc/runtime.h>                            //objective-c runtime
#include <objc/message.h>                            //objective-c runtime message

#include <CoreServices/CoreServices.h>               //Core Services
#include <CoreMIDI/MIDIServices.h>                   //Core Midi
#include <libkern/OSAtomic.h>                        //libkern

#if TARGET_OS_OSX
#include <CoreAudio/HostTime.h>                      //Core Audio
#include <ApplicationServices/ApplicationServices.h> //Cocoa
#endif
#endif


#pragma mark -- Platform Obj-C Headers

#if defined(__APPLE__) && defined(__OBJC__)  //APPLE w/ Objective-C Main
#if TARGET_OS_OSX           //MAC OSX
#import  <AppKit/AppKit.h>
#else                       //IOS + TVOS
#import  <UIKit/UIKit.h>
#endif
#endif

#pragma mark -- Standard C Dependencies (should come before platform depencies)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "float.h"
#include "limits.h"

//#include <unistd.h>


#pragma mark -- Application Obj-C Headers

#if defined(__APPLE__) && defined(__OBJC__)  //APPLE w/ Objective-C Main
#import "PbAudioApplication.h"
#import "PbAudioAppDelegate.h"
#elif defined(WIN32)
#define USE_CPP_RUNLOOP
#endif


#pragma mark -- Application Third Party Headers

/*
#ifdef __cplusplus //|| defined(__OBJC__)
//CoreTransport CXURL and CXReQL C++ APIs use CTransport (CTConnection) API internally
#include <CoreTransport/CXURL.h>
#include <CoreTransport/CXReQL.h>
#elif defined(__OBJC__)
#include <CoreTransport/NSTURL.h>
#include <CoreTransport/NSTReQL.h>
#else
#include "CoreTransport/CTransport.h"
#endif
*/


//------------------------------------------------------Linker Defines--------------------------------------------------------------------//

#pragma mark -- Obj-C Runtime Externs

#if defined(__APPLE__) && !defined(__OBJC__)
//define the NSApplicationMain objective-c runtime call to suppress warnings before linking
#if TARGET_OS_OSX
//extern int NSApplicationMain(int argc, const char *__nonnull argv[__nonnull]);
extern int NSApplicationMain(int argc, const char *_Nonnull argv[_Nonnull]);
//extern int NSApplicationMain(int argc, const char *__nonnull argv[__nonnull], id principalClassName, id delegateClassName);
#else// defined(PBA_TARGET_IOS) || defined(PBA_TARGET_TVOS)
//define the UIApplicationMain objective-c runtime call or linking will fail
extern int UIApplicationMain(int argc, char * _Nullable argv[_Nonnull], id _Nullable principalClassName, id delegateClassName);
//extern int UIApplicationMain(int argc, char * _Nullable argv[_Nonnull], NSString * _Nullable principalClassName, NSString * _Nullable delegateClassName);
#endif
#endif

//------------------------------------------------------App Definitions--------------------------------------------------------------------//

#pragma mark -- Application Globals

//ClientIDs used by CoreMIDI
static const char * _Nonnull CM_CLIENT_OWNER_ID           = "com.3rdGen.[Pb]Audio.CMidi";
static const char * _Nonnull CM_HARDWARE_DEVICE_BASE_ID   = "com.3rdGen.[Pb]Audio.CMidi.Devices";               //client apps should define suffix eg "{Device_Name}"

static const char * _Nonnull CM_PERSISTENT_THRU_BASE_ID   = "com.3rdGen.[Pb]Audio.CMidi.PersistentConnections"; //client apps should define suffix eg "{Device_Name}"
static const char * _Nonnull CM_SOFT_THRU_BASE_ID         = "com.3rdGen.[Pb]Audio.CMidi.DeviceConnections";     //client apps should define suffix eg "{Device_Name}"
static const char * _Nonnull CM_PROXY_BASE_ID             = "com.3rdGen.[Pb]Audio.CMidi.ProxyConnections";      //client apps should define suffix eg "{Device_Name}"
static const char * _Nonnull CM_INPUT_BASE_ID             = "com.3rdGen.[Pb]Audio.CMidi.InputConnections";      //client apps should define suffix eg "{Device_Name}"


//Class Names of the Custom NSApplication/UIApplication and NSAppDelegate/UIAppDelegate Cocoa App Singleton Objective-C Objects
static const char * _Nonnull CocoaAppClassName            = "PbAudioApplication";
static const char * _Nonnull CocoaAppDelegateClassName    = "PbAudioAppDelegate";

//statically define strings for executables we know we will use
//static const char * DisplaySyncProcessName      = "crTimeServer";

//define crgc_view structs for static views we know we will want to use prior to run-time
//#define NUM_VIEWS 1
//static crgc_view glView[NUM_VIEWS];

#pragma mark -- Application Thread Handles

//These are provided for you for convenience in crPlatform lib
//cr_platform_thread    cr_mainThread;
//cr_platform_thread_id cr_mainThreadID;

#pragma mark -- Event Queue Handles


#pragma mark -- Event Handles

//This event gets created by the application, but it's handle lives in crPlatform lib
//so that it can be shared between Cocoa and Core Render, or any two processes that load the same crPlatform lib
//static cr_kernel_queue_event cr_vBlankNotification;

#pragma mark -- Custom Dispatch Queues

//define strings for dispatch queues
//static const char kCRWindowEventQueue[] = "cr.3rdgen.windowEventQueue";
//static dispatch_queue_t g_windowEventQueue[NUM_VIEWS];

#pragma mark -- Custom Run Loop Modes

//define strings for custom run loop modes
//A global event queue for receiving event updates in Core Render C-Land from
//define a custom queue that will act as the interface between
//static const CFStringRef kCFCGEventReceiveRunLoopMode = CFSTR("CGEventReceiveRunLoopMode");

#pragma mark -- System Path Utilities

//#include "PathCommon.h"

#pragma mark -- System Resource Factory(s)

//These externs are provided for you for convenience in crPlatform lib
//IDWriteFactory2 *  TextFactory;
//IWICFactory2      *  ImageFactory;

#pragma mark -- Scene Resource Cache(s)

//#include "SceneResources.h"

#pragma mark -- Window + HID Event Handlers

//systems/functions that handle OS main run loop and platform window system provided event messages
//#include "UILayer.h"

#pragma mark -- Render Hardware Interface

//systems/functions that handle rendering to the crgc_view platform window
//#include "SceneView.h"

#pragma mark -- Define DisplaySync Callback Struct

//define a displaySync object that can be used to slave rendering to platform kernel display updates each view
//we declare globablly so the object reference can persist when its pointer is passed through the callback mechanism
//CRDisplaySyncCallbackRef g_dispSyncCallbackRef[NUM_VIEWS];

#pragma mark -- Timing Globals
//static uint64_t timeA, timeB;

#endif /* PBAudioAppInterface_h */
