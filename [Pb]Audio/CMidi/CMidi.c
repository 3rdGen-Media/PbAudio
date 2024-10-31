//
//  CMIDI.c
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/27/22.
//

#include "CMidi.h"


#ifdef CM_TARGET_WIN32
#include <assert.h>

//#include "CMidi/cm_winrt_ext.h"

//The import library for RoInitialize,RoUninitialize,RoGetActivationFactory is runtimeobject.lib 
//(which the MSDN documentation fails to mention).It can be found in the Windows SDK for Windows 8.
#pragma comment (lib, "runtimeobject.lib")

//It's a good idea to #include <unknwn.h> explicitly whenever you use winrt::get_unknown, 
//even if that header has been included by another header.
#include <unknwn.h>

#include <iostream>

//WRT
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Enumeration.h>

//runtimeobject.lib
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <stdio.h>

using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

//Midi2
#include <winrt/Microsoft.Windows.Devices.Midi2.h>
#include <winrt/Microsoft.Windows.Devices.Midi2.Diagnostics.h>
#include <winrt/Microsoft.Windows.Devices.Midi2.Messages.h>
#include <winrt/Microsoft.Windows.Devices.Midi2.Initialization.h>

using namespace winrt::Microsoft::Windows::Devices::Midi2;                  // SDK Core
using namespace winrt::Microsoft::Windows::Devices::Midi2::Diagnostics;     // For diagnostics loopback endpoints
using namespace winrt::Microsoft::Windows::Devices::Midi2::Messages;        // For message utilities and strong types
using namespace winrt::Microsoft::Windows::Devices::Midi2::Initialization;  // for code to check if the service is installed/running


// where you find types like IAsyncOperation, IInspectable, etc.
namespace foundation = winrt::Windows::Foundation;

std::string BooleanToString(bool value)
{
    if (value) return "true";
    else       return "false";
}

static void LPWSTR_2_CHAR(LPWSTR in_char, LPSTR out_char, size_t str_len)
{
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, in_char, -1, out_char, str_len, NULL, NULL);
}

#endif

#ifdef __APPLE__
const CFStringRef kCMidiSourcesAvailableChangedNotification      = CFSTR("CMidiSourcesAvailableChangedNotification");
const CFStringRef kCMidiDestinationsAvailableChangedNotification = CFSTR("CMidiDestinationsAvailableChangedNotification");
#else

#pragma mark -- Client Context

typedef std::shared_ptr<MidiSession>                   MIDIClientRef;   //MIDISession ref counted ptr

#if 0 //Private MIDIEndpointRef dependent definitions

typedef std::shared_ptr<MidiEndpointDeviceInformation> MIDIEndpointRef; //MIDISession ref counted ptr

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
    uintptr_t       uniqueID; //WMS hstring
#endif
}CMSource;

typedef CMSource CMDestination;

typedef struct CMConnection
{
    MIDIThruConnectionRef    connection;
    struct CMSource          source;
    struct CMDestination     destination;
    char                     name[256];
    MIDIThruConnectionParams params;

    int64_t                  eventToken;
}CMConnection;

#endif

#endif

#ifndef __APPLE__
typedef struct CMClientContext
{
    union
    {
        MIDIClientRef client;
        MIDIClientRef session;
    };

    MIDIPortRef   inPort;
    MIDIPortRef   outPort;
    MIDIPortRef   proxyPort;

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

#ifdef __cplusplus
    //CMClientContext(MIDIClientRef& midiSession) { session = midiSession; };
    ~CMClientContext() { };

    //CMClientContext(const CMClientContext&) = default;
    //CMClientContext& operator=(const CMClientContext&) = default;
#endif

}CMClientContext;
#endif

#ifdef _WIN32
// Wire up an event handler to receive the message. There is a single event handler type, but the
// MidiMessageReceivedEventArgs class provides the different ways to access the data
// Your event handlers should return quickly as they are called synchronously.

auto UniversalMessagePacketHandler(CMClientContext* midiClient, CMConnection* connection)
{
    auto MessageReceivedHandler = [&, midiClient, connection](foundation::IInspectable const& /*sender*/, MidiMessageReceivedEventArgs const& args)
    {
            // there are several ways to get the message data from the arguments. If you want to use
            // strongly-typed UMP classes, then you may start with the GetUmp() method. The GetXXX calls 
            // are all generating something within the function, so you want to call them once and then
            // keep the result around in a variable if you plan to refer to it multiple times. In 
            // contrast, the FillXXX functions will update values in provided (pre-allocated) types
            // passed in to the functions.
            auto ump = args.GetMessagePacket();

            //std::cout << std::endl;
            //std::cout << "Received UMP" << std::endl;
            //std::cout << "- Current Timestamp: " << std::dec << MidiClock::Now() << std::endl;
            //std::cout << "- UMP Timestamp:     " << std::dec << ump.Timestamp() << std::endl;
            //std::cout << "- UMP Msg Type:      0x" << std::hex << (uint32_t)ump.MessageType() << std::endl;
            //std::cout << "- UMP Packet Type:   0x" << std::hex << (uint32_t)ump.PacketType() << std::endl;
            
            //TO DO: this is causing an early return of lambda for ill formatted messages
            //std::cout << "- Message:           " << winrt::to_string(MidiMessageHelper::GetMessageDisplayNameFromFirstWord(args.PeekFirstWord())) << std::endl;

            // if you wish to cast the IMidiUmp to a specific Ump Type, you can do so using .as<T> WinRT extension

            if (ump.PacketType() == MidiPacketType::UniversalMidiPacket32)
            {
                // we'll use the Ump32 type here. This is a runtimeclass that the strongly-typed 
                // 32-bit messages derive from. There are also MidiUmp64/96/128 classes.
                auto ump32 = ump.as<MidiMessage32>();

                std::cout << "- Word 0:            0x" << std::hex << ump32.Word0() << std::endl;

                //Populate a MIDIEventList
                MIDIEventPacket packet    = { ump32.Timestamp(), 1, ump32.Word0() };
                MIDIEventList   eventList = { kMIDIProtocol_1_0, 1, packet        };

                assert(midiClient); assert(midiClient->inPort);
                midiClient->inPort(&eventList, connection);
            }

            //std::cout << std::endl;
    };

    return MessageReceivedHandler;
}



#endif

//internally CMClient memory
//struct CMClientContext* CMClient = NULL;

//winrt::com_ptr<winrt::Microsoft::Windows::Devices::Midi2::MidiSession> session;
//winrt::Microsoft::Windows::Devices::Midi2::MidiSession session = MidiSession::Create(L"Sample Session");

//winrt::Microsoft::Windows::Devices::Midi2::MidiSession^ mMidiInPortWatcher;

//mMidiInPortWatcher = ref new WinRTMidiPortWatcher(WinRTMidiPortType::In, callback);
//mMidiOutPortWatcher = ref new WinRTMidiPortWatcher(WinRTMidiPortType::Out, callback);
//mMidiInPortWatcherWrapper = std::make_shared<MidiPortWatcherWrapper>(mMidiInPortWatcher);


struct CMClientContext CMClient = {0};

//CMIDI_API CMIDI_INLINE CMClientContext* CMGetClientContext(void) { return &_CMClient; }


CMClientDriver* GetCMidiClientDriver(void)
{
#ifdef _WIN32 

#elif  defined(__GNUC__) && (__GNUC__ > 3)
    // You are OK
#else
#error Add Critical Section for your platform
#endif
    static CMClientDriver driver = { cmidi_ext_load_init };
#ifdef _WIN32
    //END Critical Section Here
#endif 

    return &driver;
}


OSStatus CMDeleteThruConnection(const char * thruID)
{
    OSStatus cmError = 0;
#ifdef __APPLE__
    CFDataRef data;
    
    CFStringRef cfThruID = CFStringCreateWithCString(CFAllocatorGetDefault(), thruID, kCFStringEncodingUTF8);
    cmError = MIDIThruConnectionFind(cfThruID, &data);
    if( cmError != noErr ) assert(1==0);
    
    unsigned long n = CFDataGetLength(data) / sizeof(MIDIThruConnectionRef);
    
    fprintf(stderr, "\nDeleting %lu thru connection(s)!\n", n);
    
    MIDIThruConnectionRef * con = (MIDIThruConnectionRef*)CFDataGetBytePtr(data);
    for(int i=0;i<n;i++) {
        //cmError = MIDIThruConnectionSetParams(thruConnection, cfDataParams);
        //cassert(cmError == noErr);
        MIDIThruConnectionDispose(*con);
        con++;
    }
    
    CFRelease(cfThruID);
#else

#endif

    return cmError;
}

OSStatus CMDeleteProxyConnection(const char * thruID)
{
    OSStatus cmError = 0;
    assert(1==0);
    return cmError;
}

OSStatus CMDeleteSoftThruConnection(const char * thruID)
{
    OSStatus cmError = 0;
    assert(1==0);
    return cmError;
}

