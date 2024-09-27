//
//  pbaudio_callbacks.h
//  CRViewer
//
//  Created by Joe Moulton on 6/25/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_callbacks_h
#define pbaudio_callbacks_h

#include "PbAudioStream.h"



#ifdef __cplusplus
extern "C" {
#endif


#ifdef __APPLE__

static OSStatus PBAudioStreamSubmitBuffers(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
    //Cast input context
    //__unsafe_unretained PBAIOAudioUnit * renderContext = (PBAIOAudioUnit *)inRefCon;
    __unsafe_unretained PBAStreamContext * streamContext = (PBAStreamContext *)inRefCon;

    //Get current timestamp
    AudioTimeStamp timestamp = *inTimeStamp;

    //perform latency compensation (iOS) only
#if TARGET_OS_IPHONE
    if ( streamContext->latencyCompensation ) timestamp.mHostTime += PBAHostTicksFromSeconds(streamContext->outputLatency);
#endif
   
    //fprintf(stderr, "\ninBusNumber: %lu, inNumberFrames: %lu", inBusNumber, inNumberFrames );
    
#ifdef PBA_DEBUG
        uint64_t start = PBACurrentTimeInHostTicks();
#endif
          
    //Use the render context callback to allow the client to fill the buffer with renderpasses
    __unsafe_unretained PBAStreamOutputPass outputpass = streamContext->outputpass;
    if ( !streamContext->bypass && outputpass ) outputpass(ioData, inNumberFrames, &timestamp);
    else *ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
    
#ifdef PBA_DEBUG
        PBAStreamReportRenderTime(streamContext, &_audioReport, PBASecondsFromHostTicks(PBACurrentTimeInHostTicks() - start), (double)inNumberFrames / streamContext->currentSampleRate);
#endif

    return noErr;
}


static OSStatus PBAIOAudioUnitInputCallback(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
    // Grab timestamp
    //__unsafe_unretained AEIOAudioUnit * THIS = (__bridge AEIOAudioUnit *)inRefCon;
    __unsafe_unretained PBAStreamContext * streamContext = (PBAStreamContext *)inRefCon;

    AudioTimeStamp timestamp = *inTimeStamp;
    printf("PBAIOAudioUnitInputCallback\n");

#if TARGET_OS_IPHONE
    if ( streamContext->latencyCompensation ) {
        timestamp.mHostTime -= PBAHostTicksFromSeconds(streamContext->inputLatency);
    }
#endif
    
    //THIS->_inputTimestamp = timestamp;
    streamContext->inputTimeStamp = timestamp;
    return noErr;
}



static void PBAIOAudioUnitStreamFormatChanged(void *inRefCon, AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement) {
    //AEIOAudioUnit * self = (__bridge AEIOAudioUnit *)inRefCon;
    __unsafe_unretained PBAStreamContext * streamContext = (PBAStreamContext *)inRefCon;
    fprintf(stdout, "PBAIOAudioUnitStreamFormatChanged\n");
    //PBAudioStreamUpdateFormat(streamContext);
    
    /*
    dispatch_async(dispatch_get_main_queue(), ^{
        if ( streamContext->running ) {
            //TO DO:  [self updateStreamFormat];
        }
    });
    */
}

static void PBAIOAudioUnitSampleRateChanged(void *inRefCon, AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement)
{

    //AEIOAudioUnit * self = (__bridge AEIOAudioUnit *)inRefCon;
    __unsafe_unretained PBAStreamContext * streamContext = (PBAStreamContext *)inRefCon;
    fprintf(stdout, "PBAIOAudioUnitSampleRateChanged\n");
    //PBAudioStreamUpdateFormat(streamContext);
    
    //Distribute the notification to registered PBAudio 'Engine' Process Clients
    dispatch_async(dispatch_get_main_queue(), ^{
        
        //CFNumberRef deviceID = CFNumberCreate(NULL, kCFNumberSInt32Type, &inObjectID);

        //[NSNotificationCenter.defaultCenter postNotificationName:AEAudioDeviceDefaultInputDeviceChangedNotification object:nil];
        
        // populate an notification
        CFDictionaryKeyCallBacks   keyCallbacks    = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
        CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
        CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
        CFDictionaryAddValue(dictionary, CFSTR("DeviceStream"),   inRefCon);
        //CFDictionaryAddValue(dictionary, CFSTR("OutputDeviceID"), deviceID);

        // post a notification
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), kPBAStreamSampleRateChangedNotification, NULL, dictionary, true);
        
        CFRelease(dictionary);
        //CFRelease(deviceID);
    });
}


