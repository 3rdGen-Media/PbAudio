//
//  PbAudioDevice.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/10/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

//Private
volatile PBAudioDevice _AudioDevices[PBA_MAX_DEVICES]     = {0};
volatile char          _DeviceNames[PBA_MAX_DEVICES][128] = {0};
volatile UInt32        _DeviceCount                       =  0;

const CFStringRef kPBADeviceDefaultInputChangedNotification  = CFSTR("PBADeviceDefaultInputDeviceChangedNotification");
const CFStringRef kPBADeviceDefaultOutputChangedNotification = CFSTR("PBADeviceDefaultOutputDeviceChangedNotification");
const CFStringRef kPBADevicesAvailableChangedNotification    = CFSTR("PBADeviceAvailableDevicesChangedNotification");

PB_AUDIO_API PB_AUDIO_INLINE  OSStatus PBAudioDeviceDefaultInputChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress *inAddresses, void *inClientData)
{
    fprintf(stdout, "PBAudioDeviceDefaultInputChanged\n");
    
    //Distribute the notification to registered PBAudio 'Engine' Process Clients
    dispatch_async(dispatch_get_main_queue(), ^{
        
        //[NSNotificationCenter.defaultCenter postNotificationName:AEAudioDeviceDefaultInputDeviceChangedNotification object:nil];
        
        // post a notification
        //CFDictionaryKeyCallBacks   keyCallbacks    = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
        //CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
        //CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
        //CFDictionaryAddValue(dictionary, CFSTR("TestKey"), CFSTR("TestValue"));
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), kPBADeviceDefaultInputChangedNotification, NULL, NULL, TRUE);
        //CFRelease(dictionary);
    });
    
    return noErr;
}

PB_AUDIO_API PB_AUDIO_INLINE  OSStatus PBAudioDeviceDefaultOutputChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress *inAddresses, void *inClientData)
{
    //Note: the inObjectID does not seem to correspond to the AudioDeviceID of the new default device :(
    fprintf(stdout, "PBAudioDeviceDefaultOutputChanged (AudioObjectID: %u)\n", inObjectID);
    
    //Distribute the notification to registered PBAudio 'Engine' Process Clients
    dispatch_async(dispatch_get_main_queue(), ^{
        
        //CFNumberRef deviceID = CFNumberCreate(NULL, kCFNumberSInt32Type, &inObjectID);

        //[NSNotificationCenter.defaultCenter postNotificationName:AEAudioDeviceDefaultInputDeviceChangedNotification object:nil];
        
        // populate an notification
        CFDictionaryKeyCallBacks   keyCallbacks    = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
        CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
        CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
        CFDictionaryAddValue(dictionary, CFSTR("DeviceStream"),   inClientData);
        //CFDictionaryAddValue(dictionary, CFSTR("OutputDeviceID"), deviceID);

        // post a notification
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), kPBADeviceDefaultOutputChangedNotification, NULL, dictionary, true);
        
        CFRelease(dictionary);
        //CFRelease(deviceID);
    });
    return noErr;
}

PB_AUDIO_API PB_AUDIO_INLINE  OSStatus PBAudioDeviceAvailableDevicesChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress *inAddresses, void *inClientData)
{
    fprintf(stdout, "PBAudioDeviceAvailableDevicesChanged\n");
    
    //Distribute the notification to registered PBAudio 'Engine' Process Clients
    dispatch_async(dispatch_get_main_queue(), ^{
        
        //[NSNotificationCenter.defaultCenter postNotificationName:AEAudioDeviceDefaultInputDeviceChangedNotification object:nil];
        
        // post a notification
        //CFDictionaryKeyCallBacks   keyCallbacks    = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
        //CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
        //CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
        //CFDictionaryAddValue(dictionary, CFSTR("TestKey"), CFSTR("TestValue"));
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), kPBADevicesAvailableChangedNotification, NULL, NULL, TRUE);
        //CFRelease(dictionary);
    });
    
    return noErr;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioRegisterDeviceListeners(PBAStreamContext* sc)
{
    OSStatus status = 0;
    
    //Note:  The client streamContext passed to PbAudio.Init() is passed as Listener inClientData property0
    //TO DO: Error Checking?
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){kAudioHardwarePropertyDefaultInputDevice,  kAudioObjectPropertyScopeGlobal}, PBAudioDeviceDefaultInputChanged,     sc);
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal}, PBAudioDeviceDefaultOutputChanged,    sc);
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){kAudioHardwarePropertyDevices,             kAudioObjectPropertyScopeGlobal}, PBAudioDeviceAvailableDevicesChanged, sc);
    
    return status;
}

