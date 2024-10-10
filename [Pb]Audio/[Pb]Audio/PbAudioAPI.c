//
//  PbAudioAPI.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 7/1/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

//Note:  DYNAMIC libraries can't resolve links to global exported vars, only functions
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

static void print_waveformat_details(PBAStreamFormat* format)
{
	fprintf(stdout, "wFormatTag = %hu\n",      format->wFormatTag);
	fprintf(stdout, "nChannels = %hu\n",       format->nChannels);
	fprintf(stdout, "nSamplesPerSec = %d\n",   format->nSamplesPerSec);
	fprintf(stdout, "nAvgBytesPerSec = %d\n",  format->nAvgBytesPerSec);
	fprintf(stdout, "nBlockAlign = %hu\n",     format->nBlockAlign);
	fprintf(stdout, "wBitsPerSample = %hu\n",  format->wBitsPerSample);
	fprintf(stdout, "cbSize = %hu\n",          format->cbSize);
}

static int PBAInitAudioStreamWithFormat(PBAStreamContext *clientStream, PBAStreamFormat * inStreamFormat, AUDCLNT_SHAREMODE shareMode)
{
	HRESULT hr;
	double devicePeriodInSeconds;
	UINT32 FramesPerPeriod;

	UINT32 bufferFrameCount;

	REFERENCE_TIME defaultDevicePeriod = 0;
	REFERENCE_TIME minDevicePeriod = 0;

    UINT32 current_period = 0;
    UINT32 DP, FP, MINP, MAXP;

	REFERENCE_TIME hnsRequestedDuration = 0;
	//REFERENCE_TIME hnsActualDuration = 0;

    PBAStreamFormat* streamFormat = inStreamFormat;

    //For shared mode you should pass in zero for the periodicity value (you can then retrieve the periodicity value used by the audio engine from IAudioClient::GetDevicePeriod). 
    // For exclusive mode, Initialize the stream to play at the minimum latency (by requesting the device period it is already using?).
	//So for both modes we will just get the device period before calling Initialize
	//hr = clientStream->audioClient->GetDevicePeriod(&defaultDevicePeriod, &minDevicePeriod);
    hr = CALL(GetDevicePeriod, clientStream->audioClient, &defaultDevicePeriod, &minDevicePeriod);
    if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by GetDevicePeriod\n", hr); assert(1 == 0); return -1; }

	fprintf(stdout, "Device Default Period size = %I64d\n", defaultDevicePeriod);
	fprintf(stdout, "Device Minimum Period size = %I64d\n", minDevicePeriod);


    //If we passed a format as input, we wish to use an exclusive mode (because setting a format for shared mode makes no sense)
    if (streamFormat) //streamContext->shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE )
    {
        clientStream->shareMode = AUDCLNT_SHAREMODE_EXCLUSIVE;
    }
    else
    {
        //Automatically get the format currently being used by the AudioClient for buffer processing if using shared mode
        //The mix format is usually 32-bit floating point format, which may not be supported in an exclusive stream mode 
#ifdef IAUDIOCLIENT3
        hr = CALL(GetCurrentSharedModeEnginePeriod, clientStream->audioClient, (WAVEFORMATEX**)&streamFormat, &current_period);
        if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by GetCurrentSharedModeEnginePeriod\n", hr); assert(1 == 0); return -1; }

        DP = FP = MINP = MAXP = 0;
        hr = CALL(GetSharedModeEnginePeriod, clientStream->audioClient, streamFormat, &DP, &FP, &MINP, &MAXP);
        if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by GetSharedModeEnginePeriod\n", hr); assert(1 == 0); return -1; }
#else //IAUDIOCLIENT3
        //if shared mode, get the format that is already being used by the audio client
        hr = CALL(GetMixFormat, clientStream->audioClient, &streamFormat); if (FAILED(hr)) { fprintf(stdout, "**** Error 0x%x returned by GetMixFormat\n", hr); assert(1 == 0); return -1; }
        print_waveformat_details(streamFormat);
        clientStream->shareMode = AUDCLNT_SHAREMODE_SHARED;
#endif

    }

	//double desiredDuration = (128. / (double)pwfx->nSamplesPerSec);
	//hnsRequestedDuration = (REFERENCE_TIME)(desiredDuration * (double)REFTIMES_PER_SEC);

	//convert period to frames
	devicePeriodInSeconds = defaultDevicePeriod / (10000.0*1000.0);
    //printf("Device Period (s) = %g s\n", devicePeriodInSeconds );
	
	FramesPerPeriod  = (UINT32)( streamFormat->nSamplesPerSec * devicePeriodInSeconds + 0.5 );
	fprintf(stdout, "Frames per Period = %u\n", FramesPerPeriod);
	
	//IMPORTANT!
	//For exclusive mode Streams we ask Initialize for a desired duration equal to the minimum period of the device for minimum latency	
    //For shared mode Streams we ask Initialize for a desired duration and a mininum period value of 0
    if(clientStream->shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE) hnsRequestedDuration = minDevicePeriod;//defaultDevicePeriod * 100;
    else                                                       hnsRequestedDuration = 0; //there are 10 million 100 ns increments in a 1 second

	fprintf(stdout, "Requested Duration = %I64d\n", hnsRequestedDuration);

	//Initialize the audio client for streaming playback in the desired shared mode
	//For simplicity, the buffer size and periodicity arguments to Initialize should be the same. 
	//This means that each audio engine pass processes one buffer length (which is required for event driven mode anyway).
	//Passing null as the last parameter will create a default audio session and add this stream to it
#ifdef IAUDIOCLIENT3
        hr = CALL(InitializeSharedAudioStream, clientStream->audioClient, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, MINP, streamFormat, NULL);
#else
        hr = CALL(Initialize, clientStream->audioClient, clientStream->shareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, hnsRequestedDuration, hnsRequestedDuration, streamFormat, NULL);
#endif

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
		alignedBufferSize = (REFERENCE_TIME) (1e7 * (double)bufferFrameCount / (double)(streamFormat->nSamplesPerSec) + 0.5); //TO DO:  compare to (REFERENCE_TIME)((double)REFTIMES_PER_SEC / pwfx->nSamplesPerSec * nFrames + 0.5);
		hnsRequestedDuration = alignedBufferSize;
		hr = CALL(Initialize, clientStream->audioClient, clientStream->shareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, alignedBufferSize, alignedBufferSize, streamFormat, NULL);
	
	} 
	
    //Attempting to init an exclusive stream on Realtek Laptop Speakers results in ERROR_BAD_COMMAND
    if (HRESULT_FROM_WIN32(ERROR_BAD_COMMAND) == hr)
    {
        //reenter function to create a shared stream
        fprintf(stderr, "**** Unable to create Exclusive Mode Stream for device.  Creating shared mode stream instead.\n");
        return PBAInitAudioStreamWithFormat(clientStream, NULL, AUDCLNT_SHAREMODE_SHARED);
    }

    //Observe Initialization Failure (to debug exclusive mode stream initialization)
    if (FAILED(hr))
    {
        fprintf(stderr, "**** Error 0x%x returned by Initialize\n", hr);
        //assert(1 == 0);
        assert(hr != AUDCLNT_E_ALREADY_INITIALIZED);
        assert(hr != AUDCLNT_E_WRONG_ENDPOINT_TYPE);
        assert(hr != AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED);
        assert(hr != AUDCLNT_E_BUFFER_SIZE_ERROR);
        assert(hr != AUDCLNT_E_CPUUSAGE_EXCEEDED);
        assert(hr != AUDCLNT_E_DEVICE_INVALIDATED);
        assert(hr != AUDCLNT_E_DEVICE_IN_USE);
        assert(hr != AUDCLNT_E_ENDPOINT_CREATE_FAILED);
        assert(hr != AUDCLNT_E_INVALID_DEVICE_PERIOD);
        assert(hr != AUDCLNT_E_UNSUPPORTED_FORMAT);
        assert(hr != AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED);
        assert(hr != AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL);
        assert(hr != AUDCLNT_E_SERVICE_NOT_RUNNING);

        assert(hr != AUDCLNT_E_INVALID_DEVICE_PERIOD);
        assert(hr != AUDCLNT_E_INVALID_STREAM_FLAG);
        assert(hr != AUDCLNT_E_INVALID_DEVICE_PERIOD);


        assert(hr != E_POINTER);
        assert(hr != E_INVALIDARG);
        assert(hr != E_OUTOFMEMORY);

        assert(hr != ERROR_BAD_COMMAND); //The device does not recognize the command.
        assert(1 == 0);

        return -1;
    }

	// Tell the audio source which format to use.
    //hr = pMySource->SetFormat(pwfx); if (FAILED(hr)) { printf("**** Error 0x%x returned by Initialize\n", hr); return -1; }

	// Get the actual size of the two allocated buffers.
	//In shared mode, we are slaved to the buffer size that is being used for the mix so we must use this value
	//In Exclusive Mode...?
    hr = CALL(GetBufferSize, clientStream->audioClient, &(clientStream->bufferFrameCount));
	
    //Interleaved buffer hack
    if (clientStream->shareMode == AUDCLNT_SHAREMODE_SHARED)
    {
        //clientStream->interleaved = true;
        clientStream->bufferFrameCount = clientStream->bufferFrameCount / 2;
    }

    if (hr != S_OK) { fprintf(stderr, "**** Error 0x%x returned by GetBufferSize\n", hr); return -1; }
	
	//create the render client for the stream (Note: Creating renderClient was moved to the audio thread)
    //hr = CALL(GetService, clientStream->audioClient, __riid(IAudioRenderClient), (void**)&(clientStream->renderClient));
	//if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by GetService\n", hr); return -1; }

	clientStream->currentSampleRate = clientStream->sampleRate = streamFormat->nSamplesPerSec;
	clientStream->devicePeriod      = hnsRequestedDuration;
    clientStream->format            = *streamFormat;
    clientStream->target            = PBAStreamFormatGetType(streamFormat); //enumerate a sample packing protocol for the given format

	return (int)clientStream->bufferFrameCount;
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
        acd = PBAudioComponentDescriptionMake(kAudioUnitManufacturer_Apple, kAudioUnitType_Output, kAudioUnitSubType_RemoteIO);
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
        
        //Get handle to default audio device
        if( streamContext->audioDevice == kAudioObjectUnknown )
        {
            //TO DO:  Handle Input Devices Also
            PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &streamContext->audioDevice );
            //streamContext->audioDevice = PBAudioStreamOutputDevice(&streamContext);//PBAudio.GetOuputDevice( );
        }
        else assert(1==0); //TO DO:
    
        //Update stream format
        PBAudioStreamUpdateFormat(streamContext, streamContext->sampleRate);

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
    PBAudioRegisterDeviceListeners(NULL, streamContext);
    
