//
//  PbAudioFormat.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 7/1/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"


PB_AUDIO_EXTERN PBAStreamFormat const _audioFormat = {

#ifdef __APPLE__
    .mFormatID          = kAudioFormatLinearPCM,
    .mFormatFlags       = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved,
    .mChannelsPerFrame  = 2,
    .mBytesPerPacket    = sizeof(float),
    .mFramesPerPacket   = 1,
    .mBytesPerFrame     = sizeof(float),
    .mBitsPerChannel    = 8 * sizeof(float),
    .mSampleRate        = 0,
#elif defined (_WIN32)
	/*.wFormatTag         =*/ 1, /* format type */
    /*.nChannels;         =*/ 2, /* number of channels (i.e. mono, stereo...) */
    /*.nSamplesPerSec	  =*/ 0, /* sample rate */
    /*.nAvgBytesPerSec    =*/ 0, /* for buffer estimation */
    /*.nBlockAlign		  =*/ 8, /* block size of data */
    /*.wBitsPerSample	  =*/ 8 * sizeof(float),     /* number of bits per sample of mono data */
    /*.cbSize			  =*/ 0             /* the count in bytes of the size of */

#endif
};
