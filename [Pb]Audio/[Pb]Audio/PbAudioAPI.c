//
//  PbAudioAPI.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 7/1/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

static int PBAInitAudioStreamWithFormat(PBAStreamContext *clientStream, WAVEFORMATEX * streamFormat, AUDCLNT_SHAREMODE shareMode)
{
	HRESULT hr;
	double devicePeriodInSeconds;
	UINT32 FramesPerPeriod;

	UINT32 bufferFrameCount;

	 REFERENCE_TIME defaultDevicePeriod = 0;
	 REFERENCE_TIME minDevicePeriod = 0;

	REFERENCE_TIME hnsRequestedDuration = 0;
	//REFERENCE_TIME hnsActualDuration = 0;



	//if exclusive mode, we can explicitly request the format we want the audioi client ot use for processing
	//hr = GetAudioClientStreamFormat(pAudioClient, shareMode, fileFormatEx, &pwfx);
	//	if (FAILED(hr)) { printf("**** Error 0x%x returned by GetAudioClientStreamFormat\n", hr); return -1; }




	//For shared mode you should pass in zero for the periodicity value (you can then retrieve the periodicity value used by the audio engine from IAudioClient::GetDevicePeriod). 
    // For exclusive mode, Initialize the stream to play at the minimum latency (by requesting the device period it is already using?).
	//So for both modes we will just get the device period before calling Initialize
	//hr = clientStream->audioClient->GetDevicePeriod(&defaultDevicePeriod, &minDevicePeriod);
    hr = CALL(GetDevicePeriod, clientStream->audioClient, &defaultDevicePeriod, &minDevicePeriod);
	if (FAILED(hr)) { printf("**** Error 0x%x returned by GetDevicePeriod\n", hr); return -1; }

	printf("Device Default Period size = %u\n", defaultDevicePeriod);
	printf("Device Minimum Period size = %u\n", minDevicePeriod);
	
	//double desiredDuration = (128. / (double)pwfx->nSamplesPerSec);
	//hnsRequestedDuration = (REFERENCE_TIME)(desiredDuration * (double)REFTIMES_PER_SEC);

	//convert period to frames
	devicePeriodInSeconds = defaultDevicePeriod / (10000.0*1000.0);
    //printf("Device Period (s) = %g s\n", devicePeriodInSeconds );
	
	FramesPerPeriod  = (UINT32)( streamFormat->nSamplesPerSec * devicePeriodInSeconds + 0.5 );
	printf("Frames per Period = %u\n", FramesPerPeriod);
	
	//IMPORTANT!
	//For exclusive mode Streams we ask Initialize for a desired duration equal to the minimum period of the device for minimum latency	
	if( shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE)
		hnsRequestedDuration = minDevicePeriod;//defaultDevicePeriod * 100;
	else //For shared mode Streams we ask Initialize for a desired duration and a mininum period value of 0
		hnsRequestedDuration =   0; //there are 10 million 100 ns increments in a 1 second

	printf("Requested Duration = %u\n", hnsRequestedDuration);

	//Initialize the audio client for streaming playback in the desired shared mode
	//For simplicity, the buffer size and periodicity arguments to Initializeshould be the same. 
	//This means that each audio engine pass processes one buffer length (which is required for event driven mode anyway).
	//Passing null as the last parameter will create a default audio session and add this stream to it
    //hr = clientStream->audioClient->Initialize( shareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, hnsRequestedDuration, hnsRequestedDuration, streamFormat, NULL);
    hr = CALL(Initialize, clientStream->audioClient, shareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, hnsRequestedDuration, hnsRequestedDuration, streamFormat, NULL);
	while (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) 
	{ 
		//UINT32 nFrames = 0;
		REFERENCE_TIME alignedBufferSize;
		//hr = clientStream->audioClient->GetBufferSize(&bufferFrameCount);
		hr = CALL(GetBufferSize, clientStream->audioClient, &bufferFrameCount);
		if (hr != S_OK) { printf("**** Error 0x%x returned by GetBufferSize\n", hr); return -1; }
		//bufferFrameCount = bufferFrameCount / pwfx->nChannels;
		printf("reinitialize bufferFrameCount = %u\n", bufferFrameCount);

		// Calculate period that would equal to the duration of proposed buffer size 
		alignedBufferSize = (REFERENCE_TIME) (1e7 * (double)bufferFrameCount / (double)(streamFormat->nSamplesPerSec) + 0.5);
		hnsRequestedDuration = alignedBufferSize;
		//hr = clientStream->audioClient->Initialize(shareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, alignedBufferSize, alignedBufferSize, streamFormat, NULL); 
		hr = CALL(Initialize, clientStream->audioClient, shareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, alignedBufferSize, alignedBufferSize, streamFormat, NULL);
	
	} 
	
	if (hr != S_OK) { printf("**** Error 0x%x returned by Initialize\n", hr); return -1; }
	assert (hr != AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED);


	// Tell the audio source which format to use.
    //hr = pMySource->SetFormat(pwfx);
    //if (FAILED(hr)) { printf("**** Error 0x%x returned by Initialize\n", hr); return -1; }

	 // Get the actual size of the two allocated buffers.
	//In shared mode, we are slaved to the buffer size that is being used for the mix so we must use this value
	//In Exclusive Mode...?
    //hr = clientStream->audioClient->GetBufferSize(&bufferFrameCount);
    hr = CALL(GetBufferSize, clientStream->audioClient, &(clientStream->bufferFrameCount));
	clientStream->bufferFrameCount = clientStream->bufferFrameCount/2;
	if (hr != S_OK) { printf("**** Error 0x%x returned by GetBufferSize\n", hr); return -1; }
	
	//create the render client for the stream
	//hr = clientStream->audioClient->GetService(IID_IAudioRenderClient,(void**)&(clientStream->renderClient));
    hr = CALL(GetService, clientStream->audioClient, __riid(IAudioRenderClient), (void**)&(clientStream->renderClient));
	if (FAILED(hr)) { printf("**** Error 0x%x returned by GetService\n", hr); return -1; }

	clientStream->currentSampleRate = clientStream->sampleRate = streamFormat->nSamplesPerSec;
	clientStream->format = *streamFormat;
	clientStream->devicePeriod = hnsRequestedDuration;

	return (int)clientStream->bufferFrameCount;
}


