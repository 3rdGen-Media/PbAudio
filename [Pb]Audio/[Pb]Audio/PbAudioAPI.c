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
    static PBAudioStreamFactory factory = { PBAudioStreamInit, PBAudioStreamStart, PBAudioStreamStop, PBAudioStreamSetInputDevice, PBAudioStreamSetOutputDevice };
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
    //streamContext->inputDevice    = kAudioObjectUnknown;
    streamContext->outputpass     = outputpass; //set the master render callback for the device stream
    streamContext->respectDefault = false;      //Enable for apps that want to use the system selected default audio device at all times

    streamContext->driverID  = -1; //no active vendor driver assigned
    streamContext->iChannels = streamContext->oChannels = 0; //channels enabled matrix
    

#ifdef __APPLE__

        //Get handle to default output audio device
        if (streamContext->audioDevice == kAudioObjectUnknown) PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &streamContext->audioDevice);

        //Determine if the requested output device has input channels available
        int inputChannelCount = PBAudioDeviceChannelCount(streamContext->audioDevice, kAudioObjectPropertyScopeInput);

        streamContext->inputEnabled  = inputChannelCount > 0; //input will always be configured but must be enabled explicitly
        streamContext->outputEnabled = true;                  //output always gets configured and enabled by default

        bool inputEnabled  = streamContext->inputEnabled;
        bool outputEnabled = streamContext->outputEnabled;
    
        AudioComponentDescription acd = {};
#if TARGET_OS_IPHONE || TARGET_OS_TVOS
        acd = PBAudioComponentDescriptionMake(kAudioUnitManufacturer_Apple, kAudioUnitType_Output, kAudioUnitSubType_RemoteIO);
#else
        acd = PBAudioComponentDescriptionMake(kAudioUnitManufacturer_Apple, kAudioUnitType_Output, kAudioUnitSubType_HALOutput);
