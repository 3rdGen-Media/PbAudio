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
    
#if TARGET_OS_OSX
    //Note:  The client streamContext passed to PbAudio.Init() is passed as Listener inClientData property0
    //TO DO: Error Checking?
    //TO DO: Remove property listeners on shutdown
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){kAudioHardwarePropertyDefaultInputDevice,  kAudioObjectPropertyScopeGlobal}, PBAudioDeviceDefaultInputChanged,     context);
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal}, PBAudioDeviceDefaultOutputChanged,    context);
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){kAudioHardwarePropertyDevices,             kAudioObjectPropertyScopeGlobal}, PBAudioDeviceAvailableDevicesChanged, context);
    
    AudioUnitAddPropertyListener( ((PBAStreamContext*)context)->audioUnit, kAudioOutputUnitProperty_CurrentDevice, PBAudioStreamDeviceChanged, context );
    
    
#else //#if TARGET_OS_IPHONE || TARGET_OS_TVOS
        
        // Watch for session interruptions
            
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


#define PBA_COM_ADDREF(punk)  if ((punk) != NULL) { CALL(AddRef, punk); }
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
   
#if TARGET_OS_OSX
    AudioObjectPropertyAddress addr = {selector, kAudioObjectPropertyScopeGlobal, 0};
    
    result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &size, pDevice);
    
    if ( !PBACheckOSStatus(result, "PBAudioDefaultDevice::kAudioHardwarePropertyDefaultInOutDevice") || *pDevice == kAudioObjectUnknown )
    {
        fprintf(stderr, ", Unable to get default audio unit device\n"); assert(1==0);
        //return nil;
    }
#else
    
#endif

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
    //Active a version 1 Audio Client
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

