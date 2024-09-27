//
//  ToneGenerator.h
//  [Pb] Audio
//
//  Created by Joe Moulton on 9/13/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#ifndef ToneGenerator_h
#define ToneGenerator_h

#include <[Pb]Audio/[Pb]Audio.h>

#ifdef __cplusplus
extern "C" {
#endif

    //TO DO:  I don't understand why this needs to go outside of extern "C"
#ifdef __BLOCKS__
PB_AUDIO_EXTERN PBARenderPass ToneGeneratorRenderPass;
#else
void CALLBACK ToneGeneratorRenderPass(struct PBABufferList* ioData,  uint32_t frames, const struct PBATimeStamp* timestamp, void* source, void* events, uint32_t nEvents);
#endif

typedef struct ToneGeneratorParams
{
    float freq;

}ToneGeneratorParams;

typedef struct ToneGenerator
{
    //Intermediate params
    //ToneGeneratorParams params;
    
    SampleBuffer SineWave;
    uint64_t     WaveSampleOffset;

    
    //real-time audio thread params
    volatile float sampleRate;
    volatile float freq;
    volatile float rate;
}ToneGenerator;

void ToneGeneratorInit(ToneGenerator* source, float freq, float sampleRate);
void ToneGeneratorSetFrequency(ToneGenerator* source, float freq, float sampleRate);


#ifdef __cplusplus
}
#endif


#endif /* ToneGenerator_h */
