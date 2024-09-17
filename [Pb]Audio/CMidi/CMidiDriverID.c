//
//  CMidiDriver.c
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/31/22.
//

#include "CMidiDriverID.h"

const int cm_driver_list_size = 5;
const char * cm_driver_list[] =
{
    "Undefined",
    "com.apple.AppleMIDIUSBDriver",
    "com.apple.AppleMIDIRTPDriver",
    "com.apple.AppleMIDIIACDriver",
    ""
};

const char * cm_driver_list_pp[] =
{
    "Undefined",
    "Apple USB",
    "Apple RTP",
    "Apple IAC",
    "Vendor"
};
