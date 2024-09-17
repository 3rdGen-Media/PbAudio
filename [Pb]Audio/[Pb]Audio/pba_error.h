//
//  pbaudio_error.h
//  CRViewer
//
//  Created by Joe Moulton on 6/25/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_error_h
#define pbaudio_error_h

//#include "PbAudioStream.h"
#include "pba_time.h"


#ifdef __APPLE__
static bool PBARateLimit(void) {
    static double lastMessage = 0;
    static int messageCount=0;
    double now = PBACurrentTimeInSeconds();
    if ( now-lastMessage > 1 ) {
        messageCount = 0;
        lastMessage = now;
    }
    if ( ++messageCount >= 10 ) {
        if ( messageCount == 10 ) {
            printf("TAAE: Suppressing some messages");
        }
        return false;
    }
    return true;
}
#else

#ifndef OSStatus 
typedef int OSStatus;
#define noErr 0
#define _Nonnull 
#endif 

#endif

/*!
 * An error occurred within AECheckOSStatus
 *
 *  Create a symbolic breakpoint with this function name to break on errors.
 */
PB_AUDIO_API PB_AUDIO_INLINE void PBAError(OSStatus result, const char * _Nonnull operation, const char * _Nonnull file, int line);

/*!
 * Check an OSStatus condition
 *
 * @param result The result
 * @param operation A description of the operation, for logging purposes
 */
#define PBACheckOSStatus(result,operation) (_PBACheckOSStatus((result),(operation),strrchr(__FILE__, '/')+1,__LINE__))
static PB_AUDIO_INLINE bool _PBACheckOSStatus(OSStatus result, const char * _Nonnull operation, const char * _Nonnull file, int line) {
    if ( result != noErr ) {
        PBAError(result, operation, file, line);
        return false;
    }
    return true;
}


static const int kRenderTimeReportInterval = 1;
static const int kRenderBudgetWarningInitialDelay = 0;
static const int kRenderBudgetWarningThreshold = 1;



typedef struct PBAStreamLatencyReport
{
    
    double firstReportTime;
    
    double lastReportTime;
    double averageRenderDurationAccumulator;
    double averageRenderDurationSampleCount;
    double maximumRenderDuration;
}PBAStreamLatencyReport;

static PBAStreamLatencyReport _audioReport;



#endif /* pbaudio_error_h */
