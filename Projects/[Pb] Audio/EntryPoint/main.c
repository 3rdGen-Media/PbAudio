//
//  main.m
//  [Pb] Audio
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//


#include "PbAudioAppInterface.h"

#pragma mark -- [Pb]Audio Stream Output Pass

#include "../ToneGenerator.h"
#include "../SamplePlayer.h"

SamplePlayer  samplePlayer  = {0};
ToneGenerator toneGenerator = {0};

#ifdef __APPLE__
PBAStreamOutputPass TestOutputPass = ^(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp)
#else
void CALLBACK TestOutputPass(struct PBABufferList* ioData, uint32_t frames, const struct PBATimeStamp* timestamp)
#endif
{
    ToneGeneratorRenderPass(ioData, frames, timestamp, &toneGenerator, NULL, 0);
};

#ifdef __APPLE__
PBAStreamOutputPass SamplerOutputPass = ^(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp)
#else
void CALLBACK SamplerOutputPass(struct PBABufferList* ioData, uint32_t frames, const struct PBATimeStamp* timestamp)
#endif
{
    //local trigger event iterator/cache
    int nTriggerEvents = 0;
    CMTriggerMessage* triggerEvent = NULL;
    CMTriggerMessage  triggerEvents[MAX_TRIGGER_EVENTS];

    // Clearing the output buffer is critical for DSP routines unless such routines inherently overwrite the buffer
    PBABufferListSilence(ioData, 0, frames);

#ifdef CR_TARGET_WIN32
    //...
#else
    struct kevent kev[MAX_TRIGGER_EVENTS];
        
    //wait without timeout for trigger input kevent from our kqueue
    struct timespec _ts; struct timespec *ts = NULL;
    ts = &_ts; ts->tv_sec = 0; ts->tv_nsec = 0;
    
    //read axis control events + button control events from the control queue
    nTriggerEvents = kevent(CMTriggerEventQueue, NULL, 0, &kev[0], MAX_TRIGGER_EVENTS, ts);
    //while (nTriggerEvents > 0)
    {
        int i; for(i=0;i<nTriggerEvents;i++)
        {
            if (kev[i].filter == EVFILT_USER)
            {
                //cache trigger event values we just pulled from the queue
                triggerEvent = (CMTriggerMessage*)(kev[i].udata);
                
                //TO DO: cache the event in the note:articulation map
                //trigger_event_frame.events[kev[i].ident] = *controlEvent; //cache previous events of type
                
                //TO DO: count # of trigger events for each articulation
                
                //Unpack/Process the Triggered Note
                uint8_t note     = CMNoteNumberFromEventWord(triggerEvent->word);
                uint8_t velocity = CMNoteVelocityFromEventWord(triggerEvent->word);
                fprintf(stdout, "\nTrigger Event (%lu) Note = %u (%u)\n", kev[i].ident, note, velocity);
                
            }
        }
        
        //Note: renderpasses must follow this loop if they reschedule events
        //nTriggerEvents = kevent(CMTriggerEventQueue, NULL, 0, &kev[0], MAX_TRIGGER_EVENTS, ts);
        //assert(nTriggerEvents == 0); //ensure we aren't overwriting trigger events wrt the current frame buffer size
    }
    
#endif
        
    SamplePlayerRenderPass(ioData, frames, timestamp, &samplePlayer, triggerEvent, 1);
};

PBAStreamOutputPass _Nullable OutputPass[MaxOutputPassID] = {0};


#pragma mark -- CMidi Device Messages

