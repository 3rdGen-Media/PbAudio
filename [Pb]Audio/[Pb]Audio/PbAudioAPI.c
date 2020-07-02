//
//  PbAudioAPI.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 7/1/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"


OSStatus PBAInitAudioStream(PBAStreamContext * streamContext)
{
#ifdef __APPLE__
        // Get an instance of the output audio unit
    
        //PBARenderContext renderContext = {0};
        /*
         //__unsafe_unretained AEAudioUnitOutput * weakSelf = self;
        streamContext->renderCallback = ^(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp) {
            //#ifdef DEBUG
            //AEManagedValueRealtimeThreadIdentifier = pthread_self();
            //#endif
            
            //Michael Tyson uses this to do some pthread locking... pthread locks are bullshit
            //Never ever lock a real-time thread!!!
            //AEManagedValueCommitPendingUpdates();
            
            //printf("PBARenderContext Render Callback\n");

            
            //If you aren't rendering, wtf do you have a render callback?
            //Eliminate redundant conditionals on the real-time thread!!
            //__unsafe_unretained AERenderer * renderer = (__bridge AERenderer*)AEManagedValueGetValue(rendererValue);
            //if ( renderer ) {
                #ifdef DEBUG
                    uint64_t start = PBACurrentTimeInHostTicks();
                #endif
                
                //AERendererRun(renderer, ioData, frames, timestamp);
                // Reset the buffer stack, and set the frame count/timestamp
                //AEBufferStackReset(THIS->_stack);
                //AEBufferStackSetFrameCount(THIS->_stack, frames);
                //AEBufferStackSetTimeStamp(THIS->_stack, timestamp);
                   
                // Clear the output buffer
                PBABufferListSilence(ioData, 0, frames);
                   
                // Run the block
                // Set our own sample time, to ensure continuity
                //AudioTimeStamp time = *timestamp;
                //time.mFlags |= kAudioTimeStampSampleTimeValid;
                //time.mSampleTime = THIS->_sampleTime;
                //THIS->_sampleTime += frames;
                
                //AERenderContext context = {
                //    .output = bufferList,
                //    .frames = frames,
                 //   .sampleRate = THIS->_sampleRate,
                  //  .timestamp = &time,
                  //  .offlineRendering = THIS->_isOffline,
                  //  .stack = THIS->_stack
                //};
                
                //block(&context);
                
            
                if( g_playbackSampleOffset < g_sineBufferLengthInSamples)
                {
                    for(int bufferIndex=0; bufferIndex< ioData->mNumberBuffers; bufferIndex++)
                    {
                        float * fBuffer = (float*)ioData->mBuffers[bufferIndex].mData;
                        memcpy(fBuffer, &(g_sineWaveBuffer[g_playbackSampleOffset+frames]), frames * sizeof(float));
                    }
                    g_playbackSampleOffset += frames;
                }
                //for(int i = 0; i<frames; i++)
                //    fBuffer[i] = 1.0f;
            
                
            
                //AERenderContextOutput
            
            
            
                #ifdef DEBUG
                    PBAStreamReportRenderTime(streamContext, &_audioReport, PBASecondsFromHostTicks(PBACurrentTimeInHostTicks() - start), (double)frames / streamContext->currentSampleRate);
                #endif
            //} else {
                //PBABufferListSilence(ioData, 0, frames);
            //}
        };
         */
        //! The audio unit. Will be NULL until setup: is called.
        streamContext->inputEnabled  = false;
        streamContext->outputEnabled = true;

        bool inputEnabled = streamContext->inputEnabled;
        bool outputEnabled = streamContext->outputEnabled;
    
        AudioComponentDescription acd = {};
#if TARGET_OS_IPHONE || TARGET_OS_TVOS
        acd = AEAudioComponentDescriptionMake(kAudioUnitManufacturer_Apple, kAudioUnitType_Output, kAudioUnitSubType_RemoteIO);
#else
        acd = AEAudioComponentDescriptionMake(kAudioUnitManufacturer_Apple, kAudioUnitType_Output, kAudioUnitSubType_HALOutput);
#endif
        
        AudioComponent inputComponent = AudioComponentFindNext(NULL, &acd);
        OSStatus result = AudioComponentInstanceNew(inputComponent, &(streamContext->audioUnit));
        if ( !PBACheckOSStatus(result, "AudioComponentInstanceNew") ) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to instantiate IO unit" }];
            printf("Unable to instantiate IO unit\n");
            return result;
        }
        
        // Set the maximum frames per slice to render
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &PBABufferStackMaxFramesPerSlice, sizeof(PBABufferStackMaxFramesPerSlice));
        PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioUnitProperty_MaximumFramesPerSlice)");
        
        // Enable/disable input
        UInt32 flag = inputEnabled ? 1 : 0;
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &flag, sizeof(flag));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_EnableIO)") )
        {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:userInfo:@{ NSLocalizedDescriptionKey: @"Unable to enable/disable input" }];
            printf("Unalbe to enable/disable input\n");
            return result;
        }
        
        // Enable/disable output
        flag = outputEnabled ? 1 : 0;
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &flag, sizeof(flag));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_EnableIO)") ) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to enable/disable output" }];
            printf("Unable to enable/disable output\n");
            return result;
        }
        
        // Set the render callback
        AURenderCallbackStruct rcbs = { .inputProc = PBAIOAudioUnitRenderCallback, .inputProcRefCon = (void*)streamContext };//(__bridge void *)(self) };
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, 0, &rcbs, sizeof(rcbs));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioUnitProperty_SetRenderCallback)") ) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to configure output render" }];
            printf("Unable to configure output render\n");
            return result;
        }

        // Set the input callback
        AURenderCallbackStruct inRenderProc;
        inRenderProc.inputProc = &PBAIOAudioUnitInputCallback;
        inRenderProc.inputProcRefCon = (void*)NULL;//(__bridge void *)self;
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &inRenderProc, sizeof(inRenderProc));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_SetInputCallback)") ) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to configure input process" }];
            printf("Unable to configure input process\n");
            return result;
        }
        
        // Initialize
        result = AudioUnitInitialize(streamContext->audioUnit);
        if ( !PBACheckOSStatus(result, "AudioUnitInitialize")) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to initialize IO unit" }];
            printf("Unable to initialize IO unit\n");
            return result;
        }
        
        //TO DO: Update stream formats
        PBAUpdateStreamFormat(streamContext);
        
        // Register a callback to watch for stream format changes
        PBACheckOSStatus(AudioUnitAddPropertyListener(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, PBAIOAudioUnitStreamFormatChanged, (void*)streamContext), "AudioUnitAddPropertyListener(kAudioUnitProperty_StreamFormat)");
        
