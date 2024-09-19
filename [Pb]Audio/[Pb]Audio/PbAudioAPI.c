//
//  PbAudioAPI.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 7/1/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

//Note:  DYNAMIC libraries can't resolve links to global exported vars, ony functions
//PB_AUDIO_API PBAudioStreamFactory PBAudio = {PBAudioStreamInit, PBAudioStreamStart, PBAudioStreamStop, PBAudioStreamSetOutputDevice};

PBAudioStreamFactory* GetPBAudioStreamFactory(void)
{
#ifdef _WIN32 

#elif  defined(__GNUC__) && (__GNUC__ > 3)
    // You are OK
#else
#error Add Critical Section for your platform
#endif
    static PBAudioStreamFactory factory = { PBAudioStreamInit, PBAudioStreamStart, PBAudioStreamStop, PBAudioStreamSetOutputDevice };
    //if (instance == NULL)
    //	instance = new CXURLInterface();

#ifdef _WIN32
                //END Critical Section Here
#endif 

    return &factory;
}

#ifdef _WIN32
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
	if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by GetDevicePeriod\n", hr); return -1; }

	fprintf(stdout, "Device Default Period size = %u\n", defaultDevicePeriod);
	fprintf(stdout, "Device Minimum Period size = %u\n", minDevicePeriod);
	
	//double desiredDuration = (128. / (double)pwfx->nSamplesPerSec);
	//hnsRequestedDuration = (REFERENCE_TIME)(desiredDuration * (double)REFTIMES_PER_SEC);

	//convert period to frames
	devicePeriodInSeconds = defaultDevicePeriod / (10000.0*1000.0);
    //printf("Device Period (s) = %g s\n", devicePeriodInSeconds );
	
	FramesPerPeriod  = (UINT32)( streamFormat->nSamplesPerSec * devicePeriodInSeconds + 0.5 );
	fprintf(stdout, "Frames per Period = %u\n", FramesPerPeriod);
	
	//IMPORTANT!
	//For exclusive mode Streams we ask Initialize for a desired duration equal to the minimum period of the device for minimum latency	
	if( shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE)
		hnsRequestedDuration = minDevicePeriod;//defaultDevicePeriod * 100;
	else //For shared mode Streams we ask Initialize for a desired duration and a mininum period value of 0
		hnsRequestedDuration =   0; //there are 10 million 100 ns increments in a 1 second

	fprintf(stdout, "Requested Duration = %u\n", hnsRequestedDuration);

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
		if (hr != S_OK) { fprintf(stderr, "**** Error 0x%x returned by GetBufferSize\n", hr); return -1; }
		//bufferFrameCount = bufferFrameCount / pwfx->nChannels;
        fprintf(stderr, "reinitialize bufferFrameCount = %u\n", bufferFrameCount);

		// Calculate period that would equal to the duration of proposed buffer size 
		alignedBufferSize = (REFERENCE_TIME) (1e7 * (double)bufferFrameCount / (double)(streamFormat->nSamplesPerSec) + 0.5);
		hnsRequestedDuration = alignedBufferSize;
		//hr = clientStream->audioClient->Initialize(shareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, alignedBufferSize, alignedBufferSize, streamFormat, NULL); 
		hr = CALL(Initialize, clientStream->audioClient, shareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, alignedBufferSize, alignedBufferSize, streamFormat, NULL);
	
	} 
	
	if (hr != S_OK) { fprintf(stderr, "**** Error 0x%x returned by Initialize\n", hr); return -1; }
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
	if (hr != S_OK) { fprintf(stderr, "**** Error 0x%x returned by GetBufferSize\n", hr); return -1; }
	
	//create the render client for the stream
	//hr = clientStream->audioClient->GetService(IID_IAudioRenderClient,(void**)&(clientStream->renderClient));
    hr = CALL(GetService, clientStream->audioClient, __riid(IAudioRenderClient), (void**)&(clientStream->renderClient));
	if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by GetService\n", hr); return -1; }

	clientStream->currentSampleRate = clientStream->sampleRate = streamFormat->nSamplesPerSec;
	clientStream->format = *streamFormat;
	clientStream->devicePeriod = hnsRequestedDuration;

	return (int)clientStream->bufferFrameCount;
}

static void print_waveformat_details(WAVEFORMATEX * waveformatEX)
{
	fprintf(stdout, "wFormatTag = %hu\n", waveformatEX->wFormatTag);
	fprintf(stdout, "nChannels = %hu\n", waveformatEX->nChannels);
	fprintf(stdout, "nSamplesPerSec = %d\n", waveformatEX->nSamplesPerSec);
	fprintf(stdout, "nAvgBytesPerSec = %d\n", waveformatEX->nAvgBytesPerSec);
	fprintf(stdout, "nBlockAlign = %hu\n", waveformatEX->nBlockAlign);
	fprintf(stdout, "wBitsPerSample = %hu\n", waveformatEX->wBitsPerSample);
	fprintf(stdout, "cbSize = %hu\n", waveformatEX->cbSize);
}
#endif