PB_AUDIO_API PB_AUDIO_INLINE PBAudioDeviceList PBAudioAvailableDevices(AudioObjectPropertyScope scope)
{
    OSStatus result;
    uint32_t deviceListSize = 0;
    uint32_t deviceCount    = 0;
#ifdef __APPLE__

#if TARGET_OS_OSX
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
    
    //Sometimes we can get two devices with the same name (1 with inputs only + 1 with outputs only)
    
    if( scope != kAudioObjectPropertyScopeGlobal)
    {
        deviceCount = 0;
        for( int i=0; i < deviceListSize/sizeof(AudioDeviceID); i++)
        {
            if( PBAudioDeviceChannelCount(_AudioDevices[i], scope) == 0 )
            {
                //shift to overwrite the duplicate to merge consecutive pairs with the device id
                int nShiftDevices = (deviceListSize/sizeof(AudioDeviceID))- i - 1;
                memmove(&_AudioDevices[i], &_AudioDevices[i+1], nShiftDevices * sizeof(PBAudioDevice));
                continue;
            }
            
            deviceCount++;
        }
        
    }
    

#else
    
    void* (*objc_ClassSelector)(Class class, SEL _cmd) = (void*)objc_msgSend;
    void* (*objc_InstanceSelector)(id self , SEL _cmd) = (void*)objc_msgSend;
    int   (*objc_InstanceSelectorInt)(id self , SEL _cmd) = (void*)objc_msgSend;

    id avSessionSharedInstance = objc_ClassSelector(objc_getClass("AVAudioSession"), sel_registerName("sharedInstance"));
    id currentOutputs = objc_InstanceSelector(objc_InstanceSelector(avSessionSharedInstance, sel_registerName("currentRoute")), sel_registerName("outputs"));
    
    deviceCount = objc_InstanceSelectorInt(currentOutputs, sel_registerName("count")); assert( deviceCount <= PBA_MAX_DEVICES);
    
#endif

#else

    volatile PBAudioDevice AudioDevices[PBA_MAX_DEVICES]   = {0};  //tmp stack memory device list
    volatile uint8_t       AudioDeviceMap[PBA_MAX_DEVICES] = {0};  //map indices of cached list to indices of new device list
    IMMDeviceCollection*   deviceCollection                = NULL; //temporary COM Object will be released within this function

    //Obtain Device Collection
    result = CALL(EnumAudioEndpoints, _PBADeviceEnumerator, eRender, DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED, &deviceCollection);
    if (FAILED(result)) { fprintf(stderr, "**** Error 0x%x returned by IMMDeviceEnumerator::EnumAudioEndpoints\n", result); assert(1 == 0);  }

    //Query Device Collection Count
    result = CALL(GetCount, deviceCollection, &deviceCount);
    if (FAILED(result)) { fprintf(stderr, "**** Error 0x%x returned by IMMDeviceCollectin::GetCount\n", result); assert(1 == 0); }

    UINT i, deviceIndex = 0;

    //create temporary audio device array on stack
    //PBAudioDevice* AudioDevices = (PBAudioDevice*)malloc(deviceCount * sizeof(PBAudioDevice));

    /*
    //clear any existing memory
    for (deviceIndex; deviceIndex < PBA_MAX_DEVICES; deviceIndex++)
    {
        if (_AudioDevices[deviceIndex])
        {
            //Release old IMMDevice*
            PBA_COM_RELEASE(_AudioDevices[deviceIndex]);
        }
    }
    */

    //Get each IMMDevice* from the IMMDeviceCollection
    for (deviceIndex=0; deviceIndex < deviceCount; deviceIndex++)
    {
        PBAudioDevice device = NULL;
        result = CALL(Item, deviceCollection, deviceIndex, &device); assert(device);
        if (FAILED(result)) { fprintf(stderr, "**** Error 0x%x returned by IMMDeviceCollection::Item\n", result); assert(1 == 0); }

        //Store the device retrieved from the IMMDeviceCollection in temporary stack list
        AudioDevices[deviceIndex] = device;

        //Get the name of the IMMDevice* retrieved from the IMMDevice Collection
        uint32_t idLen1 = 128; char id1[128] = "\0"; PBAudioDeviceID(AudioDevices[deviceIndex], id1, &idLen1);

        //Iterate the list of all cached IMMDevice*
        for (i=0; i < PBA_MAX_DEVICES; i++)
        {
            //If a previous cached AudioDevice at this index is present in the list...
            if (_AudioDevices[i])
            {
                //Get the name of the cached IMMDevice*
                uint32_t idLen2 = 128; char id2[128] = "\0"; PBAudioDeviceID(_AudioDevices[i], id2, &idLen2);

                //Compare the name opf the cached audio device against the name of the newly allocated device
                if (memcmp(id1, id2, idLen1) == 0) 
                { 
                    //Release the new audio device memory
                    PBA_COM_RELEASE(device);

                    //Replace with the cached audio device memory
                    AudioDevices[deviceIndex] = _AudioDevices[i];

                    //Update the map
                    AudioDeviceMap[i] = deviceIndex+1; //increment by 1 to allow check against > 0

                    //Move on to retrieve the next IMMDevice from the IMMDeviceCollection
                    break;
                }
            }
        }
    }

    //Release any cached devices whose list indices were not mapped
    for (deviceIndex = 0; deviceIndex < _DeviceCount; deviceIndex++)
    {
        //Consult the map and Release cached IMMDevice* if a mapped index is not present
        if (AudioDeviceMap[deviceIndex] == 0) PBA_COM_RELEASE(_AudioDevices[deviceIndex]);
    }

    //copy the temporary stack list to the cached list
    if( deviceCount ) memcpy(_AudioDevices, AudioDevices, deviceCount * sizeof(PBAudioDevice));

    /*
    release any cached devices that are no longer present as a result of the list growing or shrinking
    for (deviceIndex = deviceCount; deviceIndex < _DeviceCount; deviceIndex++)
    {
        if (_AudioDevices[deviceIndex])
        {
            //Release old IMMDevice*
            PBA_COM_RELEASE(_AudioDevices[deviceIndex]);
        }
    }
    */

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

    //fprintf(stdout, "\nPBAudioDeviceID: %S\n", pwszID);
#else
    assert(1==0);
#endif
    return result;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceName(PBAudioDevice deviceID, char * deviceName, uint32_t * nameLen)
{
    OSStatus result;
#ifdef __APPLE__
    
#if TARGET_OS_OSX
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
#endif
    

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

#if defined(__APPLE__) && TARGET_OS_OSX
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

#if defined(__APPLE__)
#if TARGET_OS_OSX
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
    void* (*objc_ClassSelector)(Class class, SEL _cmd) = (void*)objc_msgSend;
    Float64   (*objc_InstanceSelectorDouble)(id self , SEL _cmd) = (void*)objc_msgSend;

    id avSessionSharedInstance = objc_ClassSelector(objc_getClass("AVAudioSession"), sel_registerName("sharedInstance"));
    *sampleRate = objc_InstanceSelectorDouble(avSessionSharedInstance, sel_registerName("sampleRate"));
#endif
#endif

    return status;
}

//Read StreamConfiguration to an AudioBufferList to get # Supported Sample Rates
//TO DO:  return error code, accept return value as input parameter
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceNominalSampleRateCount(PBAudioDevice deviceID, AudioObjectPropertyScope scope, int * nSampleRates)
{
    OSStatus result = 0;
#if defined(__APPLE__) 
#if TARGET_OS_OSX
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
    Float64 sampleRate[3] = {44100., 48000., 96000.};
    *nSampleRates = 3;
#endif
#endif

    return result;
}

//TO DO:  This needs to take stream as input
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceSetSampleRate(PBAudioDevice deviceID, AudioObjectPropertyScope scope, double sampleRate)
{
    OSStatus status = 0;
    PBAStreamContext* streamContext = &PBAudio.OutputStreams[0];

#if defined(__APPLE__) 
#if TARGET_OS_OSX
    //Note: Explicitly stopping and starting the stream does not seem to be necessary for sample rate and buffer size changes
    //      and results in additonal warnings 'HALB_IOThread.cpp:326    HALB_IOThread::_Start: there already is a thread'
    volatile bool wasRunning = false;
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
    void* (*objc_ClassSelector)(Class class, SEL _cmd) = (void*)objc_msgSend;
    BOOL   (*objc_InstanceSelectorSetDouble)(id self , SEL _cmd, double d1, void* err) = (void*)objc_msgSend;

    //NSError* error = nil;
    id avSessionSharedInstance = objc_ClassSelector(objc_getClass("AVAudioSession"), sel_registerName("sharedInstance"));
    BOOL success = objc_InstanceSelectorSetDouble(avSessionSharedInstance, sel_registerName("setPreferredSampleRate:error:"), sampleRate, nil);
    //success  = [session setPreferredSampleRate:preferredSampleRate error:&error];

    if (! success) fprintf(stderr, "\nPBAudioDeviceSetSampleRate::AVAudioSession setPreferredSampleRate (%g) failed\n", sampleRate);

    double currentRate = 0;
    PBAudioDeviceNominalSampleRate(deviceID, scope, &currentRate);
    
    fprintf(stdout, "\nPBAudioDeviceSetSampleRate::AVAudioSession.sampleRate = %g\n", currentRate );
#endif
#endif

#if defined(_WIN32) 

    OSStatus result = 0;
    volatile bool wasRunning = false;

    if (streamContext->shareMode < PBA_DRIVER_VENDOR) assert(1 == 0);

    if (streamContext->audioClient)
    {
        //should never be called for WASAPI streams
        assert(1 == 0);
        wasRunning = streamContext->running;
        if (wasRunning) result = PBAudioStreamStop(streamContext);

        //Delete + Recreate IAudioClient against new device
        //CALL(Release, streamContext->audioClient);  streamContext->audioClient = NULL;
        //CALL(Release, streamContext->renderClient); streamContext->renderClient = NULL;
    }
    else if (streamContext->driver)
    {
        wasRunning = streamContext->running;

        //IASIO::stop
        if (wasRunning) result = PBAudioStreamStop(streamContext); 

        //IASIO dispose buffers, set sample rate, create buffers
        PBAudioDriverSetSampleRate(streamContext->driver, sampleRate, streamContext);

        //IASIO::start
        if (wasRunning) PBAudioStreamStart(streamContext);         
    }
    

#endif

    return status;
}


PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceBufferSizeRange(PBAudioDevice deviceID, uint32_t* outMinimum, uint32_t* outMaximum)
{
    OSStatus theError = 0;

#if defined(__APPLE__) && TARGET_OS_OSX
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
#if defined(__APPLE__) 
#if TARGET_OS_OSX
    AudioObjectPropertyAddress theAddress = { kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
 
    UInt32 frameSize = 0;
    UInt32 theDataSize = sizeof(UInt32);
    theError = AudioObjectGetPropertyData(inDeviceID, &theAddress, 0, NULL, &theDataSize, &frameSize);
    
    if(theError == 0) *bufferSize = frameSize;

#else
    void* (*objc_msgSendSharedInstance)(Class, SEL) = (void*)objc_msgSend;
    double (*objc_msgSendGetProperty)(void*, SEL) = (void*)objc_msgSend;
    
    id avSessionSharedInstance = objc_msgSendSharedInstance(objc_getClass("AVAudioSession"), sel_registerName("sharedInstance"));
    double duration = objc_msgSendGetProperty(avSessionSharedInstance, sel_getUid("IOBufferDuration"));
    double rate = 0.0; PBAudioDeviceNominalSampleRate( inDeviceID, kAudioObjectPropertyScopeOutput, &rate);
    
    double integral;
    double frac = modf(duration * rate, &integral);
    *bufferSize = (uint32_t)integral + (frac > 0.);
#endif
#endif

    return theError;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceSetBufferSize(PBAudioDevice deviceID, uint32_t bufferSize)
{
    OSStatus status = 0;
    int i=0;
#if defined(__APPLE__)
#if TARGET_OS_OSX
    

    PBAStreamContext * streamContext = &PBAudio.OutputStreams[0];
    //PBAudioStreamSetBypass(streamContext, true);

    //for each stream
    volatile bool wasRunning     = false;
    volatile bool wasBypassed    = false;
    volatile bool wasPassthrough = false;

    uint32_t mBytesPerFrame  = streamContext->format.mBytesPerFrame;
    uint32_t mBitsPerChannel = streamContext->format.mBitsPerChannel;

    //Stop the Audio Unit Stream
    if( streamContext->audioDevice == deviceID && streamContext->audioUnit && streamContext->running)
    {
        wasBypassed    = streamContext->bypass;             PBAudioStreamSetBypass(streamContext, true);            //bypass output renderpasses
        wasPassthrough = streamContext->passthroughEnabled; PBAudioStreamSetPassThroughState(streamContext, false); //disable input passthrough
        PBAudioStreamStop(streamContext); wasRunning  = true;
    }
    
    UInt32 propertySize = sizeof(UInt32);
    AudioObjectPropertyAddress bsPropertyAddress = {kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
    status = AudioObjectSetPropertyData(deviceID, &bsPropertyAddress, 0, nil, propertySize, &bufferSize);
    if ( !PBACheckOSStatus(status, "kAudioDevicePropertyBufferFrameSize") )
    {
        fprintf(stderr, ", Unable to set PropertyData(kAudioDevicePropertyBufferFrameSize)\n");
        assert(1==0);
        return status;
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

        if( streamContext->inputEnabled )
        {
            streamContext->inputTimeStamp.mSampleTime = 0; //conditional in output pass
            
            uint64_t nBuffers = PBA_TOTAL_BUFFER_SIZE / bufferSize;
            for(i=0; i<PBA_MAX_INFLIGHT_BUFFERS; i++)
            {
                if(streamContext->bufferList[i])  { PBABufferListFree(streamContext->bufferList[i]); streamContext->bufferList[i] = NULL; }
                if( i < nBuffers ) streamContext->bufferList[i] = PBABufferListCreateWithFormat(streamContext->format, bufferSize);
            }
            streamContext->nBuffers = nBuffers; streamContext->bufferIndex = 0;

        }
        
        if ( wasRunning )
        {
            PBAudioStreamStart(streamContext);
            PBAudioStreamSetBypass(streamContext, wasBypassed);              //restore output bypass state
            PBAudioStreamSetPassThroughState(streamContext, wasPassthrough); //restore input passthrough state
        }
    }
    
    //PBAudioStreamSetBypass(streamContext, wasBypassed);

#else
    void* (*objc_ClassSelector)(Class class, SEL _cmd) = (void*)objc_msgSend;
    BOOL   (*objc_InstanceSelectorSetDouble)(id self , SEL _cmd, double d1, void* err) = (void*)objc_msgSend;
    double (*objc_msgSendGetProperty)(void*, SEL) = (void*)objc_msgSend;

    double currentRate = 0; PBAudioDeviceNominalSampleRate(deviceID, kAudioObjectPropertyScopeOutput, &currentRate);
    double duration = (double)bufferSize / currentRate;
    
    //NSError* error = nil;
    id avSessionSharedInstance = objc_ClassSelector(objc_getClass("AVAudioSession"), sel_registerName("sharedInstance"));
    BOOL success = objc_InstanceSelectorSetDouble(avSessionSharedInstance, sel_registerName("setPreferredIOBufferDuration:error:"), duration, nil);
    //success  = [session setPreferredSampleRate:preferredSampleRate error:&error];

    if (! success) fprintf(stderr, "\nPBAudioDeviceSetBufferSize::AVAudioSession setPreferredIOBufferDuration (%g) failed\n", duration);

    duration = objc_msgSendGetProperty(avSessionSharedInstance, sel_getUid("IOBufferDuration"));
    fprintf(stdout, "\nnPBAudioDeviceSetBufferSize::AVAudioSession.bufferDuration = %g\n", duration );
#endif
#endif

    return status;
}
