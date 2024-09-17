//
//  CMidiEndpoint.h
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/28/22.
//

#ifndef CMidiEndpoint_h
#define CMidiEndpoint_h

#include <CoreServices/CoreServices.h>               //Core Services needed for EndianS32_BtoN
#include <CoreMIDI/MIDIServices.h>                   //Core Midi
#include "CMidiDriverID.h"
//#include "CMEndian.h"


// Obtain the name of an endpoint without regard for whether it has connections.
// The result should be released by the caller.
CFStringRef CMEndpointName(MIDIEndpointRef endpoint, bool isExternal, CMDriverID *driverID);

// Obtain the name of an endpoint, following connections.
// The result should be released by the caller.
CFStringRef CMConnectedEndpointName(MIDIEndpointRef endpoint, CMDriverID *driverID);

char *CMFullEndpointName(MIDIEndpointRef endpoint, char* cmname, CMDriverID *driverID);

#endif /* CMidiEndpoint_h */
