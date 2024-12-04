#ifndef _XNZ_WAV_CODEC_H_
#define _XNZ_WAV_CODEC_H_

#include "cr_file.h" //CRFile
//#include "stdint.h"         //uint types

//#include <xnz/xnz.h>

#ifdef __cplusplus
extern "C" {
#endif

//only define bool if not previoulsy defined by CoreRender crWindow global header
//and not using C++
#ifndef __cplusplus
//typedef unsigned char bool;
#ifndef bool
#define bool int
#define true 1
#define false 0
#endif
#endif


/* We are building or calling crMath as a static library */
#ifndef XNZ_WAV_API
#define XNZ_WAV_API static
#endif

//inline doesn't exist in C89, __inline is MSVC specific
#ifndef XNZ_WAV_INLINE
#ifdef _WIN32
#define XNZ_WAV_INLINE __inline
#else
#define XNZ_WAV_INLINE
#endif
#endif

//__decspec doesn't exist in C89, __declspec is MSVC specific
#ifndef XNZ_WAV_DECLSPEC
#ifdef _WIN32
#define XNZ_WAV_DECLSPEC __declspec
#else
#define XNZ_WAV_DECLSPEC
#endif
#endif

//align functions are diffent on windows vs iOS, Linux, etc.
#ifndef XNZ_WAV_ALIGN
#ifdef _WIN32
#define XNZ_WAV_ALIGN(X) 
#else
#define XNZ_WAV_ALIGN(X) __attribute__ ((aligned(X)))
#endif
#endif

#ifndef XNZ_WAV_PACK_ATTRIBUTE
#ifdef _WIN32
#define XNZ_WAV_PACK_ATTRIBUTE
#pragma pack(push, 1)
#else
#define XNZ_WAV_PACK_ATTRIBUTE __attribute__ ((packed))
#endif
#endif

#ifdef __APPLE__
#define WAVE_FORMAT_PCM         0x0001
#define WAVE_FORMAT_IEEE_FLOAT  0x0003
#define WAVE_FORMAT_ALAW        0x0006
#define WAVE_FORMAT_MULAW       0x0007
#define WAVE_FORMAT_EXTENSIBLE  65534
#endif

typedef struct WAV_SAMPLE
{
    uint8_t  bytes[4];
    uint16_t shorts[2];
    uint32_t uint;
    int32_t  sint;
    float    f32;
}WAV_SAMPLE;

typedef const enum XNZ_WAV_CHUNK_CODE
{
    WAV_RIFF = 0,
    WAV_FMT  = 1,
    WAV_CUE  = 2,
    WAV_DATA = 3,
    WAV_MAX  
};

static const uint32_t WAV_FOURCC[WAV_MAX] =
{
    (const uint32_t)MAKEFOURCC('R','I','F','F'),
    (const uint32_t)MAKEFOURCC('f','m','t',' '),
    (const uint32_t)MAKEFOURCC('c','u','e',' '),
    (const uint32_t)MAKEFOURCC('d','a','t','a'),
};

#ifdef _WIN32
#pragma pack(push, 1)
#endif

    
/*XNZ_PNG_ALIGN(4)*/ typedef struct XNZ_WAV_PACK_ATTRIBUTE xnz_wav_chunk
{
    union
    {

        uint8_t     chunkID[4];
        uint32_t    fourCC;
    };
    uint32_t        chunkSize;

}xnz_wav_chunk;



/***
 ***    IHDR STRUCT (OPCODE 'IHDR')
 ***/
//#pragma mark -- IHDR RECORD (OPCODE 'IHDR')

#define XNZ_WAV_RIFF_FIELDS \
XNZ_WAV_RIFF_FIELD(XNZ_WAV_RIFF, uint8_t,  8,  4, waveID, waveID[4], "%s")

#define XNZ_WAV_FMT_FIELDS \
XNZ_WAV_FMT_FIELD(XNZ_WAV_FMT, uint16_t, 16, 1, wFormatTag, wFormatTag, "%hu") \
XNZ_WAV_FMT_FIELD(XNZ_WAV_FMT, uint16_t, 16, 1, nChannels, nChannels, "%hu") \
XNZ_WAV_FMT_FIELD(XNZ_WAV_FMT, uint32_t, 32, 1, nSamplesPerSec, nSamplesPerSec, "%d") \
XNZ_WAV_FMT_FIELD(XNZ_WAV_FMT, uint32_t, 32, 1, nAvgBytesPerSec, nAvgBytesPerSec, "%d") \
XNZ_WAV_FMT_FIELD(XNZ_WAV_FMT, uint16_t, 16, 1, nBlockAlign, nBlockAlign, "%hu") \
XNZ_WAV_FMT_FIELD(XNZ_WAV_FMT, uint16_t, 16, 1, wBitsPerSample, wBitsPerSample, "%hu")

//XNZ_WAV_FMT_FIELD(XNZ_PNG_IHDR, uint16_t, 16, 1, wValidBitsPerSample, wValidBitsPerSample, "%hu") \
//XNZ_WAV_FMT_FIELD(XNZ_PNG_IHDR, uint32_t, 32, 1, dwChannelMask, dwChannelMask, "%d") \
//XNZ_WAV_FMT_FIELD(XNZ_PNG_IHDR, uint8_t,  8,  16, GUID, GUID, "%d") \

#define XNZ_WAV_CUE_FIELDS \
XNZ_WAV_CUE_FIELD(XNZ_WAV_CUE, uint32_t, 32, 1, nCuePoints, nCuePoints, "%d") \
XNZ_WAV_CUE_FIELD(XNZ_WAV_CUE, char,      8, 1, points,     points,     "%d")

#define XNZ_WAV_FACT_FIELDS \
XNZ_WAV_FACT_FIELD(XNZ_WAV_FACT, uint32_t, 32, 1, dwSampleLength, dwSampleLength, "%d")

#define XNZ_WAV_LIST_FIELDS \
XNZ_WAV_LIST_FIELD(XNZ_WAV_LIST, uint8_t,  8,  4, typeID, typeID[4], "%s") \
XNZ_WAV_LIST_FIELD(XNZ_WAV_LIST, char,     8,  1, data, data, "%d")


#define XNZ_WAV_DATA_FIELDS \
XNZ_WAV_DATA_FIELD(XNZ_WAV_DATA, WAV_SAMPLE, sizeof(WAV_SAMPLE)*8,  1, sample, sample, "%d")

/*
//--- define the structure, the X macro will be expanded once per field
XNZ_WAVE_ALIGN(1) typedef struct CR_PACK_ATTRIBUTE XNZ_PNG_IHDR
{
//the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_PNG_IHDR_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_PNG_IHDR_FIELDS
#undef XNZ_PNG_IHDR_FIELD
} XNZ_PNG_IHDR;
*/

//--- define the structure, the X macro will be expanded once per field
XNZ_WAV_ALIGN(1) typedef struct XNZ_WAV_PACK_ATTRIBUTE XNZ_WAV_RIFF
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_WAV_RIFF_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
        XNZ_WAV_RIFF_FIELDS
#undef  XNZ_WAV_RIFF_FIELD
} XNZ_WAV_RIFF;

