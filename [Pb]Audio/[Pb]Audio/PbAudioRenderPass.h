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
    float *     buffer;     //8 bytes
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

typedef struct CRenderTarget
{
    crgc_size       extent;
    uint64_t        renderpass;
    uint64_t        framebuffer; //Every Graphics API but Metal needs to maintain a separate reference to framebuffer renderpass attachment
    //crgl_texture    attachments[PBA_TARGET_MAX_ATTACHMENTS];
    uint32_t        pixelFormat;
    uint32_t        depthFormat;
    uint32_t        stencilFormat;
}CRenderTarget;


#define MAX_WAVE_TABLE_SIZE 1


#endif /* PbAudioRenderPass_h */
