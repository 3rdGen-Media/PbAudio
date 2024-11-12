//-----------------------------------------------------------
// Example implementation of IMMNotificationClient interface.
// When the status of audio endpoint devices change, the
// MMDevice module calls these methods to notify the client.
//-----------------------------------------------------------

#include "[Pb]Audio/[Pb]Audio.h"

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

#if !TARGET_OS_OSX
static void PBAudioSessionRouteChangeNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    fprintf(stdout, "\nPBAudioSessionRouteChangeNotificationCallback\n" );
    assert(userInfo);

    PBAudioSessionRouteChangeReason reason = 0;
    CFNumberRef reasonRef = (CFNumberRef)CFDictionaryGetValue(userInfo, CFSTR("AVAudioSessionRouteChangeReasonKey")); assert(reasonRef);
    BOOL success = CFNumberGetValue( reasonRef, kCFNumberSInt32Type, &reason); assert(success);

    switch(reason)
    {
        /// A new device became available (e.g. headphones have been plugged in).
        case(PBAudioSessionRouteChangeReasonNewDeviceAvailable):
        {
            AudioDeviceID outputDeviceID; PBAudioStreamGetOutputDevice(&PBAudio.OutputStreams[0], &outputDeviceID);
            fprintf(stdout, "\nPBAudioSessionRouteChangeReasonNewDeviceAvailable (AudioDeviceID: %u)\n", outputDeviceID);
            break;
        }
        
        case(PBAudioSessionRouteChangeReasonOldDeviceUnavailable):
        {
            AudioDeviceID outputDeviceID; PBAudioStreamGetOutputDevice(&PBAudio.OutputStreams[0], &outputDeviceID);
            fprintf(stdout, "\nPBAudioSessionRouteChangeReasonOldDeviceUnavailable (AudioDeviceID: %u)\n", outputDeviceID);
            break;
        }
            
        case(PBAudioSessionRouteChangeReasonCategoryChange):
        {
            break;
            
        }
        
        case(PBAudioSessionRouteChangeReasonOverride):
        {
            break;
            
        }
        case(PBAudioSessionRouteChangeReasonWakeFromSleep):
        {
            break;
            
        }

        case(PBAudioSessionRouteChangeReasonNoSuitableRouteForCategory):
        {
            break;
            
        }
        case(PBAudioSessionRouteChangeReasonRouteConfigurationChange):
        {
            
            break;
        }
            
            
        default:
            assert(1==0);
    }
    
}
#endif

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

static void PBAudioStreamDeviceChangedNotificationCallback(CFNotificationCenterRef center, void * observer, CFStringRef name, const void * object, CFDictionaryRef userInfo)
{
    assert(userInfo);

    PBAStreamContext * DeviceStream = (PBAStreamContext*)CFDictionaryGetValue(userInfo, CFSTR("DeviceStream"));     assert(DeviceStream);
    //CFNumberRef        DeviceRef    =       (CFNumberRef)CFDictionaryGetValue(userInfo, CFSTR("OutputDeviceID")); assert(DeviceRef);
    //BOOL success = CFNumberGetValue( DeviceRef, kCFNumberSInt32Type, &outputDeviceID); assert(success);
    
    fprintf(stdout, "\nPBAudioStreamDeviceChangedNotificationCallback (DeviceID: %lu)\n", (unsigned int)DeviceStream->audioDevice);
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
    //ToneGeneratorSetFrequency(&toneGenerator, toneGenerator.freq, DeviceStream->currentSampleRate);
    
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

static void RegisterAppNotificationObservers(void)
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
}

