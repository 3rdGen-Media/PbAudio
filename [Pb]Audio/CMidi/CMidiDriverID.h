//
//  CMidiDriver.h
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/31/22.
//

#ifndef CMidiDriver_h
#define CMidiDriver_h

#include <stdio.h>

extern const int cm_driver_list_size;
extern const char * cm_driver_list[];
extern const char * cm_driver_list_pp[];

typedef enum CMDriverID
{
    CM_DRIVER_UNDEFINED,
    CM_DRIVER_APPLE_USB,
    CM_DRIVER_APPLE_RTP,
    CM_DRIVER_APPLE_IAC,
    CM_DRIVER_VENDOR
}CMDriverID;

#endif /* CMidiDriver_h */
