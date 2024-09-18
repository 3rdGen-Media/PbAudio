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

//TO DO:  I don't understand why this needs to go outside of extern "C"
//#ifdef __BLOCKS__
PB_AUDIO_EXTERN PBARenderPass SamplePlayerRenderPass;
//#else
//static void CALLBACK SamplePlayerRenderPass(struct PBABufferList* ioData, uint32_t frames, const struct PBATimeStamp* timestamp, void* source, void* events, uint32_t nEvents);
//#endif

#ifdef __cplusplus
extern "C" {
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

void SamplePlayerInit(SamplePlayer* source, const char * audioFileURL, const char * audioFileEXT, PBAStreamFormat converterFormat);


#ifdef __cplusplus
}
#endif


#endif /* SamplePlayer_h */
