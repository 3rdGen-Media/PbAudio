//
//  PbAudioDriver.h
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/10/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#ifndef PbAudioDriver_h
#define PbAudioDriver_h

//#include <stdio.h>

#if defined( __APPLE__ ) && TARGET_OS_OSX
//#include <CoreAudio/AudioHardwareBase.h>
#endif

//#define PBA_MAX_DRIVERS        64
//#define PBA_MAX_DRIVER_OUTPUTS 64

#ifdef __APPLE__
//typedef AudioDeviceID PBAudioDevice;
#else
typedef void*  PBAudioDriver;   //opaque vendor driver
typedef long   PBAudioDriverID; //eg ASIO driver list index
typedef int    ItemCount;
#endif

typedef struct PBAudioDriverList
{
    //TO DO:  Should there be separate Input/Output Device Management?
    const PBAudioDriver* drivers;//[PBA_MAX_DEVICES];
    ItemCount            count;
}PBAudioDriverList;


#ifdef __APPLE__
//PB_AUDIO_EXTERN const CFStringRef kPBADeviceDefaultInputChangedNotification;
//PB_AUDIO_EXTERN const CFStringRef kPBADeviceDefaultOutputChangedNotification;
//PB_AUDIO_EXTERN const CFStringRef kPBADevicesAvailableChangedNotification;
#else

#endif

//Private Notifications
//PB_AUDIO_API PB_AUDIO_INLINE OSStatus     PBAudioDeviceDefaultInputChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress *inAddresses, void *inClientData);
//PB_AUDIO_API PB_AUDIO_INLINE OSStatus    PBAudioDeviceDefaultOutputChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress *inAddresses, void *inClientData);
//PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDeviceAvailableDevicesChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress *inAddresses, void *inClientData);

#pragma mark -- Opaque Driver Wrapper API

//Initialize [single threaded] COM apartment on client app thread for interfacing with Vendor ASIO Drivers
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverInitCOM();
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverReleaseCOM();

//Enumerate
PB_AUDIO_API PB_AUDIO_INLINE LONG     PBAudioAvailableDriverCount(void);
PB_AUDIO_API PB_AUDIO_INLINE LONG     PBAudioGetDriverName(int drvID, char* drvname, int drvnamesize);

//Load/Init
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioLoadVendorDriver(void** driverRef, PBAudioDriverID currentDriverID, PBAudioDriverID* driverID);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioInitVendorDriver(void* driver, void* context);

//Control
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverStart(void* driver);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverStop(void* driver);

//Sample Rate
PB_AUDIO_API PB_AUDIO_INLINE double   PBAudioDriverGetSampleRate(void* driver);
PB_AUDIO_API PB_AUDIO_INLINE int      PBAudioDriverCanSampleRate(void* driver, double sampRate);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverSetSampleRate(void* driver, double sampRate, void* context);

//Buffer Size
PB_AUDIO_API PB_AUDIO_INLINE long     PBAudioDriverGetBufferSize(void* driver, long* minSize, long* maxSize, long* preferredSize, long* granularity);

//Cleanup
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverDisposeBuffers(void* driverRef);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverShutdown(void** driverRef, PBAudioDriverID* driverID);

//Extras
PB_AUDIO_API PB_AUDIO_INLINE void     PBAudioDriverControlPanel(void* driverRef);


#endif /* PbAudioDriver_h */
