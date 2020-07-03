//
//  main.m
//  [Pb] Audio
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#ifdef _WIN32

/*
#define NOMINMAX
#define NTDDI_VERSION NTDDI_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7 

// standard definitions
#define STRICT                                                  // enable strict type-checking of Windows handles
#define WIN32_LEAN_AND_MEAN                                     // allow the exclusion of uncommon features
//#define WINVER                                          _WIN32_WINNT_WIN7  // allow the use of Windows XP specific features
//#define _WIN32_WINNT                                    _WIN32_WINNT_WIN7  // allow the use of Windows XP specific features
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES         1       // use the new secure functions in the CRT
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT   1       // use the new secure functions in the CRT
*/
#include <windows.h>            // fundamental Windows header file
//#define GLDECL WINAPI
//#define DXDECL WINAPI

#define _USE_MATH_DEFINES	//holy crap!!! must define this on ms windows to get M_PI definition!
#include <math.h>

#define THREADPROC WINAPI   //What does this do again?

//Win32 c based dependencies
#include <tchar.h>              // generic text character mapping
#include <string.h>             // includes string manipulation routines
#include <stdlib.h>           // includes standard libraries
#include <stdint.h>           // includes uint64_t, etc types

#include <stdio.h>              // includes standard input/output routines
#include <process.h>            // threading routines for the CRT
//#include <tlhelp32.h>

//For Win32 Media Manager File Loading
static const GUID _IID_MF_LOW_LATENCY = { 0x9c27891a, 0xed7a, 0x40e1, {0x88, 0xe8, 0xb2, 0x27, 0x27, 0xa0, 0x24, 0xee} };

#elif defined(__APPLE__)
#include <TargetConditionals.h>

#if    TARGET_OS_IOS || TARGET_OS_TVOS
#include <CoreFoundation/CoreFoundation.h>           //Core Foundation
#include <objc/runtime.h>                            //objective-c runtime
#include <objc/message.h>                            //objective-c runtime message
extern int UIApplicationMain(int argc, char * _Nullable *argv, id principalClassName, id delegateClassName);
#else
#include <ApplicationServices/ApplicationServices.h> //Cocoa
#include <CoreFoundation/CoreFoundation.h>           //Core Foundation
extern int NSApplicationMain(int argc, const char *__nonnull argv[__nonnull]);
#endif

//Class Names of the Custom NSApplication/UIApplication and NSAppDelegate/UIAppDelegate Cocoa App Singleton Objective-C Objects
static const char * CocoaAppClassName = "PbAudioApplication";
static const char * CocoaAppDelegateClassName = "PbAudioAppDelegate";

#pragma mark -- CFNotification Center Notifications
void mainWindowChangedNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo) {
    
    printf("\nMain Window Changed!\n");
    
    CFShow(CFSTR("Received notification (dictionary): "));
    CFShow(name);
    assert(object);
    assert(userInfo);
    // print out user info
    const void * keys;
    const void * values;
    CFDictionaryGetKeysAndValues(userInfo, &keys, &values);
    for (int i = 0; i < CFDictionaryGetCount(userInfo); i++) {
        const char * keyStr = CFStringGetCStringPtr((CFStringRef)&keys[i], CFStringGetSystemEncoding());
        const char * valStr = CFStringGetCStringPtr((CFStringRef)&values[i], CFStringGetSystemEncoding());
        printf("\t\t \"%s\" = \"%s\"\n", keyStr, valStr);
    }
    
}

void notificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo) {
    
    printf("\nnotification callback\n");
    /*
     CFShow(CFSTR("Received notification (dictionary):"));
     // print out user info
     const void * keys;
     const void * values;
     CFDictionaryGetKeysAndValues(userInfo, &keys, &values);
     for (int i = 0; i < CFDictionaryGetCount(userInfo); i++) {
     const char * keyStr = CFStringGetCStringPtr((CFStringRef)&keys[i], CFStringGetSystemEncoding());
     const char * valStr = CFStringGetCStringPtr((CFStringRef)&values[i], CFStringGetSystemEncoding());
     printf("\t\t \"%s\" = \"%s\"\n", keyStr, valStr);
     }
     */
}

