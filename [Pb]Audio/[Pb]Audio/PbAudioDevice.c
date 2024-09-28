//
//  PbAudioDevice.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/10/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

#ifdef _WIN32
#include <assert.h>
#endif

//Private
volatile PBAudioDevice _AudioDevices[PBA_MAX_DEVICES]     = {0};
volatile char          _DeviceNames[PBA_MAX_DEVICES][128] = {0};
volatile uint32_t      _DeviceCount                       =  0;

#ifdef _WIN32
IMMDeviceEnumerator*   _PBADeviceEnumerator   = NULL;
#endif


#ifdef __APPLE__
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

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioRegisterDeviceListeners(struct PBADeviceNotificationClient* notificationClient, void* context)
{
    OSStatus status = 0;
    
    //Note:  The client streamContext passed to PbAudio.Init() is passed as Listener inClientData property0
    //TO DO: Error Checking?
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){kAudioHardwarePropertyDefaultInputDevice,  kAudioObjectPropertyScopeGlobal}, PBAudioDeviceDefaultInputChanged,     context);
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal}, PBAudioDeviceDefaultOutputChanged,    context);
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){kAudioHardwarePropertyDevices,             kAudioObjectPropertyScopeGlobal}, PBAudioDeviceAvailableDevicesChanged, context);
    
    return status;
}

#else

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioRegisterDeviceListeners(struct PBADeviceNotificationClient * notificationClient, void* context)
{
    OSStatus result = 0;
    
    notificationClient->context = context;

    result = CALL(RegisterEndpointNotificationCallback, _PBADeviceEnumerator, notificationClient);
    if (FAILED(result)) { printf("**** Error 0x%x returned by RegisterEndpointNotificationCallback\n", result); assert(1 == 0);  }

    return result;
}


OSStatus PBAudioDeviceInitCOM()
{
    HRESULT hr;
    if (!_PBADeviceEnumerator)
    {
        //CoIntialize(Ex) Initializes the COM library for use by the calling thread, sets the thread's concurrency model, and creates a new apartment for the thread if one is required.
        //You should call Windows::Foundation::Initialize to initialize the thread instead of CoInitializeEx if you want to use the Windows Runtime APIs or if you want to use both COM and Windows Runtime components. 
        //Windows::Foundation::Initialize is sufficient to use for COM components.
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by CoInitializeEx\n", hr); assert(1 == 0);  return hr; }
    }

    if (_PBADeviceEnumerator) CALL(Release, _PBADeviceEnumerator); _PBADeviceEnumerator = NULL;

    //Create MMDeviceEnumerator so we can get an audio endpoint
    //CoCreateInstance Creates a single uninitialized object of the class associated with a specified CLSID.
    //Call CoCreateInstance when you want to create only one object on the local system. 
    //To create a single object on a remote system, call the CoCreateInstanceEx function. To create multiple objects based on a single CLSID, call the CoGetClassObject function.
    hr = CoCreateInstance(__clsid(MMDeviceEnumerator), NULL, CLSCTX_ALL, __riid(IMMDeviceEnumerator), (void**)&_PBADeviceEnumerator);
    if (FAILED(hr)) { fprintf(stderr, "**** Error 0x%x returned by CoCreateInstance\n", hr); assert(1 == 0); }
   
    return hr;
}


#define PBA_COM_RELEASE(punk) if ((punk) != NULL) { CALL(Release, punk); (punk) = NULL; }


#endif


#pragma mark -- Get Devices