XNZ_WAV_ALIGN(1) typedef struct XNZ_WAV_PACK_ATTRIBUTE XNZ_WAV_FMT
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_WAV_FMT_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
        XNZ_WAV_FMT_FIELDS
#undef  XNZ_WAV_FMT_FIELD
} XNZ_WAV_FMT;


XNZ_WAV_ALIGN(1) typedef struct XNZ_WAV_PACK_ATTRIBUTE XNZ_WAV_CUE
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_WAV_CUE_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
        XNZ_WAV_CUE_FIELDS
#undef  XNZ_WAV_CUE_FIELD
} XNZ_WAV_CUE;

XNZ_WAV_ALIGN(1) typedef struct XNZ_WAV_PACK_ATTRIBUTE XNZ_WAV_FACT
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_WAV_FACT_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
        XNZ_WAV_FACT_FIELDS
#undef  XNZ_WAV_FACT_FIELD
} XNZ_WAV_FACT;


XNZ_WAV_ALIGN(1) typedef struct XNZ_WAV_PACK_ATTRIBUTE XNZ_WAV_LIST
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_WAV_LIST_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
        XNZ_WAV_LIST_FIELDS
#undef  XNZ_WAV_LIST_FIELD
} XNZ_WAV_LIST;


XNZ_WAV_ALIGN(1) typedef struct XNZ_WAV_PACK_ATTRIBUTE XNZ_WAV_DATA
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_WAV_DATA_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
        XNZ_WAV_DATA_FIELDS
#undef  XNZ_WAV_DATA_FIELD
} XNZ_WAV_DATA;


