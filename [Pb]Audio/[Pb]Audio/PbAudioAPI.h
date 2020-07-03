//
//  pbaudio_engine.h
//  CRViewer
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_api_h
#define pbaudio_api_h

#include "PbAudioStream.h"

// Init/Start API
//float * g_sineWaveBuffer = NULL;
//uint64_t g_sineBufferLengthInSamples;
//uint32_t g_sineBufferChannels;
//uint64_t g_playbackSampleOffset = 0;


//On Win32, if format is NULL, it will create a shared mode device stream (which usually gives a 32-bit floating point format)
//On Darwin, if format is NULL, it will create a device stream with a 32-bit floating point format
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAInitAudioStream(PBAStreamContext * streamContext, PBAStreamFormat * format);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAStartAudioStream(PBAStreamContext * streamContext);



#endif /* pbaudio_engine_h */