static void RegisterNotificationObservers()
{
    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    assert(center);
    
    // add an observer
    CFNotificationCenterAddObserver(center, NULL, notificationCallback,
                                    CFSTR("NSApplicationDidBecomeActiveNotification"), NULL,
                                    CFNotificationSuspensionBehaviorDeliverImmediately);
    
    CFNotificationCenterAddObserver(center, NULL, notificationCallback,
                                    CFSTR("NSApplicationDidResignActiveNotification"), NULL,
                                    CFNotificationSuspensionBehaviorDeliverImmediately);
    
    CFNotificationCenterAddObserver(center, NULL, mainWindowChangedNotificationCallback,
                                    CFSTR("CGWindowDidBecomeMainNotification"), NULL,
                                    CFNotificationSuspensionBehaviorDeliverImmediately);
    
    /*
     // post a notification
     CFDictionaryKeyCallBacks keyCallbacks = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
     CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
     CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
     &keyCallbacks, &valueCallbacks);
     CFDictionaryAddValue(dictionary, CFSTR("TestKey"), CFSTR("TestValue"));
     CFNotificationCenterPostNotification(center, CFSTR("MyNotification"), NULL, dictionary, TRUE);
     CFRelease(dictionary);
     */
    
    // remove oberver
    //CFNotificationCenterRemoveObserver(center, NULL, CFSTR("TestValue"), NULL);
}

#endif //__APPLE__


#include "[Pb]Audio/[Pb]Audio.h"

//A sine wave source
static float * g_sineWaveBuffer = NULL;
static uint64_t g_sineBufferLengthInSamples;
static uint32_t g_sineBufferChannels;

//An abstract source
static float * g_sourceBuffer = NULL;
static uint64_t g_sourceBufferLengthInSamples;
static uint32_t g_sourceBufferChannels;

static uint64_t g_playbackSampleOffset = 0;


void InitPlatform()
{
    //cr_appEventQueue = 0;
#ifdef _WIN32
    //cr_mainThread = GetCurrentThread();
    //cr_mainThreadID = GetCurrentThreadId();
#elif defined(__APPLE__) && TARGET_OS_OSX
     //Initialize kernel timing mechanisms for debugging
    //monotonicTimeNanos();

    //We can do some Cocoa initializations early, but don't need to
    //Note that NSApplicationLoad will load default NSApplication class
    //Not your custom NSApplication Subclass w/ custom run loop!!!
    //NSApplicationLoad();

    /***
     *  0 Explicitly
     *
     *  OSX CoreGraphics API requires that we make the process foreground application if we want to
     *  create windows on threads other than main, or we will get an error that RegisterApplication() hasn't been called
     *  Since this is usual startup behavior for User Interactive NSApplication anyway that's fine
     ***/
    //pid_t pid;
    //CGSInitialize();

    ProcessSerialNumber psn = { 0, kCurrentProcess };
    //GetCurrentProcess(&psn);  //this is deprecated, all non-deprecated calls to make other process foreground must route through Cocoa
    /*OSStatus transformStatus = */TransformProcessType(&psn, kProcessTransformToForegroundApplication);
#endif
}