#endif
        
    return result;

#elif defined(_WIN32)

	HRESULT hr;
    PBAStreamFormat* pwfx = format;

    //Initalize COM and allocate a Device Enumerator object
    //So [Pb]Audio can create a stream against a hardware device
    PBAudioDeviceInitCOM();

    //Get the Default or Desired Audio Hardware Device Endpoint
    if (streamContext->audioDevice == kAudioObjectUnknown)
    {
        PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &(streamContext->audioDevice));
    }
    else assert(1 == 0);

    //if (streamContext->audioClient) CALL(Release, streamContext->audioClient); streamContext->audioClient = NULL;
    
    //Create the equivalent of an audioUnit for the device stream
    PBAudioActivateDevice(streamContext->audioDevice, &(streamContext->audioClient));

    //Update stream format
    //PBAudioStreamUpdateFormat(streamContext, streamContext->sampleRate);


    //TO DO:  Build a list of format enumerations to attempt for exclusive mode
    //Find a format suitable for initializing an exclusive mode stream against the device
    //PBADefaultFormatForDevice(streamContext->audioDevice, &pwfx);
    //hr = CALL(IsFormatSupported, streamContext->audioClient, AUDCLNT_SHAREMODE_EXCLUSIVE, pwfx, NULL); if (FAILED(hr)) { assert(1 == 0); }
    //print_waveformat_details(pwfx);


    //An IAudioClient object supports exactly one connection to the audio engine or audio hardware.This connection lasts for the lifetime of the IAudioClient object.
	streamContext->bufferFrameCount = (UINT32)PBAInitAudioStreamWithFormat(streamContext, pwfx, streamContext->shareMode);
	
	//Observe buffer size in samples
	//TO DO:  check return value for init error
	//bufferFrameCount = bufferFrameCount / 2;//pwfx->nChannels;
	fprintf(stdout, "PBAudioStreamInit::bufferSizeInSamples = %u\n", streamContext->bufferFrameCount );

    //if (streamContext->hEvent)  CloseHandle(streamContext->hEvent); streamContext->hEvent = NULL;

    // Create a platform event handle and register it for
    // buffer-event notifications.
    streamContext->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL); 
    if (streamContext->hEvent == NULL) { fprintf(stderr, "**** PBAudioStreamInit::CreateEvent Failed\n"); assert(1 == 0);  return -1; }

	// Associate the platform event handle with the stream callback to populate the buffer
    //hr = _PBAMasterStream.audioClient->SetEventHandle(_PBAMasterStream.hEvent);
    hr = CALL(SetEventHandle, streamContext->audioClient, streamContext->hEvent);
	if (FAILED(hr)) 
    {
        fprintf(stderr, "**** Error 0x%x returned by SetEventHandle\n", hr); 

        assert(hr != AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED);
        assert(hr != AUDCLNT_E_NOT_INITIALIZED);
        assert(hr != AUDCLNT_E_DEVICE_INVALIDATED);
        assert(hr != AUDCLNT_E_SERVICE_NOT_RUNNING);
        assert(hr != E_INVALIDARG);
        return hr;
    }

    //PBAudioRegisterDeviceListeners(&_PBADeviceNotificationClient, streamContext);

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
    //streamContext->inputGain = 1.0;//_renderContext.inputGain;
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

	HANDLE threadID = GetCurrentThread();
	
    //Elevate to 'Pro Audio' Thread Priority
    if (GetThreadPriority(threadID) != THREAD_PRIORITY_TIME_CRITICAL)
    {
        SetThreadPriority(threadID, THREAD_PRIORITY_TIME_CRITICAL);

        // Ask MMCSS to temporarily boost the thread priority
        // to reduce glitches while the low-latency stream plays.
        taskIndex = 0;
        hTask = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
        if (hTask == NULL)
        {
            hr = E_FAIL;
            printf("**** AvSetMmThreadCharacteristics (Pro Audio) failed!\n");
            assert(1 == 0);
            return -1;
        }
    }

    //create the render client for the stream
    hr = CALL(GetService, streamContext->audioClient, __riid(IAudioRenderClient), (void**)&(streamContext->renderClient));
    if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by GetService\n", hr); assert(1 == 0);  return -1; }
   
    //hr = clientStream->audioClient->Start();  // Start playing (ie start the device io callback).
	hr = CALL(Start, streamContext->audioClient);
    if (FAILED(hr)) 
    { 
        printf("**** Error 0x%x returned by Start (pAudioClient)\n", hr); 
        //assert(1 == 0);
        assert(hr != AUDCLNT_E_NOT_INITIALIZED);
        assert(hr != AUDCLNT_E_NOT_STOPPED);
        assert(hr != AUDCLNT_E_EVENTHANDLE_NOT_SET);
        assert(hr != AUDCLNT_E_DEVICE_INVALIDATED);
        assert(hr != AUDCLNT_E_SERVICE_NOT_RUNNING);

        return -1; 
    }

    //TO DO : set this somewhere
    streamContext->running = true;

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
		hr = CALL(GetBuffer, streamContext->renderClient, streamContext->bufferFrameCount, &((BYTE*)(interleavedBuffer.mData)) );
	
		//BUFFER_TOO_LARGE actually means we are asking the audio client for more buffer space than is currently available!
		//Microsoft loves to leave this cryptic little holes in their sample code demonstrating their APIs
		if( hr == AUDCLNT_E_BUFFER_TOO_LARGE )  { continue; };
        if (hr == AUDCLNT_E_DEVICE_INVALIDATED) { break;    }; //device sample rate changed via system

        //if (FAILED(hr)) { fprintf(stderr, "\n**** Error 0x%x returned by GetBuffer\n", hr); break; }
        if (FAILED(hr))
        {
            fprintf(stderr, "\n**** Error 0x%x returned by GetBuffer\n", hr); 
            //assert(1 == 0);
            assert(hr != AUDCLNT_E_BUFFER_ERROR);
            assert(hr != AUDCLNT_E_BUFFER_SIZE_ERROR);
            assert(hr != AUDCLNT_E_OUT_OF_ORDER);
            //assert(hr != AUDCLNT_E_DEVICE_INVALIDATED);
            assert(hr != AUDCLNT_E_BUFFER_OPERATION_PENDING);
            assert(hr != AUDCLNT_E_SERVICE_NOT_RUNNING);
            assert(hr != E_POINTER);


            return -1;
        }

		//Calculate the frames available (only if we choose to render to a portion of the buffer at a time in shared mode)
		UINT FramesAvailable = 0;
		UINT PaddingFrames = 0;
		
        // Get padding in existing buffer
		//hr = CALL(GetCurrentPadding, streamContext->audioClient, &PaddingFrames );
        //if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by GetCurrentPadding\n", hr); assert(1==0); }
		//fprintf(stdout, "Current padding = %d\n", PaddingFrames);

		// In HW mode, GetCurrentPadding returns the number of available frames in the  buffer, so we can just use that directly
        // In non-HW shared mode, GetCurrentPadding represents the number of queued frames (so we can subtract that from the overall number of frames we have)
        //if (m_DeviceProps.IsHWOffload) FramesAvailable = PaddingFrames;
		//else        FramesAvailable = bufferFrameCount - PaddingFrames;


        // Clearing the output buffer is critical for DSP routines unless such routines inherently overwrite the buffer
        PBABufferListSilenceWithFormat(&bufferList, &streamContext->format, 0, streamContext->bufferFrameCount);

		//Execute the client render callback
		streamContext->outputpass(&bufferList, streamContext->bufferFrameCount, NULL, streamContext);

		//Release the buffer to send it down the audio pipeline for device playback
		//hr = clientStream->renderClient->ReleaseBuffer(clientStream->bufferFrameCount, flags);
	    hr = CALL(ReleaseBuffer, streamContext->renderClient, streamContext->bufferFrameCount, flags);
		if (FAILED(hr)) { fprintf(stderr, "\n**** Error 0x%x returned by ReleaseBuffer\n", hr); break; }
	}


    //stream audioclient + renderclient housekeeping
    streamContext->running = false;
    CALL(Release, streamContext->renderClient); streamContext->renderClient = NULL;

    fprintf(stdout, "\nPBAudioStreamStart ended.\n");

    //device was invalidated as a result of external change (eg sample rate changed via system)
    //attempt to recover by recreatiing  the stream against the system default device
    if (hr == AUDCLNT_E_DEVICE_INVALIDATED) 
    {
        volatile bool wasRunning = false;// streamContext->running;

        //guard against calling this method unnecessarily
        //assert(streamContext->audioDevice != deviceID);

        if (streamContext->audioClient)
        {
            wasRunning = streamContext->running;
            if (wasRunning)
            {
                HRESULT hr = PBAudioStreamStop(streamContext);
                //if (FAILED(hr)) { printf("**** Error 0x%x returned by Stop (pAudioClient)\n", hr); }
            }

            /*
            HRESULT hr = CALL(SetEventHandle, streamContext->audioClient, 0);
            if (FAILED(hr))
            {
                fprintf(stderr, "**** Error 0x%x returned by SetEventHandle\n", hr);

                assert(hr != AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED);
                assert(hr != AUDCLNT_E_NOT_INITIALIZED);
                assert(hr != AUDCLNT_E_DEVICE_INVALIDATED);
                assert(hr != AUDCLNT_E_SERVICE_NOT_RUNNING);
                assert(hr != E_INVALIDARG);
                return hr;
            }
            */

            //Delete + Recreate IAudioClient against new device
            CALL(Release, streamContext->audioClient);  streamContext->audioClient = NULL;
            CALL(Release, streamContext->audioDevice);  streamContext->audioDevice = NULL;

            //There is a minimum delay of one buffer length needed for the system to clean up/recreate the default audio device 
            //before it can be recreated such that SetEventHandlle won't complain. Wait for the last buffer to play before stopping.
            Sleep((DWORD)((float)streamContext->bufferFrameCount / (float)streamContext->format.nSamplesPerSec * 1000.f * 2.f));

        }

        PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &(streamContext->audioDevice));

        //Create the equivalent of an audioUnit for the device stream
        PBAudioActivateDevice(streamContext->audioDevice, &streamContext->audioClient);

        //Update stream format
        //PBAudioStreamUpdateFormat(streamContext, streamContext->sampleRate);

        //TO DO:  how to handle exclusive mode streams

        //An IAudioClient object supports exactly one connection to the audio engine or audio hardware. This connection lasts for the lifetime of the IAudioClient object.
        streamContext->bufferFrameCount = (UINT32)PBAInitAudioStreamWithFormat(streamContext, NULL, streamContext->shareMode);

        fprintf(stdout, "PBAudioStreamSetOutputDevice::bufferSizeInSamples = %u\n", streamContext->bufferFrameCount);

        // Create a platform event handle and register it for
        // buffer-event notifications.
        assert(streamContext->hEvent);
        //streamContext->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        //if (streamContext->hEvent == NULL) { fprintf(stderr, "**** PBAudioStreamInit::CreateEvent Failed\n"); return -1; }

        //TO DO:  Should this be moved to the audio thread?
        // Associate the platform event handle with the stream callback to populate the buffer
        HRESULT hr = CALL(SetEventHandle, streamContext->audioClient, streamContext->hEvent);
        if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by SetEventHandle\n", hr); assert(1 == 0); }

        //TO DO:  Send out equivalent notification indicating device/format update

    }; 
    
    //if the stream ended because the cient was recreated due to a device change recursively restart processing on the stream
    PBAudioStreamStart(streamContext);

    //Cleanup Motherfucker!
    /*
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
        PBA_COM_RELEASE(clientStream->audioClient)C
        PBA_COM_RELEASE(clientStream->renderClient)
    */
