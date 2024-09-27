//
//  CMidiEventQueue.h
//  CMidi
//
//  Created by Joe Moulton on 9/16/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#ifndef CMidiEventQueue_h
#define CMidiEventQueue_h

#ifndef _WIN32
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

    //
    //#include "cr_target_os.h"

//FREQUENCY DEFINITIONS

static const float CENT_FREQ_SCALAR = 1.00057778951f;

static const float AUDIBLE_LOGARITHMIC_FREQUENCIES[11] =
{
    20.00f,
    40.00f,
    80.00f,
    160.0f,
    320.0f,
    640.0f,
    1280.0f,
    2560.0f,
    5120.0f,
    10240.0f,
    20480.0f
};

// freq definitions from A0 to C8, putting in tables is much cheaper than calculating 12th root
static const float WESTERN_NOTE_FREQUENCIES[95] =
{
    27.50f,
    29.14f,
    30.87f,
    32.70f,
    34.65f,
    36.71f,
    38.89f,
    41.20f,
    43.65f,
    46.25f,
    49.00f,
    51.91f,
    55.00f,
    58.27f,
    61.74f,
    65.41f,
    69.30f,
    73.42f,
    77.78f,
    82.41f,
    87.31f,
    92.50f,
    98.00f,
    103.83f,
    110.00f,
    116.54f,
    123.47f,
    130.81f,
    138.59f,
    146.83f,
    155.56f,
    164.81f,
    174.61f,
    185.00f,
    196.00f,
    207.65f,
    220.00f,
    233.08f,
    246.94f,
    261.63f,
    277.18f,
    293.66f,
    311.13f,
    329.63f,
    349.23f,
    369.99f,
    392.00f,
    415.30f,
    440.00f,
    466.16f,
    493.88f,
    523.25f,
    554.37f,
    587.33f,
    622.25f,
    659.26f,
    698.46f,
    739.99f,
    783.99f,
    830.61f,
    880.00f,
    932.33f,
    987.77f,
    1046.50f,
    1108.73f,
    1174.66f,
    1244.51f,
    1318.51f,
    1396.91f,
    1479.98f,
    1567.98f,
    1661.22f,
    1760.00f,
    1864.66f,
    1975.53f,
    2093.00f,
    2217.46f,
    2349.32f,
    2489.02f,
    2637.02f,
    2793.83f,
    2959.96f,
    3135.96f,
    3322.44f,
    3520.00f,
    3729.31f,
    3951.07f,
    4186.01f,
    4434.92f,
    4698.64f,
    4978.03f,
    5274.04f,
    5587.65f,
    5919.91f,
    6271.93f
};

static const float MAX_NOTE_FREQ = 4186.01f;

static const char* WESTERN_NOTE_NAMES[95] =
{
    "A0",
    "A#0",
    "B0",
    "C1",
    "C#1",
    "D1",
    "D#0",
    "E1",
    "F1",
    "F#1",
    "G1",
    "G#1",
    "A1",
    "A#1",
    "B1",
    "C2",
    "C#2",
    "D2",
    "D#2",
    "E2",
    "F2",
    "F#2",
    "G2",
    "G#2",
    "A2",
    "A#2",
    "B2",
    "C3",
    "C#3",
    "D3",
    "D#3",
    "E3",
    "F3",
    "F#3",
    "G3",
    "G#3",
    "A3",
    "A#3",
    "B3",
    "C4",
    "C#4",
    "D4",
    "D#4",
    "E4",
    "F4",
    "F#4",
    "G4",
    "G#4",
    "A4",
    "A#4",
    "B4",
    "C5",
    "C#5",
    "D5",
    "D#5",
    "E5",
    "F5",
    "F#5",
    "G5",
    "G#5",
    "A5",
    "A#5",
    "B5",
    "C6",
    "C#6",
    "D6",
    "D#6",
    "E6",
    "F6",
    "F#6",
    "G6",
    "G#6",
    "A6",
    "A#6",
    "B6",
    "C7",
    "C#7",
    "D7",
    "D#7",
    "E7",
    "F7",
    "F#7",
    "G7",
    "G#7",
    "A7",
    "A#7",
    "B7",
    "C8"
};


#if defined(_WIN32)         //MICROSOFT WINDOWS NT PLATFORMS
#define CM_TARGET_WIN32     1
#elif defined( __APPLE__ )  //APPLE DARWIN XNU PLATFORMS
#if defined(TARGET_OS_IOS) && TARGET_OS_IOS
#define CM_TARGET_IOS       1
#elif defined(TARGET_OS_TV) && TARGET_OS_TV
#define CM_TARGET_TVOS      1
#elif defined(TARGET_OS_OSX) && TARGET_OS_OSX
#define CM_TARGET_OSX       1
#endif                      //APPLE DARWIN XNU PLATFORMS
#endif


//**************************************************************************************************************************//
#pragma mark -- MICROSOFT Windows NT --
//**************************************************************************************************************************//

#ifdef CM_TARGET_WIN32                                     // CoreRender MS WINDOWS Target

#define CM_PLATFORM_EXTERN
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
//#define GLDECL WINAPI
//#define DXDECL WINAPI
//#define VKDECL WINAPI

#define _USE_MATH_DEFINES	//holy crap!!! must define this on ms windows to get M_PI definition!
#include <math.h>

#define THREADPROC WINAPI   //What does this do again?

#ifdef _DEBUG
//#include <vld.h> 
#endif


#elif defined(__APPLE__)
#define CM_PLATFORM_EXTERN extern
#include <dispatch/dispatch.h>
#if !TARGET_OS_IOS && !TARGET_OS_TV
//#include "cg_private.h"
#endif
#endif