OSStatus CMDeleteInputConnectionAtIndex(int connectionIndex)
{
    OSStatus cmError = 0;
    
    CMSource* sourceEndpoint = &(CMClient.inputConnections[connectionIndex].source);
    
#ifdef __APPLE__
    if( (cmError = MIDIPortDisconnectSource(CMClient.inPort, sourceEndpoint->endpoint)) != 0)
    {
        fprintf(stderr, "MIDIPortDisconnectSource endpoint failed to disconnect error: %d!\n", cmError);
        assert(1==0);
    }
#else

    GUID connectionGUID      = CMClient.inputConnections[connectionIndex].connection;
    winrt::guid connectionID = reinterpret_cast<winrt::guid&>(connectionGUID);

    // deregister the event by passing in the revoke token
    // TO DO: Figure out how to maintain references to endpoints on CMConnection struct
    std::cout << std::endl << "CMDeleteInputConnectionAtIndex::Deregistering event handler..." << std::endl;
    //receiveEndpoint.MessageReceived(eventRevokeToken);

    std::cout << "CMDeleteInputConnectionAtIndex::Disconnecting UMP Endpoint Connection..." << std::endl;
    CMClient.session->DisconnectEndpointConnection(connectionID);

#endif

    ItemCount nShiftConnections = CMClient.numInputConnections - connectionIndex;
    if( nShiftConnections > 1) memmove(&CMClient.inputConnections[connectionIndex], &CMClient.inputConnections[connectionIndex+1], nShiftConnections);

    CMClient.numInputConnections--;

    return cmError;
}

OSStatus CMDeleteInputConnection(uintptr_t UniqueID)
{
    OSStatus cmError    = 0;
    int connectionIndex = 0;
    

    //Update the List of Input Devices before looking for Device UniqueID
#ifdef __APPLE__
    CMUpdateInputDevices();
#else
    wchar_t inputIDKey[256];// = "\0"; //used for copy and as DOM key
    wchar_t* inputID = (wchar_t*)UniqueID; size_t  inputLen = wcslen(inputID);
    fprintf(stdout, "CMDeleteInputConnection::Input: \n\n%S\n", (wchar_t*)inputID);

    //create a copy of the uniqueID prior to overwriting CMSource memory with CMUpdateInputDevices
    wcscpy((wchar_t*)inputIDKey, inputID);
#endif

    //Get an input endpoint to populate
    //CMSource* sourceEndpoint = &(CMClient.inputConnections[CMClient.numInputConnections].source);

    //CoreMIDI: Find the corresponding source endpoint for the unique id
    //WMS:      Find the corresponding connection id for the input connection
    for(connectionIndex=0; connectionIndex<CMClient.numInputConnections;connectionIndex++)
    {

#ifdef _WIN32
        wchar_t* sourceID = (wchar_t*)CMClient.inputConnections[connectionIndex].source.uniqueID; size_t sourceLen = wcslen(sourceID);
        fprintf(stdout, "CMDeleteInputConnection::Compare A: \n\n%S\n", (wchar_t*)sourceID);
        fprintf(stdout, "CMDeleteInputConnection::Compare B: \n\n%S\n", (wchar_t*)inputIDKey);
        if (sourceLen == inputLen && _wcsicmp((wchar_t*)inputIDKey, sourceID) == 0)
#elif defined(__APPLE__)
        if(CMClient.inputConnections[connectionIndex].source.uniqueID == UniqueID)
#endif
        {
            CMDeleteInputConnectionAtIndex(connectionIndex);
            break;
        }
    }

    //The list of source endpoints MUST be updated AFTER comparison
#ifdef _WIN32
    CMUpdateInputDevices();
#endif

    //assert(1==0);
    return cmError;
}

OSStatus CMDeleteHardwareDevice(const char * deviceID)
{
    OSStatus cmError = 0;
    assert(1==0);
    return cmError;
}



void CMInitThruParamEndpoints(MIDIThruConnectionParams* thruParams, int * sources, int numSources, int * destinations, int numDestinations)
{
    int i = 0;
    
    MIDIEndpointRef sourceEndpoint = 0;
    MIDIEndpointRef destEndpoint   = 0;
    
    thruParams->numSources = numSources;
    thruParams->numDestinations = numDestinations;
    
#ifdef __APPLE__
    for( i = 0; i<numSources; i++)
    {
        fprintf(stderr, "sources[%d] = %d\n", i, sources[i]);
        sourceEndpoint = MIDIGetSource(sources[i]);
        thruParams->sources[i].endpointRef = sourceEndpoint;
        thruParams->sources[i].uniqueID = sources[i]; //we will hijack this property before creation so we can pull it out of the params for use in CMCreateThruConnection
        //thruParams.sources[i].uniqueID = 1;
        //print("thru source is \(s)")
    }
    
    for( i = 0; i<numDestinations; i++)
    {
        fprintf(stderr, "dests[%d] = %d\n", i, destinations[i]);
        destEndpoint = MIDIGetDestination(destinations[i]);     //this return value seems to be incorrect here
        thruParams->destinations[i].endpointRef = destEndpoint;
        thruParams->destinations[i].uniqueID = destinations[i]; //we will hijack this property before creation so we can pull it out of the params for use in CMCreateThruConnection
        //thruParams.destinations[i].uniqueID = 1;
        //print("thru source is \(s)")
    }
#else
    assert(1 == 0);
#endif
}

void CMInitSourceEndpoint(MIDIThruConnectionEndpoint* endpoint, int source)
{
    CMUpdateInputDevices();

    MIDIEndpointRef sourceEndpoint = 0;

    //for( i = 0; i<numSources; i++)
    //{
        fprintf(stdout, "CMInitSourceEndpoint: %d\n", source);

#ifdef __APPLE__
        sourceEndpoint = MIDIGetSource(source); assert(sourceEndpoint);
#else
        assert(1 == 0);
#endif

        endpoint->endpointRef = sourceEndpoint;
        endpoint->uniqueID    = source; //we will hijack this property before creation so we can pull it out of the params for use in CMCreateThruConnection
    //}
}

void CMInitThruParams(MIDIThruConnectionParams* thruParams, int * sources, int numSources, int * destinations, int numDestinations)
{
    //This convenience function fills the connection structure with default values: no endpoints,
    //no transformations (mostly zeroes except for the channel map). Then, just filling in the
    //source and adding one destination will create a simple, unmodified thru connection.
#ifdef __APPLE__
    MIDIThruConnectionParamsInitialize(thruParams);
#else
    assert(1 == 0);
#endif

    for(int i = 0; i< 16; i++) thruParams->channelMap[i] = i;
    
    CMInitThruParamEndpoints(thruParams, sources, numSources, destinations, numDestinations);
    
    thruParams->lowVelocity = 1;
    thruParams->highVelocity = 127;
    thruParams->lowNote = 1;
    thruParams->highNote = 127;
    
}

void CMSaveThruConnectionParams(CMConnection * thruConnection)
{
    MIDIThruConnectionParams* thruParams = &(thruConnection->params);
    int sourceID = thruParams->sources[0].uniqueID;
    int destID =  thruParams->destinations[0].uniqueID;

    //set to 0 for the CoreMidi call
    thruParams->sources[0].uniqueID = 0;
    thruParams->destinations[0].uniqueID = 0;

#ifdef __APPLE__
    //wrap MIDIThruConnectionParams in a CFDataRef
    CFDataRef cfDataParams = CFDataCreate(CFAllocatorGetDefault(), (const uint8_t*)(thruParams),  MIDIThruConnectionParamsSize(thruParams));
    OSStatus cmError = MIDIThruConnectionSetParams(thruConnection->connection, cfDataParams);
    if( cmError != 0 ) assert(1==0);
#else

#endif

    //update CMidi internal structures (ugh have to loop thru all connections)
    for( int thruIndex = 0; thruIndex < CMClient.numThruConnections; thruIndex++ )
    {
        if( thruConnection->connection == CMClient.thruConnections[thruIndex].connection )
        {
            //copy base memory
#ifdef __APPLE__
            memcpy( &(CMClient.thruConnections[thruIndex].params), thruParams, MIDIThruConnectionParamsSize(thruParams) );
#else
            memcpy(&(CMClient.thruConnections[thruIndex].params), thruParams, sizeof(MIDIThruConnectionParams));
#endif

            //TO DO: copy additional maps

            //store source endpoints on our internal connection structure
            CMClient.thruConnections[thruIndex].source = CMClient.sources[sourceID];
            CMClient.thruConnections[thruIndex].destination = CMClient.destinations[destID];

            //store source endpoints on our input conneciton structure
            thruConnection->source = CMClient.sources[sourceID];
            thruConnection->destination = CMClient.destinations[destID];
            
            break;
        }
    }

    //return input params to existing state for caller
    thruParams->sources[0].uniqueID = sourceID;
    thruParams->destinations[0].uniqueID = destID;
    
    //When CoreMidi Server sends CFDataRef to us, we are responsible for deleting it
    //but what about visa versa?
#ifdef __APPLE__

    CFRelease(cfDataParams);
#endif

}