#pragma mark -- Get Devices

PB_AUDIO_API PB_AUDIO_INLINE AudioDeviceID PBAudioStreamOutputDevice(PBAStreamContext* streamContext)
{
    OSStatus result;
    
    // Set the render callback
    AURenderCallbackStruct rcbs = { .inputProc = PBAudioStreamSubmitBuffers, .inputProcRefCon = (void*)streamContext };//(__bridge void *)(self) };
    result = AudioUnitGetProperty(streamContext->audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, 0, &rcbs, sizeof(rcbs));
    if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioUnitProperty_SetRenderCallback)") ) {
        //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to configure output render" }];
        fprintf(stderr, "Unable to configure output render\n");
        return result;
    }
    
    return 0;
}

PB_AUDIO_API PB_AUDIO_INLINE AudioDeviceID PBAudioDefaultDevice(AudioObjectPropertySelector selector)
{
    AudioDeviceID deviceID;
    UInt32 size = sizeof(deviceID);
    AudioObjectPropertyAddress addr = {selector, kAudioObjectPropertyScopeGlobal, 0};
    
    OSStatus result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &size, &deviceID);
    
    if ( !PBACheckOSStatus(result, "kAudioHardwarePropertyDefaultOutputDevice") || deviceID == kAudioObjectUnknown )
    {
        fprintf(stderr, ", Unable to get default audio unit output device\n");
        //return nil;
    }
    
    //AEAudioDevice * device = [[AEAudioDevice alloc] initWithObjectID:deviceId];
    //device.isDefault = YES;
    return deviceID;
}

PB_AUDIO_API PB_AUDIO_INLINE PBAudioDeviceList PBAudioAvailableDevices(void)
{
    UInt32 deviceListSize = 0;
    
    //Get the Size of the device list
    AudioObjectPropertyAddress deviceListAddr = {kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal};
    OSStatus result = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &deviceListAddr, 0, NULL, &deviceListSize);
    if ( !PBACheckOSStatus(result, "kAudioHardwarePropertyDevices") )
    {
        fprintf(stderr, ", Unable to get PropertyDataSize(kAudioHardwarePropertyDevices)\n");
        //return nil;
        assert(1==0);

    }
    
    UInt32 deviceCount = deviceListSize / sizeof(AudioDeviceID); assert( deviceCount <= PBA_MAX_DEVICES);
    //AudioObjectID * deviceIDs = (AudioObjectID*)malloc(deviceListSize);
    
    result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &deviceListAddr, 0, NULL, &deviceListSize, (void*)_AudioDevices);
    if ( !PBACheckOSStatus(result, "kAudioHardwarePropertyDevices") )
    {
        fprintf(stderr, ", Unable to get PropertyData(kAudioHardwarePropertyDevices)\n");
        //free(deviceIDs);
        //return nil;
        assert(1==0);
    }

    _DeviceCount = deviceCount;
    
    //free(deviceIDs);
    return (PBAudioDeviceList){(const PBAudioDevice*)&_AudioDevices[0], _DeviceCount};
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceName(PBAudioDevice deviceID, char * deviceName, UInt32 * nameLen)
{
    AudioObjectPropertyAddress propertyAddress = {
            kAudioDevicePropertyDeviceName,
            kAudioObjectPropertyScopeOutput,
            kAudioObjectPropertyElementMain
        };
    
    //UInt32 len = 128;
    //char devName[len];
    
    OSStatus result = AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, NULL, nameLen, deviceName);
    if (result != noErr)
    {
        //fprintf(stderr, "device name get error");
        assert(1==0);
    }
    
    return result;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceInputChannels(PBAudioDevice deviceID, char * deviceName, UInt32 * nameLen)
{
    OSStatus status = 0;
    
    /*
    // Get input channels
    propertyAddress.mSelector = kAudioDevicePropertyStreamConfiguration
    propertyAddress.mScope = kAudioDevicePropertyScopeInput
    status = AudioObjectGetPropertyDataSize(deviceID, &propertyAddress, 0, nil, &propertySize)
    if status == noErr {
        let bufferListPointer = UnsafeMutablePointer<AudioBufferList>.allocate(capacity: 1)
        defer { bufferListPointer.deallocate() }
        status = AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, nil, &propertySize, bufferListPointer)
        if status == noErr {
            let bufferList = UnsafeMutableAudioBufferListPointer(bufferListPointer)
            for buffer in bufferList {
                inputChannels += Int(buffer.mNumberChannels)
            }
        }
    }
    */
    
    return status;
}

