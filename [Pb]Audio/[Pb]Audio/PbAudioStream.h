//  PBAudio
//  pbaudio_stream.h
//
//  Here we define the crossplatform notion of an "Audio Stream"
//
//  Created by Joe Moulton on 6/26/20.
//  Copyright © 2020 3rdGen Multimedia
//

#ifndef pbaudio_stream_h
#define pbaudio_stream_h

//OS Includes for Creating an "Audio Stream"
#import <AudioToolbox/AudioToolbox.h>


#ifdef __APPLE__
#include <TargetConditionals.h>

#if    TARGET_OS_IOS || TARGET_OS_TVOS
//#include <CoreFoundation/CoreFoundation.h>           //Core Foundation
#include <objc/runtime.h>                            //objective-c runtime
#include <objc/message.h>                            //objective-c runtime message
//#else
//#include <ApplicationServices/ApplicationServices.h> //Cocoa
//#include <CoreFoundation/CoreFoundation.h>           //Core Foundation

typedef AudioBufferList PBABufferList;
#else // OSX

#endif
#endif

//We expose a global handle for each OS that can be used to create Stream

//On WIN32 to create a "stream" that allows rendering to a buffer to hardware we must:
//  1)  Create a WSAPI AudioClient ...


//Mac OSX to create a "stream" that allows rendering to a buffer to hardware we must:
//  1)  Create and CoreAudio RemoteIO AudioUnit
//static AudioUnit _Nullable _audioUnit;


/*!
 * PBAStream Render block
 *
 *  This defines a C-Style closure/callback for rendering to an Platform "Audio Stream" via an "AudioBufferList"
 *
 * @param ioData The audio buffer list to fill
 * @param frames The number of frames
 * @param timestamp The corresponding timestamp
 */

//TO DO:  make this a xplatform closure definition
typedef void (^PBAIOAudioUnitRenderClosure)(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp);

//Render Context

/*!
 * Render context
 *
 *  This structure is passed into the render loop block, and contains information about the
 *  current rendering environment, as well as providing access to the render's buffer stack.
 */
/*
typedef struct {
    
    //! The output buffer list. You should write to this to produce audio.
    const AudioBufferList * _Nonnull output;
    
    //! The number of frames to render to the output
    UInt32 frames;
    
    //! The current sample rate, in Hertz
    double sampleRate;
    
    //! The current audio timestamp
    const AudioTimeStamp * _Nonnull timestamp;
    
    //! Whether rendering is offline (faster than realtime)
    BOOL offlineRendering;
    
    //! The buffer stack. Use this as a workspace for generating and processing audio.
    AEBufferStack * _Nonnull stack;
    
} AERenderContext;
*/


typedef struct PBAStreamContext
{
#ifdef __APPLE__
     AudioUnit _Nullable        audioUnit;    
#endif
    PBAIOAudioUnitRenderClosure renderCallback;
    PBATimeStamp                inputTimeStamp;
    double                      sampleRate;
    double                      currentSampleRate;
    double                      inputLatency;      //iOS only
    double                      outputLatency;      //iOS only
    float                       inputGain;
    int                         nInputChannels;
    int                         maxInputChannels;
    int                         nOutputChannels;
    bool                        inputEnabled;
    bool                        outputEnabled;
    bool                        running;
    bool                        hasSetInitialStreamFormat;
    bool                        latencyCompensation;// iOS only

}PBAStreamContext;

//We expose a global handle for each OS that can be used to create Stream
PB_AUDIO_EXTERN PBAStreamContext g_renderContext;


PB_AUDIO_API PB_AUDIO_INLINE void PBAUpdateStreamFormat(PBAStreamContext * streamContext);




#if !TARGET_OS_IPHONE
static AudioDeviceID defaultDeviceForScope( AudioObjectPropertyScope scope)
{
    AudioDeviceID deviceId;
    UInt32 size = sizeof(deviceId);
    AudioObjectPropertyAddress addr =
    {
        scope == kAudioDevicePropertyScopeInput ? kAudioHardwarePropertyDefaultInputDevice : kAudioHardwarePropertyDefaultOutputDevice,
        .mScope = kAudioObjectPropertyScopeGlobal,
        .mElement = 0
    };
    if ( !PBACheckOSStatus(AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &size, &deviceId), "AudioObjectGetPropertyData") )
    {
        return kAudioDeviceUnknown;
    }
    
    return deviceId;
}

static AudioStreamBasicDescription streamFormatForDefaultDeviceScope( AudioObjectPropertyScope scope)
{
    // Get the default device
    AudioDeviceID deviceId = defaultDeviceForScope(scope);
    if ( deviceId == kAudioDeviceUnknown ) return (AudioStreamBasicDescription){};
    
    // Get stream format
    AudioStreamBasicDescription asbd;
    UInt32 size = sizeof(asbd);
    AudioObjectPropertyAddress addr = { kAudioDevicePropertyStreamFormat, scope, 0 };
    if ( !PBACheckOSStatus(AudioObjectGetPropertyData(deviceId, &addr, 0, NULL, &size, &asbd), "AudioObjectGetPropertyData") )
    {
        return (AudioStreamBasicDescription){};
    }
    
    return asbd;
}
#endif

static double PBABufferDuration(PBAStreamContext* streamContext)
{
#if TARGET_OS_IPHONE || TARGET_OS_TVOS
    //return [[AVAudioSession sharedInstance] IOBufferDuration];
    
    void* (*objc_msgSendSharedInstance)(Class, SEL) = (void*)objc_msgSend;
    id avSessionSharedInstance = objc_msgSendSharedInstance(objc_getClass("AVAudioSession"), sel_registerName("sharedInstance"));
    double (*objc_msgSendGetProperty)(void*, SEL) = (void*)objc_msgSend;
    return objc_msgSendGetProperty(avSessionSharedInstance, sel_getUid("IOBufferDuration"));//[AVAudioSession sharedInstance].outputLatency;
       
#else
    // Get the default device
    AudioDeviceID deviceId = defaultDeviceForScope(streamContext->outputEnabled ? kAudioDevicePropertyScopeOutput : kAudioDevicePropertyScopeInput);
    if ( deviceId == kAudioDeviceUnknown ) return 0.0;
    
    // Get the buffer duration
    UInt32 duration;
    UInt32 size = sizeof(duration);
    AudioObjectPropertyAddress addr =
    {
        kAudioDevicePropertyBufferFrameSize,
        streamContext->outputEnabled ? kAudioDevicePropertyScopeOutput : kAudioDevicePropertyScopeInput, 0 };
    if ( !PBACheckOSStatus(AudioObjectGetPropertyData(deviceId, &addr, 0, NULL, &size, &duration), "AudioObjectSetPropertyData") ) return 0.0;
    return (double)duration / streamContext->currentSampleRate;
#endif
}


//DEBUG

PB_AUDIO_API PB_AUDIO_INLINE void PBAStreamReportRenderTime(PBAStreamContext * streamContext, PBAStreamLatencyReport * report, double renderTime, double bufferDuration); 




#endif /* pbaudio_stream_h */