typedef struct XNZ_WAV_ARCHIVE
{
    union {
        struct {
            union{
                XNZ_WAV_RIFF*        riff;
                void*                buffer;
            };
            size_t size;
        };
        CRFile file;
    };

    //xnz_wav_chunk* chunks;

    XNZ_WAV_FMT*         fmt;
    XNZ_WAV_CUE*         cue;
    XNZ_WAV_LIST*        list;
    XNZ_WAV_FACT*        fact;

    //If the data is an odd number of bytes in length, a zero pad byte must be added at the end.
    //The pad byte is not included in ckSize
    XNZ_WAV_DATA*        data;

    WAV_SAMPLE*          samples;

}XNZ_WAV_ARCHIVE;

#ifdef _WIN32
#pragma pack(pop)   //end struct single byte packing
#endif

XNZ_WAV_API XNZ_WAV_INLINE void   xnz_wav_open(XNZ_WAV_ARCHIVE* archive, char* pngFilePath);
//XNZ_WAV_API XNZ_WAV_INLINE size_t xnz_wav_decode_blocks(XNZ_PNG_IMAGE* archive, uint8_t* dst_buf, size_t dst_size);
XNZ_WAV_API XNZ_WAV_INLINE void   xnz_wav_close(XNZ_WAV_ARCHIVE* archive);


XNZ_WAV_API XNZ_WAV_INLINE void xnz_wav_parse_chunks(XNZ_WAV_ARCHIVE* archive)
{
    

    xnz_wav_chunk* chunk = NULL;// {0};
    unsigned int  numChunks = 0;
    unsigned int  chunkDataLength= 0;

    size_t bytesRead = 0;
    char* bufferPosition = archive->file.buffer;


    fprintf(stderr, "sizeof(XNZ_RIFF_CHUNK) == %zu\n", sizeof(XNZ_WAV_RIFF));
    fprintf(stderr, "sizeof(xnz_wav_chunk) == %zu\n",  sizeof(xnz_wav_chunk));

    assert(archive->riff->waveID[0] == 'W');
    assert(archive->riff->waveID[1] == 'A');
    assert(archive->riff->waveID[2] == 'V');
    assert(archive->riff->waveID[3] == 'E');

    /*
    archive->fmt = (XNZ_WAV_FMT*)( archive->file.buffer + sizeof(XNZ_WAV_RIFF) );

    assert(archive->fmt->chunkID[0] == 'f');
    assert(archive->fmt->chunkID[1] == 'm');
    assert(archive->fmt->chunkID[2] == 't');
    assert(archive->fmt->chunkID[3] == ' ');

    fprintf(stderr, "XNZ_WAV_FMT Chunk ID: %.4s\n",      archive->fmt->chunkID);
    fprintf(stderr, "XNZ_WAV_FMT Chunk Size: %d\n",      archive->fmt->chunkSize);
    fprintf(stderr, "XNZ_WAV_FMT wFormatTag: %hu\n",     archive->fmt->wFormatTag);
    fprintf(stderr, "XNZ_WAV_FMT nChannels: %hu\n",      archive->fmt->nChannels);
    fprintf(stderr, "XNZ_WAV_FMT nSamplesPerSec: %u\n",  archive->fmt->nSamplesPerSec);
    fprintf(stderr, "XNZ_WAV_FMT nAvgBytesPerSec: %u\n", archive->fmt->nAvgBytesPerSec);
    fprintf(stderr, "XNZ_WAV_FMT nBlockAlign: %hu\n",    archive->fmt->nBlockAlign);
    fprintf(stderr, "XNZ_WAV_FMT wBitsPerSample: %hu\n", archive->fmt->wBitsPerSample);

    //if no compressed data there will be no fact chunk
    //archive->fact = (XNZ_WAV_FACT*)(archive->file.buffer + sizeof(XNZ_WAV_RIFF) + sizeof(XNZ_WAV_FMT));
    //assert(archive->fact->chunkID[0] == 'f');
    //assert(archive->fact->chunkID[1] == 'a');
    //assert(archive->fact->chunkID[2] == 'c');
    //assert(archive->fact->chunkID[3] == 't');

    archive->data = (XNZ_WAV_DATA*)(archive->file.buffer + sizeof(XNZ_WAV_RIFF) + sizeof(XNZ_WAV_FMT));

    assert(archive->data->chunkID[0] == 'd');
    assert(archive->data->chunkID[1] == 'a');
    assert(archive->data->chunkID[2] == 't');
    assert(archive->data->chunkID[3] == 'a');

    fprintf(stderr, "XNZ_WAV_DATA Chunk ID: %.4s\n", archive->data->chunkID);
    fprintf(stderr, "XNZ_WAV_DATA Chunk Size: %d\n", archive->data->chunkSize);
    */
     
    bytesRead      += sizeof(XNZ_WAV_RIFF);
    bufferPosition += sizeof(XNZ_WAV_RIFF);

    //read the header record
    while (bytesRead < archive->file.size)
    {
        //read 8 bytes into the struct containing the opcode and chunk length (in bytes) as words, respectively
        //fread(&record, 4, 1, file);
        //memcpy(&chunk, bufferPosition, sizeof(xnz_wav_chunk)); //bufferPosition += sizeof(xnz_wav_chunk);

        chunk = (xnz_wav_chunk*)bufferPosition;

        //convert 16 bits from big endian to little endian
        //record.opcode = cr_be16toh(record.opcode);//(( (record.opcode & 0xff)<<8 | (record.opcode & 0xff00)>>8 ));
        //record.length = cr_be16toh(record.length);//(( (record.length & 0xff)<<8 | (record.length & 0xff00)>>8 ));

        //OpCodeCount[record.opcode - 1]++;
        chunkDataLength = chunk->chunkSize + sizeof(xnz_wav_chunk);

        switch (chunk->fourCC)
        {
            //case (MAKEFOURCC('R','I','F','F')) :
            //    archive->riff = (XNZ_WAV_RIFF*)&chunk; chunkDataLength = sizeof(XNZ_WAV_RIFF); break;
            case (MAKEFOURCC('f','m', 't',' ')):
                archive->fmt  =  (XNZ_WAV_FMT*)chunk; break;
            case (MAKEFOURCC('c','u','e',' ')):
                archive->cue  =  (XNZ_WAV_CUE*)chunk; break;
            case (MAKEFOURCC('L','I','S','T')):
                archive->list = (XNZ_WAV_LIST*)chunk; break;
            case (MAKEFOURCC('f','a','c','t')):
                archive->fact = (XNZ_WAV_FACT*)chunk; break;
            case (MAKEFOURCC('d','a','t','a')):
                archive->data = (XNZ_WAV_DATA*)chunk; break;

            default:
                fprintf(stderr, "xnz_wav_parse_chunks::Unhandled WAV Chunk: %.4s\n", chunk->chunkID);
        }

        //seek forward to start of next record
        //fseek(file, recordDataLength, SEEK_CUR);
        bufferPosition += chunkDataLength;
        //long currentByte = ftell(file);
        bytesRead += chunkDataLength;// chunk.chunkSize;
        numChunks++;
    }

    //assert(bytesRead - sizeof(XNZ_WAV_RIFF) == archive->riff->chunkSize - 4);
}

