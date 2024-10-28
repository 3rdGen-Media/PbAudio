//
//  PbAudioStream.c
//  [Pb]AudioOSX
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

#ifdef __APPLE__
#import <AudioToolbox/AudioToolbox.h>
#endif

#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif


//PBAStreamContext	_PBAMasterStream;


#ifdef __APPLE__
const CFStringRef kPBAudioDidUpdateStreamFormatNotification  = CFSTR("PBAudioDidUpdateStreamFormatNotification");
//const CFStringRef kPBAStreamFormatChangedNotification      = CFSTR("kPBAStreamFormatChangedNotification");
const CFStringRef kPBAStreamSampleRateChangedNotification    = CFSTR("PBAudioStreamSampleRateChangedNotification");

const CFStringRef kPBAudioSessionRouteChangedNotification    = CFSTR("PBAudioSessionRouteChangedNotification");
#else

#endif

void PBAudioStreamUpdateFormat(PBAStreamContext * streamContext, double sampleRate)
{

    bool running = streamContext->running;
    bool stoppedUnit = false;
    bool hasChanges = false;
    bool iaaInput = false;
    bool iaaOutput = false;
    //double priorSampleRate = streamContext->currentSampleRate;

    fprintf(stdout, "PBAUpdateStreamFormat...\n");

#ifdef __APPLE__

#if TARGET_OS_IPHONE || TARGET_OS_TVOS
    UInt32 iaaConnected = NO;
    UInt32 size = sizeof(iaaConnected);
    if ( PBACheckOSStatus(AudioUnitGetProperty(streamContext->audioUnit, kAudioUnitProperty_IsInterAppConnected, kAudioUnitScope_Global, 0, &iaaConnected, &size), "AudioUnitGetProperty(kAudioUnitProperty_IsInterAppConnected)") && iaaConnected )
    {
        AudioComponentDescription componentDescription;
        size = sizeof(componentDescription);
        if ( PBACheckOSStatus(AudioUnitGetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_NodeComponentDescription, kAudioUnitScope_Global, 0, &componentDescription, &size), "AudioUnitGetProperty(kAudioOutputUnitProperty_NodeComponentDescription)") )
        {
            iaaOutput = YES;
            iaaInput = componentDescription.componentType == kAudioUnitType_RemoteEffect || componentDescription.componentType == kAudioUnitType_RemoteMusicEffect;
        }
    }
#endif
    bool hasSampleRateChanges = false;

    if ( streamContext->outputEnabled ) {
        // Get the current output sample rate and number of output channels
        AudioStreamBasicDescription asbd;
        UInt32 size = sizeof(asbd);
        PBACheckOSStatus(AudioUnitGetProperty(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &asbd, &size), "AudioUnitGetProperty(kAudioUnitProperty_StreamFormat)");
        if ( iaaOutput ) 
        {
            asbd.mChannelsPerFrame = 2;
        }
        
        bool hasOutputChanges     = false;

        double newSampleRate = sampleRate == 0 ? asbd.mSampleRate : sampleRate;
        if ( fabs(streamContext->currentSampleRate - newSampleRate) > DBL_EPSILON )
        {
            hasChanges = hasOutputChanges = hasSampleRateChanges = true;
            streamContext->currentSampleRate = newSampleRate;
        }
        
        if ( streamContext->nOutputChannels != (int)asbd.mChannelsPerFrame ) 
        {
            hasChanges = hasOutputChanges = true;
            streamContext->nOutputChannels = asbd.mChannelsPerFrame;
        }
        
        if ( hasOutputChanges || !streamContext->hasSetInitialStreamFormat ) 
        {
            if ( running ) 
            {
                //PBACheckOSStatus(AudioOutputUnitStop(streamContext->audioUnit), "AudioOutputUnitStop");
                //stoppedUnit = true;
            }

            // Update the stream format
            asbd = _audioFormat;
            //asbd.mFormatFlags = streamContext->format.mFormatFlags;
            asbd.mChannelsPerFrame = streamContext->nOutputChannels;
            asbd.mSampleRate = streamContext->currentSampleRate;
            PBACheckOSStatus(AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &asbd, sizeof(asbd)), "AudioUnitSetProperty(kAudioUnitProperty_StreamFormat)");
        }
        
        //set the output format onm our stream context
        memcpy(&(streamContext->format), &asbd, sizeof(PBAStreamFormat));
    }
    
    if ( streamContext->inputEnabled ) 
    {
        // Get the current input number of input channels
        AudioStreamBasicDescription asbd;
        UInt32 size = sizeof(asbd);
        PBACheckOSStatus(AudioUnitGetProperty(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 1, &asbd, &size),
                        "AudioUnitGetProperty(kAudioUnitProperty_StreamFormat)");
        
        if ( iaaInput ) asbd.mChannelsPerFrame = 2;
        
        bool hasInputChanges = false;
        
        int channels = asbd.mChannelsPerFrame;//streamContext->maxInputChannels ? MIN(asbd.mChannelsPerFrame, streamContext->maxInputChannels) : asbd.mChannelsPerFrame;
        if ( streamContext->nInputChannels != (int)channels ) 
        {
            hasChanges = hasInputChanges = true;
            streamContext->nInputChannels = channels;
        }
        
        if ( !streamContext->outputEnabled ) {
            double newSampleRate = sampleRate == 0 ? asbd.mSampleRate : sampleRate;
            if ( fabs(streamContext->currentSampleRate - newSampleRate) > DBL_EPSILON )
            {
                hasChanges = hasInputChanges = true;
                streamContext->currentSampleRate = newSampleRate;
            }
        }
        
        if ( streamContext->nInputChannels > 0 && (hasInputChanges || streamContext->hasSetInitialStreamFormat) ) 
        {
            if ( running && !stoppedUnit )
            {
                PBACheckOSStatus(AudioOutputUnitStop(streamContext->audioUnit), "AudioOutputUnitStop");
                stoppedUnit = true;
            }
            
            // Set the stream format
            asbd = _audioFormat;
            asbd.mChannelsPerFrame = streamContext->nInputChannels;
            asbd.mSampleRate = streamContext->currentSampleRate;
            PBACheckOSStatus(AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &asbd, sizeof(asbd)), "AudioUnitSetProperty(kAudioUnitProperty_StreamFormat)");
        } 
        else
        {
            memset(&(streamContext->inputTimeStamp), 0, sizeof(streamContext->inputTimeStamp));
        }
    }
        
    streamContext->target = PBAStreamFormatGetType(&streamContext->format); //enumerate a sample packing protocol for the given format

    PBAStreamFormatPrint(&(streamContext->format));

    if ( hasChanges ) 
    {
        //assert(1==0);
        //[[NSNotificationCenter defaultCenter] postNotificationName:AEIOAudioUnitDidUpdateStreamFormatNotification object:self];
        
        if(hasSampleRateChanges) 
        {
            PBAIOAudioUnitSampleRateChanged(streamContext, streamContext->audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, 0);
            //if( streamContext->running ) PBACheckOSStatus(AudioOutputUnitStart(streamContext->audioUnit), "AudioOutputUnitStart");
        }
    }
    
    streamContext->hasSetInitialStreamFormat = true;
    
    if ( stoppedUnit ) PBACheckOSStatus(AudioOutputUnitStart(streamContext->audioUnit), "AudioOutputUnitStart");
