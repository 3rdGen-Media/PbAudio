//
//  PBABufferList.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 7/1/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

#ifdef __APPLE__
PB_AUDIO_API PB_AUDIO_INLINE PBABufferList *PBABufferListCreateWithFormat(PBAStreamFormat audioFormat, int frameCount)
{
    int i, j;
    int numberOfBuffers = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? audioFormat.mChannelsPerFrame : 1;
    int channelsPerBuffer = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? 1 : audioFormat.mChannelsPerFrame;
    int bytesPerBuffer = audioFormat.mBytesPerFrame * frameCount;
    
    AudioBufferList *audio = malloc(sizeof(AudioBufferList) + (numberOfBuffers-1)*sizeof(AudioBuffer));
    if ( !audio ) {
        return NULL;
    }
    audio->mNumberBuffers = numberOfBuffers;
    for ( i=0; i<numberOfBuffers; i++ ) {
        if ( bytesPerBuffer > 0 ) {
            audio->mBuffers[i].mData = calloc(bytesPerBuffer, 1);
            if ( !audio->mBuffers[i].mData ) {
                for ( j=0; j<i; j++ ) free(audio->mBuffers[j].mData);
                free(audio);
                return NULL;
            }
        } else {
            audio->mBuffers[i].mData = NULL;
        }
        audio->mBuffers[i].mDataByteSize = bytesPerBuffer;
        audio->mBuffers[i].mNumberChannels = channelsPerBuffer;
    }
    return audio;
}


PB_AUDIO_API PB_AUDIO_INLINE PBABufferList *PBABufferListCreate(int frameCount) {
    return PBABufferListCreateWithFormat(_audioFormat, frameCount);
}

#endif

//Schedule to Audio Buffer List


PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListSilenceWithFormat(const PBABufferList *bufferList, PBAStreamFormat* audioFormat, uint32_t offset, uint32_t length)
{
    uint32_t i;
    //printf("PBAudioBufferListSilenceWithFormat\n");
#ifdef __APPLE__
	uint32_t bytesPerFrame = audioFormat->mBitsPerChannel / 8;
#else
	uint16_t bytesPerFrame = audioFormat->nBlockAlign;
#endif
    for ( i=0; i<bufferList->mNumberBuffers; i++ )
    {
        memset((char*)bufferList->mBuffers[i].mData + offset * bytesPerFrame,0, length * bytesPerFrame);
    }
}


//List silence to default engine format
//PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListSilence(const PBABufferList *bufferList, uint32_t offset, uint32_t length)
//{
//    PBABufferListSilenceWithFormat(bufferList, _audioFormat, offset, length);
//}


