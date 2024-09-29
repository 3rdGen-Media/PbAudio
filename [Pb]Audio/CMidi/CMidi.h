//
//  CTMidi.h
//  QUIC MIDI
//
//  Created by Joe Moulton on 12/26/22.
//

#ifndef CMidi_h
#define CMidi_h

/*
namespace WinRT
{
    enum WinRTMidiPortType { In, Out };
    enum WinRTMidiPortUpdateType { PortAdded, PortRemoved, EnumerationComplete };

    enum WinRTMidiErrorType {
        WINRT_NO_ERROR = 0,                         //no error
        WINRT_WINDOWS_RUNTIME_ERROR,                // unable to initialize Windows Runtime
        WINRT_WINDOWS_VERSION_ERROR,                // version of Windows does not support Windows::Devices::Midi api
        WINRT_PORTWATCHER_INITIALIZATION_ERROR,     // error initialize midi port watcher
        WINRT_INVALID_PORT_INDEX_ERROR,             // requested port index is out of range
        WINRT_OPEN_PORT_ERROR,                      // open midi port error
        WINRT_INVALID_PARAMETER_ERROR,
        WINRT_MEMORY_ERROR,
        WINRT_UNSPECIFIED_ERROR
    };

    typedef void* WinRTMidiPtr;
    typedef void* WinRTMidiPortWatcherPtr;
    typedef void* WinRTMidiInPortPtr;
    typedef void* WinRTMidiOutPortPtr;

    // Midi port changed callback
    typedef void(*MidiPortChangedCallback) (const WinRTMidiPortWatcherPtr portWatcher, WinRTMidiPortUpdateType update);

    // Midi In callback
    typedef void(*WinRTMidiInCallback) (const WinRTMidiInPortPtr port, double timeStamp, const unsigned char* message, unsigned int nBytes);

    // WinRT Midi Functions
    typedef WinRTMidiErrorType(__cdecl* WinRTMidiInitializeFunc)(MidiPortChangedCallback callback, WinRTMidiPtr* midi);
    WINRTMIDI_API WinRTMidiErrorType __cdecl winrt_initialize_midi(MidiPortChangedCallback callback, WinRTMidiPtr* winrtMidi);

    typedef void(__cdecl* WinRTMidiFreeFunc)(WinRTMidiPtr midi);
    WINRTMIDI_API void __cdecl winrt_free_midi(WinRTMidiPtr midi);

    typedef const WinRTMidiPortWatcherPtr(__cdecl* WinRTMidiGetPortWatcherFunc)(WinRTMidiPtr midi, WinRTMidiPortType type);
    WINRTMIDI_API const WinRTMidiPortWatcherPtr __cdecl winrt_get_portwatcher(WinRTMidiPtr midi, WinRTMidiPortType type);

    // WinRT Midi In Port Functions
    typedef WinRTMidiErrorType(__cdecl* WinRTMidiInPortOpenFunc)(WinRTMidiPtr midi, unsigned int index, WinRTMidiInCallback callback, WinRTMidiInPortPtr* midiPort);
    WINRTMIDI_API WinRTMidiErrorType __cdecl winrt_open_midi_in_port(WinRTMidiPtr midi, unsigned int index, WinRTMidiInCallback callback, WinRTMidiInPortPtr* midiPort);

    typedef void(__cdecl* WinRTMidiInPortFreeFunc)(WinRTMidiInPortPtr port);
    WINRTMIDI_API void __cdecl winrt_free_midi_in_port(WinRTMidiInPortPtr port);

    // WinRT Midi Out Port Functions
    typedef WinRTMidiErrorType(__cdecl* WinRTMidiOutPortOpenFunc)(WinRTMidiPtr midi, unsigned int index, WinRTMidiOutPortPtr* midiPort);
    WINRTMIDI_API WinRTMidiErrorType __cdecl winrt_open_midi_out_port(WinRTMidiPtr midi, unsigned int index, WinRTMidiOutPortPtr* midiPort);

    typedef void(__cdecl* WinRTMidiOutPortFreeFunc)(WinRTMidiOutPortPtr port);
    WINRTMIDI_API void __cdecl winrt_free_midi_out_port(WinRTMidiOutPortPtr port);

    typedef void(__cdecl* WinRTMidiOutPortSendFunc)(WinRTMidiOutPortPtr port, const unsigned char* message, unsigned int nBytes);
    WINRTMIDI_API void __cdecl winrt_midi_out_port_send(WinRTMidiOutPortPtr port, const unsigned char* message, unsigned int nBytes);

    // WinRT Midi Watcher Functions
    typedef unsigned int(__cdecl* WinRTWatcherPortCountFunc)(WinRTMidiPortWatcherPtr watcher);
    WINRTMIDI_API unsigned int __cdecl winrt_watcher_get_port_count(WinRTMidiPortWatcherPtr watcher);

    typedef const char* (__cdecl* WinRTWatcherPortNameFunc)(WinRTMidiPortWatcherPtr watcher, unsigned int index);
    WINRTMIDI_API const char* __cdecl winrt_watcher_get_port_name(WinRTMidiPortWatcherPtr watcher, unsigned int index);

    typedef WinRTMidiPortType(__cdecl* WinRTWatcherPortTypeFunc)(WinRTMidiPortWatcherPtr watcher);
    WINRTMIDI_API WinRTMidiPortType __cdecl winrt_watcher_get_port_type(WinRTMidiPortWatcherPtr watcher);
};
*/