#endif
        
        //Create the Audio Unit Component that will arbiter Input/Output for a single device stream
        AudioComponent component = AudioComponentFindNext(NULL, &acd);
        OSStatus          result = AudioComponentInstanceNew(component, &(streamContext->audioUnit));
        if ( !PBACheckOSStatus(result, "AudioComponentInstanceNew") )
        {
            fprintf(stderr, "Unable to instantiate IO unit\n");
            return result;
        }
        
        // Set the maximum frames per slice to render
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &PBABufferStackMaxFramesPerSlice, sizeof(PBABufferStackMaxFramesPerSlice));
        if( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioUnitProperty_MaximumFramesPerSlice)") ) assert(1==0);
        
        // Enable/disable input -- input is configured before output because if a device has no output then audio unit output must be disabled prior to setting device
        PBAudioStreamSetInputState(streamContext, inputEnabled ? 1 : 0);
    
        // Set the [input] render callback
        // An audio unit can register a distinct render callback for the input vs output (but this is unecessary and not recommended
        //AURenderCallbackStruct ircbs = { .inputProc = PBAudioStreamReceiveBuffers, .inputProcRefCon = (void*)streamContext };//(__bridge void *)(self) };
        //result = AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, 1, &ircbs, sizeof(ircbs));
        //if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioUnitProperty_SetRenderCallback)") )
        //{
        //    fprintf(stderr, "Unable to configure [input] render callback\n");
        //}
    
        // Set the input callback (notifies when audio is ready to be pulled from the device input)
        AURenderCallbackStruct inRenderProc = { .inputProc = PBAudioInputAvailableCallback, .inputProcRefCon = (void*)streamContext };//(__bridge void *)(self) };
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &inRenderProc, sizeof(inRenderProc));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_SetInputCallback)") )
        {
            fprintf(stderr, "Unable to configure input process\n");
            return result;
        }
    
        
        //Example: Initialize w/ the system default input device
        //if( streamContext->inputDevice == kAudioObjectUnknown  ) //get the default input device if device is unknown
        //{
        //    PBAudioDefaultDevice(kAudioHardwarePropertyDefaultInputDevice, &streamContext->inputDevice ); //get default input device (because default output device may not have input)
        //    //Set the Current Device to the AUHAL. this should be done only after IO has been enabled on the AUHAL.
        //    if( inputEnabled ) PBAudioStreamSetInputDevice(streamContext, streamContext->audioDevice);
        //}
        
        //Set the Current Device to the AUHAL. this should be done only after IO has been enabled on the AUHAL.
        if( inputEnabled ) PBAudioStreamSetInputDevice(streamContext, streamContext->audioDevice);

        // Enable/disable output
        UInt32 flag = outputEnabled ? 1 : 0;
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &flag, sizeof(flag));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_EnableIO)") ) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to enable/disable output" }];
            fprintf(stderr, "Unable to enable/disable output\n");
            return result;
        }
        
        // Set the [output] render callback
        AURenderCallbackStruct rcbs = { .inputProc = PBAudioStreamSubmitBuffers, .inputProcRefCon = (void*)streamContext };//(__bridge void *)(self) };
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, 0, &rcbs, sizeof(rcbs));
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioUnitProperty_SetRenderCallback)") ) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to configure output render" }];
            fprintf(stderr, "Unable to configure [output] render callback\n");
            return result;
        }

        // Initialize the AudioUnit
        result = AudioUnitInitialize(streamContext->audioUnit);
        if ( !PBACheckOSStatus(result, "AudioUnitInitialize")) {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to initialize IO unit" }];
            fprintf(stderr, "Unable to initialize IO unit\n");
            return result;
        }
        
        //Get handle to default output audio device (if that was the requested device)
        if( deviceID == kAudioObjectUnknown ) PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &streamContext->audioDevice );
        else assert(1==0); //TO DO:
    
        //Update stream format
        PBAudioStreamUpdateFormat(streamContext, streamContext->sampleRate);

        // Register a callback to watch for stream format changes
        // TO DO:  Pass notification through to client like device notifications
        //PBACheckOSStatus(AudioUnitAddPropertyListener(streamContext->audioUnit, kAudioUnitProperty_StreamFormat, PBAIOAudioUnitStreamFormatChanged, (void*)streamContext), "AudioUnitAddPropertyListener(kAudioUnitProperty_StreamFormat)");
        //PBACheckOSStatus(AudioUnitAddPropertyListener(streamContext->audioUnit, kAudioUnitProperty_SampleRate,   PBAIOAudioUnitSampleRateChanged,   (void*)streamContext), "AudioUnitAddPropertyListener(kAudioUnitProperty_SampleRate)");

        //Register PBAudio Private CoreAudio [Device] Object Observers
        //Notifications will be redistributed to clients via PbAudio so that
        //'Client Application' Processes can remain in sync with Master PbAudio 'Mix Engine' State
        PBAudioRegisterDeviceListeners(NULL, streamContext);
    
        if(inputChannelCount > 0)
        {
            streamContext->inputTimeStamp.mSampleTime = 0; //conditional in output pass

            //Get the [input] device buffer size so buffer list for capturing input can be reallocated with the correct format + num channels
            UInt32 bufferSize = 0; PBAudioDeviceBufferSize(streamContext->audioDevice, &bufferSize);

            uint64_t nBuffers = PBA_TOTAL_BUFFER_SIZE / bufferSize;

            for(int i=0; i<PBA_MAX_INFLIGHT_BUFFERS; i++)
            {
                if(streamContext->bufferList[i])  { PBABufferListFree(streamContext->bufferList[i]); streamContext->bufferList[i] = NULL; }
                if( i < nBuffers ) streamContext->bufferList[i] = PBABufferListCreateWithFormat(streamContext->format, bufferSize);
            }
            streamContext->nBuffers = nBuffers; streamContext->bufferIndex = 0;
            streamContext->passthroughEnabled = true; //
        }
    
    return result;

