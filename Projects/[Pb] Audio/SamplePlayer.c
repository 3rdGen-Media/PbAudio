//
//  SamplePlayer.c
//  [Pb] Audio
//
//  Created by Joe Moulton on 9/14/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include "SamplePlayer.h"

#ifdef _WIN32
#ifndef mChannelsPerFrame
#define mChannelsPerFrame nChannels
#define mBytesPerFrame    nBlockAlign
#endif
#endif


#ifdef __BLOCKS__
PBARenderPass SamplePlayerRenderPass = ^(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp, //Required
                                         PBAStreamFormatSampleType  target, void* source,  void* events, uint32_t nEvents)            //Extended
#else
void        SamplePlayerRenderPass      (struct PBABufferList*     ioData, uint32_t frames, const struct  PBATimeStamp* timestamp,    //Required 
                                        PBAStreamFormatSampleType target, void*    source, void* events, uint32_t nEvents)            //Extended
#endif
{
    //UINT64 playbackSampleOffset = 0;
    uint32_t samplesToCopy = 0;
    uint32_t remainingSamples =0;
    uint32_t bufferIndex = 0;
    uint32_t frameIndex = 0;

//#ifdef DEBUG
//        uint64_t start = PBACurrentTimeInHostTicks();
//#endif
          
        SamplePlayer * player = (SamplePlayer*)source;
    
        int32_t nSourceChannels = player->sourceAudioFile.conversionFormat.mChannelsPerFrame;
        //int32_t nStreamChannels = 2;// player->sourceAudioFile.sourceFormat.nChannels;

        int32_t nSourceSampleBytes = player->sourceAudioFile.conversionFormat.mBytesPerFrame / nSourceChannels;

        //Fill the buffer with source material
        samplesToCopy = frames;// 3 / 4;//clientStream->bufferFrameCount;// * 3 / 4; ;
        remainingSamples = (uint32_t)(player->sourceAudioFile.numFrames - player->SampleOffset);//audioEvent.audioData.size() - playbackSampleOffset;
        if(  remainingSamples > 0 && remainingSamples < samplesToCopy ) samplesToCopy = remainingSamples;

        //Debug output
        //printf("samplesToCopy = %d\n", samplesToCopy);
        //printf("remainingSamples = %d\n", remainingSamples);
        //printf("g_playbackSampleOffset = %d\n", g_playbackSampleOffset );

        if (remainingSamples > 0)
        {
            //float32 dst buffer
            //float* fBufferL = (float*)ioData->mBuffers[0].mData;
            //float* fBufferR = (float*)ioData->mBuffers[1].mData;

            //raw byte dst buffer
            char* byteBufferL = (char*)ioData->mBuffers[0].mData;
            char* byteBufferR = (char*)ioData->mBuffers[1].mData;
            char* byteBuffers[2] = {byteBufferL, byteBufferR};

            //raw byte source buffer
            //char* samplesL = (char*)(player->sourceAudioFile.samples[0]);// + frameIndex*2]
            //char* samplesR = (char*)(player->sourceAudioFile.samples[1]);// + frameIndex*2]
            
            uint64_t iL = (((player->SampleOffset) * nSourceChannels) + 0);  //samplesL = &samplesL[iL * nSourceSampleBytes]; //shortSamplesL = &shortSamplesL[iL];
            char * sampleBuffers[2] = { &player->sourceAudioFile.samples[0][iL * nSourceSampleBytes], &player->sourceAudioFile.samples[1][iL * nSourceSampleBytes] };
            
            //int16_t* shortSamplesL = (int16_t*)(player->sourceAudioFile.samples[0]);
            //int16_t* shortSamplesR = (int16_t*)(player->sourceAudioFile.samples[1]);

        //16-bit le signed integer interleaved source -> 32-bit host interleaved float

            
            //uint64_t iL = (((player->SampleOffset) * nSourceChannels) + 0); samplesL = &samplesL[iL * nSourceSampleBytes]; //shortSamplesL = &shortSamplesL[iL];
            //uint64_t iR = (((player->SampleOffset) * nSourceChannels) + 1); shortSamplesR = &shortSamplesL[iR];
            //pba_convert_s16i_f32i(&samplesL, &fBufferL, nSourceChannels, samplesToCopy);
            pb_audio_transform[player->sourceAudioFile.type][target](sampleBuffers, byteBuffers, nSourceChannels, samplesToCopy);

        //24-bit le signed integer source -> 32-bit host float
            
            //uint64_t iL = (((player->SampleOffset) * nSourceChannels) + 0); samplesL = &samplesL[iL * 3];
            //pba_convert_s24i_f32i(&samplesL, &fBufferL, nSourceChannels, samplesToCopy);
            //PbAudioConvert[player->sourceAudioFile.type][target](&samplesL, &byteBufferL, nSourceChannels, samplesToCopy);
           

            /*
            for (int i = 0; i < samplesToCopy; i++)
            {
                int32_t iSample;
                float   fSample;

                //place 24 bit le signed integer in most significant bytes of a 32 bit le signed integer
                char sourceSample[4] = { 0, samplesL[(player->SampleOffset + i) * nSourceChannels * 3 + 0], samplesL[(player->SampleOffset + i) * nSourceChannels * 3 + 1], samplesL[(player->SampleOffset + i) * nSourceChannels * 3 + 2] };
                memcpy(&iSample, sourceSample, 4);

                //map 32-bit integer range [-2147483648, 2147483647] to 32-bit float [-1.0, 1.0]
                fSample = ((float)iSample) / 2147483647.f;
                
                //memcpy(&(byteBufferL[(i * nStreamChannels + 0) * 3]), sourceSample, 3);
                //memcpy(&(byteBufferL[(i * nStreamChannels + 1) * 3]), sourceSample, 3);

                fBufferL[i * 2] = fBufferL[i * 2 + 1] = fSample;
            }
            */

        //32-bit le float source -> 32-bit host float

            /*
            float* fBufferL = (float*)ioData->mBuffers[0].mData;
            float* fBufferR = (float*)ioData->mBuffers[1].mData;

            float* samplesL = (float*)(player->sourceAudioFile.samples[0]);// + frameIndex*2]
            float* samplesR = (float*)(player->sourceAudioFile.samples[1]);// + frameIndex*2]

            memcpy(fBufferL, &(samplesL[player->SampleOffset]), samplesToCopy * sizeof(float));
            //memcpy(fBufferR, &(samplesR[player->SampleOffset]), samplesToCopy * sizeof(float));
            if( samplesR ) memcpy(fBufferR, &(samplesR[player->SampleOffset]), samplesToCopy * sizeof(float));
            else           memcpy(fBufferR, &(samplesL[player->SampleOffset]), samplesToCopy * sizeof(float));
            
            */

            player->SampleOffset += samplesToCopy;

        }

//#endif

//#ifdef DEBUG
//        PBAStreamReportRenderTime(&PBAudio.OutputStreams[0], &_audioReport, PBASecondsFromHostTicks(PBACurrentTimeInHostTicks() - start), (double)frames / PBAudio.OutputStreams[0].currentSampleRate);
//#endif

 };

