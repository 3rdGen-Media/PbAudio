//
//  SampleInstrumentParameterAddresses.h
//  SampleInstrument
//
//  Created by Joe Moulton on 10/23/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#pragma once

#include <AudioToolbox/AUParameters.h>

typedef NS_ENUM(AUParameterAddress, SampleInstrumentParameterAddress) {
    gain = 0
};