#elif defined(_WIN32)

	HRESULT hr;
    PBAStreamFormat* pwfx = format;

    //Initalize COM and allocate a Device Enumerator object
    //So [Pb]Audio can create a stream against a hardware device
    PBAudioDeviceInitCOM();

    //assume any device has at least 2 output channels
    //(these values will be updated after the stream has been configured depending on the active driver mode)
    streamContext->nInputChannels  = 0;
    streamContext->nOutputChannels = 2;

    //Get the Default or Desired Audio Hardware Device Endpoint
    if (streamContext->audioDevice == kAudioObjectUnknown) PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &(streamContext->audioDevice));
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
#if !TARGET_OS_OSX
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
    
    //TO DO: set session category?
    //AVAudioSessionCategoryPlayAndRecord
    
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
    HANDLE threadID = GetCurrentThread();

	//Interleaved Buffer for WinMM Driver Mode Render
	PBABuffer interleavedBuffer = { streamContext->format.nChannels, streamContext->format.wBitsPerSample/8, NULL};
	PBABufferList bufferList = {1, &interleavedBuffer};

    if (streamContext->driver) //streamContext->shareMode == PBA_DRIVER_VENDOR)
    {
        //Start Vendor (ASIO) Driver and Return (because it will callback on its own managed thread)
        hr = PBAudioDriverStart(streamContext->driver);
        if (hr == 0) streamContext->running = true;     //Set stream state to indicate started
        return hr;
    }

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
            hr = E_FAIL; fprintf(stderr, "**** AvSetMmThreadCharacteristics (Pro Audio) failed!\n");
            assert(1 == 0);
            return -1;
        }
    }
    
    //pro audio thread terminate condition
    if (!streamContext->audioClient)
    {
        //Repurpose the audio buffer ready event ?
        //TO DO:  pulse event that shutdown is waiting on 

        // kill this thread and its resources (CRT allocates them)
        _endthreadex(0);
        return 0;
    }
    //create the render client for the stream
    hr = CALL(GetService, streamContext->audioClient, __riid(IAudioRenderClient), (void**)&(streamContext->renderClient));
    if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by GetService\n", hr); assert(1 == 0);  return -1; }
   
    // Start scheduling audio (ie start the device io callback).
	hr = CALL(Start, streamContext->audioClient); 
    if (FAILED(hr)) 
    { 
        fprintf(stderr, "**** Error 0x%x returned by Start (pAudioClient)\n", hr); 

        assert(hr != AUDCLNT_E_NOT_INITIALIZED);
        assert(hr != AUDCLNT_E_NOT_STOPPED);
        assert(hr != AUDCLNT_E_EVENTHANDLE_NOT_SET);
        assert(hr != AUDCLNT_E_DEVICE_INVALIDATED);
        assert(hr != AUDCLNT_E_SERVICE_NOT_RUNNING);

        return -1; 
    }

    //set stream status to running if it gets this far on the 'pro audio' thread
    streamContext->running = true;

    UINT32 bufferFrameCount = streamContext->bufferFrameCount;
    hr = S_OK;

    //RENDER LOOP

	//Start the render loop that waits for the event to trigger. Each loop fills one of the two buffers.
	//TO DO:  Can we eliminate the loop altogether and just have the callback trigger when the event does?
    while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
    {
	    // Wait for next buffer event to be signaled.
        DWORD retval = WaitForSingleObject(streamContext->hEvent, 2000);
        if (retval != WAIT_OBJECT_0)
        {
            // Event handle timed out after a 2-second wait. Stop the 'audio unit' and break out of RENDER LOOP'
            if(streamContext->audioClient) hr = CALL(Stop, streamContext->audioClient);
			hr = ERROR_TIMEOUT;
			break;
		}

		//printf("Requesting %d buffer samples...\n", clientStream->bufferFrameCount);

		// Grab the next empty buffer to be scheduled
		hr = CALL(GetBuffer, streamContext->renderClient, bufferFrameCount, &((BYTE*)(interleavedBuffer.mData)) );
	
        //GetBuffer Retry/Continue Conditions
        //Microsoft loves to leave this cryptic little holes in their sample code demonstrating their APIs...
		//BUFFER_TOO_LARGE actually means we are asking the audio client for more buffer space than is currently available!
		if( hr == AUDCLNT_E_BUFFER_TOO_LARGE )  { continue; };
        if (hr == AUDCLNT_E_DEVICE_INVALIDATED) { break;    }; //device sample rate changed via system

        //GetBuffer Failure Conditions
        if (FAILED(hr))
        {
            fprintf(stderr, "\n**** Error 0x%x returned by GetBuffer\n", hr); 

            assert(hr != AUDCLNT_E_BUFFER_ERROR);
            assert(hr != AUDCLNT_E_BUFFER_SIZE_ERROR);
            assert(hr != AUDCLNT_E_OUT_OF_ORDER);
            //assert(hr != AUDCLNT_E_DEVICE_INVALIDATED);
            assert(hr != AUDCLNT_E_BUFFER_OPERATION_PENDING);
            assert(hr != AUDCLNT_E_SERVICE_NOT_RUNNING);
            assert(hr != E_POINTER);

            return -1;
        }

        //HACK: If PbAudioStreamSetOutputDriver decides to revert to last configured WASAPI stream there seems to be 
        //      a race condition around the 'running' parameter causing it to end up in the off state after entering the render loop
        streamContext->running = true;


		// Calculate the frames available (only if we choose to render to a portion of the buffer at a time in shared mode)
		//UINT FramesAvailable = 0;
		//UINT PaddingFrames = 0;
		
        // Get padding in existing buffer
		//hr = CALL(GetCurrentPadding, streamContext->audioClient, &PaddingFrames );
        //if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by GetCurrentPadding\n", hr); assert(1==0); }
		//fprintf(stdout, "Current padding = %d\n", PaddingFrames);

		// In HW mode, GetCurrentPadding returns the number of available frames in the  buffer, so we can just use that directly
        // In non-HW shared mode, GetCurrentPadding represents the number of queued frames (so we can subtract that from the overall number of frames we have)
        //if (m_DeviceProps.IsHWOffload) FramesAvailable = PaddingFrames;
		//else        FramesAvailable = bufferFrameCount - PaddingFrames;


        // Clearing the output buffer is critical for DSP routines unless such routines inherently overwrite the buffer
        PBABufferListSilenceWithFormat(&bufferList, &streamContext->format, 0, bufferFrameCount);

		//Execute the client render callback
		streamContext->outputpass(&bufferList, bufferFrameCount, NULL, streamContext);

		//Release the 'client ownership' of buffer to send it down the audio pipeline for device playback
	    hr = CALL(ReleaseBuffer, streamContext->renderClient, bufferFrameCount, flags);
		if (FAILED(hr)) { fprintf(stderr, "\n**** Error 0x%x returned by ReleaseBuffer\n", hr); break; }
	}

    //stream audioclient + renderclient housekeeping
    //Note: The running status was wrapped in a condition to make sure it isn't disabled when an ASIO driver is active
    if(!streamContext->driver) streamContext->running = false;
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
            
            //TO DO: this will never get entered because running status is disabled above
            //Is it unnecessary?  Why did I put it here in the first place then?
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
            CALL(Release, streamContext->audioClient); streamContext->audioClient = NULL;
            CALL(Release, streamContext->audioDevice); streamContext->audioDevice = NULL;

            //There is a minimum delay of one buffer length needed for the system to clean up/recreate the default audio device 
            //before it can be recreated such that SetEventHandle won't complain. Wait for the last buffer to play before stopping.
            Sleep((DWORD)((float)bufferFrameCount / (float)streamContext->format.nSamplesPerSec * 1000.f * 2.f));
        }

        //attempt to recover by recreatiing  the stream against the system default device
        PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &(streamContext->audioDevice));

        //Create the equivalent of an audioUnit for the device stream
        PBAudioActivateDevice(streamContext->audioDevice, &streamContext->audioClient);

        //Update stream format
        //PBAudioStreamUpdateFormat(streamContext, streamContext->sampleRate);

        //TO DO:  how to handle exclusive mode streams

        //Reinitialize the audioclient stream + format  to get buffer size in samples
        //An IAudioClient object supports exactly one connection to the audio engine or audio hardware. This connection lasts for the lifetime of the IAudioClient object.
        streamContext->bufferFrameCount = (UINT32)PBAInitAudioStreamWithFormat(streamContext, NULL, streamContext->shareMode);

        fprintf(stdout, "PBAudioStreamSetOutputDevice::bufferSizeInSamples = %u\n", streamContext->bufferFrameCount);

        // (Re)Create a platform event handle and register it for buffer-event notifications.
        assert(streamContext->hEvent);
        //streamContext->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        //if (streamContext->hEvent == NULL) { fprintf(stderr, "**** PBAudioStreamInit::CreateEvent Failed\n"); return -1; }

        //TO DO:  Should this be moved to the audio thread?
        // Associate the platform event handle with the stream callback to populate the buffer
        HRESULT hr = CALL(SetEventHandle, streamContext->audioClient, streamContext->hEvent);
        if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by SetEventHandle\n", hr); assert(1 == 0); }

        //TO DO:  Send out equivalent notification indicating device/format update

    }; 
    
    //reentrant if the stream ended because the client was recreated due to a device change recursively restart processing on the stream
    if(!streamContext->driver) return PBAudioStreamStart(streamContext);

    // kill this thread and its resources (CRT allocates them)
    //AvRevertMmThreadCharacteristics(hTask);
    _endthreadex(0);

