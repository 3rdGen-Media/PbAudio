//
//  pbaudio_time.h
//  CRViewer
//
//  Created by Joe Moulton on 6/25/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_time_h
#define pbaudio_time_h

#import <mach/mach_time.h>
#import <AudioToolbox/AudioToolbox.h>

static double __hostTicksToSeconds = 0.0;
static double __secondsToHostTicks = 0.0;

//typedef uint64_t AEHostTicks;

//const AudioTimeStamp AETimeStampNone = {};
typedef AudioTimeStamp PBATimeStamp;
PB_AUDIO_EXTERN const PBATimeStamp PBATimeStampNone;

PB_AUDIO_API PB_AUDIO_INLINE void PBATimeInit(void);

PB_AUDIO_API PB_AUDIO_INLINE uint64_t PBACurrentTimeInHostTicks(void);
PB_AUDIO_API PB_AUDIO_INLINE double PBACurrentTimeInSeconds(void);

PB_AUDIO_API PB_AUDIO_INLINE double PBASecondsFromHostTicks(double ticks);
PB_AUDIO_API PB_AUDIO_INLINE uint64_t PBAHostTicksFromSeconds(double seconds);

#endif /* pbaudio_time_h */