#else
    assert(1==0);
#endif //__APPLE__

}

#ifdef __APPLE__

PB_AUDIO_API PB_AUDIO_INLINE double PBABufferDuration(PBAStreamContext* streamContext)
{
#if TARGET_OS_OSX
    // Get the default device
    AudioDeviceID deviceId; PBAudioDefaultDevice(streamContext->outputEnabled ? kAudioHardwarePropertyDefaultOutputDevice : kAudioHardwarePropertyDefaultInputDevice, &deviceId);
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
#else
    void* (*objc_msgSendSharedInstance)(Class, SEL) = (void*)objc_msgSend;
    double (*objc_msgSendGetProperty)(void*, SEL) = (void*)objc_msgSend;
    id avSessionSharedInstance = objc_msgSendSharedInstance(objc_getClass("AVAudioSession"), sel_registerName("sharedInstance"));
    return objc_msgSendGetProperty(avSessionSharedInstance, sel_getUid("IOBufferDuration"));
#endif
}


//DEBUG

PB_AUDIO_API PB_AUDIO_INLINE void PBAStreamReportRenderTime(PBAStreamContext * streamContext, PBAStreamLatencyReport * report, double renderTime, double bufferDuration) {
    double now = PBACurrentTimeInSeconds();
    if ( !report->firstReportTime ) report->firstReportTime = now;
    
    if ( now - report->firstReportTime > kRenderBudgetWarningInitialDelay && renderTime > bufferDuration * kRenderBudgetWarningThreshold )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            fprintf(stdout, "Warning: render took %lfs, %0.4lf%% of buffer duration.", renderTime, (renderTime / bufferDuration) * 100.0);
            assert(1==0);
        });
    }
    
    if ( kRenderTimeReportInterval > 0 )
    {
        report->averageRenderDurationAccumulator += renderTime;
        report->averageRenderDurationSampleCount++;
        
        report->maximumRenderDuration = MAX(report->maximumRenderDuration, renderTime);
        
        if ( now - report->lastReportTime > kRenderTimeReportInterval ) {
            double average = report->averageRenderDurationAccumulator / report->averageRenderDurationSampleCount;
            double maximum = report->maximumRenderDuration;
            
            dispatch_async(dispatch_get_main_queue(), ^{
                double bufferDuration = PBABufferDuration(streamContext);
                fprintf(stdout, "\nRender time report: %lfs/%0.4lf%% average,\t%lfs/%0.4lf%% maximum\n", average, (average/bufferDuration)*100.0, maximum, (maximum/bufferDuration)*100.0);
            });
            
            report->lastReportTime = now;
            report->averageRenderDurationAccumulator = 0;
            report->averageRenderDurationSampleCount = 0;
            report->maximumRenderDuration = 0;
        }
    }
}

#endif //__APPLE__



