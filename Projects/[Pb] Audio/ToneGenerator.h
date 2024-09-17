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

extern PBARenderPass ToneGeneratorRenderPass;

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

#endif /* ToneGenerator_h */