PB_AUDIO_API PB_AUDIO_INLINE PBAudioDevice PBAudioStreamOutputDevice(PBAStreamContext* streamContext)
{
    OSStatus result;

#ifdef __APPLE__
    // Set the render callback
    AURenderCallbackStruct rcbs = { .inputProc = PBAudioStreamSubmitBuffers, .inputProcRefCon = (void*)streamContext };//(__bridge void *)(self) };
    result = AudioUnitGetProperty(streamContext->audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, 0, &rcbs, sizeof(rcbs));
    if ( !PBACheckOSStatus(result, "AudioUnitSetProperty(kAudioUnitProperty_SetRenderCallback)") ) {
        //if ( error ) *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:result userInfo:@{ NSLocalizedDescriptionKey: @"Unable to configure output render" }];
        fprintf(stderr, "Unable to configure output render\n");
        return result;
    }
#else

#endif

    return 0;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDefaultDevice(AudioObjectPropertySelector selector, PBAudioDevice* pDevice)
{
    OSStatus result;
    //PBAudioDevice deviceID;
    uint32_t size = sizeof(PBAudioDevice);

#ifdef __APPLE__
    AudioObjectPropertyAddress addr = {selector, kAudioObjectPropertyScopeGlobal, 0};
    
    result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &size, pDevice);
    
    if ( !PBACheckOSStatus(result, "kAudioHardwarePropertyDefaultOutputDevice") || *pDevice == kAudioObjectUnknown )
    {
        fprintf(stderr, ", Unable to get default audio unit output device\n"); assert(1==0);
        //return nil;
    }
#else
    //HRESULT hr = gEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, pDevice);
    result = CALL(GetDefaultAudioEndpoint, _PBADeviceEnumerator, eRender, eConsole, pDevice);
    if (FAILED(result)) { printf("**** Error 0x%x returned by GetDefaultAudioEndpoint\n", result); assert(1 == 0);  return result; }
#endif

    return result;
}

#ifdef _WIN32

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioActivateDevice(IMMDevice* device, IAUDIOCLIENT** audioClient)
{
    //Active a version 1 Aucio Client
#ifdef IAUDIOCLIENT3
    HRESULT hr = CALL(Activate, device, __riid(IAudioClient3), CLSCTX_ALL, NULL, (void**)audioClient);
#else
    HRESULT hr = CALL(Activate, device, __riid(IAudioClient2), CLSCTX_ALL, NULL, (void**)audioClient);
#endif

    if (FAILED(hr)) { printf("**** Error 0x%x returned by Activate\n", hr); assert(1 == 0); }
    return hr;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBADefaultFormatForDevice(PBAudioDevice device, PBAStreamFormat** ppFormat)
{
    HRESULT hr; PROPVARIANT prop = {0}; IPropertyStore* store = NULL;
    hr = CALL(OpenPropertyStore, device, STGM_READ, &store);           if (FAILED(hr)) { assert(1 == 0); }
    hr = CALL(GetValue, store, &PKEY_AudioEngine_DeviceFormat, &prop); if (FAILED(hr)) { assert(1 == 0); }
    *ppFormat = (PBAStreamFormat*)prop.blob.pBlobData;
    return hr;
}

static void LPWSTR_2_CHAR(LPWSTR in_char, LPSTR out_char, size_t str_len)
{
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, in_char, -1, out_char, str_len, NULL, NULL);
}
#endif

PB_AUDIO_API PB_AUDIO_INLINE PBAudioDeviceList PBAudioAvailableDevices(void)
{
    OSStatus result;
    uint32_t deviceListSize = 0;
    uint32_t deviceCount    = 0;
#ifdef __APPLE__
    //Get the Size of the device list
    AudioObjectPropertyAddress deviceListAddr = {kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal};
    result = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &deviceListAddr, 0, NULL, &deviceListSize);
    if ( !PBACheckOSStatus(result, "kAudioHardwarePropertyDevices") )
    {
        fprintf(stderr, ", Unable to get PropertyDataSize(kAudioHardwarePropertyDevices)\n");
        //return nil;
        assert(1==0);

    }
    
    deviceCount = deviceListSize / sizeof(AudioDeviceID); assert( deviceCount <= PBA_MAX_DEVICES);
    //AudioObjectID * deviceIDs = (AudioObjectID*)malloc(deviceListSize);
    
    result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &deviceListAddr, 0, NULL, &deviceListSize, (void*)_AudioDevices);
    if ( !PBACheckOSStatus(result, "kAudioHardwarePropertyDevices") )
    {
        fprintf(stderr, ", Unable to get PropertyData(kAudioHardwarePropertyDevices)\n");
        //free(deviceIDs);
        //return nil;
        assert(1==0);
    }