OSStatus PBAudioStreamInit(PBAStreamContext * streamContext, PBAStreamFormat * format, PBAudioDevice deviceID, PBAStreamOutputPass outputpass)
{
    streamContext->audioDevice    = deviceID;   //Pass 0 to select the default audio device during stream initialization
    streamContext->outputpass     = outputpass; //set the master render callback for the device stream
    streamContext->respectDefault = false;      //Enable for apps that want to use the system selected default audio device at all times
    
#ifdef __APPLE__
    
        //! The audio unit will be NULL until AudioUnitInitialize is called.
        streamContext->inputEnabled  = false;
        streamContext->outputEnabled = true;
        //streamContext->isDefault     = true; //assume we are using the default device to start the audio session stream

        bool inputEnabled  = streamContext->inputEnabled;
        bool outputEnabled = streamContext->outputEnabled;
    
        AudioComponentDescription acd = {};
#if TARGET_OS_IPHONE || TARGET_OS_TVOS
        acd = AEAudioComponentDescriptionMake(kAudioUnitManufacturer_Apple, kAudioUnitType_Output, kAudioUnitSubType_RemoteIO);
#else
        acd = PBAudioComponentDescriptionMake(kAudioUnitManufacturer_Apple, kAudioUnitType_Output, kAudioUnitSubType_HALOutput);
#endif
        
        AudioComponent inputComponent = AudioComponentFindNext(NULL, &acd);
        OSStatus result = AudioComponentInstanceNew(inputComponent, &(streamContext->audioUnit));
        if ( !PBACheckOSStatus(result, "AudioComponentInstanceNew") ) 
        {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to instantiate IO unit" }];
            fprintf(stderr, "Unable to instantiate IO unit\n");
            return result;
        }
        
        // Set the maximum frames per slice to render
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &PBABufferStackMaxFramesPerSlice, sizeof(PBABufferStackMaxFramesPerSlice));
        if( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioUnitProperty_MaximumFramesPerSlice)") ) assert(1==0);
        
    
        // Enable/disable input
        UInt32 flag = inputEnabled ? 1 : 0;
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &flag, sizeof(flag));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_EnableIO)") )
        {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:userInfo:@{ NSLocalizedDescriptionKey: @"Unable to enable/disable input" }];
            fprintf(stderr, "Unable to enable/disable input\n");
            return result;
        }
        
        // Enable/disable output
        flag = outputEnabled ? 1 : 0;
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &flag, sizeof(flag));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_EnableIO)") ) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to enable/disable output" }];
            fprintf(stderr, "Unable to enable/disable output\n");
            return result;
        }
        
        // Set the render callback
        AURenderCallbackStruct rcbs = { .inputProc = PBAudioStreamSubmitBuffers, .inputProcRefCon = (void*)streamContext };//(__bridge void *)(self) };
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, 0, &rcbs, sizeof(rcbs));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioUnitProperty_SetRenderCallback)") ) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to configure output render" }];
            fprintf(stderr, "Unable to configure output render\n");
            return result;
        }

        // Set the input callback
        AURenderCallbackStruct inRenderProc;
        inRenderProc.inputProc = &PBAIOAudioUnitInputCallback;
        inRenderProc.inputProcRefCon = (void*)NULL;//(__bridge void *)self;
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &inRenderProc, sizeof(inRenderProc));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_SetInputCallback)") ) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to configure input process" }];
            fprintf(stderr, "Unable to configure input process\n");
            return result;
        }
        
        // Initialize
        result = AudioUnitInitialize(streamContext->audioUnit);
        if ( !PBACheckOSStatus(result, "AudioUnitInitialize")) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to initialize IO unit" }];
            fprintf(stderr, "Unable to initialize IO unit\n");
            return result;
        }
        
        //Update stream format
        PBAudioStreamUpdateFormat(streamContext, streamContext->sampleRate);
    
        if( streamContext->audioDevice == kAudioObjectUnknown )
        {
            //TO DO:  Handle Input Devices Also
            PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &streamContext->audioDevice );
            //streamContext->audioDevice = PBAudioStreamOutputDevice(&streamContext);//PBAudio.GetOuputDevice( );
        }
        else assert(1==0); //TO DO:
    
        // Register a callback to watch for stream format changes
        // TO DO:  Pass notification through to client like device notifications
        //PBACheckOSStatus(AudioUnitAddPropertyListener(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, PBAIOAudioUnitStreamFormatChanged, (void*)streamContext), "AudioUnitAddPropertyListener(kAudioUnitProperty_StreamFormat)");
        //PBACheckOSStatus(AudioUnitAddPropertyListener(streamContext->audioUnit, kAudioUnitProperty_SampleRate,   PBAIOAudioUnitSampleRateChanged,   (void*)streamContext), "AudioUnitAddPropertyListener(kAudioUnitProperty_SampleRate)");

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

#elif TARGET_OS_OSX
    
    //Register PBAudio Private CoreAudio [Device] Object Observers
    //Notifications will be redistributed to clients via PbAudio so that
    //'Client Application' Processes can remain in sync with Master PbAudio 'Mix Engine' State
    //TO DO: Send out a CoreRender style notification for IPC
    PBAudioRegisterDeviceListeners(streamContext);
    
