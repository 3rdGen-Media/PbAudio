//
//  pbaudio_engine.h
//  CRViewer
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_api_h
#define pbaudio_api_h

#include "PbAudioStream.h"

//On Win32,  if format is NULL, it will create a shared mode device stream (which usually gives a 32-bit floating point format)
//On Darwin, if format is NULL, it will create a device stream with a 32-bit floating point format
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioStreamInit(PBAStreamContext * streamContext, PBAStreamFormat * format, PBAudioDevice deviceID, PBAStreamOutputPass outputpass);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioStreamStart(PBAStreamContext * streamContext);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioStreamStop(PBAStreamContext * streamContext);
PB_AUDIO_API PB_AUDIO_INLINE void     PBAudioStreamSetBypass(PBAStreamContext * streamContext, bool bypass);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioStreamSetOutputDevice(PBAStreamContext * streamContext, AudioDeviceID deviceID);

//#pragma mark -- CTConnection API Method Function Pointer Definitions
//typedef int (*CTConnectFunc)(struct CTTarget * service, CTConnectionClosure callback);
typedef OSStatus (*PBAStreamFunc)       (PBAStreamContext * streamContext);
typedef OSStatus (*PBAStreamDeviceFunc) (PBAStreamContext * streamContext, AudioDeviceID deviceID);
typedef OSStatus (*PBAStreamFormatFunc) (PBAStreamContext * streamContext, PBAStreamFormat * format);
typedef OSStatus (*PBAStreamInitFunc)   (PBAStreamContext * streamContext, PBAStreamFormat * format, PBAudioDevice deviceID, PBAStreamOutputPass outputpass);

//typedef void     (^CMidiClientReceiveBlock)(const MIDIEventList *evtlist, void * __nullable srcConnRefCon);


//#pragma mark -- Global Audio Stream Factory Object
typedef struct PBAudioStreamFactory
{
    //The Client Driver Object can create AudioUnit/AVSession + BufferList Instances
    PBAStreamInitFunc        Init;
    PBAStreamFunc            Start;
    PBAStreamFunc            Stop;

    //Hardware API
    PBAStreamDeviceFunc      SetOutputDevice;

    //We expose a global pool of stream handles for each OS that can be used to create an audio render Stream to the default hardware device
    PBAStreamContext         OutputStreams[PBA_MAX_DEVICE_OUTPUTS];
    
    //Master Device
    //AudioDeviceID                 OutputDevice;
    
}PBAudioStreamFactory;

//static const PBAudioClientDriver PBAudio = {PBAudioStreamInit, PBAudioStreamStart, PBAudioStreamSetOutputDevice};
PB_AUDIO_EXTERN PBAudioStreamFactory PBAudio;

#endif /* pbaudio_engine_h */