#endif


}

OSStatus PBAudioStreamStop(PBAStreamContext * streamContext)
{

    OSStatus result = -1;

#ifdef __APPLE__

#if (PBA_TARGET_IOS) || defined(PBA_TARGET_TVOS)

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

    assert(streamContext->running);

    if (streamContext->driver) //streamContext->shareMode == PBA_DRIVER_VENDOR)
    {
        //Stop Vendor (ASIO) Driver
        result = PBAudioDriverStop(streamContext->driver);
    }
    else
    {
        assert(streamContext->audioClient);// @"You must call setup: on this instance before starting or stopping it");

        //Stop WASAPI AudioClient
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
    }

    //Set stream state to indicate stopped
    streamContext->running = false;

#endif

    return result;

    
}


//kAsioResetRequest: Stop, Dispose Buffers, Destruct, (Re)Construct, (Re)Initialize
//Is nearly identical to PBAudioStreamSetOutputDriver
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioStreamReset(PBAStreamContext* streamContext)
{
    OSStatus result = 0;

    //deviceID might be an integer or pointer depending on the platform
    fprintf(stdout, "PBAudioStreamReset\n");

#ifdef __APPLE__
    assert(1 == 0);
#else

    volatile bool wasRunning = false;// streamContext->running;
    
    //stop active stream system or vendor driver
    if (streamContext->audioClient)
    {
        assert(1 == 0);  //this path doesn't mean anything for non-ASIO drivers
        wasRunning = streamContext->running;
        if (wasRunning) result = PBAudioStreamStop(streamContext);

        //Delete + Recreate IAudioClient against new device
        CALL(Release, streamContext->audioClient); streamContext->audioClient = NULL;
        streamContext->audioDevice = NULL; //remove active device reference on stream
        //CALL(Release, streamContext->audioDevice); streamContext->audioDevice = NULL;
        //CALL(Release, streamContext->renderClient); streamContext->renderClient = NULL;
    }
    else if (streamContext->driver)
    {
        CoInitialize(0);

        PBAudioDriverID driverID = streamContext->driverID;                        //store driver id; it will be overwritten by PBAudioDriverShutdown
        wasRunning = streamContext->running;
        if (wasRunning) result = PBAudioStreamStop(streamContext);                 //IASIO::stop
        PBAudioDriverDisposeBuffers(streamContext->driver);                        //IASIO::dispose_buffers
        PBAudioDriverShutdown(&streamContext->driver, &streamContext->driverID);   //IASIO::close, then release IASO com object
    
        //overwrite driver mode on stream
        streamContext->shareMode = PBA_DRIVER_VENDOR;

        //Load the Vendor [ASIO] Driver
        PBAudioDriverID selectedDriverID = driverID;
        if (PBAudioLoadVendorDriver(&streamContext->driver, streamContext->driverID, &selectedDriverID) == 0)
        {
            //Record active driver id on stream
            streamContext->driverID = selectedDriverID;

            //Initialize the Vendor [ASIO] Driver
            if (PBAudioInitVendorDriver(streamContext->driver, streamContext) == 0)
            {
                //debug by checking if control panel can open
                //PBAudioDriverControlPanel(streamContext->driver); 

                //Start the Vendor [ASIO] driver
                if (wasRunning) PBAudioStreamStart(streamContext); //IASIO::start

            }
            else assert(1 == 0); //if the asio driver was created but there was no physical device to initialize against
        }
        else assert(1 == 0);
    
        CoUninitialize();

    }

#endif

    //streamContext->shareMode = PBA_DRIVER_VENDOR;
    //streamContext->driverID  = driverID;

    return result;
}



