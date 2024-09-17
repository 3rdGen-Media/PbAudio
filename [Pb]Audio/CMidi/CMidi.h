//
//  CTMidi.h
//  QUIC MIDI
//
//  Created by Joe Moulton on 12/26/22.
//

#ifndef CMidi_h
#define CMidi_h

//Core
#include "CMidiMessage.h"
#include "CMidiDevice.h"
//#include "CMidiInput.h"
#include "CMidiEndpoint.h"

//Non-Essential Utilities
#include "CMEventQueue.h"

#include <CoreMIDI/MIDIThruConnection.h>             //Core Midi Thru Connections
#include <CoreMIDI/MIDIServices.h>                   //Core Midi Create Client Ports

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

CMIDI_EXTERN const CFStringRef kCMidiSourcesAvailableChangedNotification;
CMIDI_EXTERN const CFStringRef kCMidiDestinationsAvailableChangedNotification;

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
    SInt32          uniqueID;
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
OSStatus CMDeleteInputConnection(SInt32 UniqueID);
CMConnection* CMCreateInputConnectionAtIndex(const char * inputID, CMSource* sourceEndpoint, unsigned long inputIndex);
CMConnection* CMCreateInputConnection(SInt32 uniqueID);//MIDIThruConnectionEndpoint* endpoint)


//Hardware Devices
OSStatus CMDeleteHardwareDevice(const char * deviceID);
void CMCreateHardwareDevice(/*const char * deviceID,*/ CMDeviceDescription * deviceDescription, CMDisplay ** deviceDisplays, CMControl ** deviceControls, CMControl ** deviceSwitches);

ItemCount CMUpdateInputDevices(void);
ItemCount CMUpdateOutputDevices(void);
ItemCount CMUpdateThruConnections(void);
OSStatus CMClientCreate(const char * clientID, MIDINotifyBlock midiNotifyBlock, MIDIReceiveBlock midiReceiveBlock, MIDIReceiveBlock proxyReceiveBlock);

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