#ifdef __APPLE__
MIDINotifyBlock CMidiNotifyBlock = ^void(const MIDINotification *msg)
{
    //for debugging, trace change notifications:
    const char *descr[] = {
        "undefined (0)",
        "kMIDIMsgSetupChanged",
        "kMIDIMsgObjectAdded",
        "kMIDIMsgObjectRemoved",
        "kMIDIMsgPropertyChanged",
        "kMIDIMsgThruConnectionsChanged",
        "kMIDIMsgSerialPortOwnerChanged",
        "kMIDIMsgIOError"};

    fprintf(stdout, "\nMIDI Notify, messageID %d (%s)\n", (int) msg->messageID, descr[(int) msg->messageID]);
    
    if( msg->messageID == kMIDIMsgObjectAdded || msg->messageID == kMIDIMsgObjectRemoved )
    {
        MIDIObjectAddRemoveNotification* addedOrRemovedMsg = (MIDIObjectAddRemoveNotification*)msg;
        fprintf(stdout, "\nkMIDIMsgObjectAdded/kMIDIMsgObjectRemoved/: %d %d\n", addedOrRemovedMsg->parentType, addedOrRemovedMsg->childType);

        //Checking type of parent entity will inform whether source/destination endpoints have been added/removed from CoreMIDI
        //Whether added or removed, it doesn't really matter if the entire list is updated
        if( addedOrRemovedMsg->parentType == kMIDIObjectType_Entity)
        {
            CFStringRef notificationID = NULL;
            
                 if(addedOrRemovedMsg->childType == kMIDIObjectType_Source)      { CMUpdateInputDevices();  notificationID = kCMidiSourcesAvailableChangedNotification;      }
            else if(addedOrRemovedMsg->childType == kMIDIObjectType_Destination) { CMUpdateOutputDevices(); notificationID = kCMidiDestinationsAvailableChangedNotification; }
            else assert(1==0);//unhandled case
            
            //TO DO:  Distribute notification to client processes to tell them to update their input or output device list
            CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), notificationID, NULL, NULL, true);
        }
        
    }

    /*
    else if( msg->messageID == kMIDIMsgThruConnectionsChanged )
    {
        //CMidi should have already udpated its internal data structures causing this change
        //but that doesn't necessarily have to be the case if other apps are also making thru connections

        //Distribute updates to the UI
        //[[PbAudioApplicationDelegate sharedInstance] updateThruConnectionWidgets];
    }
    */

    return;

};


MIDIReceiveBlock CMidiReceiveBlock = ^void(const MIDIEventList *evtlist, void * __nullable srcConnRefCon)
{
    //OSStatus cmError;
#ifdef MIDI_DEBUG
    fprintf(stdout, "\nCMidiReceiveBlock\n");
#endif
    
    CMConnection* connection = (CMConnection*)srcConnRefCon;
    //NSString* sourceEndpointUniqueIDKey = [NSString stringWithFormat:@"%d", connection->source.uniqueID];// sourceEndpointUniqueID];
    
    //Find the DOM of the device determined by the sourcedEndpointUniqueID
    //Find †he DOM of the Soft Thru Connection associated with the same device
    //CMHardwareDevice     * MCUDevice      = [CMDevice.dictionary objectForKey:sourceEndpointUniqueIDKey];
    //CMSoftThruConnection * ThruConnection = [CMSoftThru.dictionary objectForKey:sourceEndpointUniqueIDKey];
    //CMProxyConnection    * ProxyConnection = CMProxy.documents.firstObject; //objectForKey:sourceEndpointUniqueIDKey];

    //assert(MCUDevice);
    //assert(ThruConnection);
        
    //MIDIEventListForEachEvent
    if (evtlist->numPackets > 0) //&& msgQueue)
    {
        const MIDIEventPacket * packet = &evtlist->packet[0];
        for (int i = 0; i < evtlist->numPackets; ++i)
        {
            
            for (int wordIndex = 0; wordIndex < packet->wordCount; ++wordIndex)
            {
                // Shift the message by 28 bits to get the message type nibble.
                MIDIMessageType messageType = packet->words[wordIndex] >> 28;
                
                //To get only the status nibble, shift by 20 bits (the start position of the status)
                //and then perform an AND operation to clear the message type and group nibbles.
                MIDICVStatus status = (packet->words[wordIndex] >> 20) & 0x00f;
                
#ifdef MIDI_DEBUG
                //Log Message for Debugging
                //NSMutableString * hexString = [NSMutableString string];
                //for(int wordIndex = 0; wordIndex<packet->wordCount; wordIndex++) [hexString appendString:[NSString stringWithFormat:@"%08x,", packet->words[wordIndex]]];

                fprintf(stdout, "\n------------------------------------");
                fprintf(stdout, "\nUniversal MIDI Packet nBytes:  %d", packet->wordCount);
                fprintf(stdout, "\nData:  ");
                /*for(int wordIndex = 0; wordIndex<packet->wordCount; wordIndex++)*/ fprintf(stdout, "%08x,", packet->words[wordIndex]);
                fprintf(stdout, "\n");
                fprintf(stdout, "\nMessageType:  %d = %s (%s)", messageType, CMStringForMIDIMessageType(messageType), CMStringForMIDICVStatus(status));
                fprintf(stdout, "\nEndpoint ID:  %d\n", connection->source.uniqueID);
#endif
                
                //process MIDI Event packets on input port manually
                //unpack event and queue commands for midi note audio source modules
                if (messageType == CMMessageTypeChannelVoice1 || messageType == CMMessageTypeChannelVoice2)
                {
                    if( status == CMMessageCVStatusNoteOn)
                    {
                        uint8_t note     = CMNoteNumberFromEventWord(packet->words[wordIndex]);
                        //uint8_t velocity = CMNoteVelocityFromEventWord(packet->words[wordIndex]);
                        
                        //Don't modify queue parameters on the audio source module directly to generate audio...
                        //ToneGeneratorSetFrequency(&toneGenerator, WESTERN_NOTE_FREQUENCIES[note], toneGenerator.sampleRate);

                        //Distribute the message to a realtime audio thread that can be dequeued on next buffer cycle
                        //Client can decide which granularity to use for the queue's message filter during queue creation
                        CMTriggerMessage * triggerMessage = &trigger_events[cm_trigger_event_index++]; cm_trigger_event_index = cm_trigger_event_index % MAX_TRIGGER_EVENTS;
                        
                        triggerMessage->timestamp         = packet->timeStamp;
                        triggerMessage->cursor.offset     = 0;
                        triggerMessage->word              = packet->words[wordIndex];
                        
                        struct kevent kev;
                        EV_SET(&kev, note, EVFILT_USER, 0, NOTE_TRIGGER, 0, triggerMessage);
                        kevent(CMTriggerEventQueue, &kev, 1, NULL, 0, NULL);
                        
                        //fprintf(stdout, "\nTrigger Note = %u (%u)\n", note, velocity);
                        
                    }
                    //MidiNote *note = [[MidiNote alloc] initWithPacket:packet];
                    
                }
            }
            
            packet = MIDIEventPacketNext(packet);
        }
    }
};