OSStatus PBAudioStreamGetOutputDevice(PBAStreamContext * streamContext, PBAudioDevice* deviceID)
{
    OSStatus result = 0;

#ifdef __APPLE__
    if( streamContext->audioUnit )
    {
        UInt32 size = sizeof(AudioDeviceID);
        result = AudioUnitGetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, deviceID, &size);
        
        if ( !PBACheckOSStatus(result, "AudioDeviceID(kAudioOutputUnitProperty_CurrentDevice)\n") )
        {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to start IO unit" }];
            fprintf(stderr, ", Unable to get audio unit output device\n");
            return result;
        }
    }
#else
        assert(1==0);
#endif
    
    return result;
}

OSStatus PBAudioStreamSetPassThroughState(PBAStreamContext* streamContext, uint32_t state)
{
    streamContext->passthroughEnabled = state;
    return noErr;
}

OSStatus PBAudioStreamSetInputState(PBAStreamContext* streamContext, uint32_t state)
{
    OSStatus result = 0;
#ifdef __APPLE__
    result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &state, sizeof(state));
    if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_EnableIO)") )
    {
        //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:userInfo:@{ NSLocalizedDescriptionKey: @"Unable to enable/disable input" }];
        fprintf(stderr, "PBAudioStreamSetOutputDevice::Unable to enable/disable input\n");
        return result;
    }