#endif


}

OSStatus PBAudioStreamStop(PBAStreamContext * streamContext)
{

    OSStatus result = -1;

#ifdef __APPLE__

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
    
    
#elif defined(_WIN32)

    assert(streamContext->audioClient);// @"You must call setup: on this instance before starting or stopping it");
    assert(streamContext->running);

    //Stop client
    result = CALL(Stop, streamContext->audioClient);

    if (FAILED(result))
    {
        //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to start IO unit" }];
        fprintf(stderr, ", Unable to stop IO client\n");
        assert(1 == 0);
        return result;
    }

    result = CALL(Reset, streamContext->audioClient);

    if (FAILED(result))
    {
        //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to start IO unit" }];
        fprintf(stderr, ", Unable to reset IO client\n");
        assert(1 == 0);
        return result;
    }


    streamContext->running = false;

#endif

    return result;

    
}

OSStatus PBAudioStreamSetOutputDevice(PBAStreamContext * streamContext, PBAudioDevice deviceID)
{
    OSStatus result = 0;
    
    //deviceID might be an integer or pointer depending on the platform
    fprintf(stdout, "PBAudioStreamSetOutputDevice (AudioDeviceID: %llu)\n", (uint64_t)deviceID);

#ifdef __APPLE__
    if( streamContext->audioUnit )
    {
        volatile bool wasRunning = streamContext->running;
        if(wasRunning) PBAudioStreamStop(streamContext);

        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &deviceID, sizeof(deviceID));
        
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_CurrentDevice)\n") )
        {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to start IO unit" }];
            fprintf(stderr, ", Unable to set audio unit output device\n");
            return result;
        }

        AudioFormatFlags flags = streamContext->format.mFormatFlags;
        
        //Changing the device will result in an update to the audio unit stream format
        //self.hasSetInitialStreamFormat = NO;
        PBAudioStreamUpdateFormat(streamContext, 0);
        //streamContext->isDefault = false;
        
        //HACK: After setting the buffer size on the device, the audio unit seems to forget its buffer was interleaved...
        streamContext->format.mFormatFlags = flags;
        streamContext->target = PBAStreamFormatGetType(&streamContext->format); //enumerate a sample packing protocol for the given format
        
        if ( wasRunning ) PBAudioStreamStart(streamContext);
    }
