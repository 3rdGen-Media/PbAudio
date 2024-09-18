//
//  CMIDI.c
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/27/22.
//

#include "CMidi.h"

#ifdef CM_TARGET_WIN32
#include <assert.h>
#endif

#ifdef __APPLE__
const CFStringRef kCMidiSourcesAvailableChangedNotification      = CFSTR("CMidiSourcesAvailableChangedNotification");
const CFStringRef kCMidiDestinationsAvailableChangedNotification = CFSTR("CMidiDestinationsAvailableChangedNotification");
#else

#endif

struct CMClientContext CMClient = {0};

OSStatus CMDeleteThruConnection(const char * thruID)
{
    OSStatus cmError = 0;
#ifdef __APPLLE__
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
    assert(1 == 0);
#endif

    ItemCount nShiftConnections = CMClient.numInputConnections - connectionIndex;
    if( nShiftConnections > 1) memmove(&CMClient.inputConnections[connectionIndex], &CMClient.inputConnections[connectionIndex+1], nShiftConnections);

    CMClient.numInputConnections--;

    return cmError;
}

OSStatus CMDeleteInputConnection(int32_t UniqueID)
{
    OSStatus cmError = 0;
    
    int connectionIndex = 0;
    
    //Update the List of Input Devices before looking for Device UniqueID
    CMUpdateInputDevices();

    //Get an input endpoint to populate
    //CMSource* sourceEndpoint = &(CMClient.inputConnections[CMClient.numInputConnections].source);

    //Find the corresponding source endpoint for the unique id
    for(connectionIndex=0; connectionIndex<CMClient.numInputConnections;connectionIndex++)
    {
        if(CMClient.inputConnections[connectionIndex].source.uniqueID == UniqueID )
        {
            CMDeleteInputConnectionAtIndex(connectionIndex);
            break;
        }
        
    }

    
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

    MIDIThruConnectionRef thruConnection = 0;
    
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
    CMClient.softThruConnections[thruIndex].connection = 0;//thruConnection;
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
    CMClient.proxyConnections[thruIndex].connection = 0;//thruConnection;
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

CMConnection* CMCreateInputConnectionAtIndex(const char * inputID, CMSource* sourceEndpoint, unsigned long inputIndex)
{
    OSStatus cmError = 0;

    //MIDIThruConnectionRef thruConnection = 0;
    
    int sourceID = sourceEndpoint->uniqueID;//thruParams->sources[0].uniqueID;
    //int destID   = thruParams->destinations[0].uniqueID;

#ifdef __APPLE__
    CFStringRef endpointName = NULL; //what is endpointName used for?
    if( (cmError = MIDIObjectGetStringProperty(sourceEndpoint->endpoint, kMIDIPropertyName, &endpointName)) != noErr)
    {
        fprintf(stderr, "MIDIObjectGetStringProperty string property not found error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }
    
    //send the endpointRef as the context parameter
    if( (cmError = MIDIPortConnectSource(CMClient.inPort, sourceEndpoint->endpoint, &(CMClient.inputConnections[inputIndex]))) != noErr)
    {
        fprintf(stderr, "MIDIPortConnectSource endpoint not connected error: %d!\n", cmError);
        assert(1==0);
        return nil;
    }
#else

#endif

    sourceEndpoint->uniqueID = CMClient.sources[sourceID].uniqueID;

    //Store the softThruConnection in CMidi internal data structures
    CMClient.inputConnections[inputIndex].connection = 0;//thruConnection;
    CMClient.inputConnections[inputIndex].source = CMClient.sources[sourceID];
    //CMClient.softThruConnections[thruIndex].destination = CMClient.destinations[destID];
    strcpy(CMClient.inputConnections[inputIndex].name, inputID);//, strlen(thruID));
    
    
    return &(CMClient.inputConnections[inputIndex]);
}

CMConnection* CMCreateInputConnection(int32_t uniqueID)//MIDIThruConnectionEndpoint* endpoint)
{
    int sourceIndex = 0;
    
    //Update the List of Input Devices before looking for Device UniqueID
    CMUpdateInputDevices();

    //Get an input endpoint to populate
    //MIDIThruConnectionEndpoint* sourceEndpoint = &(CMClient.inputConnections[CMClient.numInputConnections].params.sources[0]);
    CMSource * sourceEndpoint = &CMClient.inputConnections[CMClient.numInputConnections].source;
    
    //Find the corresponding source endpoint for the unique id
    for(sourceIndex=0; sourceIndex<CMClient.numSources;sourceIndex++)
    {
        if(CMClient.sources[sourceIndex].uniqueID == uniqueID ) 
        {
            //Populate input endpoint
            //CMInitSourceEndpoint(sourceEndpoint, message->system.deviceSelect);
            sourceEndpoint->uniqueID = sourceIndex;
            sourceEndpoint->endpoint = CMClient.sources[sourceIndex].endpoint;
            break;
        }
        
    } assert(sourceIndex<CMClient.numSources);
    
    //Use the CoreMIDI unique id of the input source endpoint for our DOM primary key
    //NSString * sourceKey = [NSString stringWithFormat:@"%d", sourceEndpoint->uniqueID];//[NSString stringWithUTF8String:CMClient.sources[inputIDs[0]].uniqueID]'
    char inputID[32] = "\0"; sprintf(inputID, "%d\0", uniqueID);
    
    CMConnection * conn = CMCreateInputConnectionAtIndex(inputID, sourceEndpoint, CMClient.numInputConnections);
    CMClient.numInputConnections++;
    return conn;
}

ItemCount CMGetNumberOfDevices(void)
{
#ifdef __APPLE__
    return MIDIGetNumberOfDevices();
#else
    assert(1 == 0);
#endif
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
        }

        fprintf(stderr, "%d) %s (%d) [%s]\n", i, CMFullEndpointName(endpoint, CMClient.sources[i].name, &driverID), sourceEndpointUniqueID, s);
        
        CMClient.sources[i].endpoint = endpoint;//sourceEndpointUniqueID;
        CMClient.sources[i].uniqueID = sourceEndpointUniqueID;
        CMClient.sources[i].driverID = driverID;
        //CFRelease(str);
    }

#else
    assert(1 == 0);

    //numInputs = ...
    
    // Iterate over the MIDI input devices
    fprintf(stderr, "\n%lu MIDI Input Devices...\n\n", numInputs);
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
        }
        
        fprintf(stderr, "%d) %s (%d) [%s]\n", i, CMFullEndpointName(endpoint,  CMClient.destinations[i].name, &driverID), destEndpointUniqueID, s);
        // if this is an IAC device, tuck that info away for write functions */
        //if (driverID && deviceID <= MAX_IAC_NUM) CMClient.isIAC[deviceID] = TRUE;
        
        CMClient.destinations[i].endpoint = endpoint;//sourceEndpointUniqueID;
        CMClient.destinations[i].uniqueID = destEndpointUniqueID;
        CMClient.destinations[i].driverID = driverID;
    }