#else
    assert(1 == 0);
#endif
    
    streamContext->inputEnabled = state;
    return result;
}

OSStatus PBAudioStreamSetOutputDevice(PBAStreamContext* streamContext, PBAudioDevice deviceID)
{
    OSStatus result = 0;
    
    //deviceID might be an integer or pointer depending on the platform
    fprintf(stdout, "PBAudioStreamSetOutputDevice (AudioDeviceID: %llu)\n", (uint64_t)deviceID);

#ifdef __APPLE__
    if( streamContext->audioUnit )
    {
        volatile bool wasRunning     = streamContext->running;
        volatile bool inputEnabled   = streamContext->inputEnabled;
        volatile bool wasPassthrough = streamContext->passthroughEnabled;

        //Stop Audio Unit
        if(wasRunning)
        {
            PBAudioStreamSetPassThroughState(streamContext, false); //disable input passthrough
            PBAudioStreamStop(streamContext);
        }
        
        //leave input enabled if the new output device support input channels, otherwise disable it
        int inputChannelCount = PBAudioDeviceChannelCount(deviceID, kAudioObjectPropertyScopeInput);
        
        //if the input and output audio units aren't mutually exclusive
        //if( streamContext->audioUnit && streamContext->inputUnit)
        {
            //the new device always supports output by virtue of passing an enumerated output device id to this function
            //however, input is always on audio unit prior to (re)setting the output device, since the new device may not support input
            if( inputEnabled )
            {
                //PBAudioStreamSetPassThroughState(streamContext, false);
                PBAudioStreamSetInputState(streamContext, false);
            }
        }
        
        //Set the new output device on the audio unit
        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &deviceID, sizeof(deviceID));
        
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_CurrentDevice)\n") )
        {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to start IO unit" }];
            fprintf(stderr, ", Unable to set audio unit output device\n");
            return result;
        }

        AudioFormatFlags flags = streamContext->format.mFormatFlags; //cache the old format flags... see HACK below
        PBAudioStreamUpdateFormat(streamContext, 0); //Changing the device will result in an update to the audio unit stream format
        streamContext->format.mFormatFlags = flags;  //HACK: After setting the buffer size on the device, the audio unit seems to forget its buffer was interleaved...
        streamContext->target = PBAStreamFormatGetType(&streamContext->format); //enumerate a sample packing protocol for the given format
                
        if(inputChannelCount > 0)
        {
            //Update input var state, this is critical for PBAudioStreamUpdateFormat
                  PBAudioStreamSetInputState(streamContext, true);
            //PBAudioStreamSetPassThroughState(streamContext, false);

            streamContext->inputTimeStamp.mSampleTime = 0; //conditional in output pass

            AudioFormatFlags flags = streamContext->format.mFormatFlags; //cache the old format flags... see HACK below
            PBAudioStreamUpdateFormat(streamContext, 0);                 //Update the stream format again to pick up the input format changes
            streamContext->format.mFormatFlags = flags;                  //HACK: After setting the buffer size on the device, the audio unit seems to forget its buffer was interleaved...
            streamContext->target = PBAStreamFormatGetType(&streamContext->format); //MUST ALWAYS (re)enumerate a sample packing protocol for the given format after update no exceptions
            
            //Get the [input] device buffer size so buffer list for capturing input can be reallocated with the correct format + num channels
            UInt32 bufferSize = 0; PBAudioDeviceBufferSize(deviceID, &bufferSize);

            uint64_t nBuffers = PBA_TOTAL_BUFFER_SIZE / bufferSize;
            for(int i=0; i<PBA_MAX_INFLIGHT_BUFFERS; i++)
            {
                if(streamContext->bufferList[i])  { PBABufferListFree(streamContext->bufferList[i]); streamContext->bufferList[i] = NULL; }
                if( i < nBuffers ) streamContext->bufferList[i] = PBABufferListCreateWithFormat(streamContext->format, bufferSize);
            }
            streamContext->nBuffers = nBuffers; streamContext->bufferIndex = 0;

        }
        
        //Restart Audio Unit
        if ( wasRunning )
        {
            PBAudioStreamStart(streamContext);
            PBAudioStreamSetPassThroughState(streamContext, wasPassthrough); //restore input passthrough state
        }
    }

    streamContext->audioDevice = deviceID;