XNZ_WAV_API XNZ_WAV_INLINE void xnz_wav_open(XNZ_WAV_ARCHIVE* archive, char* wavFilePath)
{

    if (wavFilePath)
    {
        //1 OPEN THE GBL OR GLTF FILE FOR READING AND GET FILESIZE USING LSEEK
        archive->file.fd   = cr_file_open(wavFilePath);
        archive->file.size = cr_file_size(archive->file.fd);
        archive->file.path = wavFilePath;

        fprintf(stderr, "\nxnz_wav_open::WAV File Size =  %lu bytes\n", archive->file.size);

        //2 MAP THE FILE TO BUFFER FOR READING
#ifndef _WIN32
        archive->file.buffer = (char*)cr_file_map_to_buffer(&(archive->file.buffer), archive->file.size, PROT_READ, MAP_SHARED | MAP_NORESERVE, archive->file.fd, 0);
        if (madvise(archive->file.buffer, (size_t)archive->file.size, MADV_SEQUENTIAL | MADV_WILLNEED) == -1) {
            printf("\nread madvise failed\n");
        }
#else
        archive->file.mFile = cr_file_map_to_buffer(&(archive->file.buffer), archive->file.size, PROT_READ, MAP_SHARED | MAP_NORESERVE, archive->file.fd, 0);
#endif
    }
    else assert(archive->file.buffer);

    //3  READ THE PNG (.png) file signature
    archive->riff = (XNZ_WAV_RIFF*)archive->file.buffer;

    assert(archive->riff->chunkID[0] == 'R');
    assert(archive->riff->chunkID[1] == 'I');
    assert(archive->riff->chunkID[2] == 'F');
    assert(archive->riff->chunkID[3] == 'F');

    /*
     //get some memory to serialize the record
     record = &(records[recordIndex]);

     //read 4 bytes into the struct containing the opcode and record length (in bytes) as shorts, respectively
     //fread(record, 4, 1, file);
     memcpy(record, bufferPosition, 4);
     bufferPosition += 4;

     //convert 16 bits from big endian to little endian
     record->opcode = xnz_be16toh(record->opcode);//(( (record.opcode & 0xff)<<8 | (record.opcode & 0xff00)>>8 ));
     record->length = xnz_be16toh(record->length);//(( (record.length & 0xff)<<8 | (record.length & 0xff00)>>8 ));

     //allocate memory for reading the FLT header record data
     //TO DO: create a pool of filesize - numRecords * sizeof(recordHeader) to grab this memory from
     recordDataLength = record->length - sizeof(char *);
     record->data = (char*)malloc(recordDataLength);
     */

     //  Read IHDR Chunk
     //
     //  The sample depth is the same as the bit depth except in the case of indexed-colour PNG images (colour type 3), in which the sample depth is always 8 bits (see 4.4: PNG image).
     //
     //  Compression method is a single-byte integer that indicates the method used to compress the image data. Only compression method 0 (deflate/inflate compression with a sliding window of at most 32768
     //  bytes) is defined in this International Standard. All conforming PNG images shall be compressed with this scheme.
     //
     //  Filter method is a single-byte integer that indicates the preprocessing method applied to the image data before compression. Only filter method 0 (adaptive filtering with five basic filter types) is
     //  defined in this International Standard. See clause 9: Filtering for details.
     //
     //  Interlace method is a single-byte integer that indicates the transmission order of the image data. Two values are defined in this International Standard: 0 (no interlace) or 1 (Adam7 interlace). See
     //  clause 8: Interlacing and pass extraction for details.
     //

    xnz_wav_parse_chunks(archive);
    

    archive->samples = &archive->data->sample;
  
}