/*************************************************************************
 * Compiler- and platform-specific preprocessor work
 *************************************************************************/

/* If we are we on Views, we want a single define for it.
 */
#if !defined(_WIN32) && (defined(__WIN32__) || defined(WIN32) || defined(__MINGW32__))
 #define _WIN32
#endif /* _WIN32 */


#if defined(CMIDI_DLL) && defined(_CMIDI_BUILD_DLL)
 /* CRGC_DLL must be defined by applications that are linking against the DLL
  * version of the CRGC library.  _CRGC_BUILD_DLL is defined by the CRGC
  * configuration header when compiling the DLL version of the library.
  */
 #error "You may not have both PB_AUDIO_DLL and _PB_AUDIO_BUILD_DLL defined"
#endif

/* CR_PRIMITIVES_API is used to declare public API functions for export
 * from the DLL / shared library / dynamic library.
 */
#if defined(_WIN32) && defined(_CMIDI_BUILD_DLL)
 /* We are building crMath as a Win32 DLL */
 #define CMIDI_API extern "C" __declspec(dllexport)
#elif defined(_WIN32) && defined(CMIDI_DLL)
 /* We are calling crMath as a Win32 DLL */
 #define CMIDI_API extern "C" __declspec(dllimport)
#elif defined(__GNUC__) && defined(_CMIDI_BUILD_DLL)
 /* We are building CR_PRIMITIVES as a shared / dynamic library */
 #define CMIDI_API __attribute__((visibility("default")))
#else
 /* We are building or calling crMath as a static library */
 #define CMIDI_API
#endif
 
//inline doesn't exist in C89, __inline is MSVC specific
#ifndef CMIDI_INLINE
#ifdef _WIN32
#define CMIDI_INLINE __inline
#else
#define CMIDI_INLINE //__inline__ __attribute__((noinline))
#endif
#endif
    
//__decspec doesn't exist in C89, __declspec is MSVC specific
#ifndef CMIDI_DECLSPEC
#ifdef _WIN32
#define CMIDI_DECLSPEC __declspec ()
#else
#define CMIDI_DECLSPEC
#endif
#endif
    
//align functions are diffent on windows vs iOS, Linux, etc.
#ifndef CMIDI_ALIGN//(X)
#ifdef _WIN32
#define CMIDI_ALIGN(X) (align(X))
#else
#define CMIDI_ALIGN(X) __attribute__ ((aligned(X)))
#endif
#endif

/* We are building or calling CRPrimitives as a static library */
#ifdef _WIN32
#define CMIDI_EXTERN
#else
#define CMIDI_EXTERN extern
#endif