#else

    volatile bool wasRunning = false;// streamContext->running;

    //guard against calling this method unnecessarily
    assert(streamContext->audioDevice != deviceID);

    if (streamContext->audioClient)
    {
        wasRunning = streamContext->running;
        if (wasRunning)
        {
            HRESULT hr = PBAudioStreamStop(streamContext);

        }
        //Delete + Recreate IAudioClient against new device
        CALL(Release, streamContext->audioClient);  streamContext->audioClient  = NULL;
        //CALL(Release, streamContext->renderClient); streamContext->renderClient = NULL;
    }

    //Create the equivalent of an audioUnit for the device stream
    PBAudioActivateDevice(deviceID, &streamContext->audioClient);

    //Update stream format
    //PBAudioStreamUpdateFormat(streamContext, streamContext->sampleRate);

    //TO DO:  how to handle exclusive mode streams

    //An IAudioClient object supports exactly one connection to the audio engine or audio hardware. This connection lasts for the lifetime of the IAudioClient object.
    streamContext->bufferFrameCount = (UINT32)PBAInitAudioStreamWithFormat(streamContext, NULL, streamContext->shareMode);

    fprintf(stdout, "PBAudioStreamSetOutputDevice::bufferSizeInSamples = %u\n", streamContext->bufferFrameCount);

    // Create a platform event handle and register it for
    // buffer-event notifications.
    assert(streamContext->hEvent);
    //streamContext->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    //if (streamContext->hEvent == NULL) { fprintf(stderr, "**** PBAudioStreamInit::CreateEvent Failed\n"); return -1; }

    //TO DO:  Should this be moved to the audio thread?
	// Associate the platform event handle with the stream callback to populate the buffer
    //hr = _PBAMasterStream.audioClient->SetEventHandle(_PBAMasterStream.hEvent);
    HRESULT hr = CALL(SetEventHandle, streamContext->audioClient, streamContext->hEvent);
	if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by SetEventHandle\n", hr); return -1; }

    //Audio Render Thread will automatically restart processing on the stream (
    //if (wasRunning) PBAudioStreamStart(streamContext);

#endif
    
    streamContext->audioDevice = deviceID;

    return result;
}


