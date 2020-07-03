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


#ifdef __APPLE__
#import <AudioToolbox/AudioToolbox.h>
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

#elif defined(_WIN32)
//WSAPI
#include <AudioClient.h>
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

#ifdef __APPLE__
typedef void (^PBAIOAudioUnitRenderClosure)(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp);
#else
typedef void (*PBAIOAudioUnitRenderClosure)(struct PBABufferList * ioData, uint32_t frames, const struct PBATimeStamp * timestamp);
#endif

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

#include <Audioclient.h>
#define CALL(method, object, ... )  (object->lpVtbl->method(object, ##__VA_ARGS__))
#define __riid(x) (REFIID)(&_IID_ ## x)
#define __clsid(x) (REFIID)(&_CLSID_ ## x)

//For CoCreateInstance to obtain Device Enumerator
static const IID _IID_IMMDeviceEnumerator = { 0xa95664d2, 0x9614, 0x4f35, { 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6 } };
//A95664D2-9614-4F35-A746-DE8DB63617E6
static const IID _CLSID_MMDeviceEnumerator = { 0xbcde0395, 0xe52f, 0x467c, {0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2E }};

//For AudioClient and RenderClient
//1CB9AD4C-DBFA-4c32-B178-C2F568A703B2
static const IID _IID_IAudioClient = { 0x1cb9ad4c, 0xdbfa, 0x4c32, { 0xb1,0x78,0xc2,0xf5,0x68,0xa7,0x03,0xb2 } };
static const IID _IID_IAudioClient2 = { 0x726778cd, 0xf60a, 0x4eda,{ 0x82,0xde,0xe4,0x76,0x10,0xcd,0x78,0xaa } };
//7ED4EE07-8E67-4CD4-8C1A-2B7A5987AD42
static const IID _IID_IAudioClient3 = { 0x7ed4ee07, 0x8e67, 0x4cd4,{ 0x8c,0x1a,0x2b,0x7a,0x59,0x87,0xad,0x42 } };

//F294ACFC-3146-4483-A7BF-ADDCA7C260E2
static const IID _IID_IAudioRenderClient = { 0xF294ACFC, 0x3146, 0x4483,{ 0xa7,0xbf,0xad,0xdc,0xa7,0xc2,0x60,0xe2 } };



typedef struct PBAStreamContext
{
#ifdef _WIN32	
	IMMDevice			*audioDevice;// = NULL;
	IAudioClient2		*audioClient;
	IAudioRenderClient	*renderClient;
	//For now we will only allow output to a single recognized hw device
	
	REFERENCE_TIME devicePeriod;
	HANDLE hEvent;
	UINT32	bufferFrameCount;
	unsigned int renderThreadID;   
	unsigned int controlThreadID;
	AUDCLNT_SHAREMODE			shareMode;
#elif defined(__APPLE__)
    AudioUnit _Nullable        audioUnit;    
#endif
	PBAStreamFormat format;
    PBAIOAudioUnitRenderClosure renderCallback;
    PBATimeStamp                inputTimeStamp;
    double                      sampleRate;
    double                      currentSampleRate;	//what is difference between currentSampleRate and sampleRate?
    double                      inputLatency;       //iOS only
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

//We expose a WIN32 global audio device enumerator (that gets initialized with COM in a multithreaded or apartment threading model)
PB_AUDIO_EXTERN IMMDeviceEnumerator *_PBADeviceEnumerator;

//We expose a global handle for each OS that can be used to create an audio render Stream to the default hardware device
PB_AUDIO_EXTERN PBAStreamContext	_PBAMasterStream;




PB_AUDIO_API PB_AUDIO_INLINE void PBAUpdateStreamFormat(PBAStreamContext * streamContext);


#ifdef __APPLE__

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

#endif //__APPLE__

//DEBUG

PB_AUDIO_API PB_AUDIO_INLINE void PBAStreamReportRenderTime(PBAStreamContext * streamContext, PBAStreamLatencyReport * report, double renderTime, double bufferDuration); 




#endif /* pbaudio_stream_h */