//Core
#include "CMidiMessage.h"
#include "CMidiDriverID.h"
#include "CMidiDevice.h"
#include "CMidiEndpoint.h"

//Non-Essential Utilities
#include "CMEventQueue.h"

#ifdef __APPLE__
#include <CoreMIDI/MIDIThruConnection.h>             //Core Midi Thru Connections
#include <CoreMIDI/MIDIServices.h>                   //Core Midi Create Client Ports
#endif

#define MAX_IAC_NUM              63

#define CM_MAX_SOURCES           64
#define CM_MAX_DESTINATIONS      64
#define CM_MAX_CONNECTIONS       64
#define CM_MAX_SOFT_CONNECTIONS  64


#ifdef __APPLE__
CMIDI_EXTERN const CFStringRef kCMidiSourcesAvailableChangedNotification;
CMIDI_EXTERN const CFStringRef kCMidiDestinationsAvailableChangedNotification;
#else


typedef int32_t ItemCount;
typedef HRESULT OSStatus;

//Block Parameters
typedef void* MIDINotification;

typedef enum MIDIProtocolID 
{
    kMIDIProtocol_1_0 = 1,
    kMIDIProtocol_2_0 = 2,
} MIDIProtocolID;

typedef struct MIDIEventPacket
{
    CMTimestamp timeStamp; //The event packet timestamp.
    uint32_t    wordCount; //The number of valid MIDI 32 - bit words in this event packet.
    uint32_t    words[64]; //A variable - length stream of native - endian 32 - bit Universal MIDI Packets(UMP).
}MIDIEventPacket;

typedef struct MIDIEventList
{
    MIDIProtocolID  protocol; //The MIDI protocol variant of the events in the list.
    uint32_t        numPackets;
    MIDIEventPacket packet[1];
}MIDIEventList;


//Block Functions
#ifndef __BLOCKS__
typedef void(*MIDINotifyBlock)   (const MIDINotification* msg);
typedef void(*MIDIReceiveBlock)  (const MIDIEventList* evtlist, void* srcConnRefCon);

//typedef void* MIDIClientRef;  //shared_ptr<MIDISession>
typedef MIDIReceiveBlock MIDIPortRef;      //C-Style Receive/Send Block Callback

#else
#error "CMidi Clang blocks need to be defined"
#endif


//Apple:   Not used for input connections, only thru/soft thru connections
//Windows: The GUID of the WinRT MidiEndpointConnection
typedef GUID   MIDIThruConnectionRef;

#endif

#ifndef __APPLE__
typedef struct MIDIThruConnectionEndpoint
{
    MIDIEndpointRef endpointRef;
    int32_t         uniqueID;

}MIDIThruConnectionEndpoint;

typedef struct MIDIThruConnectionParams
{
    MIDIThruConnectionEndpoint sources[8], destinations[8];
    uint8_t                    channelMap[16];

    uint32_t                   numSources, numDestinations;
    uint8_t                    lowNote, highNote;
    uint8_t                    lowVelocity, highVelocity;

}MIDIThruConnectionParams;
#endif

/*
//A Device is just a collection of displays + controls
typedef struct MCUDevice
{
    CMControl    controls[4][8];
    CMDeviceType type;
}MCUDevice;
*/

typedef struct CMSource
{
    MIDIEndpointRef endpoint;
    char            name[256];
    CMDriverID      driverID;
#ifdef __APPLE__
    int32_t         uniqueID; //CoreMIDI
#else
    uintptr_t        uniqueID; //WMS hstring
#endif
}CMSource;

typedef CMSource CMDestination;

typedef struct CMConnection
{
    MIDIThruConnectionRef    connection;
    CMSource                 source;
    CMDestination            destination;
    char                     name[256];
    MIDIThruConnectionParams params;

    int64_t                  eventToken;
}CMConnection;

#ifdef __APPLE__
#define CMConnectionEmpty 0
#else
#define CMConnectionEmpty {0}
#endif

//Forward Declare Private Client Context Definition
struct CMClientContext;