//OSX Platform Threading Mechanism (Grand Central Dispatch thread pools)

#pragma mark -- CMidi Process Definitions

//Define an opaque process handle for supported platform process types
#ifdef CM_TARGET_WIN32
typedef HANDLE cm_pid_t;
#elif defined(__APPLE__)
typedef pid_t  cm_pid_t;
#endif

//Define Opaque Priority Class Types for Running System Processes
typedef enum CM_PRIORITY_CLASS
{
    CM_DEFAULT_PRIORITY_CLASS = 0,
    CM_REALTIME_PRIORITY_CLASS = 0x00000100  //WIn32 equivalent
}CM_PRIORITY_CLASS;

typedef enum CM_THREAD_PRIORITY
{
    CM_THREAD_PRIORITY_DEFAULT,
    CM_THREAD_PRIORITY_CRITCAL // = THREAD_PRIORITY_TIME_CRITICAL
}CM_THREAD_PRIORITY;


#pragma mark -- Core Render Platform Thread Handle ID

#ifdef CM_TARGET_WIN32
typedef HANDLE           cm_platform_thread;
typedef unsigned int     cm_platform_thread_id;
#define cm_stdcall       __stdcall
#elif defined(__APPLE__)
typedef dispatch_queue_t cm_platform_thread;
typedef unsigned int     cm_platform_thread_id;
#define cm_stdcall
#endif

//#include "ReqlCoroutine.h"
//#include "CTError.h"
#ifdef _WIN32
#define CMThread                 HANDLE
#define CMThreadID               unsigned int
typedef HANDLE                   CMKernelQueueType;
typedef LPTHREAD_START_ROUTINE   CMThreadRoutine; //Win32 CRT Thread Routine
#define CMDispatchSource         void
//#define SSLContextRef          void*
//#define CTSocketError()        (WSAGetLastError())

typedef struct CMKernelQueue
{
    CMKernelQueueType kq;
    CMKernelQueueType pq[2];
    intptr_t          pnInflightCursors;
}CMKernelQueue;

typedef HANDLE CMKernelQueueEvent;

#elif defined(__APPLE__) || defined(__FreeBSD__) //with libdispatch
#define CMThread                 pthread_t
#define CMThreadID               unsigned int
typedef int                      CMKernelQueueType; //kqueues are just file descriptors
typedef int                      CMKernelQueueType;  //pipes are just file descriptors
//typedef void *                 (*CTThreadRoutine)(void *); //pthread routine
#define CMDispatchSource         dispatch_source_t
typedef void(^ CMDispatchSourceHandler)(void);      //clang block
//#define CTSocketError() (errno)

typedef struct CMKernelQueue
{
    CMKernelQueueType kq;
    CMKernelQueueType pq[2];
    intptr_t          pnInflightCursors;
}CMKernelQueue;

typedef struct kevent CMKernelQueueEvent;

#define CM_INCOMING_PIPE    0
#define CM_OUTGOING_PIPE    1

#endif


#pragma mark -- CMidi Kernel Event Queue

/***
 *  "Kernel Event Queues" are useful for sending/receiving and waiting on events between threads and processes when standard provided event/message queue mechanisms provided by the OS to the application dont cut the mustard)
 *
 *	Core Render defines the notion of a kernel event queue as some implementation that allows handling of associated "Kernel Events" for the purposes of interprocess and interthread communication without incurring the cost of a User Space->Kernel Space->User Space memory transfer.
 *	Core Render defines the notion of a "Kernel Event" as an event that a process thread may idle on and "woken up" by the kernel to receive the memory associated with the event directly from the kernel.
 *
 *	Note that some platforms may not have a dedicated "Kernel Event Queue" data type or handle exposed but still have "Kernel Event" type that a process thread may idle on.  For example, Win32 thread handles and their associated event queues are one in the same (see comment below).
 *	When no sufficient "Kernel Event" mechanism can be provided by or implemented on top of the OS, a run loop must fall back to polling for events which is not suitable for real-time operation :(
 ***/


//Define Opaque OS Kernel Event Queue Data Type for Queue Mechanisms exposed by the OS Kernel
#ifdef CM_TARGET_WIN32
//For Win32 Interprocess Events, there is no queue manually created or exposed with an event that can be passed and triggered/waited on between processes as with BSD kqueue + pipe combination.
//For Win32 Interthread Messages/Events, every thread comes with a dedicated event queue "under the hood" in the kernel.
//The handles to these thread event/message queues themselves are not exposed as they can be identified and sent to by their corresponding thread address stored as unsigned int
//Thread "Messages"/Events can be retrieved from a thread's manually implemented run loop by calling the GetMessage/PeekMessage API ust like on the Win32 platform main thread message/event loop message pump.
typedef unsigned int  cm_thread_queue_id;  //On Win32, a platform thread queue id is the same as a platform thread id is the same as...
typedef unsigned int  cm_kernel_queue_id;  //a platform kernel queue id
#elif defined(__APPLE__)
typedef int           cm_kernel_queue_id;  //on BSD platforms kernel queues are just kqueues, and kqueue handles are just file descriptors
#endif

//Define Opaque Event Data Type for OS Kernel Event Queue
#ifdef CM_TARGET_WIN32
typedef HANDLE        cm_kernel_queue_event;
#elif defined(__APPLE__)
typedef struct kevent cm_kernel_queue_event;
#endif

//CM_PLATFORM_EXTERN cm_kernel_queue_id CMTriggerEventQueue; //a global kqueue singleton for the main Core Midi Event Loop to IPC with real-time audio threads


//END FREQUENCY DEFINITIONS

#ifdef __cplusplus
}
#endif


#endif /* CMidiKeyboard_h */
