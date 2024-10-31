//
//  pbaudio_time.h
//  CRViewer
//
//  Created by Joe Moulton on 6/25/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_time_h
#define pbaudio_time_h


#ifdef __cplusplus
extern "C" {
#endif

//const AudioTimeStamp AETimeStampNone = {};
#ifdef __APPLE__
#import <mach/mach_time.h>
#import <AudioToolbox/AudioToolbox.h>
typedef AudioTimeStamp PBATimeStamp;
#else
#include <stdint.h>

#ifdef _WIN32
#define PBA_REFTIMES_PER_SEC  10000000
#define PBA_REFTIMES_PER_MILLISEC  10000
#endif

typedef struct PBATimeStamp
{
	double		mSampleTime;
	uint64_t	mHostTime;
	double		mRateScalar;
	uint64_t	mWordClockTime;
	//SMPTETime	mSMPTETime;
	uint32_t	mFlags;
	uint32_t	mReserved;
}PBATimeStamp;
#endif

// REFERENCE_TIME time units per second and per millisecond



static double __hostTicksToSeconds = 0.0;
static double __secondsToHostTicks = 0.0;

PB_AUDIO_EXTERN const PBATimeStamp PBATimeStampNone;


#ifdef __APPLE__
PB_AUDIO_API PB_AUDIO_INLINE void PBATimeInit(void);

PB_AUDIO_API PB_AUDIO_INLINE uint64_t PBACurrentTimeInHostTicks(void);
PB_AUDIO_API PB_AUDIO_INLINE double PBACurrentTimeInSeconds(void);

PB_AUDIO_API PB_AUDIO_INLINE double PBASecondsFromHostTicks(double ticks);
PB_AUDIO_API PB_AUDIO_INLINE uint64_t PBAHostTicksFromSeconds(double seconds);
#endif


#ifdef __cplusplus
}
#endif


#endif /* pbaudio_time_h */