static void RegisterAudioNotificationObservers(void)
{
    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    assert(center);
    
    
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
    
    CFNotificationCenterAddObserver(center, NULL, PBAudioStreamDeviceChangedNotificationCallback,
                                    CFSTR("PBAudioStreamDeviceChangedNotification"),
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

#elif defined(_WIN32) && defined(USE_CPP_RUNLOOP)

#include "Functiondiscoverykeys_devpkey.h"

#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }

class CMMNotificationClient : public IMMNotificationClient
{
    LONG _cRef;
    IMMDeviceEnumerator* _pEnumerator;

    // Private function to print device-friendly name
    HRESULT _PrintDeviceName(LPCWSTR  pwstrId);

public:
    CMMNotificationClient() :
        _cRef(1),
        _pEnumerator(NULL)
    {
    }

    ~CMMNotificationClient()
    {
        SAFE_RELEASE(_pEnumerator)
    }

    // IUnknown methods -- AddRef, Release, and QueryInterface

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulRef = InterlockedDecrement(&_cRef);
        if (0 == ulRef)
        {
            delete this;
        }
        return ulRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid, VOID** ppvInterface)
    {
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppvInterface = (IUnknown*)this;
        }
        else if (__uuidof(IMMNotificationClient) == riid)
        {
            AddRef();
            *ppvInterface = (IMMNotificationClient*)this;
        }
        else
        {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    // Callback methods for device-event notifications.

    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
        EDataFlow flow, ERole role,
        LPCWSTR pwstrDeviceId)
    {
        char* pszFlow = "?????";
        char* pszRole = "?????";

        //_PrintDeviceName(pwstrDeviceId);

        switch (flow)
        {
        case eRender:
            pszFlow = "eRender";
            break;
        case eCapture:
            pszFlow = "eCapture";
            break;
        }

        switch (role)
        {
        case eConsole:
            pszRole = "eConsole";
            break;
        case eMultimedia:
            pszRole = "eMultimedia";
            break;
        case eCommunications:
            pszRole = "eCommunications";
            break;
        }

        printf("  -->New default device: flow = %s, role = %s\n",
            pszFlow, pszRole);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId)
    {
        //_PrintDeviceName(pwstrDeviceId);

        printf("  -->Added device\n");
        return S_OK;
    };

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId)
    {
        //_PrintDeviceName(pwstrDeviceId);

        printf("  -->Removed device\n");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(
        LPCWSTR pwstrDeviceId,
        DWORD dwNewState)
    {
        char* pszState = "?????";

        //_PrintDeviceName(pwstrDeviceId);

        switch (dwNewState)
        {
        case DEVICE_STATE_ACTIVE:
            pszState = "ACTIVE";
            break;
        case DEVICE_STATE_DISABLED:
            pszState = "DISABLED";
            break;
        case DEVICE_STATE_NOTPRESENT:
            pszState = "NOTPRESENT";
            break;
        case DEVICE_STATE_UNPLUGGED:
            pszState = "UNPLUGGED";
            break;
        }

        printf("  -->New device state is DEVICE_STATE_%s (0x%8.8x)\n",
            pszState, dwNewState);

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(
        LPCWSTR pwstrDeviceId,
        const PROPERTYKEY key)
    {
        //_PrintDeviceName(pwstrDeviceId);

        printf("  -->Changed device property "
            "{%8.8x-%4.4x-%4.4x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x}#%d\n",
            key.fmtid.Data1, key.fmtid.Data2, key.fmtid.Data3,
            key.fmtid.Data4[0], key.fmtid.Data4[1],
            key.fmtid.Data4[2], key.fmtid.Data4[3],
            key.fmtid.Data4[4], key.fmtid.Data4[5],
            key.fmtid.Data4[6], key.fmtid.Data4[7],
            key.pid);
        return S_OK;
    }
};

// Given an endpoint ID string, print the friendly device name.
HRESULT CMMNotificationClient::_PrintDeviceName(LPCWSTR pwstrId)
{
    HRESULT hr = S_OK;
    IMMDevice* pDevice = NULL;
    IPropertyStore* pProps = NULL;
    PROPVARIANT varString;

    CoInitialize(NULL);
    PropVariantInit(&varString);

    if (_pEnumerator == NULL)
    {
        // Get enumerator for audio endpoint devices.
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
            NULL, CLSCTX_INPROC_SERVER,
            __uuidof(IMMDeviceEnumerator),
            (void**)&_pEnumerator);
    }
    if (hr == S_OK)
    {
        hr = _pEnumerator->GetDevice(pwstrId, &pDevice);
    }
    if (hr == S_OK)
    {
        hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    }
    if (hr == S_OK)
    {
        // Get the endpoint device's friendly-name property.
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varString);
    }
    printf("----------------------\nDevice name: \"%S\"\n"
        "  Endpoint ID string: \"%S\"\n",
        (hr == S_OK) ? varString.pwszVal : L"null device",
        (pwstrId != NULL) ? pwstrId : L"null ID");

    PropVariantClear(&varString);

    SAFE_RELEASE(pProps)
        SAFE_RELEASE(pDevice)
        CoUninitialize();
    return hr;
}


#endif //_WIN32