#else

#endif



#ifdef __APPLE__

#pragma mark -- CFNotification Center Notifications

void mainWindowChangedNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo) {
    
    fprintf(stdout, "\nMain Window Changed!\n");
    
    CFShow(CFSTR("Received notification (dictionary): ")); CFShow(name);
    assert(object);
    assert(userInfo);
    // print out user info
    const void * keys; const void * values;
    CFDictionaryGetKeysAndValues(userInfo, &keys, &values);
    for (int i = 0; i < CFDictionaryGetCount(userInfo); i++) {
        const char * keyStr = CFStringGetCStringPtr((CFStringRef)&keys[i],   CFStringGetSystemEncoding());
        const char * valStr = CFStringGetCStringPtr((CFStringRef)&values[i], CFStringGetSystemEncoding());
        fprintf(stdout, "\t\t \"%s\" = \"%s\"\n", keyStr, valStr);
    }
    
}

void NSApplicationDidBecomeActiveNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    fprintf(stdout, "\nNSApplicationDidBecomeActiveNotificationCallback\n");
}

void NSApplicationDidResignActiveNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    fprintf(stdout, "\nNSApplicationDidResignActiveNotificationCallback\n");
}

#pragma mark -- PBAudioDevice Notification Observer Callbacks

static void PBAudioDeviceDefaultOutputChangedNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    AudioDeviceID outputDeviceID; PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &outputDeviceID);
    
    assert(userInfo);

    PBAStreamContext * DeviceStream = (PBAStreamContext*)CFDictionaryGetValue(userInfo, CFSTR("DeviceStream"));     assert(DeviceStream);
    //CFNumberRef        DeviceRef    =       (CFNumberRef)CFDictionaryGetValue(userInfo, CFSTR("OutputDeviceID")); assert(DeviceRef);
    //BOOL success = CFNumberGetValue( DeviceRef, kCFNumberSInt32Type, &outputDeviceID); assert(success);
    
    fprintf(stdout, "\nPBAudioDeviceDefaultOutputChangedNotificationCallback (AudioDeviceID: %u)\n", outputDeviceID);
 
    //Check if the current stream is using the default device
    //If it is then we have discretion here to redirect the audio to the new default
    if ( DeviceStream->respectDefault )
    {
        // Replace audio device with updated system default device
        //weakSelf.audioDevice = weakSelf.outputEnabled ? AEAudioDevice.defaultOutputAudioDevice : AEAudioDevice.defaultInputAudioDevice;
        PBAudio.SetOutputDevice(DeviceStream, outputDeviceID); //This will stop and restart the audio unit attached to the stream context
    }
    
}

static void PBAudioStreamFormatChangedNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    AudioDeviceID outputDeviceID; PBAudioDefaultDevice(kAudioHardwarePropertyDefaultOutputDevice, &outputDeviceID);
    
    assert(userInfo);

    PBAStreamContext * DeviceStream = (PBAStreamContext*)CFDictionaryGetValue(userInfo, CFSTR("DeviceStream"));     assert(DeviceStream);
    //CFNumberRef        DeviceRef    =       (CFNumberRef)CFDictionaryGetValue(userInfo, CFSTR("OutputDeviceID")); assert(DeviceRef);
    //BOOL success = CFNumberGetValue( DeviceRef, kCFNumberSInt32Type, &outputDeviceID); assert(success);
    
    fprintf(stdout, "\nPBAudioStreamFormatChangedNotificationCallback (AudioDeviceID: %u)\n", outputDeviceID);
 
}

static void PBAudioStreamSampleRateChangedNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    assert(userInfo);

    PBAStreamContext * DeviceStream = (PBAStreamContext*)CFDictionaryGetValue(userInfo, CFSTR("DeviceStream"));     assert(DeviceStream);
    //CFNumberRef        DeviceRef    =       (CFNumberRef)CFDictionaryGetValue(userInfo, CFSTR("OutputDeviceID")); assert(DeviceRef);
    //BOOL success = CFNumberGetValue( DeviceRef, kCFNumberSInt32Type, &outputDeviceID); assert(success);
    
    fprintf(stdout, "\nPBAudioStreamSampleRateChangedNotificationCallback (Sample Rate: %lu)\n", (unsigned int)DeviceStream->currentSampleRate);
 
    //Modify Application State Based on Notification Here:
    ToneGeneratorSetFrequency(&toneGenerator, toneGenerator.freq, DeviceStream->currentSampleRate);
    
    //TO DO:  Reload audio files from source with conversion to new sample rate format
}


static void PBAudioDevicesAvailableChangedNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    
    fprintf(stdout, "\nPBAudioDevicesAvailableChangedNotificationCallback\n");
    
    /*
    [NSNotificationCenter.defaultCenter addObserverForName:AEAudioDeviceAvailableDevicesChangedNotification object:nil queue:nil usingBlock:^(NSNotification * note) {
        NSArray <AEAudioDevice *> * availableDevices = AEAudioDevice.availableAudioDevices;
        if ( ![availableDevices containsObject:weakSelf.audioDevice] ) {
            // Replace audio device with new default if device disappears
            weakSelf.audioDevice = weakSelf.outputEnabled ? AEAudioDevice.defaultOutputAudioDevice : AEAudioDevice.defaultInputAudioDevice;
        }
    }];
    */
}

static void CMidiSourcesAvailableChangedNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    fprintf(stdout, "\nCMidiSourcesAvailableChangedNotificationCallback\n");
}

static void CMidiDestinationsAvailableChangedNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    fprintf(stdout, "\nCMidiDestinationsAvailableChangedNotificationCallback\n");
}