/*
#if TARGET_OS_IPHONE
static void AEIOAudioUnitIAAConnectionChanged(void *inRefCon, AudioUnit inUnit, AudioUnitPropertyID inID,
                                              AudioUnitScope inScope, AudioUnitElement inElement) {
    AEIOAudioUnit * self = (__bridge AEIOAudioUnit *)inRefCon;
    dispatch_async(dispatch_get_main_queue(), ^{
        [self updateStreamFormat];
        
        UInt32 iaaConnected = NO;
        UInt32 size = sizeof(iaaConnected);
        if ( AECheckOSStatus(AudioUnitGetProperty(self.audioUnit, kAudioUnitProperty_IsInterAppConnected,
                                                  kAudioUnitScope_Global, 0, &iaaConnected, &size),
                             "AudioUnitGetProperty(kAudioUnitProperty_IsInterAppConnected)") && iaaConnected && !self.running ) {
            // Start, if connected to IAA and not running
            [self start:NULL];
        }
    });
}
#endif
*/


#pragma mark -- AVAudioSession Notification Observer Callbacks

static void AVAudioSessionInterruptionNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    fprintf(stdout, "\nAVAudioSessionInterruptionNotification\n");
    
     //CFShow(CFSTR("Received notification (dictionary):"));
     // print out user info
     
    /*
    const void * keys;
     const void * values;
     CFDictionaryGetKeysAndValues(userInfo, &keys, &values);
     char * keyStr;
     char * valStr;
     for (int i = 0; i < CFDictionaryGetCount(userInfo); i++) {
         keyStr = (char*)CFStringGetCStringPtr((CFStringRef)&keys[i], CFStringGetSystemEncoding());
         valStr = (char*)CFStringGetCStringPtr((CFStringRef)&values[i], CFStringGetSystemEncoding());
         printf("\t\t \"%s\" = \"%s\"\n", keyStr, valStr);
         
         if  ( strcmp(keyStr, AVAudioSessionInterruptionTypeKey) )
             break;
     }
     */
    
    /*
    int type = atoi(valStr);//[notification.userInfo[AVAudioSessionInterruptionTypeKey] integerValue];
    if ( type == AVAudioSessionInterruptionTypeBegan )
    {
        wasRunning = weakSelf.running;
        
        UInt32 interAppAudioConnected;
        UInt32 size = sizeof(interAppAudioConnected);
        PBACheckOSStatus(AudioUnitGetProperty(weakSelf.audioUnit, kAudioUnitProperty_IsInterAppConnected, kAudioUnitScope_Global, 0, &interAppAudioConnected, &size), "AudioUnitGetProperty");
        if ( interAppAudioConnected ) {
            // Restart immediately, this is a spurious interruption
            if ( !wasRunning ) {
                [weakSelf start:NULL];
            }
        } else {
            if ( wasRunning ) {
                [weakSelf stop];
            }
            [[NSNotificationCenter defaultCenter] postNotificationName:AEIOAudioUnitSessionInterruptionBeganNotification object:weakSelf];
        }
    } else {
        NSUInteger optionFlags =
            [notification.userInfo[AVAudioSessionInterruptionOptionKey] unsignedIntegerValue];
        if (optionFlags & AVAudioSessionInterruptionOptionShouldResume) {
            if ( wasRunning ) {
                [weakSelf start:NULL];
            }
            [[NSNotificationCenter defaultCenter] postNotificationName:AEIOAudioUnitSessionInterruptionEndedNotification object:weakSelf];
        }
    }
    */
}

static void AVAudioSessionMediaServicesWereResetNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    
    printf("\nAVAudioSessionMediaServicesWereResetNotificationCallback\n");
    //[weakSelf reload];
}

static void AVAudioSessionRouteChangeNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    printf("\nAVAudioSessionRouteChangeNotificationCallback\n");

    /*
     dispatch_async(dispatch_get_main_queue(), ^{
            renderContext->outputLatency = [AVAudioSession sharedInstance].outputLatency;
            renderContext->inputLatency = [AVAudioSession sharedInstance].inputLatency;
            //renderContext->inputGain = weakSelf.inputGain;
        });
    */
}

#else

//static OSStatus PBAudioStreamSubmitBuffers(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData)



#pragma mark -- Pure C COM Virtual Interface Implementation

//#include <windows.h>
//#include <unknwn.h>
//#include <unknwnbase.h>
//#include <objbase.h>
//#include <INITGUID.H>
#include <mmdeviceapi.h>

struct PBADeviceNotificationClient;

typedef HRESULT STDMETHODCALLTYPE QueryInterfacePtr(struct PBADeviceNotificationClient*, REFIID, void**);
typedef ULONG   STDMETHODCALLTYPE AddRefPtr(struct PBADeviceNotificationClient*);
typedef ULONG   STDMETHODCALLTYPE ReleasePtr(struct PBADeviceNotificationClient*);

typedef HRESULT STDMETHODCALLTYPE OnDeviceStateChangedPtr(struct PBADeviceNotificationClient*, LPCWSTR, DWORD);
typedef HRESULT STDMETHODCALLTYPE OnDeviceAddedPtr(struct PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId);
typedef HRESULT STDMETHODCALLTYPE OnDeviceRemovedPtr(struct PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId);
typedef HRESULT STDMETHODCALLTYPE OnDefaultDeviceChangedPtr(struct PBADeviceNotificationClient*, EDataFlow, ERole, LPCWSTR);
typedef HRESULT STDMETHODCALLTYPE OnPropertyValueChangedPtr(struct PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId, const PROPERTYKEY key);