//#ifndef __BLOCKS__
//PB_AUDIO_EXTERN PBARenderPass SamplePlayerRenderPass = SamplePlayerRenderPassCallback;
//#endif


void SamplePlayerInit(SamplePlayer* player, const char * audioFileURL, const char * audioFileEXT, PBAStreamFormatRef converterFormat)
{
    //Choose an Output Source for the Stream (Playback from PCM File)
    //g_sourceBuffer = (char*)&(audioEvent.audioData[0]);
    //g_sourceBytesPerSample = 3;//fileFormatEx->wBitsPerSample / 8;//(pwfx->nBlockAlign /pwfx->nChannels);
    //g_sourceBufferChannels = 1;//fileFormatEx->nChannels;//g_sineBufferChannels;
    //g_sourceBufferLengthInSamples = audioEvent.audioData.size() / 3;//( g_sourceBytesPerSample * g_sourceBufferChannels);
        
    int iBuffer  = 0;
    int iChannel = 0;

#ifdef _WIN32 //WIN32 Media Foundatoin unpacks source file samples to interleaved buffers
    uint32_t nSourceBufferChannels = 2;
#else
    uint32_t nSourceBufferChannels = 1;
#endif
    
    PBAFileStreamOpen(audioFileURL, audioFileEXT, converterFormat, &player->sourceAudioFile);
    //g_sourceBuffer = (float*)malloc( sourceAudioFile.samples * sizeof(float) * _PBAMasterStream.format.mChannelsPerFrame );
    
    fprintf(stdout, "sourceAudioFile length of file in frames:   %llu", player->sourceAudioFile.numFrames);

    //get a buffer for storing the samples to be submitted on the real-time render thread renderpass

#ifdef __APPLE__ //Conversion when reading from file is currently only implemented on Apple
    int nSourceBuffers       = 1; //player->sourceAudioFile.sourceFormat.mChannelsPerFrame;
    int nInterleavedChannels = player->sourceAudioFile.sourceFormat.mChannelsPerFrame; //1;
    if (converterFormat) nSourceBuffers = converterFormat->mChannelsPerFrame;
#else
    int nSourceBuffers       = 1;
    int nInterleavedChannels = player->sourceAudioFile.sourceFormat.mChannelsPerFrame;
#endif

//#ifndef XNZ_AUDIO
    int nFramesPerBuffer = player->sourceAudioFile.numFrames * nInterleavedChannels;

    //create conversion buffers
    void ** conversionBuffers = NULL; if( converterFormat ) conversionBuffers = (float**)malloc( converterFormat->mChannelsPerFrame * sizeof(float*));

    for(iBuffer = 0; iBuffer < nSourceBuffers; iBuffer++)
    {
        //until memory is a concern just allocate to the max supported output sample size of 4 bytes per sample (this makes it easier to do in place conversion)
        //TO DO: parse sample format and frame container type to determine size
               player->sourceAudioFile.samples[iBuffer] = (float*)malloc( nFramesPerBuffer * sizeof(float) );
        memset(player->sourceAudioFile.samples[iBuffer], 0,               nFramesPerBuffer * sizeof(float) );
        
        //get memory for buffers
        if( converterFormat )
        {
                conversionBuffers[iBuffer] = (float*)malloc( nFramesPerBuffer * sizeof(float) );
         memset(conversionBuffers[iBuffer], 0,               nFramesPerBuffer * sizeof(float) );
        }
    }
//#endif

    //Read frames from source file to destination buffer(s)
    //If a converterFormat was provided as input param and conversion is supported, conversion will occur during reads
    PBAFileStreamReadFrames(&player->sourceAudioFile, player->sourceAudioFile.numFrames, conversionBuffers ? conversionBuffers : player->sourceAudioFile.samples);

    if( conversionBuffers )
    {
        PBASampleType fromType = player->sourceAudioFile.type; //was set during open
        PBASampleType toType   = PBAStreamFormatGetType(converterFormat); //enumerate a sample packing protocol for the given format
        pb_audio_transform[fromType][toType](conversionBuffers, player->sourceAudioFile.samples, player->sourceAudioFile.sourceFormat.mChannelsPerFrame, player->sourceAudioFile.numFrames);
        
        //record input conversion format
        memcpy(&(player->sourceAudioFile.conversionFormat), converterFormat, sizeof(PBAStreamFormat));
        player->sourceAudioFile.type =  toType; //set the type
        
        for(iBuffer = 0; iBuffer < nSourceBuffers; iBuffer++) free(conversionBuffers[iBuffer]);
        free(conversionBuffers);
    }
    
    //It is now safe to close the system handle to the source audio file
    //PBAFileStreamClose(&player->sourceAudioFile.file);

}