#else

    volatile bool wasRunning = false;
    volatile bool needsStart = false;

    //guard against calling this method unnecessarily
    assert(streamContext->audioDevice != deviceID);

    //stop active stream system or vendor driver
    if (streamContext->audioClient)
    {
        wasRunning = streamContext->running;
        if (wasRunning) result = PBAudioStreamStop(streamContext);

        //Delete + Recreate IAudioClient against new device
        CALL(Release, streamContext->audioClient);  streamContext->audioClient  = NULL;
        //CALL(Release, streamContext->renderClient); streamContext->renderClient = NULL;
    }
    else if (streamContext->driver)
    {
        wasRunning = streamContext->running;
        if (wasRunning) result = PBAudioStreamStop(streamContext);                 //IASIO::stop
        PBAudioDriverDisposeBuffers(streamContext->driver);                        //IASIO::displose_buffers
        PBAudioDriverShutdown(&streamContext->driver, &streamContext->driverID);   //IASIO::close, then release IASO com object
        needsStart = true;                                                         //restart the winmm "pro audio" thread
    }

    //overwrite driver mode on stream
    streamContext->shareMode = PBA_DRIVER_SHARED;

    //assume any device has at least 2 output channels
    //(these values will be updated using IMMDevice API after the stream has been configured)
    streamContext->nInputChannels  = 0;
    streamContext->nOutputChannels = 2;

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

    streamContext->audioDevice = deviceID;

    //Audio Render Thread will automatically restart processing on the stream render thread
    //if (wasRunning) PBAudioStreamStart(streamContext);

    //restart the stream's winmm pro audio thread
    if (needsStart)  _beginthreadex(NULL, 0, (_beginthreadex_proc_type)PBAudio.Start, streamContext, 0, &(streamContext->audioThreadID));

#endif

    return result;
}

#ifndef __APPLE__