#else

    IMMDeviceCollection* deviceCollection = NULL;

    //Obtain Device Collection
    result = CALL(EnumAudioEndpoints, _PBADeviceEnumerator, eRender, DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED, &deviceCollection);
    if (FAILED(result)) { fprintf(stderr, "**** Error 0x%x returned by IMMDeviceEnumerator::EnumAudioEndpoints\n", result); assert(1 == 0);  }

    //Query Device Collection Count
    result = CALL(GetCount, deviceCollection, &deviceCount);
    if (FAILED(result)) { fprintf(stderr, "**** Error 0x%x returned by IMMDeviceCollectin::GetCount\n", result); assert(1 == 0); }

    UINT deviceIndex = 0;

    //clear any existing memory
    for (deviceIndex; deviceIndex < PBA_MAX_DEVICES; deviceIndex++)
    {
        if (_AudioDevices[deviceIndex])
        {
            //Release old IMMDevice*
            PBA_COM_RELEASE(_AudioDevices[deviceIndex]);
        }
    }

    //Get each IMMDevice*
    for (deviceIndex=0; deviceIndex < deviceCount; deviceIndex++)
    {
        result = CALL(Item, deviceCollection, deviceIndex, &_AudioDevices[deviceIndex]); assert(_AudioDevices[deviceIndex]);
        if (FAILED(result)) { fprintf(stderr, "**** Error 0x%x returned by IMMDeviceCollection::Item\n", result); assert(1 == 0); }
    }

    //Release Collection
    PBA_COM_RELEASE(deviceCollection)

#endif  

    _DeviceCount = deviceCount;
    
    //free(deviceIDs);
    return (PBAudioDeviceList){(const PBAudioDevice*)&_AudioDevices[0], _DeviceCount};
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceID(PBAudioDevice deviceID, char* id, uint32_t* idLen)
{
    OSStatus result;
#ifdef WIN32
    LPWSTR* pwszID = NULL;
    IPropertyStore* pProps = NULL;
    PROPVARIANT   varName;

    // Get the endpoint ID string.
    result = CALL(GetId, deviceID, &pwszID);
    if (FAILED(result)) { fprintf(stderr, "**** Error 0x%x returned by IMMDevice::GetId\n", result); assert(1 == 0); }

    //copy unicode string to bytes
    size_t w_len = wcslen(pwszID);
    wcscpy((LPWSTR*)id, pwszID);
    *idLen = w_len;

    fprintf(stdout, "\nPBAudioDeviceID: %S\n", pwszID);
#else
    assert(1==0);
#endif
    return result;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceName(PBAudioDevice deviceID, char * deviceName, uint32_t * nameLen)
{
    OSStatus result;
#ifdef __APPLE__
    AudioObjectPropertyAddress propertyAddress = {
            kAudioDevicePropertyDeviceName,
            kAudioObjectPropertyScopeOutput,
            kAudioObjectPropertyElementMain
        };
    
    //UInt32 len = 128;
    //char devName[len];
    
    result = AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, NULL, nameLen, deviceName);
    if (result != noErr)
    {
        //fprintf(stderr, "device name get error");
        assert(1==0);
    }
#else

    LPWSTR* pwszID         = NULL;
    IPropertyStore* pProps = NULL;
    PROPVARIANT   varName;

    // Get the endpoint ID string.
    result = CALL(GetId, deviceID, &pwszID);
    if (FAILED(result)) { fprintf(stderr, "**** Error 0x%x returned by IMMDevice::GetId\n", result); assert(1 == 0); }

    result = CALL(OpenPropertyStore, deviceID, STGM_READ, &pProps);
    if (FAILED(result)) { fprintf(stderr, "**** Error 0x%x returned by IMMDevice::OpenPropertyStore\n", result); assert(1 == 0); }

    // Initialize container for property value.
    PropVariantInit(&varName);

    // Get the endpoint's friendly-name property.
    result = CALL(GetValue, pProps, &PKEY_Device_FriendlyName, &varName);
    if (FAILED(result)) { fprintf(stderr, "**** Error 0x%x returned by IPropertyStore::GetValue\n", result); assert(1 == 0); }

    // GetValue succeeds and returns S_OK if PKEY_Device_FriendlyName is not found.
    // In this case vartName.vt is set to VT_EMPTY.      
    if (varName.vt != VT_EMPTY)
    {
        size_t w_len = wcslen(varName.pwszVal);
        // Print unicode endpoint friendly name and endpoint ID.
        //fprintf(stdout, "Endpoint: \"%S\" (%S)\n", varName.pwszVal, pwszID);

        //convert a unicode string that contains only ascii to UTF-8
        //memset(deviceName, '\0', w_len * sizeof(char));
        //LPWSTR_2_CHAR(varName.pwszVal, deviceName, w_len);        
        //*nameLen = strlen(deviceName);
    
        //copy unicode string to bytes
        wcscpy((LPWSTR*)deviceName, varName.pwszVal);
        *nameLen = w_len;
    }

    CoTaskMemFree(pwszID);
    pwszID = NULL;
    PropVariantClear(&varName);
    PBA_COM_RELEASE(pProps)
    //PBA_COM_RELEASE(pEndpoint)
#endif

    return result;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceInputChannels(PBAudioDevice deviceID, char * deviceName, uint32_t * nameLen)
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
    int nMonoOutputs = 0;
    //kAudioDevicePropertyStreamConfiguration returns the stream configuration of the device in an
    //AudioBufferList (with the buffer pointers set to NULL) which describes
    //the list of streams and the number of channels in each stream

#ifdef __APPLE__
    int i = 0; UInt32 propertySize = 0;
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
#else

#endif

    return nMonoOutputs;
}

