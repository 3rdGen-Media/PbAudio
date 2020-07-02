//
//  PbAudioTime.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 7/1/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

PB_AUDIO_EXTERN const PBATimeStamp PBATimeStampNone = {0};

PB_AUDIO_API PB_AUDIO_INLINE void PBATimeInit(void) {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        mach_timebase_info_data_t tinfo;
        mach_timebase_info(&tinfo);
        __hostTicksToSeconds = ((double)tinfo.numer / tinfo.denom) * 1.0e-9;
        __secondsToHostTicks = 1.0 / __hostTicksToSeconds;
    });
}

PB_AUDIO_API PB_AUDIO_INLINE uint64_t PBACurrentTimeInHostTicks(void) {
    return mach_absolute_time();
}

PB_AUDIO_API PB_AUDIO_INLINE double PBACurrentTimeInSeconds(void) {
    if ( !__hostTicksToSeconds ) PBATimeInit();
    return mach_absolute_time() * __hostTicksToSeconds;
}

PB_AUDIO_API PB_AUDIO_INLINE double PBASecondsFromHostTicks(double ticks) {
    if ( !__hostTicksToSeconds ) PBATimeInit();
    return ticks * __hostTicksToSeconds;
}

PB_AUDIO_API PB_AUDIO_INLINE uint64_t PBAHostTicksFromSeconds(double seconds) {
    if ( !__secondsToHostTicks ) PBATimeInit();
    assert(seconds >= 0);
    return round(seconds * __secondsToHostTicks);
}
