//
//  CMidiEndpoint.h
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/28/22.
//

#ifndef CMidiEndpoint_h
#define CMidiEndpoint_h

#include <stdint.h>

#include "CMidiDriverID.h"

#ifdef __APPLE__

#include <CoreServices/CoreServices.h>               //Core Services needed for EndianS32_BtoN
#include <CoreMIDI/MIDIServices.h>                   //Core Midi

//#include "CMEndian.h"


// Obtain the name of an endpoint without regard for whether it has connections.
// The result should be released by the caller.
CFStringRef CMEndpointName(MIDIEndpointRef endpoint, bool isExternal, CMDriverID *driverID);

// Obtain the name of an endpoint, following connections.
// The result should be released by the caller.
CFStringRef CMConnectedEndpointName(MIDIEndpointRef endpoint, CMDriverID *driverID);


#else

typedef uint32_t  MIDIObjectRef;
typedef void*	  MIDIEndpointRef;

char* CMFullEndpointName(MIDIEndpointRef endpoint, char* cmname, CMDriverID* driverID);

#endif

#endif /* CMidiEndpoint_h */