XNZ_AIF_API XNZ_AIF_INLINE void xnz_wav_read_samples(XNZ_WAV_ARCHIVE* archive, unsigned long long numFramesToRead, void** sampleBuffers)
{
    uint32_t nSampleFrames = archive->data->chunkSize / archive->fmt->nBlockAlign;

    switch (archive->fmt->wBitsPerSample)
    {

        case (16):
        {
            //int16_t** shortBuffers = (int16_t**)archive->samples;
            int16_t* shortSamplesL = (int16_t*)archive->samples;// shortBuffers[0];
            //int16_t* shortSamplesR = (int16_t*)(player->sourceAudioFile.samples[1]);

            //dest
            int16_t* hostBuffer = (int16_t*)sampleBuffers[0];

            for (int channel = 0; channel < archive->fmt->nChannels; channel++)
            {
                for (int frame = 0; frame < nSampleFrames; frame++)
                {
                    hostBuffer[frame * archive->fmt->nChannels + channel] = shortSamplesL[frame * archive->fmt->nChannels + channel];
                }
            }
            break;
        }

        case (24):
        {
            //int16_t** shortBuffers = (int16_t**)archive->samples;
            char* sourceSamples = (char*)archive->samples;// shortBuffers[0];
            //int16_t* shortSamplesR = (int16_t*)(player->sourceAudioFile.samples[1]);

            //dest
            char* hostBuffer = (char*)sampleBuffers[0];

            for (int channel = 0; channel < archive->fmt->nChannels; channel++)
            {
                for (int frame = 0; frame < nSampleFrames; frame++)
                {
                    memcpy(&hostBuffer[(frame * archive->fmt->nChannels + channel) * 3], &sourceSamples[(frame * archive->fmt->nChannels + channel) * 3], 3);
                }
            }
            break;

        }
            
        case (32):
        {
            //int16_t** shortBuffers = (int16_t**)archive->samples;
            char* sourceSamples = (char*)archive->samples;// shortBuffers[0];
            //int16_t* shortSamplesR = (int16_t*)(player->sourceAudioFile.samples[1]);

            //dest
            char* hostBuffer = (char*)sampleBuffers[0];

            for (int channel = 0; channel < archive->fmt->nChannels; channel++)
            {
                for (int frame = 0; frame < nSampleFrames; frame++)
                {
                    memcpy(&hostBuffer[(frame * archive->fmt->nChannels + channel) * 4], &sourceSamples[(frame * archive->fmt->nChannels + channel) * 4], 4);
                }
            }
         
            break;
        }

        default:
            assert(1 == 0);
    }
}

XNZ_WAV_API XNZ_WAV_INLINE void xnz_wav_close(XNZ_WAV_ARCHIVE* archive)
{
    CRFileClose(&(archive->file));
    //xnz_file_unmap(crate->file.mFile, crate->file.buffer);
    //xnz_file_close(crate->file.fd);
}


#ifdef __cplusplus
}
#endif


#endif //_XNZ_WAV_CODEC_H_
