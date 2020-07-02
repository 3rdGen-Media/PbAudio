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

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAInitAudioStream(PBAStreamContext * streamContext);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAStartAudioStream(PBAStreamContext * streamContext);



#endif /* pbaudio_engine_h */
