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
#include "pba_event_queue.h"



#ifdef __cplusplus
extern "C" {
#endif

//On Win32,  if format is NULL, it will create a shared mode device stream (which usually gives a 32-bit floating point format)
//On Darwin, if format is NULL, it will create a device stream with a 32-bit floating point format
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioStreamInit(PBAStreamContext * streamContext, PBAStreamFormat * format, PBAudioDevice deviceID, PBAStreamOutputPass outputpass);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioStreamStart(PBAStreamContext * streamContext);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioStreamStop(PBAStreamContext * streamContext);
PB_AUDIO_API PB_AUDIO_INLINE void     PBAudioStreamSetBypass(PBAStreamContext * streamContext, bool bypass);

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioStreamGetOutputDevice(PBAStreamContext * streamContext, PBAudioDevice* deviceID);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioStreamSetOutputDevice(PBAStreamContext * streamContext, PBAudioDevice deviceID);

//#pragma mark -- CTConnection API Method Function Pointer Definitions
//typedef int (*CTConnectFunc)(struct CTTarget * service, CTConnectionClosure callback);
typedef OSStatus (*PBAStreamFunc)       (PBAStreamContext * streamContext);
typedef OSStatus (*PBAStreamDeviceFunc) (PBAStreamContext * streamContext, PBAudioDevice deviceID);
typedef OSStatus (*PBAStreamFormatFunc) (PBAStreamContext * streamContext, PBAStreamFormat * format);
typedef OSStatus (*PBAStreamInitFunc)   (PBAStreamContext * streamContext, PBAStreamFormat * format, PBAudioDevice deviceID, PBAStreamOutputPass outputpass);


//#pragma mark -- Global Audio Stream Factory Object
typedef struct PBAudioStreamFactory
{
    //The Client Driver Object can create AudioUnit/AVSession + BufferList Instances
    PBAStreamInitFunc        Init;
    PBAStreamFunc            Start;
    PBAStreamFunc            Stop;

    //Hardware API
    PBAStreamDeviceFunc      SetOutputDevice;

    //We expose a per instance pool of audio stream containers that can be used 
    //to schedule audio buffers to a hardware device via a real-time audio thread
    PBAStreamContext         OutputStreams[PBA_MAX_DEVICE_OUTPUTS];

    //Master Device
    //AudioDeviceID          OutputDevice;

    //PBAKernelQueue           eventQueue;       //event queue
    pba_platform_thread      mainThread;      //event queue thread [pool]
    unsigned int             mainThreadID;

    //Event Queue
    PBAKernelQueue           eventQueue;       //event queue
    pba_platform_thread      eventThread;      //event queue thread [pool]
    unsigned int             eventThreadID;

    //Resource Queue
    //PBAKernelQueue         resourceQueue;    //resource queue
    //pba_platform_thread    resourceThread;   //resource queue thread [pool]
    //unsigned int           resourceThreadID;

    pba_pid_t                 pid;
    pba_kernel_queue_event    ipcNotification;	//an event that can be shared with an external process for IPC
    PBAKernelQueue            ipcEventQueue;    //a queue for the ipcNotification; can thisi be shared with eventQueue


}PBAudioStreamFactory;

//Note:  DYNAMIC libraries can only resolve links to functions but not global exported vars
//extern PBAudioStreamFactory PBAudio;// = {PBAudioStreamInit, PBAudioStreamStart, PBAudioStreamStop, PBAudioStreamSetOutputDevice};

//Global [Pb]Audio Singleton Instance Struct
PB_AUDIO_API PB_AUDIO_INLINE PBAudioStreamFactory* GetPBAudioStreamFactory(void);

//Convenience Accessor
#define PBAudio (*GetPBAudioStreamFactory())

#ifdef __cplusplus
}
#endif

#endif /* pbaudio_engine_h */
