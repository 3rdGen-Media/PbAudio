//
//  CTMidi.h
//  QUIC MIDI
//
//  Created by Joe Moulton on 12/26/22.
//

#ifndef CMidi_h
#define CMidi_h


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
 #define CMIDI_API __declspec(dllexport)
#elif defined(_WIN32) && defined(CMIDI_DLL)
 /* We are calling crMath as a Win32 DLL */
 #define CMIDI_API__declspec(dllimport)
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
#define CMIDI_DECLSPEC __declspec
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
//#include "CMidiInput.h"
#include "CMidiEndpoint.h"

//Non-Essential Utilities
#include "CMEventQueue.h"

#ifdef __APPLE__
#include <CoreMIDI/MIDIThruConnection.h>             //Core Midi Thru Connections
#include <CoreMIDI/MIDIServices.h>                   //Core Midi Create Client Ports
#endif

#define MAX_IAC_NUM         63

#define CM_MAX_SOURCES      64
#define CM_MAX_DESTINATIONS 64
#define CM_MAX_CONNECTIONS  64
#define CM_MAX_SOFT_CONNECTIONS  64

/***/

/* We are building or calling CMidi as a static library */
#ifdef _WIN32
#define CMIDI_EXTERN
#else
#define CMIDI_EXTERN extern
#endif

#ifdef __APPLE__
CMIDI_EXTERN const CFStringRef kCMidiSourcesAvailableChangedNotification;
CMIDI_EXTERN const CFStringRef kCMidiDestinationsAvailableChangedNotification;
#else

//typedef void* MIDIEndpointRef;
typedef void* MIDIThruConnectionRef;

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
    int32_t         uniqueID;
}CMSource;

typedef CMSource CMDestination;

typedef struct CMConnection
{
    MIDIThruConnectionRef    connection;
    CMSource                 source;
    CMDestination            destination;
    char                     name[256];
    MIDIThruConnectionParams params;
    
}CMConnection;

#ifndef __APPLE__
typedef void* MIDIClientRef;
typedef void* MIDIPortRef;
typedef void* MIDIClientRef;
typedef int32_t ItemCount;
typedef HRESULT OSStatus;

typedef void* MIDINotifyBlock;
typedef void* MIDIReceiveBlock;

#endif

typedef struct CMClientContext
{
    MIDIClientRef client;             /* Client handle to the MIDI server */
    MIDIPortRef   inPort;             /* Input port handle  */
    MIDIPortRef   outPort;            /* Output port handle */
    MIDIPortRef   proxyPort;          /* Seocndary Input port handle */

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
    //char        isIAC[MAX_IAC_NUM + 1];           /* is device an IAC device */
}CMClientContext;

extern struct CMClientContext CMClient;

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

void CMInitSourceEndpoint(MIDIThruConnectionEndpoint* endpoint, int source);

void CMInitThruParamEndpoints(MIDIThruConnectionParams* thruParams, int * sources, int numSources, int * destinations, int numDestinations);
void CMInitThruParams(MIDIThruConnectionParams* thruParams, int * sources, int numSources, int * destinations, int numDestinations);
void CMSaveThruConnectionParams(CMConnection * thruConnection);

//Persistent Thru Connections
OSStatus CMDeleteThruConnection(const char * thruID);
CMConnection* CMCreateThruConnectionAtIndex(const char * thruID, MIDIThruConnectionParams* thruParams, unsigned long thruIndex);
CMConnection* CMCreateThruConnection(const char * thruID, MIDIThruConnectionParams* thruParams);
void CMReplaceThruConnectionAtIndex(CMConnection * conn, const char * thruID, unsigned long thruIndex);

//"Soft" Thru Connections
OSStatus CMDeleteSoftThruConnection(const char * thruID);
CMConnection* CMCreateSoftThruConnectionAtIndex(const char * thruID, MIDIThruConnectionParams* thruParams, unsigned long thruIndex);
CMConnection* CMCreateSoftThruConnection(const char * thruID, MIDIThruConnectionParams* thruParams);
//void CMReplaceSoftThruConnectionAtIndex(CMConnection * conn, const char * thruID, unsigned long thruIndex);

//Proxy Connections
OSStatus CMDeleteProxyConnection(const char * thruID);
CMConnection* CMCreateProxyConnectionAtIndex(const char * thruID, MIDIThruConnectionParams* thruParams, unsigned long thruIndex);
CMConnection* CMCreateProxyConnection(const char * thruID, MIDIThruConnectionParams* thruParams);
//void CMReplaceSoftThruConnectionAtIndex(CMConnection * conn, const char * thruID, unsigned long thruIndex);

//Input Connections
OSStatus CMDeleteInputConnection(int32_t UniqueID);
CMConnection* CMCreateInputConnectionAtIndex(const char * inputID, CMSource* sourceEndpoint, unsigned long inputIndex);
CMConnection* CMCreateInputConnection(int32_t uniqueID);//MIDIThruConnectionEndpoint* endpoint)


//Hardware Devices
OSStatus CMDeleteHardwareDevice(const char * deviceID);
void CMCreateHardwareDevice(/*const char * deviceID,*/ CMDeviceDescription * deviceDescription, CMDisplay ** deviceDisplays, CMControl ** deviceControls, CMControl ** deviceSwitches);

ItemCount CMGetNumberOfDevices(void);
ItemCount CMUpdateInputDevices(void);
ItemCount CMUpdateOutputDevices(void);
ItemCount CMUpdateThruConnections(void);
CMIDI_API CMIDI_INLINE OSStatus CMClientCreate(const char * clientID, MIDINotifyBlock midiNotifyBlock, MIDIReceiveBlock midiReceiveBlock, MIDIReceiveBlock proxyReceiveBlock);

//#pragma mark -- CTConnection API Method Function Pointer Definitions
//typedef int (*CTConnectFunc)(struct CTTarget * service, CTConnectionClosure callback);
typedef OSStatus (*CMidiClientCreateFunc)  (const char* clientID, MIDINotifyBlock midiNotifyBlock, MIDIReceiveBlock midiReceiveBlock, MIDIReceiveBlock proxyReceiveBlock);
//typedef void     (^CMidiClientReceiveBlock)(const MIDIEventList *evtlist, void * __nullable srcConnRefCon);

//#pragma mark -- Global ReqlClientDriver Object
typedef struct CMClientDriver
{
    //The Client Driver Object can create Reql Connections
    CMidiClientCreateFunc             init;
}CMClientDriver;

static const CMClientDriver CMidi = {CMClientCreate};

#endif /* CTMidi_h */