typedef struct PBADeviceNotificationCallbacks
{
    OnDeviceStateChangedPtr*               OnDeviceStateChanged;
    OnDeviceAddedPtr*                      OnDeviceAdded;
    OnDeviceRemovedPtr*                    OnDeviceRemoved;
    OnDefaultDeviceChangedPtr*             OnDefaultDeviceChanged;
    OnPropertyValueChangedPtr*             OnPropertyValueChanged;
}PBADeviceNotificationCallbacks;

typedef struct PBADeviceNotificationClientVtbl
{
    // First 3 members must be called QueryInterface, AddRef, and Release
    QueryInterfacePtr*                     QueryInterface;
    AddRefPtr*                             AddRef;
    ReleasePtr*                            Release;

    union
    {
        struct
        {
            OnDeviceStateChangedPtr*       OnDeviceStateChanged;
            OnDeviceAddedPtr*              OnDeviceAdded;
            OnDeviceRemovedPtr*            OnDeviceRemoved;
            OnDefaultDeviceChangedPtr*     OnDefaultDeviceChanged;
            OnPropertyValueChangedPtr*     OnPropertyValueChanged;
        };  PBADeviceNotificationCallbacks callbacks;
    };


    volatile LONG                          _cRef;// = 1;

}PBADeviceNotificationClientVtbl;

typedef struct PBADeviceNotificationClient 
{ 
    struct PBADeviceNotificationClientVtbl* lpVtbl;
    void*                                   context;
}PBADeviceNotificationClient;


static ULONG STDMETHODCALLTYPE AddRef(PBADeviceNotificationClient* This){ return InterlockedIncrement(&((volatile const LONG)This->lpVtbl->_cRef)); }

static ULONG STDMETHODCALLTYPE Release(PBADeviceNotificationClient* This)
{
    ULONG ulRef = InterlockedDecrement(&((volatile const LONG)This->lpVtbl->_cRef));
    if (0 == ulRef) { /*delete this;*/ }
    return ulRef;
}

static HRESULT STDMETHODCALLTYPE QueryInterface(PBADeviceNotificationClient* This, REFIID riid, VOID** ppvInterface)
{
    //AddRef(This); *ppvInterface = (IMMNotificationClient*)This;

#ifdef __cplusplus
    if (IID_IUnknown == riid) { AddRef(This); *ppvInterface = (IUnknown*)This; }
    else if (__uuidof(IMMNotificationClient) == riid) { AddRef(This); *ppvInterface = (IMMNotificationClient*)This; }
    else { *ppvInterface = NULL; return E_NOINTERFACE; }
#else
    if (__riid(IUnknown) == riid) { AddRef(This); *ppvInterface = (IUnknown*)This; }
    else if (__riid(IMMNotificationClient) == riid) { AddRef(This); *ppvInterface = (IMMNotificationClient*)This; }
    else { *ppvInterface = NULL; return E_NOINTERFACE; }
#endif

    return S_OK;
}

// Callback methods for device-event notifications.

static HRESULT STDMETHODCALLTYPE PBADeviceStateChanged(PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    fprintf(stdout, "  -->PBADeviceStateChanged\n");
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE PBADeviceAdded(PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId)
{

    fprintf(stdout, "  -->PBADeviceAdded\n");
    return S_OK;
};

static HRESULT STDMETHODCALLTYPE PBADeviceRemoved(PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId)
{
    fprintf(stdout, "  -->PBADeviceRemoved\n");
    return S_OK;
}


static HRESULT STDMETHODCALLTYPE PBAudioDeviceDefaultOutputChanged(PBADeviceNotificationClient* This, EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
{
    fprintf(stdout, "  -->PBAudioDeviceDefaultOutputChanged\n");
    return S_OK;
}


static HRESULT STDMETHODCALLTYPE PBAudioDevicePropertyValueChanged(PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    fprintf(stdout, "  -->PBAudioDevicePropertyValueChanged\n");
    return S_OK;
}


//Since the contents of IExample_Vtbl will never change, we'll just declare it static and initialize it that way. It can be reused for arbitrary # of instances of IMMNotificationClient.
static PBADeviceNotificationClientVtbl PBADeviceNotificationClient_Vtbl = { QueryInterface, AddRef, Release, PBADeviceStateChanged, PBADeviceAdded, PBADeviceRemoved, PBAudioDeviceDefaultOutputChanged, PBAudioDevicePropertyValueChanged, 1 };

static PBADeviceNotificationClient _PBADeviceNotificationClient = { &PBADeviceNotificationClient_Vtbl, NULL };

static PBADeviceNotificationClient PbAudioDeviceNotificationClient(PBADeviceNotificationCallbacks * callbacks)
{
    PBADeviceNotificationClient client = { &PBADeviceNotificationClient_Vtbl, NULL };
    client.lpVtbl->callbacks = *callbacks;
    return client;
}

#endif //__APPLE__



#ifdef __cplusplus
}
#endif


#endif /* pbaudio_callbacks_h */