CMConnection* CMCreateThruConnectionAtIndex(const char * thruID, MIDIThruConnectionParams* thruParams, unsigned long thruIndex)
{
    OSStatus cmError = 0;

    MIDIThruConnectionRef thruConnection = {0};
    
    int sourceID = thruParams->sources[0].uniqueID;
    int destID =  thruParams->destinations[0].uniqueID;

    //set to 0 for the CoreMidi call
    thruParams->sources[0].uniqueID = 0;
    thruParams->destinations[0].uniqueID = 0;
    
#ifdef __APPLE__
    CFDataRef cfDataParams = CFDataCreate(CFAllocatorGetDefault(), (const UInt8 *)(thruParams),  MIDIThruConnectionParamsSize(thruParams)); assert(cfDataParams);

    CFStringRef cfThruID = CFStringCreateWithCString(CFAllocatorGetDefault(), thruID, kCFStringEncodingUTF8);

    if( (cmError = MIDIThruConnectionCreate(cfThruID, cfDataParams, &thruConnection)) != noErr)
    {
        fprintf(stderr, "MIDIThruConnectionCreate failed with error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }
#else

#endif

    //Store the thruConnection in CMidi internal data structures
    CMClient.thruConnections[thruIndex].connection = thruConnection;
    CMClient.thruConnections[thruIndex].source = CMClient.sources[sourceID];
    CMClient.thruConnections[thruIndex].destination = CMClient.destinations[destID];
    strcpy(CMClient.thruConnections[thruIndex].name, thruID);//, strlen(thruID));

    //assert(thruParams == &(CMClient.thruConnections[thruIndex].params));
    
#ifdef __APPLE__
    CFRelease(cfThruID);
    CFRelease(cfDataParams);
#endif

    //return input params to existing state for caller
    thruParams->sources[0].uniqueID = sourceID;
    thruParams->destinations[0].uniqueID = destID;
    
    return &(CMClient.thruConnections[thruIndex]);
}

CMConnection* CMCreateThruConnection(const char * thruID, MIDIThruConnectionParams* thruParams)
{
    CMConnection * conn = CMCreateThruConnectionAtIndex(thruID, thruParams, CMClient.numThruConnections);
    CMClient.numThruConnections++;
    return conn;
}

//For this function deviceID will already be in the name property of CMDeviceDescription input parameter
void CMCreateHardwareDevice(/*const char * deviceID,*/ CMDeviceDescription * deviceDescription, CMDisplay ** deviceDisplays, CMControl ** deviceControls, CMControl ** deviceSwitches)
{
    assert(deviceDescription);
    assert(strlen(deviceDescription->name) > 0);

    assert(deviceDisplays);
    assert(deviceControls);
    assert(deviceSwitches);

    assert(*deviceDisplays == NULL);
    assert(*deviceControls == NULL);
    assert(*deviceSwitches == NULL);

    int nDisplays = 4;
    int nControls = 32;
    int nSwitches = MCUNoteEvent_MAX;

    int controlIndex = 0;
    int switchIndex = 0;

    switch( deviceDescription->type )
    {
        case (CMDeviceType_MCU_Pro):
        {
            nDisplays = 1;                //1 Main Display
            nControls = 17;               //8 Encoders + 8 Faders + Master Fader
            nSwitches = MCUNoteEvent_MAX; //All Buttons
            
            //Allocate Displays
            *deviceDisplays = (CMDisplay*)malloc( nDisplays * sizeof( CMDisplay ));
            memset(*deviceDisplays, 0, nDisplays * sizeof(CMDisplay));

            //Allocate Controls
            *deviceControls = (CMControl*)malloc( nControls * sizeof( CMControl ));
            memset(*deviceControls, 0, nControls * sizeof(CMControl));

            //Allocate Switches
            *deviceSwitches = (CMControl*)malloc( nSwitches * sizeof( CMControl ));
            memset(*deviceSwitches, 0, nSwitches * sizeof(CMControl));
            
            //Assign Encoder Controls
            for( controlIndex = 0;  controlIndex < 8; controlIndex++)
            {
                ((*deviceControls)[controlIndex]).type = CMControlTypeEncoder;
            }

            //Assign Fader Controls
            for( ; controlIndex < nControls; controlIndex++)
            {
                ((*deviceControls)[controlIndex]).type = CMControlTypeFader;
            }
            
            //Assign Switch Controls
            for( switchIndex = 0;  switchIndex < nSwitches; switchIndex++)
            {
                ((*deviceSwitches)[switchIndex]).type = CMControlTypeSwitch;
            }
            
            deviceDescription->nDisplays = nDisplays;
            deviceDescription->nControls = nControls;
            deviceDescription->nSwitches = nSwitches;
            
            return;
        }
        case (CMDeviceType_MCU_Extender):
        {
            nDisplays = 1;  //1 Main Display
            nControls = 17; //8 encoders + 8 faders, we also gratuitously add another fader
                            //so MCU Pro can be used with the XT User interface
            nSwitches = 48; //Channels 1 - 8 buttons [Including V-Select]

            //Allocate Displays
            *deviceDisplays = (CMDisplay*)malloc( nDisplays * sizeof( CMDisplay ));
            memset(*deviceDisplays, 0, nDisplays * sizeof(CMDisplay));
            
            //Allocate Controls
            *deviceControls = (CMControl*)malloc( 32 * sizeof( CMControl ));
            memset(*deviceControls, 0, 32 * sizeof(CMControl));
            
            //Allocate Switches
            *deviceSwitches = (CMControl*)malloc( nSwitches * sizeof( CMControl ));
            memset(*deviceSwitches, 0, nSwitches * sizeof(CMControl));
            
            //Assign Encoder Controls
            for( controlIndex = 0;  controlIndex < 8; controlIndex++)
            {
                ((*deviceControls)[controlIndex]).type = CMControlTypeEncoder;
            }

            //Assign Fader Controls
            for( ; controlIndex < nControls; controlIndex++)
            {
                ((*deviceControls)[controlIndex]).type = CMControlTypeFader;
            }
            
            //Assign Switch Controls
            for( switchIndex = 0;  switchIndex < nSwitches; switchIndex++)
            {
                ((*deviceSwitches)[switchIndex]).type = CMControlTypeSwitch;
            }
            
            deviceDescription->nDisplays = nDisplays;
            deviceDescription->nControls = nControls;
            deviceDescription->nSwitches = nSwitches;
            
            return;
        }
        case (CMDeviceType_MCU_Commander):
        {
            int nDisplays = 4;  //1 Main Display + 3 Additional Displays
            int nControls = 32; //4 Rows of 8 Encoders
            int nSwitches = 32 + 19; //4 Rows of 8 Encoders + bottom Row

            //Allocate Displays
            *deviceDisplays = (CMDisplay*)malloc( nDisplays * sizeof( CMDisplay ));
            memset(*deviceDisplays, 0, nDisplays * sizeof(CMDisplay));
            
            //Allocate Controls
            *deviceControls = (CMControl*)malloc( nControls * sizeof( CMControl ));
            memset(*deviceControls, 0, nControls * sizeof(CMControl));
            
            //Allocate Switches
            *deviceSwitches = (CMControl*)malloc( nSwitches * sizeof( CMControl ));
            memset(*deviceSwitches, 0, nSwitches * sizeof(CMControl));
            
            //Assign Encoder Controls
            for( controlIndex = 0; controlIndex < nControls; controlIndex++)
            {
                ((*deviceControls)[controlIndex]).type = CMControlTypeEncoder;
            }
            
            //Assign Switch Controls
            for( switchIndex = 0;  switchIndex < nSwitches; switchIndex++)
            {
                ((*deviceSwitches)[switchIndex]).type = CMControlTypeSwitch;
            }
            
            deviceDescription->nDisplays = nDisplays;
            deviceDescription->nControls = nControls;
            deviceDescription->nSwitches = nSwitches;

            return;
        }
        default:
            assert(1==0);
    }
}


void CMReplaceThruConnectionAtIndex(CMConnection * conn, const char * thruID, unsigned long thruIndex)
{
    //TO DO:  check strings for equality and input verification
    assert(thruIndex < CMClient.numThruConnections);
    
    //delete existing CoreMIDI thru connection with id
    OSStatus cmError = CMDeleteThruConnection(CMClient.thruConnections[thruIndex].name);
    assert(cmError == 0);

    //recreate the CoreMIDI thru connection with the new id at the same internal CMidi Thru Connection memory location
    CMConnection* thruConnection = CMCreateThruConnectionAtIndex(thruID, &(conn->params), thruIndex);
    assert(thruConnection);
    
    //update the input CMConnection structure with the CMidi CMConnection structure vars it needs after creating new connection
    conn->connection = thruConnection->connection;
    conn->source = thruConnection->source;
    conn->destination = thruConnection->destination;
    strcpy(conn->name, thruID);//, strlen(thruID));
}


CMConnection* CMCreateSoftThruConnectionAtIndex(const char * thruID, MIDIThruConnectionParams* thruParams, unsigned long thruIndex)
{
    OSStatus cmError = 0;

    //MIDIThruConnectionRef thruConnection = 0;
    
    int sourceID = thruParams->sources[0].uniqueID;
    int destID   = thruParams->destinations[0].uniqueID;

#ifdef __APPLE__
    CFStringRef endpointName = NULL; //what is endpointName used for?
    if( (cmError = MIDIObjectGetStringProperty(thruParams->sources[0].endpointRef, kMIDIPropertyName, &endpointName)) != noErr)
    {
        fprintf(stderr, "MIDIObjectGetStringProperty string property not found error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }
    
    //send the endpointRef as the context parameter
    if( (cmError = MIDIPortConnectSource(CMClient.inPort, thruParams->sources[0].endpointRef, &(CMClient.softThruConnections[thruIndex]))) != noErr)
    {
        fprintf(stderr, "MIDIPortConnectSource endpoint not connected error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }
#else

#endif
    
    /*
    //set to 0 for the CoreMidi call
    thruParams->sources[0].uniqueID = 0;
    thruParams->destinations[0].uniqueID = 0;
    
    CFDataRef cfDataParams = CFDataCreate(CFAllocatorGetDefault(), (const UInt8 *)(thruParams),  MIDIThruConnectionParamsSize(thruParams));
    assert(cfDataParams);
    
    //CMidiThruCleanup();

    CFStringRef cfThruID = CFStringCreateWithCString(CFAllocatorGetDefault(), thruID, kCFStringEncodingUTF8);

    if( (cmError = MIDIThruConnectionCreate(cfThruID, cfDataParams, &thruConnection)) != noErr)
    {
        fprintf(stderr, "MIDIThruConnectionCreate failed with error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }
    */
    
    //Store the softThruConnection in CMidi internal data structures
    CMClient.softThruConnections[thruIndex].connection = CMConnectionEmpty; //{0}; //thruConnection;
    CMClient.softThruConnections[thruIndex].source = CMClient.sources[sourceID];
    CMClient.softThruConnections[thruIndex].destination = CMClient.destinations[destID];
    strcpy(CMClient.softThruConnections[thruIndex].name, thruID);//, strlen(thruID));

    //assert(thruParams == &(CMClient.thruConnections[thruIndex].params));
    
    
    //CFRelease(cfThruID);
    //CFRelease(cfDataParams);
    
    //return input params to existing state for caller
    //thruParams->sources[0].uniqueID = sourceID;
    //thruParams->destinations[0].uniqueID = destID;
    
    return &(CMClient.softThruConnections[thruIndex]);
}

CMConnection* CMCreateSoftThruConnection(const char * thruID, MIDIThruConnectionParams* thruParams)
{
    CMConnection * conn = CMCreateSoftThruConnectionAtIndex(thruID, thruParams, CMClient.numSoftThruConnections);
    CMClient.numSoftThruConnections++;
    return conn;
}


CMConnection* CMCreateProxyConnectionAtIndex(const char * thruID, MIDIThruConnectionParams* thruParams, unsigned long thruIndex)
{
    OSStatus cmError = 0;

    //MIDIThruConnectionRef thruConnection = 0;
    
    int sourceID = thruParams->sources[0].uniqueID;
    int destID   = thruParams->destinations[0].uniqueID;

#ifdef __APPLE__
    CFStringRef endpointName = NULL; //what is endpointName used for?
    if( (cmError = MIDIObjectGetStringProperty(thruParams->sources[0].endpointRef, kMIDIPropertyName, &endpointName)) != noErr)
    {
        fprintf(stderr, "MIDIObjectGetStringProperty string property not found error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }
    
    //send the endpointRef as the context parameter
    if( (cmError = MIDIPortConnectSource(CMClient.proxyPort, thruParams->sources[0].endpointRef, &(CMClient.proxyConnections[thruIndex]))) != noErr)
    {
        fprintf(stderr, "MIDIPortConnectSource endpoint not connected error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }
#else

#endif
    
    /*
    //set to 0 for the CoreMidi call
    thruParams->sources[0].uniqueID = 0;
    thruParams->destinations[0].uniqueID = 0;
    
    CFDataRef cfDataParams = CFDataCreate(CFAllocatorGetDefault(), (const UInt8 *)(thruParams),  MIDIThruConnectionParamsSize(thruParams));
    assert(cfDataParams);
    
    //CMidiThruCleanup();

    CFStringRef cfThruID = CFStringCreateWithCString(CFAllocatorGetDefault(), thruID, kCFStringEncodingUTF8);

    if( (cmError = MIDIThruConnectionCreate(cfThruID, cfDataParams, &thruConnection)) != noErr)
    {
        fprintf(stderr, "MIDIThruConnectionCreate failed with error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }
    */
    
    //Store the softThruConnection in CMidi internal data structures
    CMClient.proxyConnections[thruIndex].connection = CMConnectionEmpty; //{0};//thruConnection;
    CMClient.proxyConnections[thruIndex].source = CMClient.sources[sourceID];
    CMClient.proxyConnections[thruIndex].destination = CMClient.destinations[destID];
    strcpy(CMClient.proxyConnections[thruIndex].name, thruID);//, strlen(thruID));

    //assert(thruParams == &(CMClient.thruConnections[thruIndex].params));
    
    
    //CFRelease(cfThruID);
    //CFRelease(cfDataParams);
    
    //return input params to existing state for caller
    //thruParams->sources[0].uniqueID = sourceID;
    //thruParams->destinations[0].uniqueID = destID;
    
    return &(CMClient.proxyConnections[thruIndex]);
}

CMConnection* CMCreateProxyConnection(const char * thruID, MIDIThruConnectionParams* thruParams)
{
    CMConnection * conn = CMCreateProxyConnectionAtIndex(thruID, thruParams, CMClient.numProxyConnections);
    CMClient.numProxyConnections++;
    return conn;
}

CMConnection* CMCreateInputConnectionAtIndex(const char * inputID, CMSource* sourceEndpoint, unsigned long sourceIndex, unsigned long inputIndex)
{
    OSStatus cmError = 0;

    //MIDIThruConnectionRef thruConnection = 0;
    
    int sourceID = sourceIndex; //sourceEndpoint->uniqueID;
    //int destID   = thruParams->destinations[0].uniqueID;

#ifdef __APPLE__
    /*
    CFStringRef endpointName = NULL; //what is endpointName used for?
    if( (cmError = MIDIObjectGetStringProperty(sourceEndpoint->endpoint, kMIDIPropertyName, &endpointName)) != noErr)
    {
        fprintf(stderr, "MIDIObjectGetStringProperty string property not found error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }
    */
    
    //send the endpointRef as the context parameter
    if( (cmError = MIDIPortConnectSource(CMClient.inPort, sourceEndpoint->endpoint, &(CMClient.inputConnections[inputIndex]))) != noErr)
    {
        fprintf(stderr, "MIDIPortConnectSource endpoint not connected error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }

    //Note: On Darwin CMConnection.connection is a ThruConnection
    CMClient.inputConnections[inputIndex].connection = 0;

    //we did some hijacking of the uniqueID/lis index, here we reset it to the true uniqueID
    //sourceEndpoint->uniqueID = CMClient.sources[sourceID].uniqueID;
#else
    

    //get the winrt::hstring back from the HSTRING raw buffer
    assert(sourceEndpoint->uniqueID);
    winrt::hstring hEndpointID = winrt::hstring((wchar_t*)sourceEndpoint->uniqueID);
    auto endpoint = MidiEndpointDeviceInformation::CreateFromEndpointDeviceId(hEndpointID);


    //get winrt session + endpoint back from C-Land
    //winrt::take_ownership_from_abi_t    own;
    //IMidiEndpointDeviceInformation endpoint = IMidiEndpointDeviceInformation(sourceEndpoint->endpoint, own);
    //IMidiSession                    session = IMidiSession(CMClient.client, own);

    std::cout << "Connecting to endpoint: " << winrt::to_string(endpoint.EndpointDeviceId()) << std::endl;
     auto endpointConnection = CMClient.session->CreateEndpointConnection(endpoint.EndpointDeviceId());    
    //std::cout << "Created endpoint connection: " << winrt::to_string(endpointConnection.ConnectionId()) << std::endl;

    // store the connection id
    winrt::guid connectionID   = endpointConnection.ConnectionId();
    GUID        connectionGUID = reinterpret_cast<GUID&>(connectionID); //convert to abi w/out <unknwn.h>

    // the returned token is used to deregister the event later.
    auto eventRevokeToken = endpointConnection.MessageReceived(UniversalMessagePacketHandler(&CMClient, &CMClient.inputConnections[inputIndex]));
    
    //Note: On Windows MIDI Services CMConnection.connection is the GUID of a MidiEndpointConnection reference
    CMClient.inputConnections[inputIndex].connection = connectionGUID; // *(void**)&endpointConnection;//thruConnection;
    CMClient.inputConnections[inputIndex].eventToken = eventRevokeToken.value;//thruConnection;

    // once you have wired up all your event handlers, added any filters/listeners, etc.
    // You can open the connection. Doing this will query the cache for the in-protocol 
    // endpoint information and function blocks. If not there, it will send out the requests
    // which will come back asynchronously with responses.
    endpointConnection.Open();

#endif

    //Store the input connection in CMidi internal data structures
    CMClient.inputConnections[inputIndex].source = CMClient.sources[sourceID];
    strcpy(CMClient.inputConnections[inputIndex].name, inputID);//, strlen(thruID));
        
    return &(CMClient.inputConnections[inputIndex]);
}

CMConnection* CMCreateInputConnection(uintptr_t uniqueID)//MIDIThruConnectionEndpoint* endpoint)
{
    int sourceIndex      =   0;
    char inputIDKey[256] = "\0"; //used for copy and as DOM key

#ifndef __APPLE__
    wchar_t* inputID = (wchar_t*)uniqueID; size_t  inputLen = wcslen(inputID);
    fprintf(stdout, "CMCreateInputConnection::Input: \n\n%S\n", (wchar_t*)inputID);

    //create a copy of the uniqueID prior to overwriting CMSource memory with CMUpdateInputDevices
    wcscpy((wchar_t*)inputIDKey, inputID);
#endif
    
    //Update the List of Input Devices before looking for Device UniqueID
    CMUpdateInputDevices();

    //Get an input endpoint to populate
    //MIDIThruConnectionEndpoint* sourceEndpoint = &(CMClient.inputConnections[CMClient.numInputConnections].params.sources[0]);
    CMSource * sourceEndpoint = &CMClient.inputConnections[CMClient.numInputConnections].source;
    
    //Find the corresponding source endpoint for the unique id
    for(sourceIndex=0; sourceIndex<CMClient.numSources;sourceIndex++)
    {
#ifdef __APPLE__

        //the CMSource UniqueID stores an int32_t
        //the CMSource endpoint stores a CoreMidi MidiEndpointRef

        if(CMClient.sources[sourceIndex].uniqueID == (int32_t)uniqueID ) 
        {
            //Populate input endpoint
            sourceEndpoint->uniqueID = CMClient.sources[sourceIndex].uniqueID;// sourceIndex;
            sourceEndpoint->endpoint = CMClient.sources[sourceIndex].endpoint;

            //Use the CoreMIDI unique id of the input source endpoint for our DOM primary key
            //NSString * sourceKey = [NSString stringWithFormat:@"%d", sourceEndpoint->uniqueID];//[NSString stringWithUTF8String:CMClient.sources[inputIDs[0]].uniqueID]'
            sprintf(inputIDKey, "%d\0", (int32_t)uniqueID);

            break;
        }
#else
        //the CMSource UniqueID stores a raw HSTRING buffer ptr
        //the CMSource endpoint stores an IUnknown to the ABI implementation of the WinRT endpoint class

        wchar_t* sourceID = (wchar_t*)CMClient.sources[sourceIndex].uniqueID; size_t sourceLen = wcslen(sourceID);

        fprintf(stdout, "CMCreateInputConnection::Compare A: \n\n%S\n", (wchar_t*)sourceID);
        fprintf(stdout, "CMCreateInputConnection::Compare B: \n\n%S\n", (wchar_t*)inputIDKey);

        //sometimes WMS returns capitalized version of id string so a case-insensitive compare is needed
        if (sourceLen == inputLen && _wcsicmp((wchar_t*)inputIDKey, sourceID) == 0 )
        {
            //Populate input endpoint
            sourceEndpoint->uniqueID = CMClient.sources[sourceIndex].uniqueID;
            sourceEndpoint->endpoint = CMClient.sources[sourceIndex].endpoint;
            
            //convert a unicode string that contains only ascii to UTF-8
            //memset(inputIDKey, '\0', 256 * sizeof(char));
            //LPWSTR_2_CHAR(inputID, inputIDKey, inputLen);        
            
            break;
        }
#endif
    } 
    
    assert(sourceIndex<CMClient.numSources);
    
    //Create the input connection in our array of connections using inputIDKey for DOM hashing
    CMConnection * conn = CMCreateInputConnectionAtIndex((const char*)inputIDKey, sourceEndpoint, sourceIndex, CMClient.numInputConnections);
    CMClient.numInputConnections++;
    return conn;
}

ItemCount CMGetNumberOfDevices(void)
{
#ifdef __APPLE__
    return MIDIGetNumberOfDevices();
#else
    auto endpoints = MidiEndpointDeviceInformation::FindAll(MidiEndpointDeviceInformationSortOrder::Name, MidiEndpointDeviceInformationFilters::AllStandardEndpoints);
    return endpoints.Size();
#endif
}

template <typename T>
T convert_from_abi(IUnknown* from)
{
    T to{ nullptr }; // `T` is a projected type.

    winrt::check_hresult(from->QueryInterface(winrt::guid_of<T>(), winrt::copy_from_abi(from, to)));

    return to;
}

const CMSource* CMGetSource(int srcIndex)
{
    assert(CMClient.sources[srcIndex].endpoint);
    //assert(CMClient.sources[srcIndex].uniqueID);    
    //auto endpoint = convert_from_abi((IUnknown*)CMClient.sources[srcIndex].endpoint);

    // Convert from an ABI type.
    //MidiEndpointDeviceInformation endpoint = convert_from_abi<MidiEndpointDeviceInformation>((IUnknown*)CMClient.sources[srcIndex].endpoint);

    //winrt::copy_from_abi((IUnknown*)CMClient.sources[srcIndex].endpoint, endpoint)

    //reassign hstring abi memory
    //UINT32* hLen = 0; HSTRING h = reinterpret_cast<HSTRING>(get_abi(endpoint.EndpointDeviceId()));
    //PCWSTR rawBuffer = WindowsGetStringRawBuffer(h, hLen);
    //fprintf(stdout, "\nCMGetSource::HSTRING = %S\n", rawBuffer);
    //CMClient.sources[srcIndex].uniqueID = (uintptr_t)rawBuffer;    // (uintptr_t)(endpoint.EndpointDeviceId().data());

    return &CMClient.sources[srcIndex];
}

const CMDestination* CMGetDestination(int dstIndex)
{
    return &CMClient.destinations[dstIndex];
}

ItemCount CMUpdateInputDevices(void)
{
    int i = 0;
    ItemCount numInputs = 0;
    MIDIEndpointRef endpoint;
    //OSStatus CMError = noErr;

#ifdef __APPLE__

    numInputs = MIDIGetNumberOfSources();

    // Iterate over the MIDI input devices
    fprintf(stderr, "\n%lu MIDI Input Devices...\n\n", numInputs);
    
    for (i = 0; i < numInputs; i++)
    {
        CMDriverID driverID = 0;
        endpoint = MIDIGetSource(i);
        if (endpoint == 0) {
            continue;
        }
        
        OSStatus cmError;
        int32_t sourceEndpointUniqueID = 0;
        char s[32]; // driver name may truncate, but that's OK

        cmError = MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyUniqueID, &sourceEndpointUniqueID); assert(!cmError);

        //set the first input we see to the default */
        //if (pm_default_input_device_id == -1)
        //    pm_default_input_device_id = pm_descriptor_len;
        
        //Register this device with PortMidi
        // pm_add_device("CoreMIDI",
        //         cm_get_full_endpoint_name(endpoint, &isIACflag), TRUE, FALSE,
        //         (void *) (intptr_t) endpoint, &pm_macosx_in_dictionary);
        
        MIDIEntityRef entity = 0;
        MIDIEndpointGetEntity(endpoint, &entity);
        // (entity == 0) assert(1==0); //probably virtual
        CFStringRef str = NULL;
        //*isIAC = FALSE;
        
        // begin with the endpoint's name
        //extern const CFStringRef kMIDIPropertyDriverOwner;
        MIDIObjectGetStringProperty(entity, kMIDIPropertyDriverOwner, &str);
        if (str != NULL) {
            CFStringGetCString(str, s, 31, kCFStringEncodingUTF8);
            s[31] = 0;  // make sure it is terminated just to be safe
            
            for ( int dID = 1; dID < cm_driver_list_size; dID++)
            {
                if( strcmp(cm_driver_list[dID], s) == 0 )
                {
                    driverID = dID;
                    break;
                }
            }
            
            CFRelease(str);
        }

        fprintf(stderr, "%d) %s (%d) [%s]\n", i, CMFullEndpointName(endpoint, CMClient.sources[i].name, &driverID), sourceEndpointUniqueID, s);
        
        CMClient.sources[i].endpoint = endpoint;//sourceEndpointUniqueID;
        CMClient.sources[i].uniqueID = sourceEndpointUniqueID;
        CMClient.sources[i].driverID = driverID;
        //CFRelease(str);
    }

#else
    //assert(1 == 0);

    
    // Iterate over the MIDI input devices

    auto endpoints = MidiEndpointDeviceInformation::FindAll(MidiEndpointDeviceInformationSortOrder::Name, MidiEndpointDeviceInformationFilters::AllStandardEndpoints);

    numInputs = endpoints.Size();

    fprintf(stdout, "\n%lu MIDI Input Devices...\n\n", numInputs);

    std::cout << endpoints.Size() << " endpoints returned" << std::endl << std::endl;

    for (i=0; i < numInputs; i++)
    //for (auto const& endpoint : endpoints)
    {
        auto endpoint = endpoints.GetAt(i);
        //winrt::Microsoft::Windows::Devices::Midi2::MidiEndpointDeviceInformation* endpoint = &ep;

        //fprintf(stderr, "%d) %s (%d) [%s]\n", i, CMFullEndpointName(endpoint, CMClient.sources[i].name, &driverID), sourceEndpointUniqueID, s);

        std::cout << "Identification" << std::endl;
        std::cout << "- Name:    " << winrt::to_string(endpoint.Name()) << std::endl;
        std::cout << "- Id:      " << winrt::to_string(endpoint.EndpointDeviceId()) << std::endl << std::endl;

        //strcpy(CMClient.sources[i].name, winrt::to_string(endpoint.Name()).c_str());
        size_t w_len = endpoint.Name().size();// wcslen(winrt::str(endpoint.Name()).);
        //copy unicode string to bytes
        wcscpy((wchar_t*)CMClient.sources[i].name, endpoint.Name().data());
        //*nameLen = w_len;

        //CMClient.sources[i].uniqueID = (uintptr_t)winrt::get_unknown(endpoint.EndpointDeviceId()); // (uintptr_t)(endpoint.EndpointDeviceId().data());
        
        

        //Extract HSTRING from winrt::hstring using winrt abi
        UINT32* hLen=0; HSTRING h = reinterpret_cast<HSTRING>(detach_abi(endpoint.EndpointDeviceId()));

        //copy_to_abi
        PCWSTR rawBuffer = WindowsGetStringRawBuffer(h, hLen);
        fprintf(stdout, "\nHSTRING = %S\n", rawBuffer);
        CMClient.sources[i].uniqueID = (uintptr_t)rawBuffer;    // (uintptr_t)(endpoint.EndpointDeviceId().data());
        CMClient.sources[i].endpoint = winrt::get_abi(endpoint);// (void*)winrt::get_unknown(endpoint); //store void* to IUnknown

        //TO DO:  I can't figure out how to convert back from abi
        //convert_from_abi
        //auto cppwinrt_ptr{ to_winrt<winrt::com_ptr<ID2D1Factory1>>(com_ptr) };
        //auto aEndpoint = winrt::Microsoft::Windows::Devices::Midi2::MidiEndpointDeviceInformation::convert_from_abi<winrt::Uri>(ptr.get());

        //CMClient.sources[i].driverID = driverID;

        /*
        auto parent = endpoint.GetParentDeviceInformation();

        if (parent != nullptr)
        {
            std::cout << "- Parent:  " << winrt::to_string(parent.Id()) << std::endl;
        }
        else
        {
            std::cout << "- Parent:  Unknown" <<std::endl;
        }

        if (endpoint.EndpointPurpose() == MidiEndpointDevicePurpose::NormalMessageEndpoint)
        {
            std::cout << "- Purpose: Application MIDI Traffic" << std::endl;
        }
        else if (endpoint.EndpointPurpose() == MidiEndpointDevicePurpose::VirtualDeviceResponder)
        {
            std::cout << "- Purpose: Virtual Device Responder" << std::endl;
        }
        else if (endpoint.EndpointPurpose() == MidiEndpointDevicePurpose::DiagnosticLoopback)
        {
            std::cout << "- Purpose: Diagnostics use" << std::endl;
        }
        else if (endpoint.EndpointPurpose() == MidiEndpointDevicePurpose::DiagnosticPing)
        {
            std::cout << "- Purpose: Internal Diagnostics Ping" << std::endl;
        }
        else
        {
            std::cout << "- Purpose: Unknown" << std::endl;
        }

        // Note: Most of these std::cout calls should really be std::wcout due to the format
        // of the string data. Similarly, conversions to std::string should be std::wstring
        
        // info gathered through endpoint discovery
        auto declaredEndpointInfo = endpoint.GetDeclaredEndpointInfo();

        std::cout << std::endl << "Endpoint Metadata" << std::endl;
        std::cout << "- Product Instance Id:    " << winrt::to_string(declaredEndpointInfo.ProductInstanceId) << std::endl;
        std::cout << "- Endpoint-supplied Name: " << winrt::to_string(declaredEndpointInfo.Name) << std::endl;
    
        // Device Identity
        auto declaredDeviceIdentity = endpoint.GetDeclaredDeviceIdentity();

        std::cout << std::endl << "Device Identity" << std::endl;
        std::cout << "- System Exclusive Id:    "
            << declaredDeviceIdentity.SystemExclusiveIdByte1 << " " 
            << declaredDeviceIdentity.SystemExclusiveIdByte2 << " "
            << declaredDeviceIdentity.SystemExclusiveIdByte3
            << std::endl;

        std::cout << "- Device Family:          "
            << declaredDeviceIdentity.DeviceFamilyMsb << " "
            << declaredDeviceIdentity.DeviceFamilyLsb
            << std::endl;

        std::cout << "- Device Family Model:    "
            << declaredDeviceIdentity.DeviceFamilyModelNumberMsb << " "
            << declaredDeviceIdentity.DeviceFamilyModelNumberLsb
            << std::endl;

        std::cout << "- Software Revision Lvel: "
            << declaredDeviceIdentity.SoftwareRevisionLevelByte1 << " "
            << declaredDeviceIdentity.SoftwareRevisionLevelByte2 << " "
            << declaredDeviceIdentity.SoftwareRevisionLevelByte3 << " "
            << declaredDeviceIdentity.SoftwareRevisionLevelByte4
            << std::endl;


        // user-supplied info
        auto userInfo = endpoint.GetUserSuppliedInfo();
        std::cout << std::endl << "User-supplied Metadata" << std::endl;
        std::cout << "- User-supplied Name: " << winrt::to_string(userInfo.Name) << std::endl;
        std::cout << "- User Description:   " << winrt::to_string(userInfo.Description) << std::endl;
        std::cout << "- Small Image Path:   " << winrt::to_string(userInfo.SmallImagePath) << std::endl;
        std::cout << "- Large Image Path:   " << winrt::to_string(userInfo.LargeImagePath) << std::endl;

        std::cout << std::endl << "Endpoint Supported Capabilities" << std::endl;
        std::cout << "- UMP Major.Minor:   " << declaredEndpointInfo.SpecificationVersionMajor << "." << declaredEndpointInfo.SpecificationVersionMinor << std::endl;
        std::cout << "- MIDI 1.0 Protocol: " << BooleanToString(declaredEndpointInfo.SupportsMidi10Protocol) << std::endl;
        std::cout << "- MIDI 2.0 Protocol: " << BooleanToString(declaredEndpointInfo.SupportsMidi20Protocol) << std::endl;
        std::cout << "- Sending JR Time:   " << BooleanToString(declaredEndpointInfo.SupportsSendingJitterReductionTimestamps) << std::endl;
        std::cout << "- Receiving JR Time: " << BooleanToString(declaredEndpointInfo.SupportsReceivingJitterReductionTimestamps) << std::endl;
        //std::cout << "- Multi-client:      " << BooleanToString(declaredEndpointInfo.SupportsMultiClient) << std::endl;

        // TODO: Configured protocol

        auto transportInfo = endpoint.GetTransportSuppliedInfo();
        std::cout << std::endl << "Transport Information" << std::endl;
        std::cout << "- Transport-supplied Name: " << winrt::to_string(transportInfo.Name) << std::endl;
        std::cout << "- Description:             " << winrt::to_string(transportInfo.Description) << std::endl;
        std::cout << "- Transport Id:            " << winrt::to_string(winrt::to_hstring(transportInfo.TransportId)) << std::endl;
        std::cout << "- Transport Mnemonic:      " << winrt::to_string(transportInfo.TransportAbbreviation) << std::endl;

        if (transportInfo.NativeDataFormat == MidiEndpointNativeDataFormat::Midi1ByteFormat)
        {
            std::cout << "- Native Data Format:      MIDI 1.0 Byte Stream" << std::endl;
        }
        else if (transportInfo.NativeDataFormat == MidiEndpointNativeDataFormat::UniversalMidiPacketFormat)
        {
            std::cout << "- Native Data Format:      MIDI 2.0 UMP" << std::endl;
        }
        else 
        {
            std::cout << "- Native Data Format:      Unknown" << std::endl;
        }

        std::cout << std::endl << "Function Block Information" << std::endl;

        // Function Blocks

        auto functionBlocks = endpoint.GetDeclaredFunctionBlocks();
        std::cout << "- Static Blocks?:  " << BooleanToString(declaredEndpointInfo.HasStaticFunctionBlocks) << std::endl;
        std::cout << "- Block Count:     " << functionBlocks.Size() << std::endl;

        for (auto const& functionBlock : functionBlocks)
        {
            std::cout << "  - " << functionBlock.Number() << " : " << winrt::to_string(functionBlock.Name()) << std::endl;
        }
        

        // Group Terminal Blocks
        auto groupTerminalBlocks = endpoint.GetGroupTerminalBlocks();
        std::cout << std::endl << "Group Terminal Blocks" << std::endl;
        std::cout << "- Block Count:     " << groupTerminalBlocks.Size() << std::endl;

        for (auto const& groupTerminalBlock : groupTerminalBlocks)
        {
            std::cout << "  - " << groupTerminalBlock.Number() << " : " << winrt::to_string(groupTerminalBlock.Name()) << std::endl;
        }



        std::cout << "--------------------------------------------------------------------------" << std::endl << std::endl;
        */

    }

#endif

    CMClient.numSources = numInputs;
    return numInputs;
}


ItemCount CMUpdateOutputDevices(void)
{
    int i = 0;
    ItemCount numOutputs = 0;
    MIDIEndpointRef endpoint;
    //OSStatus CMError = noErr;
    
#ifdef __APPLE__

    numOutputs = MIDIGetNumberOfDestinations();

    //assert(CMClient.client != 0);

    // Iterate over the MIDI output devices
    fprintf(stderr, "\n%lu MIDI Output Devices...\n\n", numOutputs);
    for (i = 0; i < numOutputs; i++) 
    {
        CMDriverID driverID = 0;
        //CMDeviceId deviceID = 0;
        endpoint = MIDIGetDestination(i);
        if (endpoint == 0) continue;
        
        OSStatus cmError;
        int32_t destEndpointUniqueID = 0;
        char s[32]; // driver name may truncate, but that's OK

        cmError = MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyUniqueID, &destEndpointUniqueID);
        assert(!cmError);
        
        // set the first output we see to the default */
        //if (pm_default_output_device_id == -1)
        //    pm_default_output_device_id = pm_descriptor_len;

        // Register this device with PortMidi */
        //deviceID = pm_add_device("CoreMIDI",
        //        cm_get_full_endpoint_name(endpoint, &isIACflag), FALSE, FALSE,
        //        (void *) (intptr_t) endpoint, &pm_macosx_out_dictionary);
        
        MIDIEntityRef entity = 0;
        MIDIEndpointGetEntity(endpoint, &entity);
        //if (entity == 0) assert(1==0); //probably virtual
        CFStringRef str = NULL;
        //*isIAC = FALSE;
        
        // begin with the endpoint's name
        //extern const CFStringRef kMIDIPropertyDriverOwner;
        MIDIObjectGetStringProperty(entity, kMIDIPropertyDriverOwner, &str);
        if (str != NULL) {
            CFStringGetCString(str, s, 31, kCFStringEncodingUTF8);
            s[31] = 0;  // make sure it is terminated just to be safe
            
            for ( int dID = 1; dID < cm_driver_list_size; dID++)
            {
                if( strcmp(cm_driver_list[dID], s) == 0 )
                {
                    driverID = dID;
                    break;
                }
            }
            
            CFRelease(str);
        }
        
        fprintf(stderr, "%d) %s (%d) [%s]\n", i, CMFullEndpointName(endpoint,  CMClient.destinations[i].name, &driverID), destEndpointUniqueID, s);
        // if this is an IAC device, tuck that info away for write functions */
        //if (driverID && deviceID <= MAX_IAC_NUM) CMClient.isIAC[deviceID] = TRUE;
        
        CMClient.destinations[i].endpoint = endpoint;//sourceEndpointUniqueID;
        CMClient.destinations[i].uniqueID = destEndpointUniqueID;
        CMClient.destinations[i].driverID = driverID;
    }
#else
 // Iterate over the MIDI input devices

    auto endpoints = MidiEndpointDeviceInformation::FindAll(MidiEndpointDeviceInformationSortOrder::Name, MidiEndpointDeviceInformationFilters::AllStandardEndpoints);

    numOutputs = endpoints.Size();

    fprintf(stdout, "\n%lu MIDI Output Devices...\n\n", numOutputs);

    std::cout << endpoints.Size() << " endpoints returned" << std::endl << std::endl;

    for (auto const& endpoint : endpoints)
    {
        std::cout << "Identification" << std::endl;
        std::cout << "- Name:    " << winrt::to_string(endpoint.Name()) << std::endl;
        std::cout << "- Id:      " << winrt::to_string(endpoint.EndpointDeviceId()) << std::endl;

        //strcpy(CMClient.destinations[i].name, winrt::to_string(endpoint.Name()).c_str());

        size_t w_len = endpoint.Name().size();// wcslen(winrt::str(endpoint.Name()).);
        //copy unicode string to bytes
        wcscpy((wchar_t*)CMClient.destinations[i].name, endpoint.Name().data());
        //*nameLen = w_len;


        CMClient.destinations[i].endpoint = *(void**)(&endpoint);
        //CMClient.destinations[i].uniqueID = dstEndpointUniqueID;
        //CMClient.destinations[i].driverID = driverID;

        /*
        auto parent = endpoint.GetParentDeviceInformation();

        if (parent != nullptr)
        {
            std::cout << "- Parent:  " << winrt::to_string(parent.Id()) << std::endl;
        }
        else
        {
            std::cout << "- Parent:  Unknown" << std::endl;
        }

        if (endpoint.EndpointPurpose() == MidiEndpointDevicePurpose::NormalMessageEndpoint)
        {
            std::cout << "- Purpose: Application MIDI Traffic" << std::endl;
        }
        else if (endpoint.EndpointPurpose() == MidiEndpointDevicePurpose::VirtualDeviceResponder)
        {
            std::cout << "- Purpose: Virtual Device Responder" << std::endl;
        }
        else if (endpoint.EndpointPurpose() == MidiEndpointDevicePurpose::DiagnosticLoopback)
        {
            std::cout << "- Purpose: Diagnostics use" << std::endl;
        }
        else if (endpoint.EndpointPurpose() == MidiEndpointDevicePurpose::DiagnosticPing)
        {
            std::cout << "- Purpose: Internal Diagnostics Ping" << std::endl;
        }
        else
        {
            std::cout << "- Purpose: Unknown" << std::endl;
        }

        // Note: Most of these std::cout calls should really be std::wcout due to the format
        // of the string data. Similarly, conversions to std::string should be std::wstring

        // info gathered through endpoint discovery
        auto declaredEndpointInfo = endpoint.GetDeclaredEndpointInfo();

        std::cout << std::endl << "Endpoint Metadata" << std::endl;
        std::cout << "- Product Instance Id:    " << winrt::to_string(declaredEndpointInfo.ProductInstanceId) << std::endl;
        std::cout << "- Endpoint-supplied Name: " << winrt::to_string(declaredEndpointInfo.Name) << std::endl;

        // Device Identity
        auto declaredDeviceIdentity = endpoint.GetDeclaredDeviceIdentity();

        std::cout << std::endl << "Device Identity" << std::endl;
        std::cout << "- System Exclusive Id:    "
            << declaredDeviceIdentity.SystemExclusiveIdByte1 << " "
            << declaredDeviceIdentity.SystemExclusiveIdByte2 << " "
            << declaredDeviceIdentity.SystemExclusiveIdByte3
            << std::endl;

        std::cout << "- Device Family:          "
            << declaredDeviceIdentity.DeviceFamilyMsb << " "
            << declaredDeviceIdentity.DeviceFamilyLsb
            << std::endl;

        std::cout << "- Device Family Model:    "
            << declaredDeviceIdentity.DeviceFamilyModelNumberMsb << " "
            << declaredDeviceIdentity.DeviceFamilyModelNumberLsb
            << std::endl;

        std::cout << "- Software Revision Lvel: "
            << declaredDeviceIdentity.SoftwareRevisionLevelByte1 << " "
            << declaredDeviceIdentity.SoftwareRevisionLevelByte2 << " "
            << declaredDeviceIdentity.SoftwareRevisionLevelByte3 << " "
            << declaredDeviceIdentity.SoftwareRevisionLevelByte4
            << std::endl;


        // user-supplied info
        auto userInfo = endpoint.GetUserSuppliedInfo();
        std::cout << std::endl << "User-supplied Metadata" << std::endl;
        std::cout << "- User-supplied Name: " << winrt::to_string(userInfo.Name) << std::endl;
        std::cout << "- User Description:   " << winrt::to_string(userInfo.Description) << std::endl;
        std::cout << "- Small Image Path:   " << winrt::to_string(userInfo.SmallImagePath) << std::endl;
        std::cout << "- Large Image Path:   " << winrt::to_string(userInfo.LargeImagePath) << std::endl;

        std::cout << std::endl << "Endpoint Supported Capabilities" << std::endl;
        std::cout << "- UMP Major.Minor:   " << declaredEndpointInfo.SpecificationVersionMajor << "." << declaredEndpointInfo.SpecificationVersionMinor << std::endl;
        std::cout << "- MIDI 1.0 Protocol: " << BooleanToString(declaredEndpointInfo.SupportsMidi10Protocol) << std::endl;
        std::cout << "- MIDI 2.0 Protocol: " << BooleanToString(declaredEndpointInfo.SupportsMidi20Protocol) << std::endl;
        std::cout << "- Sending JR Time:   " << BooleanToString(declaredEndpointInfo.SupportsSendingJitterReductionTimestamps) << std::endl;
        std::cout << "- Receiving JR Time: " << BooleanToString(declaredEndpointInfo.SupportsReceivingJitterReductionTimestamps) << std::endl;
        //std::cout << "- Multi-client:      " << BooleanToString(declaredEndpointInfo.SupportsMultiClient) << std::endl;

        // TODO: Configured protocol

        auto transportInfo = endpoint.GetTransportSuppliedInfo();
        std::cout << std::endl << "Transport Information" << std::endl;
        std::cout << "- Transport-supplied Name: " << winrt::to_string(transportInfo.Name) << std::endl;
        std::cout << "- Description:             " << winrt::to_string(transportInfo.Description) << std::endl;
        std::cout << "- Transport Id:            " << winrt::to_string(winrt::to_hstring(transportInfo.TransportId)) << std::endl;
        std::cout << "- Transport Mnemonic:      " << winrt::to_string(transportInfo.TransportAbbreviation) << std::endl;

        if (transportInfo.NativeDataFormat == MidiEndpointNativeDataFormat::Midi1ByteFormat)
        {
            std::cout << "- Native Data Format:      MIDI 1.0 Byte Stream" << std::endl;
        }
        else if (transportInfo.NativeDataFormat == MidiEndpointNativeDataFormat::UniversalMidiPacketFormat)
        {
            std::cout << "- Native Data Format:      MIDI 2.0 UMP" << std::endl;
        }
        else
        {
            std::cout << "- Native Data Format:      Unknown" << std::endl;
        }

        std::cout << std::endl << "Function Block Information" << std::endl;

        // Function Blocks

        auto functionBlocks = endpoint.GetDeclaredFunctionBlocks();
        std::cout << "- Static Blocks?:  " << BooleanToString(declaredEndpointInfo.HasStaticFunctionBlocks) << std::endl;
        std::cout << "- Block Count:     " << functionBlocks.Size() << std::endl;

        for (auto const& functionBlock : functionBlocks)
        {
            std::cout << "  - " << functionBlock.Number() << " : " << winrt::to_string(functionBlock.Name()) << std::endl;
        }


        // Group Terminal Blocks
        auto groupTerminalBlocks = endpoint.GetGroupTerminalBlocks();
        std::cout << std::endl << "Group Terminal Blocks" << std::endl;
        std::cout << "- Block Count:     " << groupTerminalBlocks.Size() << std::endl;

        for (auto const& groupTerminalBlock : groupTerminalBlocks)
        {
            std::cout << "  - " << groupTerminalBlock.Number() << " : " << winrt::to_string(groupTerminalBlock.Name()) << std::endl;
        }

        std::cout << "--------------------------------------------------------------------------" << std::endl << std::endl;
        */

    }


#endif
 
    CMClient.numDestinations = numOutputs;
    return numOutputs;
}

ItemCount CMUpdateThruConnections(void)
{
#ifdef __APPLE__
    CFDataRef data = nil;
#endif

    int numThruConnections = 0;
    MIDIThruConnectionParams thruParams = {0};
    
    //TO DO:  Load stored keys from cache with CoreFoundation
    const char * stored_keys[] = {"Test"};
    int numStoredKeys = 1;
    
    //For each stored key find a midi thru connection
    for(int keyIndex = 0; keyIndex<numStoredKeys; keyIndex++)
    {
        unsigned long n = 0;
        MIDIThruConnectionRef* con = NULL;
        const char * storedKey = stored_keys[keyIndex];

#ifdef __APPLE__
        CFStringRef cfKey = CFStringCreateWithCString(CFAllocatorGetDefault(), storedKey, kCFStringEncodingUTF8);
        OSStatus cmError = MIDIThruConnectionFind(cfKey, &data);
        if( cmError != 0 ) assert(1==0);
        n = CFDataGetLength(data) / sizeof(MIDIThruConnectionRef);
        
        fprintf(stderr, "\nFound %lu existing thru connection(s)!\n", n);
        
        con = (MIDIThruConnectionRef*)CFDataGetBytePtr(data);
#else        

        //MIDIThruConnectionRef* con = (MIDIThruConnectionRef*)CFDataGetBytePtr(data);

#endif


        for(int i=0;i<n;i++)
        {
            //cmError = MIDIThruConnectionSetParams(thruConnection, cfDataParams);
            //cassert(cmError == noErr);
            
            //memset(&thruParams, 0, MIDIThruConnectionParamsSize());

#ifdef __APPLE__
            MIDIThruConnectionParamsInitialize(&thruParams);
            CFDataRef cfDataParams = nil;//CFDataCreate(CFAllocatorGetDefault(), (const UInt8 *)(&thruParams),  MIDIThruConnectionParamsSize());
            cmError = MIDIThruConnectionGetParams(*con, &cfDataParams);
            if( cmError != noErr ) assert(1==0);
            MIDIThruConnectionParams* tmpThruParams = (MIDIThruConnectionParams*)CFDataGetBytePtr(cfDataParams);

#else
            MIDIThruConnectionParams tmpThruParamsData = {0};
            MIDIThruConnectionParams* tmpThruParams = (MIDIThruConnectionParams*)&tmpThruParamsData;

#endif
            
            CMClient.thruConnections[i].connection = *con;
            memcpy(CMClient.thruConnections[i].name, storedKey, strlen(storedKey));

            //use the thru connection params to find core midi source and destination endpoint refs
            CMClient.thruConnections[i].source.endpoint = tmpThruParams->sources[0].endpointRef;
            CMClient.thruConnections[i].destination.endpoint = tmpThruParams->destinations[0].endpointRef;
            
#ifdef __APPLE__
            CMFullEndpointName(tmpThruParams->sources[0].endpointRef, CMClient.thruConnections[i].source.name, &(CMClient.thruConnections[i].source.driverID));
            CMFullEndpointName(tmpThruParams->destinations[0].endpointRef, CMClient.thruConnections[i].destination.name, &(CMClient.thruConnections[i].destination.driverID));
#endif
            
            numThruConnections++;
            //MIDIThruConnectionDispose(*con);
            con++;
#ifdef __APPLE__
            CFRelease(cfDataParams);
#endif
        }

#ifdef __APPLE__
        CFRelease(cfKey);
#endif

    }
    
    CMClient.numThruConnections = numThruConnections;
    return numThruConnections;
}


#ifdef _WIN32
IMidiServicesInitializer* _CMServicesInitializer = NULL;

int CMidiInitSession()
{
    HRESULT hr;
    //CoIntialize(Ex) Initializes the COM library for use by the calling thread, sets the thread's concurrency model, and creates a new apartment for the thread if one is required.
    //You should call Windows::Foundation::Initialize to initialize the thread instead of CoInitializeEx if you want to use the Windows Runtime APIs or if you want to use both COM and Windows Runtime components.
    //Windows::Foundation::Initialize is sufficient to use for COM components.
    //HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    //if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by CoInitializeEx\n", hr); return -1; }

    //Create MMDeviceEnumerator so we can get an audio endpoint
    //CoCreateInstance Creates a single uninitialized object of the class associated with a specified CLSID.
    //Call CoCreateInstance when you want to create only one object on the local system.
    //To create a single object on a remote system, call the CoCreateInstanceEx function. To create multiple objects based on a single CLSID, call the CoGetClassObject function.


    //hr = CoCreateInstance(__clsid(MidiServicesInitializer), NULL, CLSCTX_ALL, __riid(IMidiServicesInitializer), (void**)&_CMServicesInitializer);
    //if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by CoCreateInstance\n", hr); return -1; }
    return 0;



}


#endif

CMIDI_API CMIDI_INLINE OSStatus CMClientCreate(const char * clientID, MIDINotifyBlock midiNotifyBlock, MIDIReceiveBlock midiReceiveBlock, MIDIReceiveBlock proxyReceiveBlock)
{
    OSStatus cmError = 0;


#ifdef __APPLE__

    MIDIClientRef client = CMClient.client; assert(client == 0);

    //memset(isIAC, 0, sizeof(isIAC)); /* initialize all FALSE */
    CFStringRef cfClientID = CFStringCreateWithCString(CFAllocatorGetDefault(), clientID, kCFStringEncodingUTF8);

    // Create the CoreMidi Client App ClientRef obj while waiting for the result in synchronous + blocking fashion
    // on the current run loop (even though the run loop may not been started yet)

    //if ((cmError = MIDIClientCreate(CFSTR("CTMidi Client"), &CTMidiNotifyProc, NULL, &client)) != noErr)
    cmError = MIDIClientCreateWithBlock(cfClientID, &(CMClient.client), midiNotifyBlock);
    CFRelease(cfClientID);
    if( cmError != noErr )
    {
        fprintf(stderr, "\nMidiClientCreateWithBlock() failed with error: %d!", cmError);
        assert(1==0);
        return cmError;
    }
#else
   
    winrt::init_apartment();

    OutputDebugString(L"CMClientCreate::1\n");

    /*
    // Initialize the Windows Runtime.
    RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize))
    {
        assert(1 == 0);
        //return PrintError(__LINE__, initialize);
    }
    */

    // Check to see if Windows MIDI Services is installed and running on this PC
    if (!MidiServicesInitializer::EnsureServiceAvailable())
    {
        // you may wish to fallback to an older MIDI API if it suits your application's workflow
        std::cout << std::endl << "** Windows MIDI Services is not running on this PC **" << std::endl;
        return 1;
    }
    else
    {
        std::cout << std::endl << "Verified that the MIDI Service is available and started" << std::endl;

        // bootstrap the SDK runtime
        MidiServicesInitializer::InitializeSdkRuntime();
    }

    // create the MIDI session, giving us access to Windows MIDI Services. An app may open 
    // more than one session. If so, the session name should be meaningful to the user, like
    // the name of a browser tab, or a project.

    OutputDebugString(L"CMClientCreate::2\n");

    //Create Session/Application Port
    //session = MidiSession::Create(L"Sample Session");

    CMClient.session = std::make_shared<MidiSession>(MidiSession::Create(L"CMidi Session"));


    // Convert to an ABI type.
    //winrt::com_ptr<IMidiSession> ptr{ aSession.as<IMidiSession>() };

    // Convert from an ABI type.
    //uri = ptr.as<winrt::Uri>();
    //winrt::IStringable uriAsIStringable{ ptr.as<winrt::IStringable>() };


    //CMClient = new CMClientContext( session );// winrt::get_unknown(session);//  *(void**)(&session);

    //TO DO:  What to do with CMidiNotifyBlock?

    OutputDebugString(L"CMClientCreate::3\n");


    /*
    auto endpointAId = MidiDiagnostics::DiagnosticsLoopbackAEndpointDeviceId();
    auto endpointBId = MidiDiagnostics::DiagnosticsLoopbackBEndpointDeviceId();

    auto sendEndpoint = session.CreateEndpointConnection(endpointAId);
    std::cout << "Connected to sending endpoint: " << winrt::to_string(endpointAId) << std::endl;

    auto receiveEndpoint = session.CreateEndpointConnection(endpointBId);
    std::cout << "Connected to receiving endpoint: " << winrt::to_string(endpointBId) << std::endl;


    // the returned token is used to deregister the event later.
    auto eventRevokeToken = receiveEndpoint.MessageReceived(MessageReceivedHandler);
    */

#endif
    
    // Determine the number of MIDI devices on the system */
    CMClient.numDevices = CMGetNumberOfDevices();
    fprintf(stderr, "# Midi Devices = %lu\n", CMClient.numDevices);
    //if (CMClient.numDevices <= 0) { assert( 1==0); return cmError; }

    //OutputDebugString(L"CMClientCreate::4\n");

    CMUpdateInputDevices();

    //OutputDebugString(L"CMClientCreate::5\n");

    CMUpdateOutputDevices();
    //CMUpdateThruConnections();
    
    //OutputDebugString(L"CMClientCreate::6\n");

    if( midiReceiveBlock )
    {
        //Create Input & Output Ports
#ifdef __APPLE__
        //cmError = MIDIInputPortCreate(CMClient.client, CFSTR("Input"), midiInputCallback, (__bridge_retained void *)self, &CMClient.inPort), "MIDI input port error");
        cmError = MIDIInputPortCreateWithProtocol(CMClient.client, CFSTR("Input"), kMIDIProtocol_1_0, &CMClient.inPort, midiReceiveBlock);
        if( cmError != noErr )
        {
            fprintf(stderr, "\nMIDIInputPortCreateWithProtocol() failed with error: %d!", cmError);
            assert(1==0);
            return cmError;
        }
#else
        CMClient.inPort = midiReceiveBlock;
#endif
    }
    
    if( proxyReceiveBlock )
    {
        //Create Input & Output Ports
#ifdef __APPLE__
        //cmError = MIDIInputPortCreate(CMClient.client, CFSTR("Input"), midiInputCallback, (__bridge_retained void *)self, &CMClient.inPort), "MIDI input port error");
        cmError = MIDIInputPortCreateWithProtocol(CMClient.client, CFSTR("Input"), kMIDIProtocol_1_0, &CMClient.proxyPort, proxyReceiveBlock);
        if( cmError != noErr )
        {
            fprintf(stderr, "\nMIDIInputPortCreateWithProtocol() failed with error: %d!", cmError);
            assert(1==0);
            return cmError;
        }
#else

#endif

    }
    
    
    //always create output port
#ifdef __APPLE__
    cmError = MIDIOutputPortCreate(CMClient.client, CFSTR("Output"), &CMClient.outPort);
    if( cmError != noErr )
    {
        fprintf(stderr, "\nMIDIOutputPortCreate failed with error: %d!", cmError);
        assert(1==0);
        return cmError;
    }
#else

#endif
    
    return cmError;
}


CMIDI_API CMIDI_INLINE OSStatus CMClientDestroy(CMClientContext* clientContext)
{

    return 0;
}

#pragma mark -- DLL Client Entry
