//
//  CMidiDevice.c
//  MCUCommander
//
//  Created by Joe Moulton on 11/27/23.
//

#include "CMidiDevice.h"

const int cm_device_list_size = 3; //excluding Unknown for now
const char * cm_device_names[] =
{
    //"Undefined",
    "Mackie Control Universal PRO",
    "Mackie Control Universal XT",
    "Mackie Control Universal C4",
    "Unknown Device"
};


const char * cm_device_image_names[] =
{
    //"Undefined",
    "MackieControlPro.png",
    "MackieControlExtenderPro.png",
    "MackieControlC4.png",
    "Unknown Device.png"
};

