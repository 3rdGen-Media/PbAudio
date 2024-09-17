//
//  CMidiDevice.h
//  MCUCommander
//
//  Created by Joe Moulton on 11/27/23.
//

#ifndef CMidiDevice_h
#define CMidiDevice_h

#include "CMidiControl.h"
#include <MacTypes.h>

//We will hijack OSX ColorSyncDeprecated CMDeviceID (UInt32) for our own API
//#ifndef CMDeviceID
typedef UInt32 CMDeviceId;
//#endif


enum CMDeviceTypeEnum
{
    //CMDeviceTypeUndefined        =   0x00,
    CMDeviceType_MCU_Pro           =   0x00,
    CMDeviceType_MCU_Extender      =   0x01,
    CMDeviceType_MCU_Commander     =   0x02,
    CMDeviceTypeUnknown            =   0xF0
};


enum  MCUButtonShapeEnum
{
    //CMDeviceTypeUndefined         =   0x00,
    MCUButtonShape_Rect             =   0x00,
    MCUButtonShape_Rect_Small       =   0x01,
    MCUButtonShape_Circle           =   0x02,
};


enum MCUButtonIconEnum
{
    //CMDeviceTypeUndefined     =   0x00,
    MCUButtonIcon_None          =   0x00,
    MCUButtonIcon_LeftArrow     =   0x01,
    MCUButtonIcon_UpArrow       =   0x02,
    MCUButtonIcon_RightArrow    =   0x03,
    MCUButtonIcon_DownArrow     =   0x04
};


typedef uint8_t CMDeviceType;
typedef uint8_t MCUButtonShape;
typedef uint8_t MCUButtonIcon;

typedef struct CMDeviceDescription
{
    char                     name[256];
    CMDeviceType             type;
    uint8_t                  nDisplays;
    uint8_t                  nControls;
    uint8_t                  nSwitches;
    
}CMDeviceDescription;

extern const int    cm_device_list_size;
extern const char * cm_device_names[];
extern const char * cm_device_image_names[];

#endif /* CMidiDevice_h */