#else
    assert(1 == 0);

    //numOutputs = ...
    
    //fprintf(stderr, "\n%lu MIDI Output Devices...\n\n", numOutputs);

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
            
            CMFullEndpointName(tmpThruParams->sources[0].endpointRef, CMClient.thruConnections[i].source.name, &(CMClient.thruConnections[i].source.driverID));
            CMFullEndpointName(tmpThruParams->destinations[0].endpointRef, CMClient.thruConnections[i].destination.name, &(CMClient.thruConnections[i].destination.driverID));

            
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


CMIDI_API CMIDI_INLINE OSStatus CMClientCreate(const char * clientID, MIDINotifyBlock midiNotifyBlock, MIDIReceiveBlock midiReceiveBlock, MIDIReceiveBlock proxyReceiveBlock)
{
    OSStatus cmError = 0;

    MIDIClientRef client = CMClient.client;
    assert(client == 0);
    //memset(isIAC, 0, sizeof(isIAC)); /* initialize all FALSE */

#ifdef __APPLE__
    CFStringRef cfClientID = CFStringCreateWithCString(CFAllocatorGetDefault(), clientID, kCFStringEncodingUTF8);


    // Create the CoreMidi Client App ClientRef obj while waiting for the result in synchronous + blocking fashion
    // on the current run loop (even though the run loop may not been started yet)

    /*
    if ( (cmError = MIDIClientCreate(CFSTR("CTMidi Client"), &CTMidiNotifyProc, NULL, &client)) != noErr)
    {
        fprintf(stderr, "\nMidiClientCreate() failed with error: %d!", cmError);
        assert(1==0);
        return cmError;
    }
    */
    
    cmError = MIDIClientCreateWithBlock(cfClientID, &(CMClient.client), midiNotifyBlock);
    CFRelease(cfClientID);
    if( cmError != noErr )
    {
        fprintf(stderr, "\nMidiClientCreateWithBlock() failed with error: %d!", cmError);
        assert(1==0);
        return cmError;
    }
#else

#endif

    /*
    // Create the input port
    //typedef void (*MIDIReadProc)(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon);
    if ((cmError = MIDIInputPortCreate(client, CFSTR("Input port"), device_read_callback, NULL, &portIn)) != noErr)
    {
        fprintf(stderr, "MIDIInputPortCreate()failed with error: %d!\n", cmError);
        return cmError;
    }

    // Create the output port
    if ((cmError = MIDIOutputPortCreate(client, CFSTR("Output port"), &portOut)) != noErr)
    {
        fprintf(stderr, "MIDIOutputPortCreate()failed with error: %d!\n", cmError);
        return cmError;
    }
    */
    
    // Determine the number of MIDI devices on the system */
    CMClient.numDevices = CMGetNumberOfDevices();
    fprintf(stderr, "# Midi Devices = %lu\n", CMClient.numDevices);
    //if (CMClient.numDevices <= 0) { assert( 1==0); return cmError; }

    CMUpdateInputDevices();
    CMUpdateOutputDevices();
    //CMUpdateThruConnections();
    
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
