//
//  PbAudioStream.c
//  [Pb]AudioOSX
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

IMMDeviceEnumerator *_PBADeviceEnumerator = NULL;
PBAStreamContext g_renderContext;// = {0};

#ifdef __APPLE__
void PBAUpdateStreamFormat(PBAStreamContext * streamContext)
{
    bool running = streamContext->running;
    bool stoppedUnit = false;
    bool hasChanges = false;
    bool iaaInput = false;
    bool iaaOutput = false;
    
    printf("PBAUpdateStreamFormat\n");
    
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
            iaaInput = componentDescription.componentType == kAudioUnitType_RemoteEffect
                || componentDescription.componentType == kAudioUnitType_RemoteMusicEffect;
        }
    }
#endif
    
    if ( streamContext->outputEnabled ) {
        // Get the current output sample rate and number of output channels
        AudioStreamBasicDescription asbd;
        UInt32 size = sizeof(asbd);
        PBACheckOSStatus(AudioUnitGetProperty(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &asbd, &size), "AudioUnitGetProperty(kAudioUnitProperty_StreamFormat)");
        if ( iaaOutput ) {
            asbd.mChannelsPerFrame = 2;
        }
        
        bool hasOutputChanges = false;
        
        double newSampleRate = streamContext->sampleRate == 0 ? asbd.mSampleRate : streamContext->sampleRate;
        if ( fabs(streamContext->currentSampleRate - newSampleRate) > DBL_EPSILON ) {
            hasChanges = hasOutputChanges = true;
            streamContext->currentSampleRate = newSampleRate;
        }
        
        if ( streamContext->nOutputChannels != (int)asbd.mChannelsPerFrame ) {
            hasChanges = hasOutputChanges = true;
            streamContext->nOutputChannels = asbd.mChannelsPerFrame;
        }
        
        if ( hasOutputChanges || !streamContext->hasSetInitialStreamFormat ) {
            if ( running ) {
                PBACheckOSStatus(AudioOutputUnitStop(streamContext->audioUnit), "AudioOutputUnitStop");
                stoppedUnit = true;
            }

            // Update the stream format
            asbd = _audioFormat;
            asbd.mChannelsPerFrame = streamContext->nOutputChannels;
            asbd.mSampleRate = streamContext->currentSampleRate;
            PBACheckOSStatus(AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &asbd, sizeof(asbd)), "AudioUnitSetProperty(kAudioUnitProperty_StreamFormat)");
        }
    }
    
    if ( streamContext->inputEnabled ) {
        // Get the current input number of input channels
        AudioStreamBasicDescription asbd;
        UInt32 size = sizeof(asbd);
        PBACheckOSStatus(AudioUnitGetProperty(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,
                                             1, &asbd, &size),
                        "AudioUnitGetProperty(kAudioUnitProperty_StreamFormat)");
        
        if ( iaaInput ) {
            asbd.mChannelsPerFrame = 2;
        }
        
        bool hasInputChanges = false;
        
        int channels = streamContext->maxInputChannels ? MIN(asbd.mChannelsPerFrame, streamContext->maxInputChannels) : asbd.mChannelsPerFrame;
        if ( streamContext->nInputChannels != (int)channels ) {
            hasChanges = hasInputChanges = true;
            streamContext->nInputChannels = channels;
        }
        
        if ( !streamContext->outputEnabled ) {
            double newSampleRate = streamContext->sampleRate == 0 ? asbd.mSampleRate : streamContext->sampleRate;
            if ( fabs(streamContext->currentSampleRate - newSampleRate) > DBL_EPSILON ) {
                hasChanges = hasInputChanges = true;
                streamContext->currentSampleRate = newSampleRate;
            }
        }
        
        if ( streamContext->nInputChannels > 0 && (hasInputChanges || streamContext->hasSetInitialStreamFormat) ) {
            if ( running && !stoppedUnit ) {
                PBACheckOSStatus(AudioOutputUnitStop(streamContext->audioUnit), "AudioOutputUnitStop");
                stoppedUnit = true;
            }
            
            // Set the stream format
            asbd = _audioFormat;
            asbd.mChannelsPerFrame = streamContext->nInputChannels;
            asbd.mSampleRate = streamContext->currentSampleRate;
            PBACheckOSStatus(AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &asbd, sizeof(asbd)), "AudioUnitSetProperty(kAudioUnitProperty_StreamFormat)");
        } else {
            memset(&(streamContext->inputTimeStamp), 0, sizeof(streamContext->inputTimeStamp));
        }
    }
    
    
    printf("PBAUpdateStreamFormat currentSampleRate = %g\n", streamContext->currentSampleRate);
    //if ( hasChanges ) {
    //    [[NSNotificationCenter defaultCenter] postNotificationName:AEIOAudioUnitDidUpdateStreamFormatNotification object:self];
    //}
    
    streamContext->hasSetInitialStreamFormat = true;
    
    if ( stoppedUnit ) {
        PBACheckOSStatus(AudioOutputUnitStart(streamContext->audioUnit), "AudioOutputUnitStart");
    }
}




//DEBUG

PB_AUDIO_API PB_AUDIO_INLINE void PBAStreamReportRenderTime(PBAStreamContext * streamContext, PBAStreamLatencyReport * report, double renderTime, double bufferDuration) {
    double now = PBACurrentTimeInSeconds();
    if ( !report->firstReportTime ) report->firstReportTime = now;
    
    if ( now - report->firstReportTime > kRenderBudgetWarningInitialDelay && renderTime > bufferDuration * kRenderBudgetWarningThreshold )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            printf("Warning: render took %lfs, %0.4lf%% of buffer duration.", renderTime, (renderTime / bufferDuration) * 100.0);
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
                printf("Render time report: %lfs/%0.4lf%% average,\t%lfs/%0.4lf%% maximum\n", average, (average/bufferDuration)*100.0, maximum, (maximum/bufferDuration)*100.0);
            });
            
            report->lastReportTime = now;
            report->averageRenderDurationAccumulator = 0;
            report->averageRenderDurationSampleCount = 0;
            report->maximumRenderDuration = 0;
        }
    }
}

#endif __APPLE__



