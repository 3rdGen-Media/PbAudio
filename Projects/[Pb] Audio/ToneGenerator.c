//
//  ToneGenerator.c
//  [Pb] Audio
//
//  Created by Joe Moulton on 9/13/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include "ToneGenerator.h"

#ifdef __BLOCKS__
PBARenderPass ToneGeneratorRenderPass = ^ (AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp, void* source, void* events, UInt32 nEvents)
#else
void CALLBACK ToneGeneratorRenderPass(struct PBABufferList * ioData, uint32_t frames, const struct PBATimeStamp * timestamp, void* source, void* events, uint32_t nEvents)
#endif
{
    //UINT64 playbackSampleOffset = 0;
    uint32_t samplesToCopy = 0;
    uint32_t remainingSamples =0;
    uint32_t bufferIndex = 0;
    uint32_t frameIndex = 0;
        
    //Michael Tyson uses this to do some pthread locking... pthread locks are bullshit
    //Never ever lock a real-time thread!!!
    //AEManagedValueCommitPendingUpdates();0
        
//#ifdef DEBUG
//    uint64_t start = PBACurrentTimeInHostTicks();
//#endif
        
    ToneGenerator * generator = (ToneGenerator*)source;
    
    float * fBufferL = (float*)ioData->mBuffers[0].mData;
    float * fBufferR = (float*)ioData->mBuffers[1].mData;

    float * samplesL = (float*)(generator->SineWave.buffer);// + frameIndex*2]
    float * samplesR = (float*)(generator->SineWave.buffer);// + frameIndex*2]

    // Clear the output buffer
    //PBABufferListSilence(ioData, 0, frames);
    
    //Fill the buffer with source material
    samplesToCopy = frames;//clientStream->bufferFrameCount;// * 3 / 4; ;
    remainingSamples = (uint32_t)(generator->SineWave.length - generator->WaveSampleOffset);//audioEvent.audioData.size() - playbackSampleOffset;
    if(  remainingSamples > 0 && remainingSamples < samplesToCopy ) samplesToCopy = remainingSamples;

    //Debug output
    //printf("samplesToCopy = %d\n", samplesToCopy);
    //printf("remainingSamples = %d\n", remainingSamples);
    //printf("g_playbackSampleOffset = %d\n", g_playbackSampleOffset );

    float rate = generator->rate;
    static float position = 0.f;

    //TO DO: how to detect interleaved formats or prevent them
#ifdef WIN32
    bool interleaved = true;
#else
    bool interleaved = false;
#endif
    
    for ( int i=0; i<frames; i++ )
    {
        //float * fBufferL = (float*)ioData->mBuffers[0].mData;
        //float * fBufferR = (float*)ioData->mBuffers[1].mData;
        double sample = pseudo_sin(rate, &position) - 0.5;
        
        if( interleaved ) fBufferL[i*2] = fBufferL[i*2+1] = sample;
        else              fBufferL[i] = fBufferR[i] = sample;
    }

//#ifdef DEBUG
//        PBAStreamReportRenderTime(&PBAudio.OutputStreams[0], &_audioReport, PBASecondsFromHostTicks(PBACurrentTimeInHostTicks() - start), (double)frames / PBAudio.OutputStreams[0].currentSampleRate);
//#endif
 };

//#ifndef __BLOCKS
//PB_AUDIO_EXTERN PBARenderPass ToneGeneratorRenderPass = ToneGeneratorRenderPassCallback;
//#endif

void ToneGeneratorSetFrequency(ToneGenerator* source, float freq, float sampleRate)
{
    source->sampleRate = sampleRate;
    source->freq = freq;
    source->rate = freq / sampleRate;
}

void ToneGeneratorInit(ToneGenerator* source, float freq, float sampleRate)
{
#ifdef _WIN32 //WIN32 uses interleaved buffers
   uint32_t nSineBufferChannels = 2;
#else
   uint32_t nSineBufferChannels = 1;
#endif
    
    //Generate an Output Source for the Stream (Playback from FLoating Point Sine Wave Buffer)
    uint64_t renderDataLengthInSamples = (uint64_t)sampleRate * 5;
    source->SineWave.length = renderDataLengthInSamples;
    
    source->SineWave.length = sampleRate*5.f;
    double currentSampleRate = PBAudio.OutputStreams[0].currentSampleRate;
    GenerateSineSamplesFloat(&source->SineWave.buffer, source->SineWave.length, freq, nSineBufferChannels, currentSampleRate, 0.25f, NULL);
    ToneGeneratorSetFrequency(source, freq, sampleRate);
}