static void print_waveformat_details(WAVEFORMATEX * waveformatEX)
{
	printf("wFormatTag = %hu\n", waveformatEX->wFormatTag);
	printf("nChannels = %hu\n", waveformatEX->nChannels);
	printf("nSamplesPerSec = %d\n", waveformatEX->nSamplesPerSec);
	printf("nAvgBytesPerSec = %d\n", waveformatEX->nAvgBytesPerSec);
	printf("nBlockAlign = %hu\n", waveformatEX->nBlockAlign);
	printf("wBitsPerSample = %hu\n", waveformatEX->wBitsPerSample);
	printf("cbSize = %hu\n", waveformatEX->cbSize);

}

OSStatus PBAInitAudioStream(PBAStreamContext * streamContext, PBAStreamFormat * format)
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
#elif defined(_WIN32)

	HRESULT hr;
	WAVEFORMATEX * pwfx = format;
	//(because 32-bit buffers are not supported by the hw device itself)
    //TO DO:  enumerate the modes to choose from

	//If we passed a format as input, we wish to use an exclusive mode
	if( pwfx ) //streamContext->shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE )
	{
		streamContext->shareMode = AUDCLNT_SHAREMODE_EXCLUSIVE;
	}
	else
	{
		//Automatically get the format currently being used by the AudioClient for buffer processing if using shared mode
		//The mix format is usually 32-bit floating point format, which may not be supported in an exclusive stream mode 
	
		//if shared mode, get the format that is already being used by the audio client
		//hr = _PBAMasterStream.audioClient->GetMixFormat( &pwfx );
		hr = CALL(GetMixFormat, streamContext->audioClient, &pwfx);
		if (FAILED(hr)) { printf("**** Error 0x%x returned by GetMixFormat\n", hr); return -1; }
		print_waveformat_details(pwfx);
		streamContext->shareMode = AUDCLNT_SHAREMODE_SHARED;

	}
		
	streamContext->bufferFrameCount = (UINT32)PBAInitAudioStreamWithFormat(streamContext, pwfx, streamContext->shareMode);
	
	//Observe buffer size in samples
	//TO DO:  check return value for init error
	//bufferFrameCount = bufferFrameCount / 2;//pwfx->nChannels;
	printf("bufferSizeInSamples = %u\n", streamContext->bufferFrameCount );

    // Create a platform event handle and register it for
    // buffer-event notifications.
    streamContext->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (streamContext->hEvent == NULL) { printf("**** Create Event Failed\n", hr); return -1; }

	// Associate the platform event handle with the stream callback to populate the buffer
    //hr = _PBAMasterStream.audioClient->SetEventHandle(_PBAMasterStream.hEvent);
    hr = CALL(SetEventHandle, streamContext->audioClient, streamContext->hEvent);
	if (FAILED(hr)) { printf("**** Error 0x%x returned by SetEventHandle\n", hr); return -1; }


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
#elif defined(_WIN32)

	HRESULT hr;
	DWORD flags = 0;

	//Task is used to elevate thread to ProAudio Latency
	HANDLE hTask = NULL;
	DWORD taskIndex = 0;	
	//PBAStreamContext * clientStream = (PBAStreamContext*)stream;
	
	char * buffer = NULL;
	
	//for render callback
	PBABuffer interleavedBuffer = { streamContext->format.nChannels, streamContext->format.wBitsPerSample/8, NULL};
	PBABufferList bufferList = {1, &interleavedBuffer};

	HANDLE threadID;
	threadID = GetCurrentThread();
	SetThreadPriority(threadID, THREAD_PRIORITY_TIME_CRITICAL);

	 // Ask MMCSS to temporarily boost the thread priority
    // to reduce glitches while the low-latency stream plays.
    taskIndex = 0;
    hTask = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
    if (hTask == NULL)
    {
        hr = E_FAIL;
		printf("**** AvSetMmThreadCharacteristics (Pro Audio) failed!\n"); return -1;    
	}

   
    //hr = clientStream->audioClient->Start();  // Start playing (ie start the device io callback).
	hr = CALL(Start, streamContext->audioClient);
    if (FAILED(hr)) { printf("**** Error 0x%x returned by Start (pAudioClient)\n", hr); return -1; }

	//RENDER LOOP
	hr = S_OK;
	
	//Start the render loop that waits for the event to trigger
	// Each loop fills one of the two buffers.
	//TO DO:  Can we eliminate the loop altogether and just have the callback trigger when the event does?
    while ( flags != AUDCLNT_BUFFERFLAGS_SILENT)
    {
	    // Wait for next buffer event to be signaled.
        DWORD retval = WaitForSingleObject(streamContext->hEvent, 2000);
        if (retval != WAIT_OBJECT_0)
        {
            // Event handle timed out after a 2-second wait.
            //hr = clientStream->audioClient->Stop();
            hr = CALL(Stop, streamContext->audioClient);
			hr = ERROR_TIMEOUT;
			break;
		}

		//printf("Requesting %d buffer samples...\n", clientStream->bufferFrameCount);

		// Grab the next empty buffer from the audio device.
	    //hr = clientStream->renderClient->GetBuffer(clientStream->bufferFrameCount, &pData);
		hr = CALL(GetBuffer, streamContext->renderClient, streamContext->bufferFrameCount, &((BYTE*)(interleavedBuffer.mData)) );
	
		//BUFFER_TOO_LARGE actually means we are asking the audio client for more buffer space than is currently available!
		//Microsoft loves to leave this cryptic little holes in their sample code demonstrating their APIs
		if( hr == AUDCLNT_E_BUFFER_TOO_LARGE ) { continue; } ;
        if (FAILED(hr)) { printf("**** Error 0x%x returned by GetBuffer\n", hr); break; }

		//Calculate the frames available (only if we choose to render to a portion of the buffer at a time in shared mode)
		//FramesAvailable = 0;
		//PaddingFrames = 0;
		// Get padding in existing buffer
		//hr = pAudioClient->GetCurrentPadding( &PaddingFrames );
        //if (FAILED(hr)) { printf("**** Error 0x%x returned by GetCurrentPadding\n", hr); break; }
		//printf("Current padding = %d\n", PaddingFrames);

		// In HW mode, GetCurrentPadding returns the number of available frames in the 
		// buffer, so we can just use that directly
		//if (m_DeviceProps.IsHWOffload) FramesAvailable = PaddingFrames;
		// In non-HW shared mode, GetCurrentPadding represents the number of queued frames
		// so we can subtract that from the overall number of frames we have
		//else
		//	FramesAvailable = bufferFrameCount - PaddingFrames;

		//Package the buffer as a PBABufferList
		//interleavedBuffer.mData = clientStream->pData;
		//PBABufferList bufferList;

		//Execute the client render callback
		streamContext->renderCallback(&bufferList, streamContext->bufferFrameCount, NULL);

		//Release the buffer to send it down the audio pipeline for device playback
		//hr = clientStream->renderClient->ReleaseBuffer(clientStream->bufferFrameCount, flags);
	    hr = CALL(ReleaseBuffer, streamContext->renderClient, streamContext->bufferFrameCount, flags);
		if (FAILED(hr)) { printf("**** Error 0x%x returned by ReleaseBuffer\n", hr); break; }
		//pData = NULL;
	}

	
	/*
    // Wait for the last buffer to play before stopping.
	Sleep(2000);
	//if( hr = S_OK) 
	//	Sleep(1000);

    //hr = clientStream->audioClient->Stop();  // Stop playing.
	hr = CALL(Stop, clientStream->audioClient);
	if (FAILED(hr)) { printf("**** Error 0x%x returned by Stop (pAudioClient)\n", hr); }

	//Cleanup Motherfucker!

		if (clientStream->hEvent != NULL)
		{
			CloseHandle(clientStream->hEvent);
		}
		if (hTask != NULL)
		{
			AvRevertMmThreadCharacteristics(hTask);
		}
		//CoTaskMemFree(pwfx);
		PBA_COM_RELEASE(_PBADeviceEnumerator)
		PBA_COM_RELEASE(clientStream->audioDevice)
		PBA_COM_RELEASE(clientStream->audioClient)
		PBA_COM_RELEASE(clientStream->renderClient)
	*/

#endif
    
}