//TO DO:  return error code, accept return value as input parameter
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceNominalSampleRate(PBAudioDevice deviceID, AudioObjectPropertyScope scope, double* sampleRate)
{
    OSStatus status = 0;

#ifdef __APPLE__
    UInt32 propertySize = sizeof(Float64);
    AudioObjectPropertyAddress srPropertyAddress = {kAudioDevicePropertyNominalSampleRate, scope, kAudioObjectPropertyElementMain};
    status = AudioObjectGetPropertyData(deviceID, &srPropertyAddress, 0, nil, &propertySize, sampleRate);
    if ( !PBACheckOSStatus(status, "kAudioDevicePropertyNominalSampleRate") )
    {
        fprintf(stderr, ", Unable to get PropertyData(kAudioDevicePropertyNominalSampleRate)\n");
        //return nil;
        assert(1==0);

    }
#else

#endif

    return status;
}

//Read StreamConfiguration to an AudioBufferList to get # Supported Sample Rates
//TO DO:  return error code, accept return value as input parameter
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceNominalSampleRateCount(PBAudioDevice deviceID, AudioObjectPropertyScope scope, int * nSampleRates)
{
    OSStatus result = 0;
#ifdef __APPLE__
    UInt32 srListSize = 0;
    AudioObjectPropertyAddress srPropertyAddress = {kAudioDevicePropertyAvailableNominalSampleRates, scope, kAudioObjectPropertyElementMain};

    //Get the Size of the sample rate list
    AudioObjectPropertyAddress srListAddress = {kAudioDevicePropertyAvailableNominalSampleRates, scope, kAudioObjectPropertyElementMain};
    result = AudioObjectGetPropertyDataSize(deviceID, &srListAddress, 0, NULL, &srListSize);
    if ( !PBACheckOSStatus(result, "kAudioDevicePropertyAvailableNominalSampleRates") )
    {
        fprintf(stderr, ", Unable to get PropertyDataSize(kAudioDevicePropertyAvailableNominalSampleRates)\n");
        //return nil;
        assert(1==0);

    }
    
    *nSampleRates = srListSize / sizeof(AudioValueRange); //assert( deviceCount <= PBA_MAX_DEVICES);
    
#else

#endif

    return result;
}


PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceSetSampleRate(PBAudioDevice deviceID, AudioObjectPropertyScope scope, double sampleRate)
{
    OSStatus status = 0;
#ifdef __APPLE__
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
    status = AudioObjectSetPropertyData(deviceID, &srPropertyAddress, 0, nil, propertySize, &sampleRate);
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
#else

#endif

    return status;
}


PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceBufferSizeRange(PBAudioDevice deviceID, uint32_t* outMinimum, uint32_t* outMaximum)
{
    OSStatus theError = 0;

#ifdef __APPLE__
    AudioObjectPropertyAddress theAddress = { kAudioDevicePropertyBufferFrameSizeRange, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
 
    AudioValueRange theRange = { 0, 0 };
    UInt32 theDataSize = sizeof(AudioValueRange);
    theError = AudioObjectGetPropertyData(deviceID, &theAddress, 0, NULL, &theDataSize, &theRange); assert(theError == noErr);
    
    if(theError == 0)
    {
        *outMinimum = theRange.mMinimum;
        *outMaximum = theRange.mMaximum;
    }
#else

#endif

    return theError;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceBufferSize(PBAudioDevice inDeviceID, uint32_t* bufferSize)
{
    OSStatus theError = 0;
#ifdef __APPLE__
    AudioObjectPropertyAddress theAddress = { kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
 
    UInt32 frameSize = 0;
    UInt32 theDataSize = sizeof(UInt32);
    theError = AudioObjectGetPropertyData(inDeviceID, &theAddress, 0, NULL, &theDataSize, &frameSize);
    
    if(theError == 0) *bufferSize = frameSize;

#else

#endif

    return theError;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceSetBufferSize(PBAudioDevice deviceID, uint32_t bufferSize)
{
    OSStatus status = 0;
#ifdef __APPLE__
    //for each stream
    volatile bool wasRunning  = false;
    //volatile bool wasBypassed = false;

    PBAStreamContext * streamContext = &PBAudio.OutputStreams[0];
    //PBAudioStreamSetBypass(streamContext, true);
    
    uint32_t mBytesPerFrame  = streamContext->format.mBytesPerFrame;
    uint32_t mBitsPerChannel = streamContext->format.mBitsPerChannel;

    if( streamContext->audioDevice == deviceID && streamContext->audioUnit && streamContext->running)
    {
        PBAudioStreamStop(streamContext); wasRunning = true;
    }
    
    UInt32 propertySize = sizeof(UInt32);
    AudioObjectPropertyAddress bsPropertyAddress = {kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
    status = AudioObjectSetPropertyData(deviceID, &bsPropertyAddress, 0, nil, propertySize, &bufferSize);
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
        AudioFormatFlags flags = streamContext->format.mFormatFlags;
        PBAudioStreamUpdateFormat(streamContext, streamContext->currentSampleRate);
        
        //HACK: After setting the buffer size on the device, the audio unit seems to forget its buffer was non-interleaved...
        streamContext->format.mFormatFlags = flags;
        
        //Similarly, mBytesPerFrame changed from 4 to 8 (it should have always been 8) so it is unreliable ... use bitsPerChannel insteasd
        //assert(streamContext->format.mBytesPerFrame  == mBytesPerFrame);
        assert(streamContext->format.mBitsPerChannel == mBitsPerChannel);
        
        streamContext->target = PBAStreamFormatGetType(&streamContext->format); //enumerate a sample packing protocol for the given format



        if ( wasRunning ) PBAudioStreamStart(streamContext);

    }
    
    //PBAudioStreamSetBypass(streamContext, wasBypassed);

#else

#endif

    return status;
}
