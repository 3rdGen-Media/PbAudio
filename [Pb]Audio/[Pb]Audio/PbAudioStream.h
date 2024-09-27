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
#define CALL(method, object, ... )  (object->lpVtbl->method(object, ##__VA_ARGS__))
#define __riid(x) (REFIID)(&_IID_ ## x)
#define __clsid(x) (REFIID)(&_CLSID_ ## x)


static const IID _IID_IUnknown              = { 0x00000000, 0x0000, 0x0000, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }};

//For CoCreateInstance to obtain Device Enumerator
static const IID _IID_IMMDeviceEnumerator   = { 0xa95664d2, 0x9614, 0x4f35, { 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6 }};
static const IID _CLSID_MMDeviceEnumerator  = { 0xbcde0395, 0xe52f, 0x467c, { 0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2E }};
static const IID _IID_IMMNotificationClient = { 0x7991eec9, 0x7e89, 0x4d85, { 0x83, 0x90, 0x6c, 0x70, 0x3c, 0xec, 0x60, 0xc0 }};

//For AudioClient and RenderClient
static const IID _IID_IAudioClient          = { 0x1cb9ad4c, 0xdbfa, 0x4c32, { 0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2 }};
static const IID _IID_IAudioClient2         = { 0x726778cd, 0xf60a, 0x4eda, { 0x82, 0xde, 0xe4, 0x76, 0x10, 0xcd, 0x78, 0xaa }};
static const IID _IID_IAudioClient3         = { 0x7ed4ee07, 0x8e67, 0x4cd4, { 0x8c, 0x1a, 0x2b, 0x7a, 0x59, 0x87, 0xad, 0x42 }};

//F294ACFC-3146-4483-A7BF-ADDCA7C260E2
static const IID _IID_IAudioRenderClient    = { 0xF294ACFC, 0x3146, 0x4483, { 0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2 }};

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

struct PBAStreamContext; //fwd

#ifdef __APPLE__
PB_AUDIO_EXTERN const CFStringRef kPBAStreamFormatChangedNotification;     //Format was changed as a Result of UpdateStream
PB_AUDIO_EXTERN const CFStringRef kPBAStreamSampleRateChangedNotification; //Sample Rate Was Changed As a Result of UpdateStream
PB_AUDIO_EXTERN const CFStringRef kPBASampleRateChangedNotification;       //Sample Rate Was Changed As a Result of DeviceSetlRate

typedef void (^PBAStreamOutputPass)(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp, struct PBAStreamContext* stream);
#else
typedef void (*PBAStreamOutputPass)(struct PBABufferList* ioData, uint32_t frames, const struct PBATimeStamp* timestamp, struct PBAStreamContext* stream);
#endif

#ifdef __APPLE__
typedef void (^PBARenderPass)(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp, void* _Nullable source, void* _Nullable events, UInt32 nEvents);
#else
typedef void (*PBARenderPass)(struct PBABufferList * ioData, uint32_t frames, const struct PBATimeStamp * timestamp, void* source, void* events, uint32_t nEvents);
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


#define IAUDIOCLIENT3

#ifdef IAUDIOCLIENT3
#define IAUDIOCLIENT IAudioClient3
#else
#define IAUDIOCLIENT IAudioClient2
#endif

typedef enum PBADriverMode
{
    PBA_DRIVER_SHARED     = AUDCLNT_SHAREMODE_SHARED,
    PBA_DRIVER_EXCLUSIVE  = AUDCLNT_SHAREMODE_EXCLUSIVE,
    //PBA_DRIVER_VENDOR,
    PBA_NUM_DRIVER_MODES
}PBADriverMode;

typedef struct PBAStreamContext
{
#ifdef _WIN32	

    IMMDevice			      *audioDevice;
    IMMNotificationClient     *notifyClient;

    IAUDIOCLIENT              *audioClient;
    IAudioRenderClient	      *renderClient;

    AUDCLNT_SHAREMODE          shareMode;

    //For now we will only allow output to a single recognized hw device
    HANDLE                     hEvent;
    unsigned int               audioThreadID;

	REFERENCE_TIME             devicePeriod;
	UINT32	                   bufferFrameCount;
	
#elif defined(__APPLE__)
    AudioUnit _Nullable        audioUnit;
    AudioDeviceID              audioDevice;
#endif
	

    PBAStreamFormat             format;
    PBAStreamFormatSampleType   target;

    PBAStreamOutputPass         outputpass;
    PBATimeStamp                inputTimeStamp;
    
    double                      sampleRate;
    double                      currentSampleRate;	//what is difference between currentSampleRate and sampleRate?
    double                      inputLatency, outputLatency; //iOS only
    int                         nInputChannels, nOutputChannels;
    //int                         maxInputChannels;
    
    //TO DO:  make these bitflags
	bool                        inputEnabled, outputEnabled;
    bool                        running, bypass;
    bool                        hasSetInitialStreamFormat;
    bool                        latencyCompensation;// iOS only
    
    bool                        respectDefault;

}PBAStreamContext;

#ifdef _WIN32
//We expose a WIN32 global audio device enumerator (that gets initialized with COM in a multithreaded or apartment threading model)
//PB_AUDIO_EXTERN IMMDeviceEnumerator *_PBADeviceEnumerator;
#endif


PB_AUDIO_API PB_AUDIO_INLINE void PBAudioStreamUpdateFormat(PBAStreamContext * streamContext, double sampleRate);

#ifdef __APPLE__

PB_AUDIO_EXTERN const CFStringRef kPBAudioDidUpdateStreamFormatNotification;


PB_AUDIO_API PB_AUDIO_INLINE double PBABufferDuration(PBAStreamContext* streamContext);

#endif //__APPLE__

//PBA_DEBUG
PB_AUDIO_API PB_AUDIO_INLINE void PBAStreamReportRenderTime(PBAStreamContext * streamContext, PBAStreamLatencyReport * report, double renderTime, double bufferDuration); 




#endif /* pbaudio_stream_h */