#ifdef __APPLE__
typedef struct CMClientContext
{
    MIDIClientRef client;             // Client handle to the MIDI server 
    MIDIPortRef   inPort;             // Input port handle  
    MIDIPortRef   outPort;            // Output port handle
    MIDIPortRef   proxyPort;          // Seocndary Input port handle

    //CoreMIDI
    ItemCount numDevices;
    ItemCount numSources;
    ItemCount numDestinations;
    
    //CMDOM
    ItemCount numInputConnections;      //CMInputConnection:    Input Midi Connections Only
    ItemCount numThruConnections;       //CMThruConnection:     System Thru Connections
    ItemCount numSoftThruConnections;   //CMSoftThruConnection: Application Thru Connections
    ItemCount numProxyConnections;      //CMProxyConnections:   ...
    ItemCount numHardwareDevices;

    //CoreMIDI
    CMSource      sources[CM_MAX_SOURCES];
    CMDestination destinations[CM_MAX_DESTINATIONS];
    
    //CMDOM
    CMConnection  inputConnections[CM_MAX_CONNECTIONS];
    CMConnection  thruConnections[CM_MAX_CONNECTIONS];
    CMConnection  softThruConnections[CM_MAX_SOFT_CONNECTIONS];
    CMConnection  proxyConnections[CM_MAX_SOFT_CONNECTIONS];

    CMDeviceDescription  hardwareDevices[CM_MAX_SOFT_CONNECTIONS];
    
    uint8_t       activeDevice;
    //char        isIAC[MAX_IAC_NUM + 1];           
}CMClientContext;
#endif

#ifndef _CMIDI_BUILD_DLL
extern struct CMClientContext CMClient; //expose internal memory directly for static libs
#endif

/*
static void CMidiStatusNotification(const MIDINotification *msg)
{
    //for debugging, trace change notifications:
    const char *descr[] = {
        "undefined (0)",
        "kMIDIMsgSetupChanged",
        "kMIDIMsgObjectAdded",
        "kMIDIMsgObjectRemoved",
        "kMIDIMsgPropertyChanged",
        "kMIDIMsgThruConnectionsChanged",
        "kMIDIMsgSerialPortOwnerChanged",
        "kMIDIMsgIOError"};

    fprintf(stderr, "MIDI Notify, messageID %d (%s)\n", (int) msg->messageID,
           descr[(int) msg->messageID]);
    
    return;
}
*/

//typedef void (*MIDINotifyProc)(const MIDINotification *message, void *refCon);
//typedef void (^MIDINotifyBlock)(const MIDINotification *message);
//static void CMidiNotifyProc(const MIDINotification *msg, void *refCon) { CMidiStatusNotification(msg); }
//static MIDINotifyBlock CMidiNotifyBlock = ^void(const MIDINotification *msg) { CMidiStatusNotification(msg); };

void          CMInitSourceEndpoint(MIDIThruConnectionEndpoint* endpoint, int source);

void          CMInitThruParamEndpoints(MIDIThruConnectionParams* thruParams, int * sources, int numSources, int * destinations, int numDestinations);
void          CMInitThruParams(MIDIThruConnectionParams* thruParams, int * sources, int numSources, int * destinations, int numDestinations);
void          CMSaveThruConnectionParams(CMConnection * thruConnection);

//Persistent Thru Connections
CMConnection* CMCreateThruConnectionAtIndex(const char * thruID, MIDIThruConnectionParams* thruParams, unsigned long thruIndex);
CMConnection* CMCreateThruConnection(const char * thruID, MIDIThruConnectionParams* thruParams);
OSStatus      CMDeleteThruConnection(const char* thruID);
void          CMReplaceThruConnectionAtIndex(CMConnection * conn, const char * thruID, unsigned long thruIndex);

//"Soft" Thru Connections
CMConnection* CMCreateSoftThruConnectionAtIndex(const char * thruID, MIDIThruConnectionParams* thruParams, unsigned long thruIndex);
CMConnection* CMCreateSoftThruConnection(const char * thruID, MIDIThruConnectionParams* thruParams);
OSStatus      CMDeleteSoftThruConnection(const char* thruID);