//Read StreamConfiguration to an AudioBufferList to get # Ouptut Channels
PB_AUDIO_API PB_AUDIO_INLINE int PBAudioDeviceChannelCount(PBAudioDevice deviceID, AudioObjectPropertyScope scope)
{
    //kAudioDevicePropertyStreamConfiguration returns the stream configuration of the device in an
    //AudioBufferList (with the buffer pointers set to NULL) which describes
    //the list of streams and the number of channels in each stream

    int i = 0; int nMonoOutputs = 0; UInt32 propertySize = 0;
    AudioObjectPropertyAddress scPropertyAddress = {kAudioDevicePropertyStreamConfiguration, scope, kAudioObjectPropertyElementMain};
    
    // Get output channels
    OSStatus status = AudioObjectGetPropertyDataSize(deviceID, &scPropertyAddress, 0, nil, &propertySize); assert(status == noErr);
    
    //int numberOfBuffers   = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? audioFormat.mChannelsPerFrame : 1;
    //int channelsPerBuffer = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? 1 : audioFormat.mChannelsPerFrame;
    //int bytesPerBuffer = audioFormat.mBytesPerFrame * frameCount;
    
    AudioBufferList *bufferList = malloc(sizeof(AudioBufferList)); /* + (numberOfBuffers-1)*sizeof(AudioBuffer));*/ assert(bufferList);
    //audio->mNumberBuffers = numberOfBuffers;
    
    status = AudioObjectGetPropertyData(deviceID, &scPropertyAddress, 0, nil, &propertySize, bufferList); assert(status == noErr);
    
    for ( i=0; i<bufferList->mNumberBuffers; i++ ) nMonoOutputs += bufferList->mBuffers[i].mNumberChannels;
    
    
    //TO DO: Manage pool of bufferLists that can be used for Device/Stream Configuration queries
    free(bufferList);
    
    return nMonoOutputs;
}

//TO DO:  return error code, accept return value as input parameter
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceNominalSampleRate(PBAudioDevice deviceID, AudioObjectPropertyScope scope, Float64* sampleRate)
{
    UInt32 propertySize = sizeof(Float64);
    AudioObjectPropertyAddress srPropertyAddress = {kAudioDevicePropertyNominalSampleRate, scope, kAudioObjectPropertyElementMain};
    OSStatus status = AudioObjectGetPropertyData(deviceID, &srPropertyAddress, 0, nil, &propertySize, sampleRate);
    if ( !PBACheckOSStatus(status, "kAudioDevicePropertyNominalSampleRate") )
    {
        fprintf(stderr, ", Unable to get PropertyData(kAudioDevicePropertyNominalSampleRate)\n");
        //return nil;
        assert(1==0);

    }
    
    return status;
}

//Read StreamConfiguration to an AudioBufferList to get # Supported Sample Rates
//TO DO:  return error code, accept return value as input parameter
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceNominalSampleRateCount(PBAudioDevice deviceID, AudioObjectPropertyScope scope, int * nSampleRates)
{
    UInt32 srListSize = 0;
    AudioObjectPropertyAddress srPropertyAddress = {kAudioDevicePropertyAvailableNominalSampleRates, scope, kAudioObjectPropertyElementMain};

    //Get the Size of the sample rate list
    AudioObjectPropertyAddress srListAddress = {kAudioDevicePropertyAvailableNominalSampleRates, scope, kAudioObjectPropertyElementMain};
    OSStatus result = AudioObjectGetPropertyDataSize(deviceID, &srListAddress, 0, NULL, &srListSize);
    if ( !PBACheckOSStatus(result, "kAudioDevicePropertyAvailableNominalSampleRates") )
    {
        fprintf(stderr, ", Unable to get PropertyDataSize(kAudioDevicePropertyAvailableNominalSampleRates)\n");
        //return nil;
        assert(1==0);

    }
    
    *nSampleRates = srListSize / sizeof(AudioValueRange); //assert( deviceCount <= PBA_MAX_DEVICES);
    return result;
}


PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceSetSampleRate(PBAudioDevice deviceID, AudioObjectPropertyScope scope, Float64 sampleRate)
{
    //Note: Explicitly stopping and starting the stream does not seem to be necessary for sample rate and buffer size changes
    //      and results in additonal warnings 'HALB_IOThread.cpp:326    HALB_IOThread::_Start: there already is a thread'
    volatile bool wasRunning = false;
    PBAStreamContext * streamContext = &PBAudio.OutputStreams[0];
    PBAudioStreamSetBypass(streamContext, true);
    if( streamContext->audioDevice == deviceID && streamContext->audioUnit && streamContext->running)
    {
        //Note: We want to stop the audio unit stream before changing the sample rate on the device
        //      but we want PBAudioStreamUpdateFormat to restart the audio unit stream when we feed it a new sample rate
        PBAudioStreamStop(streamContext); wasRunning = true; //streamContext->running = true;
    }


    UInt32 propertySize = sizeof(Float64);
    AudioObjectPropertyAddress srPropertyAddress = {kAudioDevicePropertyNominalSampleRate, scope, kAudioObjectPropertyElementMain};
    OSStatus status = AudioObjectSetPropertyData(deviceID, &srPropertyAddress, 0, nil, propertySize, &sampleRate);
    if ( !PBACheckOSStatus(status, "kAudioDevicePropertyNominalSampleRate") )
    {
        fprintf(stderr, ", Unable to set PropertyData(kAudioDevicePropertyNominalSampleRate)\n");
        assert(1==0);
    }
    
    /*
    Float64 deviceSampleRate = 0.0;
    while( memcmp(&sampleRate, &deviceSampleRate, sizeof(Float64)) != 0)
    {
        //fprintf(stdout, "Waiting for Sample Rate Change to Complete...");
        PBAudioDeviceNominalSampleRate(deviceID, scope, &deviceSampleRate);
    }
    */
    
    if( streamContext->audioDevice == deviceID && streamContext->audioUnit )
    {
        //wasRunning = streamContext->running; streamContext->running = false;
        PBAudioStreamUpdateFormat(streamContext, sampleRate);
        if ( wasRunning ) PBAudioStreamStart(streamContext);
    }
    PBAudioStreamSetBypass(streamContext, false);
    return status;
}


PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceBufferSizeRange(PBAudioDevice deviceID, UInt32* outMinimum, UInt32* outMaximum)
{
    AudioObjectPropertyAddress theAddress = { kAudioDevicePropertyBufferFrameSizeRange, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
 
    AudioValueRange theRange = { 0, 0 };
    UInt32 theDataSize = sizeof(AudioValueRange);
    OSStatus theError = AudioObjectGetPropertyData(deviceID, &theAddress, 0, NULL, &theDataSize, &theRange); assert(theError == noErr);
    
    if(theError == 0)
    {
        *outMinimum = theRange.mMinimum;
        *outMaximum = theRange.mMaximum;
    }
    
    return theError;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceBufferSize(PBAudioDevice inDeviceID, UInt32* bufferSize)
{
    AudioObjectPropertyAddress theAddress = { kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
 
    UInt32 frameSize = 0;
    UInt32 theDataSize = sizeof(UInt32);
    OSStatus theError = AudioObjectGetPropertyData(inDeviceID, &theAddress, 0, NULL, &theDataSize, &frameSize);
    
    if(theError == 0) *bufferSize = frameSize;
    return theError;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceSetBufferSize(PBAudioDevice deviceID, UInt32 bufferSize)
{
    //for each stream
    volatile bool wasRunning = false;
    PBAStreamContext * streamContext = &PBAudio.OutputStreams[0];
    if( streamContext->audioDevice == deviceID && streamContext->audioUnit && streamContext->running)
    {
        PBAudioStreamStop(streamContext); wasRunning = true;
    }
    
    UInt32 propertySize = sizeof(UInt32);
    AudioObjectPropertyAddress bsPropertyAddress = {kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
    OSStatus status = AudioObjectSetPropertyData(deviceID, &bsPropertyAddress, 0, nil, propertySize, &bufferSize);
    if ( !PBACheckOSStatus(status, "kAudioDevicePropertyBufferFrameSize") )
    {
        fprintf(stderr, ", Unable to set PropertyData(kAudioDevicePropertyBufferFrameSize)\n");
        //return nil;
        assert(1==0);
    }
    
    // Set the maximum frames per slice to render
    //result = AudioUnitSetProperty(streamContext->audioUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &PBABufferStackMaxFramesPerSlice, sizeof(PBABufferStackMaxFramesPerSlice));
    //if( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioUnitProperty_MaximumFramesPerSlice)") ) assert(1==0);

    if( streamContext->audioDevice == deviceID && streamContext->audioUnit )
    {
        PBAudioStreamUpdateFormat(streamContext, streamContext->currentSampleRate);
        if ( wasRunning ) PBAudioStreamStart(streamContext);
    }

    return status;
}