#endif
        
    return result;

#elif defined(_WIN32)

	HRESULT hr;
	WAVEFORMATEX * pwfx = format;
	//(because 32-bit buffers are not supported by the hw device itself)
    //TO DO:  enumerate the modes to choose from

    //Initalize COM and allocate a Device Enumerator object
    //So [Pb]Audio can create a stream against a hardware device
    PBAudioDeviceInitCOM();

    if (streamContext->audioDevice == kAudioObjectUnknown)
    {
        //Get the Default or Desired Audio Hardware Device Endpoint + Active a context to the device if needed
        PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &(streamContext->audioDevice));
        PBAudioActivateDevice(streamContext->audioDevice, &(streamContext->audioClient));
    }
    else assert(1 == 0);

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
		if (FAILED(hr)) { fprintf(stdout, "**** Error 0x%x returned by GetMixFormat\n", hr); return -1; }
		print_waveformat_details(pwfx);
		streamContext->shareMode = AUDCLNT_SHAREMODE_SHARED;

	}
		
	streamContext->bufferFrameCount = (UINT32)PBAInitAudioStreamWithFormat(streamContext, pwfx, streamContext->shareMode);
	
	//Observe buffer size in samples
	//TO DO:  check return value for init error
	//bufferFrameCount = bufferFrameCount / 2;//pwfx->nChannels;
	fprintf(stdout, "bufferSizeInSamples = %u\n", streamContext->bufferFrameCount );

    // Create a platform event handle and register it for
    // buffer-event notifications.
    streamContext->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (streamContext->hEvent == NULL) { printf("**** Create Event Failed\n", hr); return -1; }

	// Associate the platform event handle with the stream callback to populate the buffer
    //hr = _PBAMasterStream.audioClient->SetEventHandle(_PBAMasterStream.hEvent);
    hr = CALL(SetEventHandle, streamContext->audioClient, streamContext->hEvent);
	if (FAILED(hr)) { printf("**** Error 0x%x returned by SetEventHandle\n", hr); return -1; }

    return hr;
#endif
    
}

void PBAudioStreamSetBypass(PBAStreamContext * streamContext, bool bypass)
{
    streamContext->bypass = bypass;
}

OSStatus PBAudioStreamStart(PBAStreamContext * streamContext)
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
    
    //TO DO: Check to see if the audio format changed after starting the audio session
    //PBAudioStreamUpdateFormat(streamContext, streamContext->currentSampleRate);
    
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
        if (FAILED(hr)) { fprintf(stderr, "\n**** Error 0x%x returned by GetBuffer\n", hr); break; }

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
		streamContext->outputpass(&bufferList, streamContext->bufferFrameCount, NULL);

		//Release the buffer to send it down the audio pipeline for device playback
		//hr = clientStream->renderClient->ReleaseBuffer(clientStream->bufferFrameCount, flags);
	    hr = CALL(ReleaseBuffer, streamContext->renderClient, streamContext->bufferFrameCount, flags);
		if (FAILED(hr)) { fprintf(stderr, "\n**** Error 0x%x returned by ReleaseBuffer\n", hr); break; }
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

OSStatus PBAudioStreamStop(PBAStreamContext * streamContext)
{
    
    

#ifdef __APPLE__
    OSStatus result = -1;
    bool active;

#if (CR_TARGET_IOS) || defined(CR_TARGET_TVOS)

#endif
    
    assert(streamContext->audioUnit);// @"You must call setup: on this instance before starting or stopping it");
    assert(streamContext->running);
        
    // Stop unit
    result = AudioOutputUnitStop(streamContext->audioUnit);
    
    if ( !PBACheckOSStatus(result, "AudioOutputUnitStop") )
    {
        //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to start IO unit" }];
        printf("Unable to stop IO unit\n");
        return result;
    }
    
    streamContext->running = false;
    
    return result;
    
#elif defined(_WIN32)

   //TO DO: ...

#endif
    
}

OSStatus PBAudioStreamSetOutputDevice(PBAStreamContext * streamContext, PBAudioDevice deviceID)
{
    OSStatus status = 0;
    
    fprintf(stdout, "PBAudioStreamSetOutputDevice (AudioDeviceID: %u)\n", deviceID);

#ifdef __APPLE__
    if( streamContext->audioUnit )
    {
        volatile bool wasRunning = streamContext->running;
        if(wasRunning) PBAudioStreamStop(streamContext);

        OSStatus result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &deviceID, sizeof(deviceID));
        
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_CurrentDevice)\n") )
        {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to start IO unit" }];
            fprintf(stderr, ", Unable to set audio unit output device\n");
            return result;
        }

        //Changing the device will result in an update to the audio unit stream format
        //self.hasSetInitialStreamFormat = NO;
        PBAudioStreamUpdateFormat(streamContext, 0);
        //streamContext->isDefault = false;
        
        if ( wasRunning ) PBAudioStreamStart(streamContext);
    }
#else
    assert(1 == 0);
#endif
    
    streamContext->audioDevice = deviceID;

    return status;
}


