//
//  main.m
//  [Pb] Audio
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//



#ifdef __APPLE__
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

#endif //__APPLE__

#include "[Pb]Audio/[Pb]Audio.h"


// Init/Start API
static float * g_sineWaveBuffer = NULL;
static uint64_t g_sineBufferLengthInSamples;
static uint32_t g_sineBufferChannels;
static uint64_t g_playbackSampleOffset = 0;

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


void InitPlatform()
{
    //cr_appEventQueue = 0;
#ifdef CR_TARGET_WIN32
    //cr_mainThread = GetCurrentThread();
    //cr_mainThreadID = GetCurrentThreadId();
#elif defined(CR_TARGET_OSX)
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


#pragma mark -- Main

int StartPlatformEventLoop(int argc, const char * argv[])
{
#ifdef CR_TARGET_WIN32
    MSG msg;
    int exitStatus;
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
            //observe our custom defined quit message
            if( msg.message == CR_PLATFORM_EVENT_MSG_LOOP_QUIT)
            {
                fprintf(stdout, "\nCR_PLATFORM_EVENT_MSG_LOOP_QUIT\n");
                appIsRunning = false;
            }
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



static void GenerateSineSamplesFloat(float **Buffer, size_t BufferLengthInSamples, unsigned long Frequency, unsigned short ChannelCount, unsigned long SamplesPerSecond, double Amplitude, double * InitialTheta)
{
    double sampleIncrement = (Frequency * (M_PI*2)) / (double)SamplesPerSecond;
    
    double theta = (InitialTheta!=NULL) ? *InitialTheta : 0;
    
    //allocate a buffer
    *Buffer = (float*)malloc( BufferLengthInSamples * ChannelCount * sizeof(float));
    
    for(size_t i = 0; i<BufferLengthInSamples; i++)
    {
        
        double sinValue = Amplitude * sin(theta);
        for(size_t j=0;j<ChannelCount;j++)
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




int main(int argc, const char * argv[]) {
 
    //cr_mesh_vbo * vertexBuffer;
    //int * indexBuffer;
    //cr_mesh_parse_obj_to_vbo(&vertexBuffer, &indexBuffer);
    //free(vertexBuffer);
    //free(indexBuffer);
    //return 0;
    
    //const char * pathToFile = "/Users/jmoulton/Development/svn/CoreRender/assets/models/Inanimate/Shapes/UnitCube/UnitCubeMayaExport.abc";
    //cr_mesh_parse_abc_to_vbo(pathToFile);
    //return 0;
    
   /***
    * 0     Do any necessary platform initializations
    ***/
    InitPlatform();


     //__unsafe_unretained AEAudioUnitOutput * weakSelf = self;
    g_renderContext.renderCallback = ^(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp) {
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
                    memcpy(fBuffer, &(g_sineWaveBuffer[g_playbackSampleOffset]), frames * sizeof(float));
                }
                g_playbackSampleOffset += frames;
            }
            //for(int i = 0; i<frames; i++)
            //    fBuffer[i] = 1.0f;
        
            
        
            //AERenderContextOutput
        
        
        
            #ifdef DEBUG
        PBAStreamReportRenderTime(&g_renderContext, &_audioReport, PBASecondsFromHostTicks(PBACurrentTimeInHostTicks() - start), (double)frames / g_renderContext.currentSampleRate);
            #endif
        //} else {
            //PBABufferListSilence(ioData, 0, frames);
        //}
    };
    
    PBAInitAudioStream(&g_renderContext);
    unsigned long Frequency = 440;
    uint64_t renderDataLenghtInSamples = (uint64_t)g_renderContext.currentSampleRate * 5;
    g_sineBufferLengthInSamples = renderDataLenghtInSamples;
    //Initialize a 32-bit floating point sine wave buffer
    GenerateSineSamplesFloat(&g_sineWaveBuffer, g_sineBufferLengthInSamples, Frequency, 1, g_renderContext.currentSampleRate, 1.f, NULL);
     
    PBAStartAudioStream(&g_renderContext);
    
    
   /***
    * 1     Init Core Render
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
     * 2  If we did not create per view display sync threads above, create per display threads/processes here:
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
