//
//  pba_event_queue.h
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/9/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#ifndef pba_event_queue_h
#define pba_event_queue_h

//we wish to use the dispatch_async_f api to pass our function callbacks
//defined in as argument sto NSApplicationMain
//typedef void (*CRCocoaEventHandler)(void*);

#ifdef __cplusplus
extern "C" {
#endif
    
//
//#include "pba_target_os.h"

#if defined(_WIN32)         //MICROSOFT WINDOWS NT PLATFORMS
#define CR_TARGET_WIN32     1
#elif defined( __APPLE__ )  //APPLE DARWIN XNU PLATFORMS
#if defined(TARGET_OS_IOS) && TARGET_OS_IOS
#define CR_TARGET_IOS       1
#elif defined(TARGET_OS_TV) && TARGET_OS_TV
#define CR_TARGET_TVOS      1
#elif defined(TARGET_OS_OSX) && TARGET_OS_OSX
#define CR_TARGET_OSX       1
#endif                      //APPLE DARWIN XNU PLATFORMS
#endif
    
#if defined(__APPLE__)
#include <dispatch/dispatch.h>
#if !TARGET_OS_IOS && !TARGET_OS_TV
//#include "cg_private.h"
#endif
#endif


//OSX Platform Threading Mechanism (Grand Central Dispatch thread pools)
 
#pragma mark -- Core Render Process Definitions

//Define an opaque process handle for supported platform process types
#ifdef CR_TARGET_WIN32
typedef HANDLE pba_pid_t;
#elif defined(__APPLE__)
typedef pid_t  pba_pid_t;
#endif

/*
//global display sync process handle
PB_AUDIO_EXTERN pba_pid_t pba_displaySyncProcess;

//Define Opaque Priority Class Types for Running System Processes
typedef enum CR_PRIORITY_CLASS
{
    CR_DEFAULT_PRIORITY_CLASS = 0,
    CR_REALTIME_PRIORITY_CLASS = 0x00000100  //WIn32 equivalent
}CR_PRIORITY_CLASS;

typedef enum CR_THREAD_PRIORITY
{
    CR_THREAD_PRIORITY_DEFAULT,
    CR_THREAD_PRIORITY_CRITCAL // = THREAD_PRIORITY_TIME_CRITICAL
}CR_THREAD_PRIORITY;

#pragma mark -- Core Render Platform Kernel Message Definitions

//Define Opaque OS Platform Kernel Message Type Provided by the OS (ie Mach Messages for Darwin)
//#ifdef CR_TARGET_WIN32
//what to put here?
//#elif defined(__APPLE__)
//typedef mach_msg pba_kernel_msg;
//#endif

#pragma mark -- Core Render Platform Run Loop Event Message Type Definitions

*/

//Define Opaque OS Platform Event Loop Message Data Type Provided by the OS Windowing API (ie GDI for Win32 or Core Graphics for Cocoa)
#ifdef _WIN32
typedef MSG        pba_platform_event_msg;  //an OS defined container for wrapping a system genereated kernel message delivered to an application's main event loop or async window event loop
#elif defined(__APPLE__) && defined(TARGET_OS_OSX) && TARGET_OS_OSX
typedef CGEventRef pba_platform_event_msg;  //an OS defined container for wrapping a system genereated kernel message delivered to an application's main event loop or async window event loop
#elif defined(__APPLE__) && defined(TARGET_OS_IOS) && TARGET_OS_IOS
typedef uintptr_t  pba_platform_event_msg;
#endif

//Extend the OS Platform Defined Event Loop Message Enumerations (for defining/handling custom messages platform provided event queue on the main event loop)
#ifndef WM_APP
#define WM_APP 0x8000
#endif
#define PBA_EVENT_MSG_BASE_ID                       WM_APP

//System Run Loop Messages
#define PBA_EVENT_MSG_LOOP_QUIT                     PBA_EVENT_MSG_BASE_ID   + 1

//Real-Time Audio Thread Messages
#define PBA_EVENT_NOTE_TRIGGER                      PBA_EVENT_MSG_LOOP_QUIT + 1 // HID Event

//Application Run Loop Messages
#define PBA_EVENT_UMP_CONTROL                       PBA_EVENT_NOTE_TRIGGER  + 1 // HID Event


//#define PBA_EVENT_MSG_BASE_ID                       CR_PLATFORM_EVENT_MSG_LOOP_QUIT + 1
//#define CR_PLATFORM_WINDOW_EVENT_MSG_PAUSE            CR_PLATFORM_WINDOW_EVENT_MSG_BASE_ID
//#define CR_PLATFORM_WINDOW_EVENT_MSG_RESIZE            CR_PLATFORM_WINDOW_EVENT_MSG_BASE_ID + 1
//#define CR_PLATFORM_WINDOW_EVENT_MSG_SHOW            CR_PLATFORM_WINDOW_EVENT_MSG_BASE_ID + 2
//#define CR_PLATFORM_WINDOW_EVENT_MSG_RECREATE       CR_PLATFORM_WINDOW_EVENT_MSG_BASE_ID + 3
//#define CR_PLATFORM_WINDOW_EVENT_MSG_CLOSE            CR_PLATFORM_WINDOW_EVENT_MSG_BASE_ID + 4

//TO DO: Better distinguish between a scene event and simulation control event
//#define CR_PLATFORM_WINDOW_CONTROL_EVENT            CR_PLATFORM_WINDOW_EVENT_MSG_CLOSE + 1 // HID Event
//#define CR_SCENE_CONTROL_EVENT                        CR_PLATFORM_WINDOW_EVENT_MSG_CLOSE + 2 // Scene ECS Event
//#define CR_SIMULATION_CONTROL_EVENT                    CR_PLATFORM_WINDOW_EVENT_MSG_CLOSE + 3 // Scene Simulation Event

/*
//enumerate options for a bitfield option mask
typedef enum CR_PLATFORM_WINDOW_RESIZE_OPTION
{
    CR_PLATFORM_WINDOW_RESIZE_OPTION_NONE            = 1 << 0,    // 0000 0001, //Standard resize of window
    CR_PLATFORM_WINDOW_RESIZE_OPTION_BORDERLESS        = 1 << 1,    // 0000 0010,
    CR_PLATFORM_WINDOW_RESIZE_OPTION_FULLSCREEN        = 1 << 2    // 0000 0100, //Fullscreen option will make the win32 window the size of the screen

}CR_PLATFORM_WINDOW_RESIZE_OPTION;

//Make "CR_MAIN_EVENT_LOOP" substring synonomous with "CR_PLATFORM_EVENT_LOOP"
#define CR_MAIN_EVENT_LOOP CR_PLATFORM_EVENT_LOOP

*/

#pragma mark -- Core Render Platform Thread Handle ID

#ifdef CR_TARGET_WIN32
typedef HANDLE           pba_platform_thread;
typedef unsigned int     pba_platform_thread_id;
#define pba_stdcall __stdcall
#elif defined(__APPLE__)
//typedef dispatch_queue_t pba_platform_thread;
//typedef unsigned int     pba_platform_thread_id;
typedef pthread_t        pba_platform_thread;
typedef unsigned int     pba_platform_thread_id;
#define pba_stdcall
#endif

//#include "ReqlCoroutine.h"
//#include "CTError.h"
#ifdef _WIN32
//#define CTThread               HANDLE
//#define CTThreadID             unsigned int
typedef HANDLE                   PBAKernelQueueType;
typedef LPTHREAD_START_ROUTINE   CTThreadRoutine; //Win32 CRT Thread Routine
#define CRDispatchSource         void
//#define SSLContextRef          void*
//#define CTSocketError()        (WSAGetLastError())

typedef struct PBAKernelQueue
{
    PBAKernelQueueType kq;
    PBAKernelQueueType pq[2];
    intptr_t          pnInflightCursors;
}PBAKernelQueue;

typedef HANDLE PBAKernelQueueEvent;

static const int CTSOCKET_DEFAULT_BLOCKING_OPTION = 0;
#elif defined(__APPLE__) || defined(__FreeBSD__) //with libdispatch
//#define CTThread               pthread_t
//#define CTThreadID             unsigned int
typedef int                      PBAKernelQueueType; //kqueues are just file descriptors
typedef int                      CRKernelPipeType;  //pipes are just file descriptors
//typedef void *                 (*CTThreadRoutine)(void *); //pthread routine
#define CRDispatchSource         dispatch_source_t
typedef void(^CTDispatchSourceHandler)(void);      //clang block
//#define CTSocketError() (errno)

typedef struct PBAKernelQueue
{
    PBAKernelQueueType kq;
    PBAKernelQueueType pq[2];
    intptr_t          pnInflightCursors;
}PBAKernelQueue;

typedef struct kevent PBAKernelQueueEvent;

#define CR_INCOMING_PIPE    0
#define CR_OUTGOING_PIPE    1

#endif

#pragma mark -- Core Render Kernel Event Queue

/***
 *  "Kernel Event Queues" are useful for sending/receiving and waiting on events between threads and processes when standard provided event/message queue mechanisms provided by the OS to the application dont cut the mustard)
 *
 *    Core Render defines the notion of a kernel event queue as some implementation that allows handling of associated "Kernel Events" for the purposes of interprocess and interthread communication without incurring the cost of a User Space->Kernel Space->User Space memory transfer.
 *    Core Render defines the notion of a "Kernel Event" as an event that a process thread may idle on and "woken up" by the kernel to receive the memory associated with the event directly from the kernel.
 *
 *    Note that some platforms may not have a dedicated "Kernel Event Queue" data type or handle exposed but still have "Kernel Event" type that a process thread may idle on.  For example, Win32 thread handles and their associated event queues are one in the same (see comment below).
 *    When no sufficient "Kernel Event" mechanism can be provided by or implemented on top of the OS, a run loop must fall back to polling for events which is not suitable for real-time operation :(
 ***/

//Define Opaque OS Kernel Event Queue Data Type for Queue Mechanisms exposed by the OS Kernel
#ifdef CR_TARGET_WIN32
//For Win32 Interprocess Events, there is no queue manually created or exposed with an event that can be passed and triggered/waited on between processes as with BSD kqueue + pipe combination.
//For Win32 Interthread Messages/Events, every thread comes with a dedicated event queue "under the hood" in the kernel.
//The handles to these thread event/message queues themselves are not exposed as they can be identified and sent to by their corresponding thread address stored as unsigned int
//Thread "Messages"/Events can be retrieved from a thread's manually implemented run loop by calling the GetMessage/PeekMessage API ust like on the Win32 platform main thread message/event loop message pump.
typedef unsigned int  pba_thread_queue_id;  //On Win32, a platform thread queue id is the same as a platform thread id is the same as...
typedef unsigned int  pba_kernel_queue_id;  //a platform kernel queue id
#elif defined(__APPLE__)
typedef int           pba_kernel_queue_id;  //on BSD platforms kernel queues are just kqueues, and kqueue handles are just file descriptors
#endif


//Define Opaque Event Data Type for OS Kernel Event Queue
#ifdef CR_TARGET_WIN32
typedef HANDLE        pba_kernel_queue_event;
#elif defined(__APPLE__)
typedef struct kevent pba_kernel_queue_event;
#endif

#pragma mark -- Core Render Kernel Event Queue Event Enumerations


//example display sync process/thread kernel queue events
typedef enum crdisplay_event
{
    crdisplay_event_sync_running_start,
    crdisplay_event_vsync,                 //definitely vblank
    crdisplay_event_vblank_notification, //might be vblank, might be a premediated vblank notification, depending on the platform
    crdisplay_event_next_frame_time,
    crdisplay_event_idle,
    crdisplay_event_timeout,
    crdisplay_event_out_of_range
}crdisplay_event;

//example application event loop kernel queue events
typedef enum crevent_type
{
    crevent_init,
    crevent_exit,                   //we received a message to shutdown the applicaition
    crevent_menu,                   //we received an event as a result of user clicking the menu
    crevent_register_view,          //register a platform created view/layer backed by an accelerated gl context to render with Core Render
    crevent_main_window_changed,    //the application event loop was notifi
    crevent_platform_event,            //the application event loop received a platform event from the platform main event loop (or one of the window control threads, less likely)
    crevent_graphics,
    crevent_idle,
    crevent_timeout,
    crevent_out_of_range
}crevent_type;

typedef enum pba_system_event_group
{
    pba_no_group,
    pba_midi_input_connection,          //send a start/stop to indicate create/remove
    pba_midi_create_input_connection,   //send a start to indicate create
    pba_midi_remove_input_connection    //send a stop to indicate remove
}pba_system_event_group;

typedef enum pba_sysex_event_type
{
    //audio stream events
    pba_stream_init,
    pba_stream_start,
    pba_stream_stop,
    pba_stream_change_outputpass,
}pba_sysex_event_type;


//example application menu events
typedef enum crmenu_event
{
    crmenu_quit,
    crmenu_close_window,
    crmenu_toggle_fullscreen
}crmenu_event;

typedef enum crgraphics_event
{
    crgraphics_image_load

}crgraphics_event;


#pragma mark -- Core Render System Event Enumerations

//HID [Joystick] Control Events
typedef enum pba_control_event_type
{
    //use for state only; do not send as event
    pba_move_nan,

    //events
    pba_move_forward,
    pba_move_right,
    pba_move_up,
    pba_rotate_yaw,
    pba_rotate_pitch,
    //pba_possess_entity,
    pba_control_event_type_max

}pba_control_event_type;

//HID [Button] Control Events
typedef enum pba_button_event_type
{
    //use for state only; do not send as event
    pba_button_inactive       = 0, //button noop/released state
    pba_button_tap            = 1, //state that equates to a command that means generate a 'button  tapped event'
    pba_button_press          = 2, //state that equates to a command that means generate a 'button pressed event'
   
    //events
    pba_button_tapped         = pba_control_event_type_max+1, //button pressed with expected release event to follow
    pba_button_pressed        = pba_control_event_type_max+2, //button pressed with expected release event to follow
    pba_button_released       = pba_control_event_type_max+3, //button released
    pba_button_event_type_max = pba_button_released + 1

}pba_button_event_type;

//HID [Trigger] Control Events
/*
typedef enum pba_dpad_event_type
{
    pba_dpad_left  = pba_button_event_type_max,
    pba_dpad_right = pba_button_event_type_max + 1,
    pba_dpad_up    = pba_button_event_type_max + 2,
    pba_dpad_down  = pba_button_event_type_max + 3,
    pba_dpad_event_type_max = pba_dpad_down + 1
}pba_button_event_type;
*/

typedef struct pba_control_event
{
    uint64_t                event_time; //the time the event was issued
    union
    {
        double              value;
        uint64_t            subtype;
    };
    pba_control_event_type   type;
}pba_control_event;


//GPU Resource Events
typedef enum pba_resource_event_type
{
    pba_resource_nan,
    pba_resoure_load_mesh,
    pba_resource_load_mesh_list,
    pba_resource_event_type_max
}pba_resource_event_type;

typedef struct pba_resource_event
{
    uint64_t                event_time; //the time the event was issued
    void*                   rglrc;      //the graphics context/queue to load
    void*                   resource;   //the graphics context/queue to load
    PBAKernelQueueType       kq;         //the queue to dispatch to on async event completion
    pba_resource_event_type  type;
}pba_resource_event;

//ECS Scene Events
typedef enum pba_scene_event_type
{
    pba_scene_nan,
    pba_scene_add_entity,
    pba_scene_remove_entity,
    pba_scene_event_type_max
}pba_scene_event_type;

typedef struct pba_scene_event
{
    uint64_t                event_time; //the time the event was issued
    void*                   value;
    void*                   command;    //an optional command buffer to be executed on the receiving queue
    pba_scene_event_type     type;
}pba_scene_event;


typedef enum pba_simulation_event_type
{
    pba_sim_nan,
    pba_sim_time,
    pba_simulation_event_type_max,
}pba_simulation_event_type;

typedef struct pba_simulation_event
{
    uint64_t                 event_time; //the time the event was issued
    double                   value;
    pba_simulation_event_type type;
}pba_simulation_event;

  
/*
typedef struct pba_control_event_frame
{
    pba_control_event events[pba_control_event_type_max];
}pba_control_event_frame;
*/

#if CR_TARGET_WIN32
typedef void (CALLBACK* HIDCTRL) (UINT, struct pba_control_event*, void* ctx);
#else

#endif
 
#pragma mark -- Declare Opaque File Descriptor
    
#ifdef CR_TARGET_WIN32
typedef HANDLE      pba_file_descriptor;
static const char * pba_process_ext = ".exe";
#elif defined(CR_TARGET_OSX)
typedef int         pba_file_descriptor;
static const char * pba_process_ext = "";
#endif


#pragma mark -- Declare Main Thread Run Loop References

//PB_AUDIO_EXTERN pba_platform_thread    pba_mainThread;
//PB_AUDIO_EXTERN pba_platform_thread_id pba_mainThreadID;

//PB_AUDIO_EXTERN pba_platform_thread    pba_eventThread;
//PB_AUDIO_EXTERN pba_platform_thread_id pba_eventThreadID;

#pragma mark -- Declare Thread Local Storage IDs

//#ifdef CR_TARGET_WIN32
//PB_AUDIO_EXTERN DWORD                 pba_tlsIndex;
//#endif

#pragma mark -- Define Global Kernel Queues and corresponding Events (That live in crPlatform memory space)

//define a global vertical retrace notification event suitable for use on all platforms
//PB_AUDIO_EXTERN pba_kernel_queue_event pba_vBlankNotification;  //a native windows event associated with the display vertical retrace that can be waited on for thread synchronization
                                                                //(A HANDLE is just a void*, a kqueue is just an int file descriptor)

#pragma mark -- Declare Global Event Queues and Pipes (for responding/handling events sent to a threads queue or sending messages to queues between process, respectively)

#define CR_INCOMING_PIPE 0  //always use the same indexes for reading/writing to a pipe pair for posterity
#define CR_OUTGOING_PIPE 1  //always use the same indexes for reading/writing to a pipe pair for posterity

//a kqueue singleton designed to that lives in [Pb]Audio shared lib so it can be shared and used for kernel level communication between
//threads, processes and most importantly shared between application frameworks such as Cocoa and CoreRender, CoreTransport or [Pb]Audio

static const char * kPBAMainEventQueue     = "cr.3rdgen.[Pb]Audio.MainEventQueue";       //an identifier that can be used with pba_mainEventQueue
//static const char * kCRPlatformEventQueue = "cr.3rdgen.crPlatform.pba_platformEventQueue";  //an identifier that can be used with pba_platformEventQueue
//static const char * kCRDisplayEventQueue  = "cr.3rdgen.crPlatform.pba_DisplayEventQueue";   //an identifier that can be used with pba_DisplayEventQueue
//static const char * kCRResourceEventQueue = "cr.3rdgen.crPlatform.pba_ResourceEventQueue";  //an identifier that can be used with pba_DisplayEventQueue

//extern           pba_kernel_queue_id pba_platformEventQueue;  //a global kqueue singleton for injecting events into the platform event loop [if one exists, and it should]
//PB_AUDIO_EXTERN  pba_kernel_queue_id PBAudioEventQueue;       //a global kqueue singleton for the main Core Render application event loop
//PB_AUDIO_EXTERN  pba_kernel_queue_id pba_inputEventQueue;     //a global kqueue singleton for distributing user input updates from main thread run loop

//a global read/write pipe pair singleton for sending blob messages to to the pba_displayEventQueue (allows waking kqueue from a separate process)
//extern int pba_displayEventPipe[2];
//PB_AUDIO_EXTERN  PBAKernelQueue      pba_displayEventQueue;

extern int64_t pba_mainWindow;            //Main platform window id/handle
//extern crgc_view * pba_mainView;       //Main crgc_view reference

PB_AUDIO_API PB_AUDIO_INLINE PBAKernelQueue PBAKernelQueueCreate(void);

#ifndef CR_TARGET_WIN32
PB_AUDIO_API PB_AUDIO_INLINE uintptr_t pba_event_queue_wait_with_timeout(int kqueue, struct kevent * kev, int16_t eventFilter, uintptr_t timeoutEvent, uintptr_t outOfRangeEvent, uintptr_t eventRangeStart, uintptr_t eventRangeEnd, uint32_t timeout);
#endif
    
#ifdef __cplusplus
}
#endif

#endif /* pba_event_queue_h */