OSStatus PBAudioStreamSetOutputDriver(PBAStreamContext* streamContext, PBAudioDriverID driverID)
{
    OSStatus result = 0;

    //deviceID might be an integer or pointer depending on the platform
    fprintf(stdout, "PBAudioStreamSetOutputDriver (AudioDriverID: %ld)\n", (PBAudioDriverID)driverID);

#ifdef __APPLE__
    assert(1 == 0); //In the long long ago, CoreAudio made ASIO obsolete on Darwin Platforms
#else

    volatile bool wasRunning = false;// streamContext->running;

    PBAudioDevice   prevDevice   = NULL;
    void*           prevDriver   = NULL;
    PBAudioDriverID prevDriverID = -1;

    //guard against calling this method unnecessarily
    assert(streamContext->driverID != driverID);

    //stop active stream system or vendor driver
    if (streamContext->audioClient) 
    {
        wasRunning = streamContext->running;
        if (wasRunning) result = PBAudioStreamStop(streamContext);

        prevDevice = streamContext->audioDevice;

        //Delete + Recreate IAudioClient against new device
        CALL(Release, streamContext->audioClient); streamContext->audioClient = NULL;
        streamContext->audioDevice = NULL; //remove active device reference on stream
        //CALL(Release, streamContext->audioDevice); streamContext->audioDevice = NULL;
        //CALL(Release, streamContext->renderClient); streamContext->renderClient = NULL;
    }
    else if (streamContext->driver)
    {
        prevDriver = streamContext->driver;
        wasRunning = streamContext->running;
        if (wasRunning) result = PBAudioStreamStop(streamContext);                 //IASIO::stop
        PBAudioDriverDisposeBuffers(streamContext->driver);                        //IASIO::dispose_buffers
        PBAudioDriverShutdown(&streamContext->driver, &streamContext->driverID);   //IASIO::close, then release IASO com object
    }

    //overwrite driver mode on stream
    streamContext->shareMode = PBA_DRIVER_VENDOR;

    if (driverID < 0) return result;

    //Load the Vendor [ASIO] Driver
    PBAudioDriverID selectedDriverID = driverID;
    if (driverID > -1 && PBAudioLoadVendorDriver(&streamContext->driver, streamContext->driverID, &selectedDriverID) == 0)
    {
        //Record active driver id on stream
        streamContext->driverID = selectedDriverID; 

        //Initialize the Vendor [ASIO] Driver
        if (PBAudioInitVendorDriver(streamContext->driver, streamContext) == 0 )
        {
            //debug by checking if control panel can open
            //PBAudioDriverControlPanel(streamContext->driver); 

            //Start the Vendor [ASIO] driver
            if (wasRunning) PBAudioStreamStart(streamContext); //IASIO::start
        }
        else //if the asio driver was created but there was no physical device to initialize against (observed with UAD drivers)
        {
            //Destroy the IASIO driver that failed to init
            PBAudioDriverShutdown(&streamContext->driver, &streamContext->driverID);

            //Restore the last configured device audioClient or IASIO driver
            //Since we are restoring we don't really care about observing the return codes
            if (prevDevice) PBAudioStreamSetOutputDevice(streamContext, prevDevice);
            if (prevDriver && prevDriverID > -1)
            {
                selectedDriverID = prevDriverID;
                if (PBAudioLoadVendorDriver(&streamContext->driver, streamContext->driverID, &selectedDriverID) != 0) return -1;
                if (PBAudioInitVendorDriver(streamContext->driver, streamContext) != 0) return -1;
                if (wasRunning) PBAudioStreamStart(streamContext); //IASIO::start
            }

            return -1;
        }
    }
    else assert(1 == 0);

#endif

    //streamContext->shareMode = PBA_DRIVER_VENDOR;
    //streamContext->driverID  = driverID;

    return result;
}

#endif



OSStatus PBAudioStreamSetInputDevice(PBAStreamContext * streamContext, PBAudioDevice deviceID)
{
    OSStatus result = 0;
    
    //deviceID might be an integer or pointer depending on the platform
    fprintf(stdout, "PBAudioStreamSetOutputDevice (AudioDeviceID: %llu)\n", (uint64_t)deviceID);

#ifdef __APPLE__
    if( streamContext->audioUnit )
    {
        //volatile bool wasRunning = streamContext->running;
        //if(wasRunning) PBAudioStreamStop(streamContext);

        result = AudioUnitSetProperty(streamContext->audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &deviceID, sizeof(deviceID));
        
        if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioOutputUnitProperty_CurrentDevice)\n") )
        {
            //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to start IO unit" }];
            fprintf(stderr, ", Unable to set audio unit output device\n");
            return result;
        }
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
    
    //streamContext->inputDevice = deviceID;

    return result;
}



