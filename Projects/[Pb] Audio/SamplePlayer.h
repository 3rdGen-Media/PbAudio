//
//  SamplePlayer.h
//  [Pb] Audio
//
//  Created by Joe Moulton on 9/14/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#ifndef SamplePlayer_h
#define SamplePlayer_h

#include <[Pb]Audio/[Pb]Audio.h>

#ifdef __cplusplus
extern "C" {
#endif

    //TO DO:  I don't understand why this needs to go outside of extern "C"
#ifdef __BLOCKS__
PB_AUDIO_EXTERN PBARenderPass SamplePlayerRenderPass;
#else
void CALLBACK SamplePlayerRenderPass(struct PBABufferList* ioData, uint32_t frames, const struct PBATimeStamp* timestamp, PBAStreamFormatSampleType target, void* source, void* events, uint32_t nEvents);
#endif


typedef struct SamplePlayerParams
{
    float freq;

}SamplePlayerParams;

typedef struct SamplePlayer
{
    //Intermediate params
    //ToneGeneratorParams params;
    
    SampleBuffer SampleBuffer;
    uint64_t     SampleOffset;

    //An abstract source
    PBAFile      sourceAudioFile;// = {0};
    
}SamplePlayer;

void SamplePlayerInit(SamplePlayer* player, const char* audioFileURL, const char* audioFileEXT, PBAStreamFormatRef converterFormat);
void SamplePlayerDestroy(SamplePlayer* player);


#ifdef __cplusplus
}
#endif


#endif /* SamplePlayer_h */