#if TARGET_OS_IPHONE || TARGET_OS_TVOS
       // __weak typeof(self) weakSelf = self;
        
        // Watch for session interruptions
        __block BOOL wasRunning;
        
    
        CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
        assert(center);
        
        // add an observer
        //self.sessionInterruptionObserverToken =
        CFNotificationCenterAddObserver(center, NULL, AVAudioSessionInterruptionNotificationCallback,
                                        CFSTR("AVAudioSessionInterruptionNotification"), NULL,
                                        CFNotificationSuspensionBehaviorDeliverImmediately);

        // Watch for media reset notifications
        //self.mediaResetObserverToken =
        CFNotificationCenterAddObserver(center, NULL, AVAudioSessionMediaServicesWereResetNotificationCallback,
                                        CFSTR("AVAudioSessionMediaServicesWereResetNotification"), NULL,
                                        CFNotificationSuspensionBehaviorDeliverImmediately);
        
        // Watch for audio route changes
        //self.routeChangeObserverToken =
        CFNotificationCenterAddObserver(center, NULL, AVAudioSessionRouteChangeNotificationCallback,
                                        CFSTR("AVAudioSessionRouteChangeNotification"), NULL,
                                        CFNotificationSuspensionBehaviorDeliverImmediately);
    
 
        // Register callback to watch for Inter-App Audio connections
        //PBACheckOSStatus(AudioUnitAddPropertyListener(_audioUnit, kAudioUnitProperty_IsInterAppConnected, AEIOAudioUnitIAAConnectionChanged, (__bridge void*)self), "AudioUnitAddPropertyListener(kAudioUnitProperty_IsInterAppConnected)");
#endif
        
        //TO DO: Send out a CoreRender style notification
        // Send out a notification that the
        //[[NSNotificationCenter defaultCenter] postNotificationName:AEIOAudioUnitDidSetupNotification object:self];
        
        return result;
#endif
    
}


OSStatus PBAStartAudioStream(PBAStreamContext * streamContext)
{

#ifdef __APPLE__
    OSStatus result = -1;
    bool active;
#if (CR_TARGET_IOS) || defined(CR_TARGET_TVOS)
    // Activate audio session
    /*
    NSError * e;
    if ( ![[AVAudioSession sharedInstance] setActive:YES error:&e] )
    {
        NSLog(@"Couldn't activate audio session: %@", e);
        if ( error ) *error = e;
        return NO;
    }
    */
    //((id (*)(id, SEL, void*))objc_msgSend)(objc_msgSend(objc_getClass("CRMetalInterface"), sel_registerName("sharedInstance")), sel_getUid("displayLoop:"), view);
    void* (*objc_msgSendSharedInstance)(Class, SEL) = (void*)objc_msgSend;
    id avSessionSharedInstance = objc_msgSendSharedInstance(objc_getClass("AVAudioSession"), sel_registerName("sharedInstance"));
    
    //id nsError =
    BOOL (*objc_msgSendSetActive)(void*, SEL, BOOL, id) = (void*)objc_msgSend;
    active = objc_msgSendSetActive(avSessionSharedInstance, sel_getUid("setActive:error:"), YES, NULL);
    if ( !active )
    {
       printf("Couldn't activate audio session!");
       //if ( error ) *error = e;
       return result;
    }
    
    double (*objc_msgSendGetProperty)(void*, SEL) = (void*)objc_msgSend;

    streamContext->outputLatency = objc_msgSendGetProperty(avSessionSharedInstance, sel_getUid("outputLatency"));//[AVAudioSession sharedInstance].outputLatency;
    streamContext->inputLatency = objc_msgSendGetProperty(avSessionSharedInstance, sel_getUid("inputLatency"));//[AVAudioSession sharedInstance].outputLatency;
    streamContext->inputGain = 1.0;//_renderContext.inputGain;
#endif
    
    //TO DO:
    PBAUpdateStreamFormat(streamContext);
    
    // Start unit
    result = AudioOutputUnitStart(streamContext->audioUnit);
    
    if ( !PBACheckOSStatus(result, "AudioOutputUnitStart") )
    {
        //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to start IO unit" }];
        printf("Unable to start IO unit\n");
        return result;
    }
    
    //TO DO : set this somewhere
    streamContext->running = true;
    
    return result;
#endif
    
}
