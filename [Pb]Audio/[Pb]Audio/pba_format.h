//
//  pbaudio_format.h
//  CRViewer
//
//  Created by Joe Moulton on 6/25/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_format_h
#define pbaudio_format_h

#include "pba_error.h"
/*!
 * The audio description used throughout TAAE
 *
 *  This is 32-bit floating-point, non-interleaved stereo PCM.
 */
//AudioStreamBasicDescription
//This structure encapsulates all the information for describing the basic
//format properties of a stream of audio data.

#ifdef __APPLE__
#import <AudioToolbox/AudioToolbox.h>
typedef AudioStreamBasicDescription PBAStreamFormat;
typedef AudioComponentDescription	PBAComponentDescription;
#elif defined(_WIN32)
#include <mfapi.h>
#include <mmdeviceapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
//#pragma comment(lib, "mfapi.lib")

#pragma comment(lib, "avrt.lib")

typedef WAVEFORMATEX PBAStreamFormat;
#endif
//By default on Darwin platforms, we will request a 32-bit floating point non-interleaved linear PCM stereo Format
//On WIN32 all formats are interleaved by default?
PB_AUDIO_EXTERN PBAStreamFormat const _audioFormat;


static PBAStreamFormat PBAStreamFormatWithChannelsAndRate(int channels, double rate)
{
	PBAStreamFormat description = _audioFormat;
#ifdef __APPLE__
    description.mChannelsPerFrame = channels;
    description.mSampleRate = rate;
#elif defined(_WIN32)
	description.nChannels = channels;
	description.nSamplesPerSec = (DWORD)rate;
#endif
	return description;
}





//AEChannelSet AEChannelSetDefault = {0, 1};



#ifdef __APPLE__
//AudioComponentDescription
//A structure used to describe the unique and identifying IDs of an audio component
static AudioComponentDescription PBAudioComponentDescriptionMake(OSType manufacturer, OSType type, OSType subtype) {
    AudioComponentDescription description;
    memset(&description, 0, sizeof(description));
    description.componentManufacturer = manufacturer;
    description.componentType = type;
    description.componentSubType = subtype;
    return description;
}
#endif

PB_AUDIO_API PB_AUDIO_INLINE void PBAStreamFormatPrint(PBAStreamFormat * format);

#endif /* pbaudio_format_h */
