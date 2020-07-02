//
//  pbaudio_format.h
//  CRViewer
//
//  Created by Joe Moulton on 6/25/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_format_h
#define pbaudio_format_h

#import <AudioToolbox/AudioToolbox.h>
#include "PbAudioError.h"
/*!
 * The audio description used throughout TAAE
 *
 *  This is 32-bit floating-point, non-interleaved stereo PCM.
 */
//AudioStreamBasicDescription
//This structure encapsulates all the information for describing the basic
//format properties of a stream of audio data.
typedef AudioStreamBasicDescription PBAStreamFormat;

//By default on Darwin platforms, we will request a 32-bit floating point non-interleaved linear PCM stereo Format
//On WIN32 all formats are interleaved by default?
PB_AUDIO_EXTERN PBAStreamFormat const _audioFormat;


static PBAStreamFormat PBAStreamFormatWithChannelsAndRate(int channels, double rate)
{
    PBAStreamFormat description = _audioFormat;
    description.mChannelsPerFrame = channels;
    description.mSampleRate = rate;
    return description;
}





//AEChannelSet AEChannelSetDefault = {0, 1};




//AudioComponentDescription
//A structure used to describe the unique and identifying IDs of an audio component
static AudioComponentDescription AEAudioComponentDescriptionMake(OSType manufacturer, OSType type, OSType subtype) {
    AudioComponentDescription description;
    memset(&description, 0, sizeof(description));
    description.componentManufacturer = manufacturer;
    description.componentType = type;
    description.componentSubType = subtype;
    return description;
}

#endif /* pbaudio_format_h */