static void RegisterNotificationObservers(void)
{
    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    assert(center);
    
    // add an observer
    CFNotificationCenterAddObserver(center, NULL, NSApplicationDidBecomeActiveNotificationCallback,
                                    CFSTR("NSApplicationDidBecomeActiveNotification"), NULL,
                                    CFNotificationSuspensionBehaviorDeliverImmediately);
    
    CFNotificationCenterAddObserver(center, NULL, NSApplicationDidResignActiveNotificationCallback,
                                    CFSTR("NSApplicationDidResignActiveNotification"), NULL,
                                    CFNotificationSuspensionBehaviorDeliverImmediately);
    
    CFNotificationCenterAddObserver(center, NULL, mainWindowChangedNotificationCallback,
                                    CFSTR("CGWindowDidBecomeMainNotification"), NULL,
                                    CFNotificationSuspensionBehaviorDeliverImmediately);
    
    //[Pb]Audio Device Notifications
    
    //__weak typeof(self) weakSelf = self;
    //self.defaultDeviceObserverToken =
    CFNotificationCenterAddObserver(center, NULL, PBAudioDeviceDefaultOutputChangedNotificationCallback,
                                    CFSTR("PBADeviceDefaultOutputDeviceChangedNotification"), //_PBAMasterStream->outputEnabled ? kPBADeviceDefaultOutputChangedNotification : kPBADeviceDefaultInputChangedNotification,
                                    NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    

    //[Pb]Audio Stream Config Notifications

    //self.deviceAvailabilityObserverToken =
    CFNotificationCenterAddObserver(center, NULL, PBAudioDevicesAvailableChangedNotificationCallback,
                                    CFSTR("PBADeviceAvailableDevicesChangedNotification"),
                                    NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    
    CFNotificationCenterAddObserver(center, NULL, PBAudioStreamSampleRateChangedNotificationCallback,
                                    CFSTR("PBAudioStreamSampleRateChangedNotification"),
                                    NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    
    
    //CMidi Device Config Notifications

    CFNotificationCenterAddObserver(center, NULL, CMidiSourcesAvailableChangedNotificationCallback,
                                    CFSTR("CMidiSourcesAvailableChangedNotification"),
                                    NULL, CFNotificationSuspensionBehaviorDeliverImmediately);

    CFNotificationCenterAddObserver(center, NULL, CMidiDestinationsAvailableChangedNotificationCallback,
                                    CFSTR("CMidiDestinationsAvailableChangedNotification"),
                                    NULL, CFNotificationSuspensionBehaviorDeliverImmediately);

    
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



void InitPlatform()
{
    //cr_appEventQueue = 0;
#ifdef _WIN32
    //cr_mainThread = GetCurrentThread();
    //cr_mainThreadID = GetCurrentThreadId();
    //WAVEFORMATEX desiredStreamFormat;// = NULL;        //stream buffer format

    HANDLE pID = 0;
    HANDLE threadID;

    //Elevate Process And Thread Priorities
    pID = (HANDLE)GetProcessId(GetCurrentProcess()); threadID = GetCurrentThread();
    SetPriorityClass(pID, REALTIME_PRIORITY_CLASS);
    //SetThreadPriority(threadID, THREAD_PRIORITY_TIME_CRITICAL);
    
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

void ExitHandler(void)
{
    fprintf(stderr, "Exiting via Exit Handler...\n");

    //TO DO: PbAudio Cleanup
    //TO DO: CMidi Cleanup

    //CMidiThruCleanup();
    
    //pause for leak tracking
    //fscanf(stdin, "c");
}

int StartPlatformEventLoop(int argc, const char * argv[])
{
    fprintf(stdout, "\nStartPlatformEventLoop");

#if defined(_WIN32) //Vanilla Run Loop

    /*
    MSG msg;
    //int exitStatus;
    bool appIsRunning = true;

    //will just idle for now (in a multiwindow scenario this is best)
    //_idleEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    while (appIsRunning)
    {
        //filter for events we would like to be notified of in the message queue, much like a kqueue or [NSApplication nextEventWith:]
        //Use GetMessage to wait until the message arrives, or PeekMessage to return immediately
        //if(PeekMessage(&msg, NULL, CR_PLATFORM_EVENT_MSG_LOOP_QUIT, CR_PLATFORM_EVENT_MSG_LOOP_QUIT, PM_REMOVE))
        if(GetMessage(&msg, NULL, 0, 0) && msg.message != WM_QUIT)  //run the event loop until it spits out error or quit
        {
			//pass on the messages to the __main_event_queue for processing
			//TranslateMessage(&msg);
			DispatchMessage(&msg);
            memset(&msg, 0, sizeof(MSG));
        }
    }
    */

    DemoApp app;

    if (SUCCEEDED(app.Initialize()))
    {
        app.RunMessageLoop();
    }
    else assert(1 == 0);


    //TO DO: Cleanup();
    
    return 0;
#elif defined(__APPLE__) //Cocoa Run Loop

#if TARGET_OS_OSX
    atexit(&ExitHandler);
    NSApplicationMain(argc, argv);       //Cocoa Application Event Loop
#else //#if TARGET_OS_IOS || TARGET_OS_TVOS

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

#endif
#endif

    fprintf(stdout, "\nStartPlatformEventLoop() End\n");
    return -1;
}



void PBAudioInit(void)
{
    int i = 0;

    //Process and Thread Handles
    PBAStreamFormat desiredStreamFormat;
#ifdef WIN32
    
    //Send the desired format to PBAInitStream if we wish to use an exclusive mode stream on Win32
    //or a non-default format on Darwin platforms
    desiredStreamFormat.wFormatTag      = WAVE_FORMAT_PCM;//((WAVEFORMATEXTENSIBLE*)pwfx)->SubFormat;
    desiredStreamFormat.nChannels       = 2;//pwfx->nChannels;
    desiredStreamFormat.nSamplesPerSec  = 48000;//pwfx->nSamplesPerSec;
    desiredStreamFormat.nAvgBytesPerSec = 288000;//g_format.nSamplesPerSec * 8;//pwfx->nAvgBytesPerSec;
    desiredStreamFormat.nBlockAlign     = 6;//pwfx->nBlockAlign;
    desiredStreamFormat.wBitsPerSample  = 24;//pwfx->wBitsPerSample;
    desiredStreamFormat.cbSize          = 0;
#elif defined(__APPLE__)
    //AudioStreamBasicDescription audioDescription;
    memset(&desiredStreamFormat, 0, sizeof(desiredStreamFormat));
    desiredStreamFormat.mFormatID          = kAudioFormatLinearPCM;
    desiredStreamFormat.mFormatFlags       = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked |kAudioFormatFlagIsNonInterleaved;
    desiredStreamFormat.mChannelsPerFrame  = 2;
    desiredStreamFormat.mBytesPerPacket    = sizeof(float);
    desiredStreamFormat.mFramesPerPacket   = 1;
    desiredStreamFormat.mBytesPerFrame     = sizeof(float);
    desiredStreamFormat.mBitsPerChannel    = 8 * sizeof(float);
    desiredStreamFormat.mSampleRate        = 48000;
#endif

    //A stream must first be initialized in order to know the system sample rate setting of the device if a compatible format isn't requested explicitly
    PBAudio.Init(&PBAudio.OutputStreams[0], NULL, kAudioObjectUnknown, SamplerOutputPass);
    
    //Load some audio from disk while converting to the desired format
    const char * audioFileURL = "/Users/jmoulton/Music/iTunes/iTunes Media/Music/Unknown Artist/Unknown Album/Print#45.1.aif";//Assets/DecadesMix\0";
    const char * audioFileExt = "aif\0";
    
    //const char * audioFileURL = "/Users/jmoulton/Music/iTunes/iTunes Media/Music/ArticulationLayers/65 Drum Samples/56442_Surfjira_Snare_HeadShot_Hard.wav";
    //const char * audioFileExt = "wav\0";
        
    ToneGeneratorInit(&toneGenerator, 440.f, PBAudio.OutputStreams[0].currentSampleRate);           //Initialize a 32-bit floating point sine wave buffer
    SamplePlayerInit(&samplePlayer, audioFileURL, audioFileExt, PBAudio.OutputStreams[0].format);   //Read an audio file from disk to formatted buffer for playback
    
    OutputPass[TestOutputPassID]    = TestOutputPass;
    OutputPass[SamplerOutputPassID] = SamplerOutputPass;
    
}

//must return void* in order to use with GCD dispatch_async_f
static void* CRRunLoop(void* opaqueQueue)
{   
    //fprintf("\ninit app_event_loop\n");
#ifdef CR_TARGET_WIN32
    //Currently does nothing
#elif defined(__APPLE__)
    int eventQueue = (int)opaqueQueue;
    while (1)
    {
        struct kevent kev;
        crevent_type kev_type;

        //idle until we receive kevent from our kqueue
        kev_type = (crevent_type)pba_event_queue_wait_with_timeout(eventQueue, &kev, EVFILT_USER, crevent_timeout, crevent_out_of_range, crevent_init, crevent_graphics, UINT_MAX);

        if( kev_type == crevent_platform_event  )
        {
             //cr_platform_event_msg cgEvent = (cr_platform_event_msg)(kev.udata);
             //handle_platform_event(cgEvent);
        }
        else if( kev_type == crevent_main_window_changed )
        {
             //CGWindowID windowID = (CGWindowID)(kev.udata);
             //cr_mainWindow = windowID;
             //fprintf(stderr, "\ncrevent_main_window_changed %lld\n", cr_mainWindow);
        }
        else if( kev_type == crevent_init)
        {
             //init_crgc_views();      //configure crgc_view options as desired before creating platform window backed by an accelerated graphics context
             //create_crgc_views();    //create platform window and graphics context
        }
        else if( kev_type == crevent_register_view )
        {
             fprintf(stderr, "\ncrevent_register_view\n");
             
             //get the pointer to the crgc_view that lives in Cocoa Land
             //__block crgc_view * view = (crgc_view*)(kev.udata);
             
             //create crgc_view and launch mach event listener on deicated view control thread
             //dispatch_queue_attr_t attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INTERACTIVE, 0);
             //view->controlThread = dispatch_queue_create(view->title, attr);
             //dispatch_async(view->controlThread, ^{ create_crgc_view(view); });

             //create_crgc_view(view);
             //create_crgc_view_display_loop(view);
        }
        else if( kev_type == crevent_graphics )
        {
             fprintf(stderr, "crevent_graphics\n");
             //_paintingTexture = *((GLuint*)kev.udata);
        }
        else if( kev_type == crevent_exit)
        {
            //CRCleanup();
        }
        else if( kev_type == crevent_menu)
        {
            //handle_menu_event((crmenu_event)kev.udata);
        }
    }
#endif
    return NULL;
}

//must return void* in order to use with GCD dispatch_async_f
static void* PBAudioEventLoop(void* opaqueQueue)
{
    fprintf(stdout, "\nPBAudioEventLoop");

#ifdef CR_TARGET_WIN32
    //Currently does nothing
#elif defined(__APPLE__)
    int eventQueue = (int)opaqueQueue;
    while (1)
    {
        struct kevent kev;
        CMMessageType kev_type;
        
        //SYSEX: [F7, manufacturer id, channel id, device id, command id, param id, param value, F7]
        
        //idle until we receive kevent from our kqueue
        kev_type = (CMMessageType)pba_event_queue_wait_with_timeout(eventQueue, &kev, EVFILT_USER, CMMessageTypeTimeout, CMMessageTypeUnknownF, CMMessageTypeUtility, CMMessageTypeData128, UINT_MAX);
        CMUniversalMessage * message = (CMUniversalMessage*)kev.udata;

        switch( kev_type )
        {
            case (CMMessageTypeSystem):
            {
                fprintf(stdout, "\nCMMessageTypeSystem (status = %u)", message->system.status);

                if( message->group == pba_midi_input_connection)
                {
                        if( message->system.status == CMStatusStart) CMCreateInputConnection(message->system.deviceSelect);
                   else if( message->system.status == CMStatusStop)  CMDeleteInputConnection(message->system.deviceSelect);

                        break;
                }
            }
                
            case (CMMessageTypeData128):
            {
                fprintf(stdout, "\nCMMessageTypeData128 (status = %u)", message->system.status);
                
                if(message->system.status == CMSysExStatusComplete)
                {
                    uint8_t oemID      = message->data128.sysex8.data[0];
                    uint8_t streamID   = message->data128.sysex8.data[1];
                    uint8_t deviceID   = message->data128.sysex8.data[2];
                    uint8_t commandID  = message->data128.sysex8.data[3];
                    uint8_t paramID    = message->data128.sysex8.data[4];
                    uint8_t paramValue = message->data128.sysex8.data[5];

                    //TO DO:  Read the exclusive bytes to determine which sysex command to execute
                    if(commandID == pba_stream_change_outputpass) PBAudio.OutputStreams[streamID].outputpass = OutputPass[paramID];

                    break;

                }
                    
            }
                
            default:
                assert(1==0);
        }
    }
#endif

    return NULL;
}


void StartAudioMessageEventLoop(void)
{
    //*** [Pb]Audio ***//

    //Create IOCP port/kqueue for [Pb]Audio Application<->Process IPC
    PBAudio.eventQueue = PBAKernelQueueCreate();

    //It is not be strictly necessary to occur before but for posterity events are registered to the queue prior to initializing [Pb]Audio
#ifdef __APPLE__
    uint64_t audioEvent;

    //register kevents [for Universal Midi Protocol Message Types] that have been repurposed to remotely control the [Pb]Audio 'Engine' process
    for (audioEvent = CMMessageTypeUtility; audioEvent < CMMessageTypeTimeout; ++audioEvent)
    {
        struct kevent kev;
        EV_SET(&kev, audioEvent, EVFILT_USER, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL);
        kevent((int)PBAudio.eventQueue.kq, &kev, 1, NULL, 0, NULL);
    }
#else

#endif

    /***
     * Init [Pb]Audio [Session/Stream] for Scheduling Audio [Buffers] to a Real-Time Thread
     * 
     ***/
    PBAudioInit();

    /***
     * Start receiving audio stream buffer callbacks issued on a corresponding real-time audio thread
     * Darwin: the system provides the client access to a real-time audio thread of elevated priority [via a callback]
     * Win32:  the client is responsible for creating the real-time audio thread of elevated priority
     *
     * In both cases, the system provides the buffers that are consumed from the real-time audio thread
     ***/
#if defined(__APPLE__)
    PBAudio.Start(&PBAudio.OutputStreams[0]);
#else
     _beginthreadex(NULL, 0, (_beginthreadex_proc_type)PBAudio.Start, &PBAudio.OutputStreams[0], 0, &(PBAudio.OutputStreams[0].audioThreadID));
#endif


#ifdef CR_TARGET_WIN32

    //Standalone Engine Process Option: Launch Event Loop on Current Thread that will serve as the Main Event Loop in [Pb]Audio C-Land
    //PBAudioEventLoop((void*)PBAudio.eventQueue.kq);

    //Shared Application + Engine Process Option:  Launch a single thread that that will serve as the Main Event Loop in [Pb]Audio C-Land
    //_beginthreadex(NULL, 0, pbaudio_stream_render, &_PBAMasterStream, 0, &(_PBAMasterStream.renderThreadID));
    //PBAudio.eventQueue.kq = PBAudio.eventThreadID; //When there is no IOCP thread pool, the thread handle itself is used to access the msg queue associated with the thread
     
    //Thread Pool Option:  Launch a concurrent [IOCP] thread pool that will serve as the Main Event Loop in [Pb]Audio C - Land

#elif defined(__APPLE__)
    
    //Standalone Engine Process Option:  Launch Event Loop on Current Thread that will serve as the Main Event Loop in [Pb]Audio C-Land
    //PBAudioEventLoop((void*)PBAudio.eventQueue.kq);
    
    //Shared Application + Engine Process Option:  Launch a pthread that that will serve as the Main Event Loop in [Pb]Audio C-Land
    pthread_attr_t attr;
    struct sched_param sched_param;
    int sched_policy = SCHED_FIFO;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, sched_policy);
    sched_param.sched_priority = sched_get_priority_max(sched_policy);
    pthread_attr_setschedparam(&attr, &sched_param);
    pthread_create(&PBAudio.eventThread, &attr, PBAudioEventLoop, (void*)PBAudio.eventQueue.kq);
    pthread_attr_destroy(&attr);
    //pthread_mutex_init(&_mutex, NULL); //locks are for losers
    //pthread_setname_np("com.3rdgen.pbaudio.event-loop");

    /***
     *  Thread Pool Option:  Launch a concurrent thread pool that will serve as the Main Event Loop in [Pb]Audio C-Land
     *
     *  Its responsibilities include:
     *
     *      --Receiving events from the NSApplication run loop and forwarding them to the appropriate crgc_view window event thread/queue as necessary
     *      --Sending events and notifications back to Cocoa if needed
     *      --Managing the Core Render C-Land Application State
     ***/
    /*
     dispatch_queue_attr_t attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_CONCURRENT, QOS_CLASS_USER_INTERACTIVE, 0);
     dispatch_queue_t queue     = dispatch_queue_create(kPBAMainEventQueue, attr);
    
     //a neat feature of GCD is that it exposes a hash dictionary for each dispatch_queue_t
     //making GCD a great candidate for cross-platform threading across all platforms
     //for( int viewIndex = 0; viewIndex < NUM_VIEWS; viewIndex++) dispatch_queue_set_specific(glView[viewIndex].controlThread, (void*)(glView[viewIndex].window), &(glView[viewIndex]), NULL);
     //glView[viewIndex].controlQueue = dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0);

     dispatch_async(queue, ^{ PBAudioEventLoop((void*)PBAudio.eventQueue.kq); });
    */
    
#endif   

//*** CMidi ***//

#if defined(__APPLE__)
    //register kevents [for Midi Note On Types] that can be queued to trigger audio on the real-time audio thread
    uint64_t triggerEvent; CMTriggerEventQueue = kqueue(); //create kqueue
    for (triggerEvent = 0; triggerEvent < 128; ++triggerEvent)
    {
        struct kevent kev;
        EV_SET(&kev, triggerEvent, EVFILT_USER, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL);
        kevent(CMTriggerEventQueue, &kev, 1, NULL, 0, NULL);
    }
#else

    
#endif

    CMidi.init(CM_CLIENT_OWNER_ID, CMidiNotifyBlock, CMidiReceiveBlock, NULL);

}
 
int main(int argc, const char * argv[]) {
 
	//HRESULT hr;

    /***
     * 0  Do any necessary platform initializations
     ***/
    InitPlatform();
    
    /***
     * 1  Init Core Render
     *
     * --  Create a CRView (ie an OS Platform Window backed by a Accelerated Graphics Context) with its own dedicated event queue thread
     * --  Create an [OpenGL, Vulkan and/or DirectX] Accelerated Graphics Context to backup the platform window with its own dedicated render thread
     * --  Start Kernel Level Display Sync Event Callback/Process to slave each window's dedicated display loop or render callback if desired
     * --  On iOS/tvOS these things will occur through UIApplication and Cocoa will notify CoreRender after creating views/layers so they can be rendered to
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
     *  4  Launch a HID Event poll on dedicated thread
     ***/
     //CreateHIDEventHandlers(kHIDUsage_GD_Keyboard, &(glView[0]));

    /***
     *  5  Register for a IPC/notification API
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
     *  Launch a dedicated thread that will act as arbiter for our [Pb]Audio Application<->Process Domain
     *  Responsibilities include:
     *
     *  --  Managing the application state
     ***/
    StartAudioMessageEventLoop();

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
    StartPlatformEventLoop(argc, argv); //CFRunLoopRun();
    
    /***
     *  6 Cleanup!  Cleanup should be done appropriately on each thread and the application
     *           should wait until all are finished.  Perhaps with a semaphore.
     ***/

    //1  Destroy Graphics Context for crgc_view
    //2  Destroy Platform Window for each crgc_view
    //3  Shutdown event queues, file descriptors, pipes and threads
    
    //  3.1  Destroy the displaySyncProcess
    //TerminateDisplaySyncProcess(cr_displaySyncProcess);
    
    //4  Remove IPC and Notification Observers
    
    //Clean Up System Resource Factory(s)
    //UIReleaseResourceFactory();

}