//void CMReplaceSoftThruConnectionAtIndex(CMConnection * conn, const char * thruID, unsigned long thruIndex);

//Proxy Connections
CMConnection* CMCreateProxyConnectionAtIndex(const char * thruID, MIDIThruConnectionParams* thruParams, unsigned long thruIndex);
CMConnection* CMCreateProxyConnection(const char * thruID, MIDIThruConnectionParams* thruParams);
OSStatus      CMDeleteProxyConnection(const char* thruID);
//void CMReplaceSoftThruConnectionAtIndex(CMConnection * conn, const char * thruID, unsigned long thruIndex);

//Input Connections
                       CMConnection* CMCreateInputConnectionAtIndex(const char * inputID, CMSource* sourceEndpoint, unsigned long sourceIndex, unsigned long inputIndex);
CMIDI_API CMIDI_INLINE CMConnection* CMCreateInputConnection(uintptr_t uniqueID);//MIDIThruConnectionEndpoint* endpoint)
CMIDI_API CMIDI_INLINE OSStatus      CMDeleteInputConnection(uintptr_t UniqueID);

//Hardware Devices
OSStatus CMDeleteHardwareDevice(const char * deviceID);
void     CMCreateHardwareDevice(/*const char * deviceID,*/ CMDeviceDescription * deviceDescription, CMDisplay ** deviceDisplays, CMControl ** deviceControls, CMControl ** deviceSwitches);

CMIDI_API CMIDI_INLINE ItemCount CMGetNumberOfDevices(void);
CMIDI_API CMIDI_INLINE ItemCount CMUpdateInputDevices(void);
CMIDI_API CMIDI_INLINE ItemCount CMUpdateOutputDevices(void);
CMIDI_API CMIDI_INLINE ItemCount CMUpdateThruConnections(void);

CMIDI_API CMIDI_INLINE const CMSource*      CMGetSource(int srcIndex);
CMIDI_API CMIDI_INLINE const CMDestination* CMGetDestination(int dstIndex);

CMIDI_API CMIDI_INLINE OSStatus CMClientCreate(const char * clientID, MIDINotifyBlock midiNotifyBlock, MIDIReceiveBlock midiReceiveBlock, MIDIReceiveBlock proxyReceiveBlock);
//CMIDI_API CMIDI_INLINE OSStatus CMClientDestroy(CMClientContext* clientContext);

//#pragma mark -- CTConnection API Method Function Pointer Definitions
//typedef int (*CTConnectFunc)(struct CTTarget * service, CTConnectionClosure callback);
typedef OSStatus       (*CMidiClientCreateFunc)  (const char* clientID, MIDINotifyBlock midiNotifyBlock, MIDIReceiveBlock midiReceiveBlock, MIDIReceiveBlock proxyReceiveBlock);
typedef ItemCount      (*CMidiUpdateCountFunc)       (void);
typedef const CMSource*      (*CMidiSourceFunc)            (int srcIndex);
typedef const CMDestination* (*CMidiDestinationFunc)       (int dstIndex);

typedef CMConnection*  (*CMidiCreateConnectionFunc)   (uintptr_t uniqueID);
typedef OSStatus       (*CMidiDeleteConnectionFunc)   (uintptr_t uniqueID);

//#pragma mark -- Global ReqlClientDriver Object
CMIDI_DECLSPEC typedef struct CMClientDriver
{
    //The Client Driver Object can create Reql Connections
    CMidiClientCreateFunc             Init;
    
    //Manage client list of devices
    CMidiUpdateCountFunc              UpdateInputDevices;
    CMidiUpdateCountFunc              UpdateOutputDevices;

    //Retrieve individual endpoints from list
    CMidiSourceFunc                   Source;
    CMidiDestinationFunc              Destination;

    //Create endpoint connections
    CMidiCreateConnectionFunc         CreateInputConnection;
    CMidiCreateConnectionFunc         CreateOutputConnection;

    //Remove endpoint connections
    CMidiDeleteConnectionFunc         DeleteInputConnection;
    CMidiDeleteConnectionFunc         DeleteOutputConnection;

    cm_kernel_queue_id                triggerEventQueue; //a global kqueue singleton for the main Core Midi Event Loop to IPC with real-time audio threads

}CMClientDriver;

