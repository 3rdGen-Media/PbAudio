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

extern PBARenderPass SamplePlayerRenderPass;

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

#endif /* SamplePlayer_h */
