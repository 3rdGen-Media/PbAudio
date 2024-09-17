//
//  PbAudioDevice.h
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/10/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#ifndef PbAudioDevice_h
#define PbAudioDevice_h

//#include <stdio.h>
#include <CoreAudio/AudioHardwareBase.h>


//typedef uint8_t PBAudioDeviceType;

/*
typedef struct CMDeviceDescription
{
    char                     name[256];
    CMDeviceType             type;
    uint8_t                  nDisplays;
    uint8_t                  nControls;
    uint8_t                  nSwitches;
    
}CMDeviceDescription;
*/

#define PBA_MAX_DEVICES        64
#define PBA_MAX_DEVICE_OUTPUTS 64

#ifdef __APPLE__
typedef AudioDeviceID PBAudioDevice;

#else

#endif

typedef struct PBAudioDeviceList
{
    //TO DO:  Should there be separate Input/Output Device Management?
    const PBAudioDevice *devices;//[PBA_MAX_DEVICES];
          ItemCount     count;
}PBAudioDeviceList;

//extern const int    cm_device_list_size;
//extern const char * cm_device_names[];
//extern const char * cm_device_image_names[];

//extern NSString * const AEAudioDeviceDefaultInputDeviceChangedNotification;
//extern NSString * const AEAudioDeviceDefaultOutputDeviceChangedNotification;
//extern NSString * const AEAudioDeviceAvailableDevicesChangedNotification;

PB_AUDIO_EXTERN const CFStringRef kPBADeviceDefaultInputChangedNotification;
PB_AUDIO_EXTERN const CFStringRef kPBADeviceDefaultOutputChangedNotification;
PB_AUDIO_EXTERN const CFStringRef kPBADevicesAvailableChangedNotification;

//Private Notifications
//PB_AUDIO_API PB_AUDIO_INLINE OSStatus     PBAudioDeviceDefaultInputChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress *inAddresses, void *inClientData);
//PB_AUDIO_API PB_AUDIO_INLINE OSStatus    PBAudioDeviceDefaultOutputChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress *inAddresses, void *inClientData);
//PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceAvailableDevicesChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress *inAddresses, void *inClientData);

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioRegisterDeviceListeners(PBAStreamContext* streamContext);

#pragma mark -- Get Devices


//Query Devices Top Level Data
PB_AUDIO_API PB_AUDIO_INLINE OSStatus          PBAudioDeviceName(PBAudioDevice deviceID, char * deviceName, UInt32 * nameLen);
PB_AUDIO_API PB_AUDIO_INLINE AudioDeviceID     PBAudioDefaultDevice(AudioObjectPropertySelector selector);
PB_AUDIO_API PB_AUDIO_INLINE PBAudioDeviceList PBAudioAvailableDevices(void);

//Query Stream Device
//PB_AUDIO_API PB_AUDIO_INLINE AudioDeviceID PBAudioStreamOutputDevice(PBAStreamContext* streamContext);

//Query Device Stream Configuration
PB_AUDIO_API PB_AUDIO_INLINE int                PBAudioDeviceChannelCount(PBAudioDevice deviceID, AudioObjectPropertyScope scope);

//Query Device Sample Rate(s)
PB_AUDIO_API PB_AUDIO_INLINE OSStatus      PBAudioDeviceNominalSampleRate(PBAudioDevice deviceID, AudioObjectPropertyScope scope, Float64* sampleRate);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceNominalSampleRateCount(PBAudioDevice deviceID, AudioObjectPropertyScope scope, int * nSampleRates);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus          PBAudioDeviceSetSampleRate(PBAudioDevice deviceID, AudioObjectPropertyScope scope, Float64 sampleRate);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus        PBAudioDeviceBufferSizeRange(PBAudioDevice deviceID, UInt32* outMinimum, UInt32* outMaximum);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus             PBAudioDeviceBufferSize(PBAudioDevice deviceID, UInt32* bufferSize);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus          PBAudioDeviceSetBufferSize(PBAudioDevice deviceID, UInt32  bufferSize);

//+ (AEAudioDevice *)defaultInputAudioDevice;
//+ (AEAudioDevice *)audioDeviceWithUID:(NSString *)UID;

/*
+ (AEAudioDevice *)defaultInputAudioDevice {
    AudioDeviceID deviceId;
    UInt32 size = sizeof(deviceId);
    AudioObjectPropertyAddress addr = {kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal};
    if ( !AECheckOSStatus(AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &size, &deviceId), "kAudioHardwarePropertyDefaultInputDevice") || deviceId == kAudioObjectUnknown ) {
        return nil;
    }
    AEAudioDevice * device = [[AEAudioDevice alloc] initWithObjectID:deviceId];
    device.isDefault = YES;
    return device;
}


+ (AEAudioDevice *)audioDeviceWithUID:(NSString *)UID {
    AudioDeviceID deviceId;
    UInt32 size = sizeof(deviceId);
    CFStringRef UIDStr = (__bridge CFStringRef)UID;
    AudioObjectPropertyAddress addr = {kAudioHardwarePropertyTranslateUIDToDevice, kAudioObjectPropertyScopeGlobal};
    if ( !AECheckOSStatus(AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, sizeof(UIDStr), &UIDStr, &size, &deviceId), "kAudioHardwarePropertyTranslateUIDToDevice") || deviceId == kAudioObjectUnknown ) {
        return nil;
    }
    return [[AEAudioDevice alloc] initWithObjectID:deviceId];
}
*/

#endif /* PbAudioDevice_h */