int StartPlatformEventLoop(int argc, const char * argv[])
{
#ifdef _WIN32
    MSG msg;
    //int exitStatus;
    bool appIsRunning = true;

    //will just idle for now (in a multiwindow scenario this is best)
    //later we will probably put the physics loop here
    //_idleEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    while (appIsRunning)
    {
        //filter for events we would like to be notified of in the message queue, much like a kqueue or [NSApplication nextEventWith:]
        //Use GetMessage to wait until the message arrives, or PeekMessage to return immediately
        //if(PeekMessage(&msg, NULL, CR_PLATFORM_EVENT_MSG_LOOP_QUIT, CR_PLATFORM_EVENT_MSG_LOOP_QUIT, PM_REMOVE))
        if(GetMessage(&msg, NULL, 0, 0) && msg.message != WM_QUIT)  //run the event loop until it spits out error or quit
        {
			/*
            //observe our custom defined quit message
            if( msg.message == CR_PLATFORM_EVENT_MSG_LOOP_QUIT)
            {
                fprintf(stdout, "\nCR_PLATFORM_EVENT_MSG_LOOP_QUIT\n");
                appIsRunning = false;
            }
			*/
			//pass on the messages to the __main_event_queue for processing
			//TranslateMessage(&msg);
			DispatchMessage(&msg);
            memset(&msg, 0, sizeof(MSG));
        }
    }
    //TO DO: Cleanup();
    return 0;
#elif defined(__APPLE__)

#if TARGET_OS_IOS || TARGET_OS_TVOS
    // Create an @autoreleasepool, using the old-stye API.
    // Note that while NSAutoreleasePool IS deprecated, it still exists
    // in the APIs for a reason, and we leverage that here. In a perfect
    // world we wouldn't have to worry about this, but, remember, this is C.
    

    id (*objc_ClassSelector)(Class class, SEL _cmd) = (void*)objc_msgSend;//objc_msgSend(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc")), sel_registerName("init"))
    id (*objc_InstanceSelector)(id self, SEL _cmd) = (void*)objc_msgSend;//objc_msgSend(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc")), sel_registerName("init"))
    //id (*objc_init)(id self, SEL _cmd) = (void*)objc_msgSend;//objc_msgSend(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc")), sel_registerName("init"))

    //id (*objc_msgSendCreateAutoreleasePool)(id self, SEL _cmd, SEL _cmd2) = (void*)objc_msgSend;//objc_msgSend(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc")), sel_registerName("init"))
    //id (*crObjc_msgSend)(id self) = (void*)objc_msgSend;

    id autoreleasePool = objc_InstanceSelector(objc_ClassSelector(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc")), sel_registerName("init"));
    //pre 10.15
    //id autoreleasePool = objc_msgSend(objc_msgSend(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc")), sel_registerName("init"));


    
    // Get a reference to the file's URL
    CFStringRef cfAppClassName = CFStringCreateWithCString(kCFAllocatorDefault, CocoaAppClassName, CFStringGetSystemEncoding());
    CFStringRef cfAppDelegateClassName = CFStringCreateWithCString(kCFAllocatorDefault, CocoaAppDelegateClassName, CFStringGetSystemEncoding());
    // Notice the use of CFSTR here. We cannot use an objective-c string
    // literal @"someStr", as that would be using objective-c, obviously.
    UIApplicationMain(argc, (char* _Nullable *)argv, (id)cfAppClassName, (id)cfAppDelegateClassName);
    CFRelease( cfAppClassName );
    CFRelease( cfAppDelegateClassName );
    
    
    objc_InstanceSelector(autoreleasePool, sel_registerName("drain"));
    
    
    //OMG, objc_msgSend defintion has changed as of OSX 10.15 Catalina.  FUCK YOU APPLE!!!
    //((id (*)(id, SEL, void*))objc_msgSend)(objc_msgSend(objc_getClass("CRMetalInterface"), sel_registerName("sharedInstance")), sel_getUid("displayLoop:"), view);
    //void* (*objc_msgSendSharedInstance)(Class, SEL) = (void*)objc_msgSend;
    //id cmiSharedInstance = objc_msgSendSharedInstance(objc_getClass("CRMetalInterface"), sel_registerName("sharedInstance"));
    //void* (*objc_msgSendDisplayLoop)(void*, SEL, void*) = (void*)objc_msgSend;
    //return objc_msgSendDisplayLoop(objc_msgSend(objc_getClass("CRMetalInterface"), sel_registerName("sharedInstance")), sel_getUid("displayLoop:"), view);
#else
    NSApplicationMain(argc, argv);       //Cocoa Application Event Loop
#endif
#endif
    printf("\nStartPlatformEventLoop() End\n");
    return -1;
}



static void GenerateSineSamplesFloat(float **Buffer, size_t BufferLengthInSamples, unsigned long Frequency, unsigned short ChannelCount, double SamplesPerSecond, double Amplitude, double * InitialTheta)
{
	size_t i, j;
    double sampleIncrement = (Frequency * (M_PI*2)) / SamplesPerSecond;
    
    double theta = (InitialTheta!=NULL) ? *InitialTheta : 0;
    
    //allocate a buffer
    *Buffer = (float*)malloc( BufferLengthInSamples * ChannelCount * sizeof(float));
    
    for(i = 0; i<BufferLengthInSamples; i++)
    {
        
        double sinValue = Amplitude * sin(theta);
        for(j=0;j<ChannelCount;j++)
        {
            (*Buffer)[i*ChannelCount+j] = (float)sinValue;
        }
        theta+=sampleIncrement;
    }
    
    if( InitialTheta != NULL )
    {
        *InitialTheta = theta;
    }
    
}


int PBAInitCOM()
{
	//CoIntialize(Ex) Initializes the COM library for use by the calling thread, sets the thread's concurrency model, and creates a new apartment for the thread if one is required.
	//You should call Windows::Foundation::Initialize to initialize the thread instead of CoInitializeEx if you want to use the Windows Runtime APIs or if you want to use both COM and Windows Runtime components. 
	//Windows::Foundation::Initialize is sufficient to use for COM components.
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) { printf("**** Error 0x%x returned by CoInitializeEx\n", hr); return -1; }
	
	//Create MMDeviceEnumerator so we can get an audio endpoint
	//CoCreateInstance Creates a single uninitialized object of the class associated with a specified CLSID.
	//Call CoCreateInstance when you want to create only one object on the local system. 
	//To create a single object on a remote system, call the CoCreateInstanceEx function. To create multiple objects based on a single CLSID, call the CoGetClassObject function.
    hr = CoCreateInstance(__clsid(MMDeviceEnumerator), NULL, CLSCTX_ALL, __riid(IMMDeviceEnumerator), (void**)&_PBADeviceEnumerator);
	if (FAILED(hr)) { printf("**** Error 0x%x returned by CoCreateInstance\n", hr); return -1; }
	return 0;
}

int PBAGetDefaultAudioDevice(IMMDevice **pDevice)
{
    //HRESULT hr = gEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, pDevice);
	HRESULT hr = CALL(GetDefaultAudioEndpoint, _PBADeviceEnumerator, eRender, eConsole, pDevice);
	if (FAILED(hr)) { printf("**** Error 0x%x returned by GetDefaultAudioEndpoint\n", hr); return -1; }
	
	return 0;
}


int PBAActivateAudioDevice(IMMDevice * device, IAudioClient2 ** audioClient)
{
	//Active a version 1 Aucio Client
    //HRESULT hr = clientStream->gDevice->Activate( IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&(clientStream->audioClient));
	//HRESULT hr = PBADeviceActivate( clientStream->gDevice, IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&(clientStream->audioClient));
	HRESULT hr = CALL(Activate, device, __riid(IAudioClient), CLSCTX_ALL, NULL, (void**)audioClient);
    
	if (FAILED(hr)) { printf("**** Error 0x%x returned by Activate\n", hr); return -1; }
	return 0;
}



typedef union fourByteInt
{
	uint32_t i;
	char bytes[4];
}fourByteInt;

#define PBA_COM_RELEASE(punk) if ((punk) != NULL) { CALL(Release, punk); (punk) = NULL; }

static unsigned int __stdcall pbaudio_stream_render(void * stream)
{
	//int bytesPerSample;
	HRESULT hr;
	DWORD flags = 0;

	//Task is used to elevate thread to ProAudio Latency
	HANDLE hTask = NULL;
	DWORD taskIndex = 0;
	
	
	PBAStreamContext * clientStream = (PBAStreamContext*)stream;

	//UINT64 playbackSampleOffset = 0;
	//UINT64 samplesToCopy = 0;
	//UINT64 remainingSamples =0;
	
	fourByteInt fourI = {0};
	char * buffer = NULL;
	//BYTE *pData;// = {NULL, NULL};
	
	//for render callback
	PBABuffer interleavedBuffer = { clientStream->format.nChannels, clientStream->format.wBitsPerSample/8, NULL};
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

	/*
	// To reduce latency, load the first buffer with data
    // from the audio source before starting the stream.
    //hr = clientStream->renderClient->GetBuffer(clientStream->bufferFrameCount ,&pData);
	hr = CALL(GetBuffer, clientStream->renderClient, clientStream->bufferFrameCount, &pData);
	if (FAILED(hr)) { printf("**** Error 0x%x returned by GetBuffer\n", hr); return -1; }

	//Adjust buffer size in samples (shared mode streams only)
	//clientStream->bufferFrameCount =  clientStream->bufferFrameCount /4;// pwfx->nBlockAlign;
	//printf("bufferSizeInSamples = %u\n", bufferFrameCount);

	//Calculate the frames available (shared mode streams only)
	//UINT32 FramesAvailable = 0;
	//UINT32 PaddingFrames = 0;

	// Get padding in existing buffer (shared mode streams only)
	//hr = clientStream->audioClient->GetCurrentPadding( &PaddingFrames );
    //if (FAILED(hr)) { printf("**** Error 0x%x returned by GetCurrentPadding\n", hr); return -1; }
	//printf("Current padding = %d\n", PaddingFrames);

	//Copy samples to output render buffer
	//hr = pMySource->LoadData(bufferFrameCount, pData, &flags);

	samplesToCopy = clientStream->bufferFrameCount;
	remainingSamples = g_sourceBufferLengthInSamples - playbackSampleOffset;//audioEvent.audioData.size() - playbackSampleOffset;

	printf("samplesToCopy = %d\n", samplesToCopy);
	printf("remainingSamples = %d\n", remainingSamples);
	printf("playbackSampleOffset = %d\n", playbackSampleOffset);
	printf("g_sourceBufferLengthInSamples  = %d\n", g_sourceBufferLengthInSamples);

	if(  remainingSamples > 0 && remainingSamples < samplesToCopy )
		samplesToCopy = remainingSamples;

	//memcpy(pData, &(audioEvent.audioData[playbackSampleOffset]), samplesToCopy );
	//memcpy(pData, &(g_sourceBuffer[playbackSampleOffset*g_sourceBufferChannels*g_sourceBytesPerSample]), samplesToCopy*g_sourceBufferChannels*g_sourceBytesPerSample );
	//memcpy(pData, &(g_sineWaveBuffer[playbackSampleOffset*g_sourceBufferChannels]), samplesToCopy*g_sourceBufferChannels*3);
	memcpy(pData, &(g_sineWaveBuffer[playbackSampleOffset*g_sourceBufferChannels]), samplesToCopy*g_sineBufferChannels*sizeof(float));
	
	//bytesPerSample = clientStream->format.nBlockAlign / clientStream->format.nChannels;
	playbackSampleOffset += samplesToCopy;

	//release the output render buffer of size bufferFrameCount after copying data to the buffer
    //hr = clientStream->renderClient->ReleaseBuffer(clientStream->bufferFrameCount, flags);
    hr = CALL(ReleaseBuffer, clientStream->renderClient, clientStream->bufferFrameCount, flags);
	
	if (FAILED(hr)) { printf("**** Error 0x%x returned by ReleaseBuffer\n", hr); return -1; }
	pData = NULL;
	*/

   
    //hr = clientStream->audioClient->Start();  // Start playing.
	hr = CALL(Start, clientStream->audioClient);
    if (FAILED(hr)) { printf("**** Error 0x%x returned by Start (pAudioClient)\n", hr); return -1; }

	//RENDER LOOP
	hr = S_OK;
	
	// Each loop fills one of the two buffers.
    while ( flags != AUDCLNT_BUFFERFLAGS_SILENT)
    {
	    // Wait for next buffer event to be signaled.
        DWORD retval = WaitForSingleObject(clientStream->hEvent, 2000);
        if (retval != WAIT_OBJECT_0)
        {
            // Event handle timed out after a 2-second wait.
            //hr = clientStream->audioClient->Stop();
            hr = CALL(Stop, clientStream->audioClient);
			hr = ERROR_TIMEOUT;
			break;
		}

		//printf("Requesting %d buffer samples...\n", clientStream->bufferFrameCount);

		// Grab the next empty buffer from the audio device.
	    //hr = clientStream->renderClient->GetBuffer(clientStream->bufferFrameCount, &pData);
		hr = CALL(GetBuffer, clientStream->renderClient, clientStream->bufferFrameCount, &((BYTE*)(interleavedBuffer.mData)) );
	
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
		clientStream->renderCallback(&bufferList, clientStream->bufferFrameCount, NULL);

		/*
		//Fill the buffer with source material

		UINT64 playbackSampleOffset = 0;
		UINT64 samplesToCopy = 0;
		UINT64 remainingSamples =0;

		samplesToCopy = clientStream->bufferFrameCount;// * 3 / 4; ;
		remainingSamples = g_sourceBufferLengthInSamples - playbackSampleOffset;//audioEvent.audioData.size() - playbackSampleOffset;

		if(  remainingSamples > 0 && remainingSamples < samplesToCopy )
			samplesToCopy = remainingSamples;
		if( remainingSamples < 1 )
			break;
		
		//printf("Requesting %d buffer samples...\n", clientStream->bufferFrameCount);

		//Debug output
		//printf("samplesToCopy = %d\n", samplesToCopy);
		//printf("remainingSamples = %d\n", remainingSamples);		
		//printf("playbackSampleOffset = %d\n", playbackSampleOffset);

		//memcpy(pData, &(audioEvent.audioData[playbackSampleOffset]), samplesToCopy );
		memcpy(pData, &(g_sineWaveBuffer[playbackSampleOffset*g_sourceBufferChannels]), samplesToCopy*g_sineBufferChannels*sizeof(float));
	    //memcpy(pData, &(g_sourceBuffer[playbackSampleOffset*g_sourceBufferChannels*g_sourceBytesPerSample]), samplesToCopy*g_sourceBufferChannels*g_sourceBytesPerSample );		
		playbackSampleOffset += samplesToCopy;

		if(playbackSampleOffset >= g_sourceBufferLengthInSamples )
			break;
		*/

		//hr = clientStream->renderClient->ReleaseBuffer(clientStream->bufferFrameCount, flags);
	    hr = CALL(ReleaseBuffer, clientStream->renderClient, clientStream->bufferFrameCount, flags);
		if (FAILED(hr)) { printf("**** Error 0x%x returned by ReleaseBuffer\n", hr); break; }
		//pData = NULL;


    }

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

	free(g_sineWaveBuffer);
	g_sineWaveBuffer = NULL;

	return 0;
}

void PBAMasterRenderCallback(struct PBABufferList * ioData, uint32_t frames, const struct PBATimeStamp * timestamp)
//^(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp) {
{
	//UINT64 playbackSampleOffset = 0;
	uint32_t samplesToCopy = 0;
	uint32_t remainingSamples =0;
	uint32_t bufferIndex = 0;

//#ifdef DEBUG
	//printf("PBAMasterRenderCallback\n");
	//AEManagedValueRealtimeThreadIdentifier = pthread_self();
//#endif
        
	//Michael Tyson uses this to do some pthread locking... pthread locks are bullshit
	//Never ever lock a real-time thread!!!
	//AEManagedValueCommitPendingUpdates();
        

	//If you aren't rendering, wtf do you have a render callback?
	//Eliminate redundant conditionals on the real-time thread!!
	//__unsafe_unretained AERenderer * renderer = (__bridge AERenderer*)AEManagedValueGetValue(rendererValue);
	//if ( renderer ) 
	//{
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

		//Fill the buffer with source material
		samplesToCopy = frames;//clientStream->bufferFrameCount;// * 3 / 4; ;
		remainingSamples = (uint32_t)(g_sourceBufferLengthInSamples - g_playbackSampleOffset);//audioEvent.audioData.size() - playbackSampleOffset;
		if(  remainingSamples > 0 && remainingSamples < samplesToCopy )
			samplesToCopy = remainingSamples;

		//Debug output
		//printf("samplesToCopy = %d\n", samplesToCopy);
		//printf("remainingSamples = %d\n", remainingSamples);		
		//printf("g_playbackSampleOffset = %d\n", g_playbackSampleOffset );

#ifdef __APPLE__  
		if( g_playbackSampleOffset < g_sineBufferLengthInSamples)
		{
			for(bufferIndex=0; bufferIndex< ioData->mNumberBuffers; bufferIndex++)
			{
				float * fBuffer = (float*)ioData->mBuffers[bufferIndex].mData;
				memcpy(fBuffer, &(g_sineWaveBuffer[g_playbackSampleOffset]), frames * sizeof(float));
			}
			g_playbackSampleOffset += frames;
		}
		//for(int i = 0; i<frames; i++)
		//    fBuffer[i] = 1.0f;
#elif defined(_WIN32)

		if( remainingSamples > 0 )
		{
			for(bufferIndex=0; bufferIndex< ioData->mNumberBuffers; bufferIndex++)
			{
				float * fBuffer = (float*)ioData->mBuffers[bufferIndex].mData;
				memcpy(fBuffer, &(g_sineWaveBuffer[g_playbackSampleOffset*g_sourceBufferChannels]), samplesToCopy * g_sineBufferChannels * sizeof(float));
			}

			//memcpy(pData, &(audioEvent.audioData[playbackSampleOffset]), samplesToCopy );
			//memcpy(pData, &(g_sineWaveBuffer[g_playbackSampleOffset*g_sourceBufferChannels]), samplesToCopy*g_sineBufferChannels*sizeof(float));
			//memcpy(pData, &(g_sourceBuffer[playbackSampleOffset*g_sourceBufferChannels*g_sourceBytesPerSample]), samplesToCopy*g_sourceBufferChannels*g_sourceBytesPerSample );		
			g_playbackSampleOffset += samplesToCopy;

			//if(playbackSampleOffset >= g_sourceBufferLengthInSamples )
			//	break;
		}

#endif

		//AERenderContextOutput      
#ifdef DEBUG
		PBAStreamReportRenderTime(&_PBAMasterStream, &_audioReport, PBASecondsFromHostTicks(PBACurrentTimeInHostTicks() - start), (double)frames / _PBAMasterStream.currentSampleRate);
#endif
	//} else {
		//PBABufferListSilence(ioData, 0, frames);
	//}
 }//;

 
int main(int argc, const char * argv[]) {
 
	//HRESULT hr;
	WAVEFORMATEX desiredStreamFormat;// = NULL;		//stream buffer format

	//cr_sound_event audioEvent = {0};

	//Source Buffer
	unsigned long Frequency;
    uint64_t renderDataLenghtInSamples;

	//Process and Thread Handles
	HANDLE pID = 0;
	HANDLE threadID;

	//Elevate Process And Thread Priorities
	GetProcessId(pID);
	threadID = GetCurrentThread();
	SetPriorityClass(pID, REALTIME_PRIORITY_CLASS);
	SetThreadPriority(threadID, THREAD_PRIORITY_TIME_CRITICAL);
	
    
   /***
    * 0     Do any necessary platform initializations
    ***/
    InitPlatform();

   /***
    * 1     Init [Pb]Audio Stream for Playback
	***/
	//Initalize COM and allocate a Device Enumerator object
	//So [Pb]Audio can create a stream against a hardware device
	//TO DO:  How to abstract device setup in cross-platform manner?
#ifdef WIN32
	PBAInitCOM();

	//Get the Default or Desired Audio Hardware Device Endpoint + Active a context to the device if needed
	PBAGetDefaultAudioDevice(&(_PBAMasterStream.audioDevice));
	PBAActivateAudioDevice(_PBAMasterStream.audioDevice, &(_PBAMasterStream.audioClient));
	
	//Send the desired format to PBAInitStream if we wish to use an exclusive mode stream on Win32
	//or a non-default format on Darwin platforms
	desiredStreamFormat.wFormatTag = WAVE_FORMAT_PCM;//((WAVEFORMATEXTENSIBLE*)pwfx)->SubFormat;
	desiredStreamFormat.nChannels = 2;//pwfx->nChannels;
	desiredStreamFormat.nSamplesPerSec = 48000;//pwfx->nSamplesPerSec;
	desiredStreamFormat.nAvgBytesPerSec = 288000;//g_format.nSamplesPerSec * 8;//pwfx->nAvgBytesPerSec;
	desiredStreamFormat.nBlockAlign = 6;//pwfx->nBlockAlign;
	desiredStreamFormat.wBitsPerSample = 24;//pwfx->wBitsPerSample;
	desiredStreamFormat.cbSize = 0;
#endif

	//set the master render callback for the device stream
	_PBAMasterStream.renderCallback = PBAMasterRenderCallback;
       
	//We must first init the stream in order to know the sample rate if we don't request a format explicitly
    PBAInitAudioStream(&_PBAMasterStream, NULL);

	//Choose an Output Source for the Stream (Playback from PCM File)
	//g_sourceBuffer = (char*)&(audioEvent.audioData[0]);
	//g_sourceBytesPerSample = 3;//fileFormatEx->wBitsPerSample / 8;//(pwfx->nBlockAlign /pwfx->nChannels);
	//g_sourceBufferChannels = 1;//fileFormatEx->nChannels;//g_sineBufferChannels;
	//g_sourceBufferLengthInSamples = audioEvent.audioData.size() / 3;//( g_sourceBytesPerSample * g_sourceBufferChannels);
	
	//Generate an Output Source for the Stream (Playback from FLoating Point Sine Wave Buffer)
	Frequency = 440;
    renderDataLenghtInSamples = (uint64_t)_PBAMasterStream.currentSampleRate * 5;
	g_sineBufferLengthInSamples = g_sourceBufferLengthInSamples = renderDataLenghtInSamples;
	g_sineBufferChannels = g_sourceBufferChannels = 2;
	//Initialize a 32-bit floating point sine wave buffer
    GenerateSineSamplesFloat(&g_sineWaveBuffer, g_sineBufferLengthInSamples, Frequency, g_sineBufferChannels, _PBAMasterStream.currentSampleRate, 0.25f, NULL);
    
	//Start the audio stream render callback to fire from a blocking loop on the current thread
	PBAStartAudioStream(&_PBAMasterStream);
	//Or start a dedicated audio rendering callback thread for the stream
	//_beginthreadex(NULL, 0, pbaudio_stream_render, &_PBAMasterStream, 0, &(_PBAMasterStream.renderThreadID));
    

   /***
    * 2     Init Core Render
    *
    * --  Create a CRView (ie an OS Platform Window backed by a Accelerated Graphics Context) with its own dedicated event queue thread
    * --  Create an [OpenGL] Accelerated Graphics Context to backup the platform window with its own dedicated render thread
    * --  Start Kernel Level Display Sync Event Callback/Process to slave each window's dedicated display loop or render callback if desired
    * --  On iOS/tvOS these things will occur through UIApplication and Cocoa will notify us of views being created so we can render to them
    ***/
#if !defined(CR_TARGET_IOS) && !defined(CR_TARGET_TVOS)
    //init_crgc_views();      //configure crgc_view options as desired before creating platform window backed by an accelerated graphics context
    //create_crgc_views();    //create platform window and graphics context
#endif
    
    
    /***
     * 3  If we did not create per view display sync threads above, create per display threads/processes here:
     *     Start Kernel Level Display Sync Event Thread/Process that posts update events to a kqueue
     *     so that all threads may register to listen on the queue to respond to display sync events from a single source
     ***/
    //StartDisplaySyncThread();
    /*
    CreateDisplaySyncMessageQueue();
#if !defined(CR_TARGET_IOS) && !defined(CR_TARGET_TVOS)
    cr_displaySyncProcess = StartDisplaySyncProcess(CR_REALTIME_PRIORITY_CLASS);      //Launches crTimeServer background process to vBlank events associated with the display
#endif
    */
    
   /***
    *  3  Launch an HID Event poll on dedicated thread
    ***/
    //CreateHIDEventHandlers(kHIDUsage_GD_Keyboard, &(glView[0]));

   /***
    *  4  Register for a IPC/notification API
    *
    *  Option 5.1:  CoreFoundation CFNotificationCenter
    *
    *  Despite managing all window related UI, we still rely on Cocoa to:
    *
    *  1.  load application info.pist and bundle resources,
    *  2.  Setup Application Notifications using NSNotification Center
    *  3.  Create Application Menus
    *  4.  Code Signing (only a Cocoa app can get into an App Store and may be restricted by entitlements)
    *  5.  Create an NSApplication and start its run loop to drive the message pump with [NSEvent nextEvent:]
    *
    *  We can slowly try to migrate away from any reliance on Cocoa by taking care of these things ourselves in C with the help of CoreFoundation,
    *  but it is ultimately a huge boon to be able to add the objective c-runtime and have these methods to call into it from C!
    *  What is also really cool is that we can communicate between our Pure Obj-C and Pure C walls by using messaging mechanisms like
    *  NSNotificationCenter/CFNotificationCenter
    ***/
#ifdef CR_TARGET_OSX
     RegisterNotificationObservers();
#endif
    /***
     *  5 Start the Application Event or Simulation Event Loop on a dedicated thread!
     *
     *  Depending on the platform, we may have several options:
     ***/
    
    /***
     *  5.1  Launch Core Render C-Land Application Event Loop
     *
     *  Launch a dedicated thread that will act as arbiter for our Core Render Application Domain
     *  Responsibilities include:
     *
     *  --  Pulling CGEvents from a kqueue bsd mechanism that are sent from the CRApplication.m run loop in CocoaLand
     *  --  Managing the application state
     ***/
    //InitApplicationEventLoop();

   /***
    *  5.2:  Start the Platform Mandated Event Loop on the Main Thread!
    *
    *  On Win32, this just starts a runloop on the main thread that waits for events with Get/PeekMessage API
    *
    *  On OSX, this is functionally similar to calling the following in main.m file which will start the platform main event loop
    *  via [NSApplication run]:
    *
    *      void NSApplicationMain(int argc, char *argv[]) {
    *      [NSApplication sharedApplication];
    *      [NSBundle loadNibNamed:@"myMain" owner:NSApp];
    *      [NSApp run];
    *
    ***/
    //Start the Win32/Cocoa Main Event Message pump on a Run Loop on the Main Application Thread
    StartPlatformEventLoop(argc, argv);
    //CFRunLoopRun();
    
    /***
     *  6 Cleanup!  Cleanup should be done appropriately on each thread and the application
     *              should wait until all are finished.  Perhaps with a semaphore.
     ***/

    //1  Destroy Graphics Context for crgc_view
    //2  Destroy Platform Window for each crgc_view
    //3  Shutdown event queues, file descriptors, pipes and threads
    
    //  3.1  Destroy the displaySyncProcess
    //TerminateDisplaySyncProcess(cr_displaySyncProcess);
    //TerminateProcess( disp_sync_proc_info.hProcess, 1);

    //4  Remove IPC and Notification Observers
}