//Load functions from DLL
static OSStatus cmidi_ext_load(CMClientDriver* client);
static OSStatus cmidi_ext_load_init(const char* clientID, MIDINotifyBlock midiNotifyBlock, MIDIReceiveBlock midiReceiveBlock, MIDIReceiveBlock proxyReceiveBlock);

#ifdef __APPLE__ //TO DO: Provide an appropriate way to optionally expose static lib function population
static const CMClientDriver CMidi = { CMClientCreate, CMUpdateInputDevices, CMUpdateOutputDevices, CMGetSource, CMGetDestination, CMCreateInputConnection, NULL, CMDeleteInputConnection, NULL };
#else
static CMClientDriver CMidi = { cmidi_ext_load_init };// { CMClientCreate };

#include <stdio.h>
#include <tchar.h>
#include <assert.h>

static OSStatus cmidi_ext_load(CMClientDriver* client)
{
    HMODULE cmidi_dll;
    OSStatus ret = 0;

#ifdef _DEBUG
    cmidi_dll = LoadLibraryEx(_T("CMidid.dll"), NULL, 0);
#else
    cmidi_dll = LoadLibraryEx(_T("CMidi.dll"), NULL, 0);
#endif

    if (!cmidi_dll)
    {
        fprintf(stderr, "\ncmidi_dll.dll not found.\n");
        return -1;
    }

    //Load CMidi API functions from dll
    client->Init                  = (CMidiClientCreateFunc)   GetProcAddress(cmidi_dll, "CMClientCreate");             assert(client->Init);

    client->UpdateInputDevices    = (CMidiUpdateCountFunc)    GetProcAddress(cmidi_dll, "CMUpdateInputDevices");       assert(client->UpdateInputDevices);
    client->UpdateOutputDevices   = (CMidiUpdateCountFunc)    GetProcAddress(cmidi_dll, "CMUpdateOutputDevices");      assert(client->UpdateInputDevices);

    client->Source                = (CMidiSourceFunc)         GetProcAddress(cmidi_dll, "CMGetSource");                assert(client->Source);
    client->Destination           = (CMidiDestinationFunc)    GetProcAddress(cmidi_dll, "CMGetDestination");           assert(client->Destination);

    client->CreateInputConnection = (CMidiCreateConnectionFunc)GetProcAddress(cmidi_dll, "CMCreateInputConnection");   assert(client->CreateInputConnection);
    //client->CreateOutputConnection = (CMidiCreateConnectionFunc)GetProcAddress(cmidi_dll, "CMCreateOutputConnection"); assert(client->CreateOutputConnection);

    client->DeleteInputConnection = (CMidiDeleteConnectionFunc)GetProcAddress(cmidi_dll, "CMDeleteInputConnection");   assert(client->DeleteInputConnection);
    //client->DeleteOutputConnection = (CMidiDeleteConnectionFunc)GetProcAddress(cmidi_dll, "CMDeleteOutputConnection"); assert(client->DeleteOutputConnection);

    return ret;
}

#endif

static OSStatus cmidi_ext_load_init(const char* clientID, MIDINotifyBlock midiNotifyBlock, MIDIReceiveBlock midiReceiveBlock, MIDIReceiveBlock proxyReceiveBlock)
{
    OSStatus ret = 0;
#ifdef __APPLE__
    //CMidi.Init = CMClientCreate;
#else
    //Load CMidi API functions from DLL
    ret = cmidi_ext_load(&CMidi);
#endif
    
    //Call the Init function loaded from dll
    CMidi.Init(clientID, midiNotifyBlock, midiReceiveBlock, proxyReceiveBlock);

    return ret;
}



#endif /* CTMidi_h */
