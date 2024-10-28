//
//  PbAudioRenderPass.h
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/14/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#ifndef PbAudioRenderPass_h
#define PbAudioRenderPass_h

typedef struct PBAudioModule
{
    PBARenderPass renderpass;
}PBAudioModule;

typedef struct SampleBuffer // 16 bytes
{
    void *      buffer[2];  //8 bytes
    uint64_t    length;     //8 bytes, 1would this ever need to be larger than 4 bytes?
}SampleBuffer;

typedef struct SampleBufferCursor // 16 bytes
{
    uint64_t    offset;
}SampleBufferCursor;

typedef SampleBufferCursor SampleCursor;


typedef enum PBATargetAttachments
{
    PBA_TARGET_STEREO_L  = 0,
    PBA_TARGET_STEREO_R  = 1,
    //PBA_TARGET_FRONT_L   = 2,
    //PBA_TARGET_FRONT_R   = 3,
    PBA_TARGET_MAX_ATTACHMENTS
}PBATargetAttachments;


#define MAX_WAVE_TABLE_SIZE 1


#endif /* PbAudioRenderPass_h */
