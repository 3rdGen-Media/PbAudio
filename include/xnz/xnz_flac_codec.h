#ifndef _XNZ_FLAC_CODEC_H_
#define _XNZ_FLAC_CODEC_H_

#include "cr_file.h" //CRFile
//#include "stdint.h"         //uint types

#include <xnz/xnz.h>

#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h> //MD5 hash
#endif

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
#ifndef XNZ_FLAC_API
#define XNZ_FLAC_API static
#endif

//inline doesn't exist in C89, __inline is MSVC specific
#ifndef XNZ_FLAC_INLINE
#ifdef _WIN32
#define XNZ_FLAC_INLINE __inline
#else
#define XNZ_FLAC_INLINE
#endif
#endif

//__decspec doesn't exist in C89, __declspec is MSVC specific
#ifndef XNZ_FLAC_DECLSPEC
#ifdef _WIN32
#define XNZ_FLAC_DECLSPEC __declspec
#else
#define XNZ_FLAC_DECLSPEC
#endif
#endif

//align functions are diffent on windows vs iOS, Linux, etc.
#ifndef XNZ_FLAC_ALIGN
#ifdef _WIN32
#define XNZ_FLAC_ALIGN(X) 
#else
#define XNZ_FLAC_ALIGN(X) __attribute__ ((aligned(X)))
#endif
#endif

#ifndef XNZ_FLAC_PACK_ATTRIBUTE
#ifdef _WIN32
#define XNZ_FLAC_PACK_ATTRIBUTE
#pragma pack(push, 1)
#else
#define XNZ_FLAC_PACK_ATTRIBUTE __attribute__ ((packed))
#endif
#endif

#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                \
    ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | \
     ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))
#endif /* defined(MAKEFOURCC) */


typedef unsigned long long uint64;


typedef struct FLAC_SAMPLE
{
    uint8_t  bytes[4];
    uint16_t shorts[2];
    uint32_t uint;
    int32_t  sint;
    float    f32;
}FLAC_SAMPLE;

//EA IFF 85 chunks in precedent order
typedef const enum XNZ_FLAC_CHUNK_CODE
{
    FLAC_FORM = 0,
    FLAC_COMM = 1,
    FLAC_SSND = 2,
    FLAC_MARK = 3,
    FLAC_INST = 4,
    FLAC_COMT = 5,

    //Text Chunks
    FLAC_NAME = 6,
    FLAC_AUTH = 7,
    FLAC_CPRT = 8,
    FLAC_ANNO = 9,

    FLAC_AESD = 10,
    FLAC_MIDI = 11,
    FLAC_APPL = 12,
        
    FLAC_MAX
};

static const uint32_t FLAC_FOURCC[FLAC_MAX] =
{
    (const uint32_t)MAKEFOURCC('F','O','R','M'),
    (const uint32_t)MAKEFOURCC('C','O','M','M'),
    (const uint32_t)MAKEFOURCC('S','S','N','D'),
    (const uint32_t)MAKEFOURCC('M','A','R','K'),
    (const uint32_t)MAKEFOURCC('I','N','S','T'),
    (const uint32_t)MAKEFOURCC('C','O','M','T'),
    (const uint32_t)MAKEFOURCC('N','A','M','E'),
    (const uint32_t)MAKEFOURCC('A','U','T','H'),
    (const uint32_t)MAKEFOURCC('C','P','R','T'),
    (const uint32_t)MAKEFOURCC('A','N','N','O'),
    (const uint32_t)MAKEFOURCC('A','E','S','D'),
    (const uint32_t)MAKEFOURCC('M','I','D','I'),
    (const uint32_t)MAKEFOURCC('A','P','P','L'),
};

#ifdef _WIN32
#pragma pack(push, 1)
#endif


/*XNZ_PNG_ALIGN(4)*/ typedef struct XNZ_FLAC_PACK_ATTRIBUTE xnz_flac_chunk
{
    union
    {

        uint8_t     chunkID[4];
        uint32_t    fourCC;
    };
    int32_t        chunkSize;

}xnz_flac_chunk;

#define XNZ_FLAC_CHUNK_FIELDS \
XNZ_FLAC_CHUNK_FIELD(xnz_flac_chunk, uint8_t,  8,  4, chunkID,   chunkID[4], "%s") \
XNZ_FLAC_CHUNK_FIELD(xnz_flac_chunk, int32_t, 32,  1, chunkSize, chunkSize, "%hu") \



/*XNZ_PNG_ALIGN(4)*/ /*typedef struct XNZ_FLAC_PACK_ATTRIBUTE xnz_flac_metadata_block_header
{
    union
    {

        uint8_t     chunkID[4];
        uint32_t    length;
    };


}xnz_flac_metadata_block_header;
*/

/***
***    IHDR STRUCT (OPCODE 'IHDR')
***/
//#pragma mark -- IHDR RECORD (OPCODE 'IHDR')

#define XNZ_FLAC_METADATA_STREAMINFO_FIELDS \
XNZ_FLAC_METADATA_STREAMINFO_FIELD(xnz_flac_metadata_stream_info , uint64_t,  64,  1, nSampleFrames,   nSampleFrames,      "llu") \
XNZ_FLAC_METADATA_STREAMINFO_FIELD(xnz_flac_metadata_stream_info , uint32_t,  32,  1, minFrameSize,    minFrameSize,       "lu") \
XNZ_FLAC_METADATA_STREAMINFO_FIELD(xnz_flac_metadata_stream_info , uint32_t,  32,  1, maxFrameSize,    maxFrameSize,       "lu") \
XNZ_FLAC_METADATA_STREAMINFO_FIELD(xnz_flac_metadata_stream_info , uint32_t,  32,  1, sampleRate,      sampleRate,         "lu") \
XNZ_FLAC_METADATA_STREAMINFO_FIELD(xnz_flac_metadata_stream_info , uint16_t,  16,  1, minBlockSize,    minBlockSize,       "hu") \
XNZ_FLAC_METADATA_STREAMINFO_FIELD(xnz_flac_metadata_stream_info , uint16_t,  16,  1, maxBlockSize,    maxBlockSize,       "hu") \
XNZ_FLAC_METADATA_STREAMINFO_FIELD(xnz_flac_metadata_stream_info , uint8_t,    8,  1, nChannels,       nChannels,          "u") \
XNZ_FLAC_METADATA_STREAMINFO_FIELD(xnz_flac_metadata_stream_info , uint8_t,    8,  1, bitsPerSample,   bitsPerSample,      "u") \
XNZ_FLAC_METADATA_STREAMINFO_FIELD(xnz_flac_metadata_stream_info , uint8_t,    8, 16, signature,       signature[16],          "%x") \

//XNZ_FLAC_STREAMINFO_FIELD(XNZ_FLAC_STREAMINFO , uint8_t,  8,  4, magicNumber, magicNumber[4], "%s") \
//XNZ_FLAC_STREAMINFO_FIELD(XNZ_FLAC_STREAMINFO , uint8_t,  8,  4, magicNumber, magicNumber[4], "%s") \


typedef char float_ext[10]; //80 bit IEEE Standard 754

#define XNZ_FLAC_COMM_FIELDS \
XNZ_FLAC_COMM_FIELD(XNZ_FLAC_COMM, uint16_t,   16, 1, nChannels,     nChannels,     "%hu") \
XNZ_FLAC_COMM_FIELD(XNZ_FLAC_COMM, uint32_t,   32, 1, nSampleFrames, nSampleFrames, "%d") \
XNZ_FLAC_COMM_FIELD(XNZ_FLAC_COMM, uint16_t,   16, 1, sampleSize,    sampleSize,    "%hu") \
XNZ_FLAC_COMM_FIELD(XNZ_FLAC_COMM, char ,      80, 1, sampleRate,    sampleRate[10],    "%s")

//XNZ_FLAC_FMT_FIELD(XNZ_PNG_IHDR, uint16_t, 16, 1, wValidBitsPerSample, wValidBitsPerSample, "%hu") \
//XNZ_FLAC_FMT_FIELD(XNZ_PNG_IHDR, uint32_t, 32, 1, dwChannelMask, dwChannelMask, "%d") \
//XNZ_FLAC_FMT_FIELD(XNZ_PNG_IHDR, uint8_t,  8,  16, GUID, GUID, "%d") \

XNZ_FLAC_ALIGN(1) typedef struct XNZ_FLAC_PACK_ATTRIBUTE xnz_flac_comment
{
    uint32_t            timeStamp;
    short               marker;
    unsigned short      count;
    //char                text;
} xnz_flac_comment;

#define XNZ_FLAC_COMT_FIELDS \
XNZ_FLAC_COMT_FIELD(XNZ_FLAC_COMT, uint16_t,          16,                      1, nComments,     nComments,     "%hu")

//XNZ_FLAC_COMT_FIELD(XNZ_FLAC_COMT, xnz_flac_comment,   sizeof(xnz_flac_comment), 1, comment, comment, "%d") \
//XNZ_FLAC_COMT_FIELD(XNZ_FLAC_COMT, uint16_t,   16, 1, sampleSize,    sampleSize,    "%hu") \
//XNZ_FLAC_COMT_FIELD(XNZ_FLAC_COMT, float_ext , 80, 1, sampleRate,    sampleRate,    "%s")


#define XNZ_FLAC_CUE_FIELDS \
XNZ_FLAC_CUE_FIELD(XNZ_FLAC_CUE, uint32_t, 32, 1, nCuePoints, nCuePoints, "%d") \
XNZ_FLAC_CUE_FIELD(XNZ_FLAC_CUE, char,      8, 1, points,     points,     "%d")

#define XNZ_FLAC_FACT_FIELDS \
XNZ_FLAC_FACT_FIELD(XNZ_FLAC_FACT, uint32_t, 32, 1, dwSampleLength, dwSampleLength, "%d")

#define XNZ_FLAC_LIST_FIELDS \
XNZ_FLAC_LIST_FIELD(XNZ_FLAC_LIST, uint8_t,  8,  4, typeID, typeID[4], "%s") \
XNZ_FLAC_LIST_FIELD(XNZ_FLAC_LIST, char,     8,  1, data, data, "%d")



#define XNZ_FLAC_SSND_FIELDS \
XNZ_FLAC_SSND_FIELD(XNZ_FLAC_SSND, uint32_t,   32,                    1, offset,    offset, "%d") \
XNZ_FLAC_SSND_FIELD(XNZ_FLAC_SSND, uint32_t,   32,                    1, blockSize, blockSize, "%d") \

//XNZ_FLAC_SSND_FIELD(XNZ_FLAC_SSND, FLAC_SAMPLE, sizeof(FLAC_SAMPLE)*8,  1, sample,    sample, "%d")

/*
//--- define the structure, the X macro will be expanded once per field
XNZ_FLACE_ALIGN(1) typedef struct CR_PACK_ATTRIBUTE XNZ_PNG_IHDR
{
//the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_PNG_IHDR_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_PNG_IHDR_FIELDS
#undef XNZ_PNG_IHDR_FIELD
} XNZ_PNG_IHDR;
*/

//--- define the structure, the X macro will be expanded once per field
XNZ_FLAC_ALIGN(1) typedef struct XNZ_FLAC_PACK_ATTRIBUTE xnz_flac_metadata_streaminfo
{
    //uint8_t     chunkID[4];
    //uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_FLAC_METADATA_STREAMINFO_FIELD(xnz_flac_struct_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
        XNZ_FLAC_METADATA_STREAMINFO_FIELDS
#undef  XNZ_FLAC_METADATA_STREAMINFO_FIELD
} xnz_flac_metadata_streaminfo;

    
XNZ_FLAC_ALIGN(1) typedef struct XNZ_FLAC_PACK_ATTRIBUTE XNZ_FLAC_COMM
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_FLAC_COMM_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_FLAC_COMM_FIELDS
#undef  XNZ_FLAC_COMM_FIELD
} XNZ_FLAC_COMM;


XNZ_FLAC_ALIGN(1) typedef struct XNZ_FLAC_PACK_ATTRIBUTE XNZ_FLAC_COMT
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_FLAC_COMT_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
        XNZ_FLAC_COMT_FIELDS
#undef  XNZ_FLAC_COMT_FIELD
} XNZ_FLAC_COMT;

/*
XNZ_FLAC_ALIGN(1) typedef struct XNZ_FLAC_PACK_ATTRIBUTE XNZ_FLAC_CUE
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_FLAC_CUE_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_FLAC_CUE_FIELDS
#undef  XNZ_FLAC_CUE_FIELD
} XNZ_FLAC_CUE;

XNZ_FLAC_ALIGN(1) typedef struct XNZ_FLAC_PACK_ATTRIBUTE XNZ_FLAC_FACT
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_FLAC_FACT_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_FLAC_FACT_FIELDS
#undef  XNZ_FLAC_FACT_FIELD
} XNZ_FLAC_FACT;


XNZ_FLAC_ALIGN(1) typedef struct XNZ_FLAC_PACK_ATTRIBUTE XNZ_FLAC_LIST
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_FLAC_LIST_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_FLAC_LIST_FIELDS
#undef  XNZ_FLAC_LIST_FIELD
} XNZ_FLAC_LIST;

*/

XNZ_FLAC_ALIGN(1) typedef struct XNZ_FLAC_PACK_ATTRIBUTE XNZ_FLAC_SSND
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_FLAC_SSND_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_FLAC_SSND_FIELDS
#undef  XNZ_FLAC_SSND_FIELD
} XNZ_FLAC_SSND;


#pragma mark -- SERIALIZE



/***
 ***    UNIVERSAL SERIALIZE FUNCTION FOR ALL RECORDS DATA FIELDS PARSED BY X-MACROS
 ***
 ***    Converts FLT Record Big Endian Data Types to host format (e.g. Intel = Little Endian) using system level conversion calls
 ***/
static void xnz_flac_chunk_field_serialize(void* data, unsigned int elementSizeInBits, unsigned int numElements)
{
    int i;
    //if the data type is 8 bits in size, such as char, then there is no need to convert it
    //because endianness only affects byte order and char is a single byte data type
    if (elementSizeInBits == 8)
        return;


    /*
    if( numElements > 1 )
    {
        if( elementSizeInBits == 64 )
        {
            unsigned long * doubleData = (unsigned long *)data;
            for(int i = 0; i< numElements; i++)
            {
                doubleData[i] = cr_be64toh(doubleData[i]);
            }

        }
        else if( elementSizeInBits == 32 )
        {
            unsigned int * intData = (unsigned int *)data;
            for(int i = 0; i< numElements; i++)
            {
                intData[i] = cr_be32toh(intData[i]);
            }

        }
        else if( elementSizeInBits == 16 )
        {
            short * shortData = (short *)data;
            for(int i = 0; i< numElements; i++)
            {
                shortData[i] = cr_be16toh(shortData[i]);
            }
        }
    }
    else
    {
     */
    if (elementSizeInBits == 64)
    {
        uint64_t* doubleData = (uint64_t*)data;
        *doubleData = xnz_be64toh(*doubleData);
        for (i = 1; i < numElements; i++)
        {
            doubleData[i] = xnz_be64toh(doubleData[i]);
        }

    }
    else if (elementSizeInBits == 32)
    {
        uint32_t* intData = (uint32_t*)data;
        *intData = xnz_be32toh(*intData);
        for (i = 1; i < numElements; i++)
        {
            intData[i] = xnz_be32toh(intData[i]);
        }

    }
    else if (elementSizeInBits == 16)
    {
        uint16_t* shortData = (uint16_t*)data;
        *shortData = xnz_be16toh(*shortData);
        for (i = 1; i < numElements; i++)
        {
            shortData[i] = xnz_be16toh(shortData[i]);
        }
    }

    //}
}


//Serialize specific record types;  X-MACROS are used to iterate over each field in each flt record struct with minimal code
static void xnz_flac_comm_chunk_serialize(void* chunk_data)
{
#define XNZ_FLAC_CHUNK_FIELD(xnz_chunk_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) \
        xnz_flac_chunk_field_serialize(&(((xnz_chunk_data*)chunk_data)->varName), dataTypeSizeInBits, numElements);
        XNZ_FLAC_CHUNK_FIELDS
#undef  XNZ_FLAC_CHUNK_FIELD


#define XNZ_FLAC_COMM_FIELD(xnz_chunk_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) \
        xnz_flac_chunk_field_serialize(&(((xnz_chunk_data*)chunk_data)->varName), dataTypeSizeInBits, numElements);
        XNZ_FLAC_COMM_FIELDS
#undef  XNZ_FLAC_COMM_FIELD

}

//Serialize specific record types;  X-MACROS are used to iterate over each field in each flt record struct with minimal code
static void xnz_flac_ssnd_chunk_serialize(void* chunk_data)
{
#define XNZ_FLAC_CHUNK_FIELD(xnz_chunk_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) \
        xnz_flac_chunk_field_serialize(&(((xnz_chunk_data*)chunk_data)->varName), dataTypeSizeInBits, numElements);
        XNZ_FLAC_CHUNK_FIELDS
#undef  XNZ_FLAC_CHUNK_FIELD


#define XNZ_FLAC_SSND_FIELD(xnz_chunk_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) \
        xnz_flac_chunk_field_serialize(&(((xnz_chunk_data*)chunk_data)->varName), dataTypeSizeInBits, numElements);
        XNZ_FLAC_SSND_FIELDS
#undef  XNZ_FLAC_SSND_FIELD

}



/***
 ***    PRINT FUNCTIONS FOR ALL [SUPPORTED] RECORD TYPES
 ***/
#pragma mark -- PRINT


 /***
  ***    UNIVERSAL PRINT FUNCTION FOR ALL RECORD DATA FIELDS PARSED BY X-MACROS
  ***/
static void xnz_flac_chunk_field_print(char* formatString, char* catString, char* fieldName, void* data, unsigned int elementSizeInBits, unsigned int numElements/*, char * formatSpecifier */, bool floatingPoint)
{
    int i, bit;
    unsigned* bits;

    if (strstr(fieldName, "reserved") != NULL)
        return;

    if (elementSizeInBits == 64)
    {
        if (floatingPoint)
        {
            double* doubleData = (double*)data;
            fprintf(stderr, formatString, *doubleData);
            for (i = 1; i < numElements; i++)
            {
                fprintf(stderr, catString, doubleData[i]);
            }
        }
        else
        {
            uint64_t* intData = (uint64_t*)data;
            fprintf(stderr, formatString, *intData);
            for (i = 1; i < numElements; i++)
            {
                fprintf(stderr, catString, intData[i]);
            }
        }
    }
    else if (elementSizeInBits == 32)
    {

        if (strstr(fieldName, "flags") != NULL)
        {
            fprintf(stderr, "\n\t%s = ", fieldName);
            bits = xnz_convert_to_binary((unsigned*)data, elementSizeInBits);

            for (bit = elementSizeInBits; bit--;)
                fprintf(stderr, "%u", +bits[bit]);

            free(bits);
        }
        else
        {
            if (floatingPoint)
            {
                float* floatData = (float*)data;
                fprintf(stderr, formatString, *floatData);
                for (i = 1; i < numElements; i++)
                {
                    fprintf(stderr, catString, floatData[i]);
                }
            }
            else
            {
                uint32_t* intData = (uint32_t*)data;
                fprintf(stderr, formatString, *intData);
                for (i = 1; i < numElements; i++)
                {
                    fprintf(stderr, catString, intData[i]);
                }
            }


        }
    }
    else if (elementSizeInBits == 16)
    {
        if (strstr(fieldName, "flags") != NULL)
        {
            fprintf(stderr, "\n\t%s = ", fieldName);
            bits = xnz_convert_to_binary((unsigned*)data, elementSizeInBits);

            for (bit = elementSizeInBits; bit--;)
                fprintf(stderr, "%u", +bits[bit]);

            free(bits);
        }
        else
        {
            short* shortData = (short*)data;
            fprintf(stderr, formatString, *shortData);
            for (i = 1; i < numElements; i++)
            {
                fprintf(stderr, catString, shortData[i]);
            }
        }
    }
    else
    {

        //how to differentiate between 8-bit integers stored as chars and and chararacter arrays which are strings?
        //solution is to assign %u instead of percent %s for static char arrays defined using x macros and print each element consecutively

        //arrays of 8-bit ints currently unsupported
        char* charData = (char*)data;
        if (numElements > 1)
            fprintf(stderr, formatString, charData);
        else
            fprintf(stderr, formatString, *charData);
        //for(int i = 1; i< numElements; i++)
        //{
        //    printf(catString, shortData[i]);
        //}
    }

}

//sprintf (formatString, "\t%s = %s\n", #varName, stringFormat); \
//printf(formatString, ((cr_flt_record_data *)flt_record_data)->varName);

static void xnz_flac_comm_chunk_print(void* chunk_data)
{
    char formatString[256];
    char catString[256];

#define XNZ_FLAC_COMM_FIELD(xnz_chunk_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) \
       sprintf (formatString, "\n\t%s = %s", #varName, stringFormat); \
       sprintf(catString, ", %s", stringFormat); \
       xnz_flac_chunk_field_print(formatString, catString, #varName, &(((xnz_chunk_data*)chunk_data)->varName), dataTypeSizeInBits, numElements, strncmp(#datatype, "float", 5) == 0 || strncmp(#datatype, "double", 6) == 0 );
       XNZ_FLAC_COMM_FIELDS
#undef XNZ_FLAC_COMM_FIELD

}





static void xnz_flac_chunk_print(xnz_flac_chunk* chunk)
{
    /*
    fprintf(stderr, "\%s\n{", CR_FLT_RECORD_NAMES[flt_record->opcode - 1]);
    fprintf(stderr, "\n\topcode = %hu", flt_record->opcode);
    fprintf(stderr, "\n\tlength = %hu", flt_record->length);

    if (flt_record->opcode == CR_FLT_RECORD_HEADER)
    {
        cr_flt_header_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_GROUP)
    {
        cr_flt_group_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_OBJECT)
    {
        cr_flt_object_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_FACE)
    {
        cr_flt_face_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_MATERIAL_PALETTE)
    {
        cr_flt_material_palette_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_TEXTURE_PALETTE)
    {
        cr_flt_texture_palette_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_VERTEX_PALETTE)
    {
        cr_flt_vertex_palette_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_VERTEX_COLOR)
    {
        cr_flt_vertex_color_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_VERTEX_COLOR_NORMAL)
    {
        cr_flt_vertex_color_normal_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_VERTEX_COLOR_NORMAL_UV)
    {
        cr_flt_vertex_color_normal_uv_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_VERTEX_COLOR_UV)
    {
        cr_flt_vertex_color_uv_record_print(flt_record->data);
    }
    else if (flt_record->opcode == CR_FLT_RECORD_VERTEX_LIST)
    {
        cr_flt_vertex_list_record_print(flt_record->data, (flt_record->length - sizeof(char *)));
    }

    fprintf(stderr, "\n}\n");
    */

}


typedef enum XNZ_FLAC_METADATA_BLOCK_TYPE
{
    FLAC_METADATA_BLOCK_STREAMINFO,
    FLAC_METADATA_BLOCK_PADDING,
    FLAC_METADATA_BLOCK_APPLICATION,
    FLAC_METADATA_BLOCK_SEEKTABLE,
    FLAC_METADATA_BLOCK_VORBIS_COMMENT,
    FLAC_METADATA_BLOCK_CUESHEET,
    FLAC_METADATA_BLOCK_PICTURE,
    //FLAC_METADATA_BLOCK_RESERVED,   // 7-126: reserved
    FLAC_METADATA_BLOCK_INVALID = 127 //   127: invalid, to avoid confusion with a frame sync code
    
}XNZ_FLAC_METADATA_BLOCK_TYPE;

static uint32_t FLAC_SAMPLE_RATES[16] =
{
    0,
    88200,
    176400,
    192000,
    8000,
    16000,
    22050,
    24000,
    32000,
    44100,
    48000,
    96000,
    12, //get 8 bit sample rate (in kHz) from end of header
    13, //get 16 bit sample rate (in Hz) from end of header
    14, //get 16 bit sample rate (in tens of Hz) from end of header
    15  //invalid, to prevent sync-fooling string of 1s
};


typedef struct xnz_flac_frame
{
    uint32_t blockSize;
    uint32_t sampleRate;
    
    uint16_t syncCode;
    uint16_t blockingStrategy;
    
    uint8_t  nChannels;
    uint8_t  sampleSize;
    
    uint8_t  channelID;
    //uint8_t  channelID;

}xnz_flac_frame;


typedef struct XNZ_FLAC_ARCHIVE
{
    union {
        struct {
            union {
                void* buffer;
            };
            size_t size;
        };
        CRFile file;
    };

    //xnz_flac_chunk* chunks;

    xnz_flac_metadata_streaminfo streaminfo;

    uint8_t * audioFrames;

    char* samples;

}XNZ_FLAC_ARCHIVE;

#ifdef _WIN32
#pragma pack(pop)   //end struct single byte packing
#endif

XNZ_FLAC_API XNZ_FLAC_INLINE void xnz_flac_open(XNZ_FLAC_ARCHIVE* archive, const char* filepath);
XNZ_FLAC_API XNZ_FLAC_INLINE void xnz_parse_metadata_blocks(XNZ_FLAC_ARCHIVE* archive);
//XNZ_FLAC_API XNZ_FLAC_INLINE size_t xnz_parse_metadata_blocks(XNZ_PNG_IMAGE* archive, uint8_t* dst_buf, size_t dst_size);
XNZ_FLAC_API XNZ_FLAC_INLINE void xnz_flac_close(XNZ_FLAC_ARCHIVE* archive);

XNZ_FLAC_API XNZ_FLAC_INLINE void xnz_parse_metadata_block_streaminfo(XNZ_FLAC_ARCHIVE* archive, xnz_bitstream* bs)
{
    //read STREAMINFO
    archive->streaminfo.minBlockSize  = (xnz_bitstream_read_bits(bs, 8) << 8)  | xnz_bitstream_read_bits(bs, 8);
    archive->streaminfo.maxBlockSize  = (xnz_bitstream_read_bits(bs, 8) << 8)  | xnz_bitstream_read_bits(bs, 8);
        
    archive->streaminfo.minFrameSize   = (xnz_bitstream_read_bits(bs, 8) << 16) | (xnz_bitstream_read_bits(bs, 8) << 8)  | xnz_bitstream_read_bits(bs, 8);//  minFrameSize = xnz_be32toh(minFrameSize) >> 8;
    archive->streaminfo.maxFrameSize   = (xnz_bitstream_read_bits(bs, 8) << 16) | (xnz_bitstream_read_bits(bs, 8) << 8)  | xnz_bitstream_read_bits(bs, 8);
    archive->streaminfo.sampleRate     = (xnz_bitstream_read_bits(bs, 8) << 12) | (xnz_bitstream_read_bits(bs, 8) << 4);
    uint32_t sharedByte                =  xnz_bitstream_read_bits(bs, 8);
    archive->streaminfo.sampleRate    |= (sharedByte >> 4);

    archive->streaminfo.nChannels      = ((sharedByte >> 1) & 0x00000007) + 1; //get 3 bits corresponding to nChannels
    uint32_t sharedByte2               = xnz_bitstream_read_bits(bs, 8);
    archive->streaminfo.bitsPerSample  = (((sharedByte & 0x01) << 4) | (sharedByte2 >> 4)) + 1;

    archive->streaminfo.nSampleFrames  =  ((uint64_t)(sharedByte2 & 0x0F) << 32) |
                             (uint64_t) ((xnz_bitstream_read_bits(bs, 8) << 24) | (xnz_bitstream_read_bits(bs, 8) << 16) | (xnz_bitstream_read_bits(bs, 8) << 8) | xnz_bitstream_read_bits(bs, 8));

    fprintf(stdout, "xnz_flac_metadata_block_streaminfo::minBlockSize = %hu\n",   archive->streaminfo.minBlockSize);
    fprintf(stdout, "xnz_flac_metadata_block_streaminfo::maxBlockSize = %hu\n",   archive->streaminfo.maxBlockSize);
    fprintf(stdout, "xnz_flac_metadata_block_streaminfo::minFrameSize  = %u\n",   archive->streaminfo.minFrameSize);
    fprintf(stdout, "xnz_flac_metadata_block_streaminfo::maxFrameSize  = %u\n",   archive->streaminfo.maxFrameSize);
    fprintf(stdout, "xnz_flac_metadata_block_streaminfo::sampleRate    = %d\n",   archive->streaminfo.sampleRate);
    fprintf(stdout, "xnz_flac_metadata_block_streaminfo::nChannels     = %d\n",   archive->streaminfo.nChannels);
    fprintf(stdout, "xnz_flac_metadata_block_streaminfo::bitsPerSample = %d\n",   archive->streaminfo.bitsPerSample);
    fprintf(stdout, "xnz_flac_metadata_block_streaminfo::nSampleFrames = %llu\n", archive->streaminfo.nSampleFrames);
    
    fprintf(stdout, "xnz_flac_metadata_block_streaminfo::md5 = \n");

    for( int i=0; i<16; i++)
    {
        archive->streaminfo.signature[i] = xnz_bitstream_read_bits(bs, 8);
        fprintf(stdout, "%.2x ", archive->streaminfo.signature[i]);
    }
    
    fprintf(stdout, "\n");

}

XNZ_AIF_API XNZ_AIF_INLINE void xnz_flac_parse_metadata_blocks(XNZ_FLAC_ARCHIVE* archive)
{
    uint32_t   METADATA_BLOCK_HEADER_SIZE = 4;
    uint64_t METADATA_OFFSET              = 4; //after magic number

    uint8_t  lastFlag     = 0;
    uint8_t  blockType    = 0;
    uint32_t blockLength  = 0;
    
    xnz_bitstream bs = {0}; uint8_t * src_ptr = NULL;//(uint8_t*)archive->file.buffer + 4;
    
    while ( !lastFlag)
    {
        src_ptr = (uint8_t*)archive->file.buffer + METADATA_OFFSET;
        
        //Initialize/Reinitialize bit-stream using client updated compressed byte-stream ptr:
        //  on subsequent clients calls to xnz_stream_inflate the bistream needs to be
        //  properly "reinitialized" without overwriting the previously prefetched stream bits
        memset(&bs, 0, sizeof(xnz_bitstream)); xnz_bitstream_attach(&bs, src_ptr, METADATA_BLOCK_HEADER_SIZE);
        
        //read METADATA_BLOCK_HEADER
        uint8_t sharedByte = xnz_bitstream_read_bits(&bs, 8);
        lastFlag    =  sharedByte >> 7;
        blockType   =  sharedByte & 0x7F;
        blockLength = (xnz_bitstream_read_bits(&bs, 8) << 16) | (xnz_bitstream_read_bits(&bs, 8) << 8)  | xnz_bitstream_read_bits(&bs, 8);

        fprintf(stdout, "\n");
        fprintf(stdout, "METADATA_BLOCK_HEADER::lastFlag     = %d\n", lastFlag);
        fprintf(stdout, "METADATA_BLOCK_HEADER::blockType    = %d\n", blockType);
        fprintf(stdout, "METADATA_BLOCK_HEADER::blockLength  = %u\n", blockLength);
        fprintf(stdout, "\n");
        
        METADATA_OFFSET += METADATA_BLOCK_HEADER_SIZE;

        //reinitialize stream for reading block data of size block length at the start of block
        memset(&bs, 0, sizeof(xnz_bitstream));
        src_ptr = (uint8_t*)archive->file.buffer + METADATA_OFFSET;
        xnz_bitstream_attach(&bs, src_ptr, blockLength);
        
        switch(blockType)
        {
            case (FLAC_METADATA_BLOCK_STREAMINFO):
                xnz_parse_metadata_block_streaminfo(archive, &bs); break;
                
            default:
                break;
        }

        METADATA_OFFSET += blockLength;

        
    }
    
    archive->audioFrames = (uint8_t*)archive->file.buffer + METADATA_OFFSET;
}

XNZ_FLAC_API XNZ_FLAC_INLINE void xnz_flac_open(XNZ_FLAC_ARCHIVE* archive, const char* filepath)
{
    if (filepath)
    {
        //1 OPEN THE GBL OR GLTF FILE FOR READING AND GET FILESIZE USING LSEEK
        archive->file.fd = cr_file_open(filepath);
        archive->file.size = cr_file_size(archive->file.fd);
        archive->file.path = (char*)filepath;

        fprintf(stderr, "\nxnz_flac_open::AIF File Size =  %lu bytes\n", archive->file.size);

        //2 MAP THE FILE TO BUFFER FOR READING
#ifndef _WIN32
        archive->file.buffer = cr_file_map_to_buffer(&(archive->file.buffer), archive->file.size, PROT_READ, MAP_SHARED | MAP_NORESERVE, archive->file.fd, 0);
        if (madvise(archive->file.buffer, (size_t)archive->file.size, MADV_SEQUENTIAL | MADV_WILLNEED) == -1) {
            printf("\nread madvise failed\n");
        }
#else
        archive->file.mFile = cr_file_map_to_buffer(&(archive->file.buffer), archive->file.size, PROT_READ, MAP_SHARED | MAP_NORESERVE, archive->file.fd, 0);
#endif
    }
    else assert(archive->file.buffer);

    //3  READ THE FLAC (.flac) file signature
    uint8_t * magicNumber = (uint8_t*)archive->file.buffer;
    assert(magicNumber[0] == 'f');
    assert(magicNumber[1] == 'L');
    assert(magicNumber[2] == 'a');
    assert(magicNumber[3] == 'C');

    xnz_flac_parse_metadata_blocks(archive);

}


#define XNZ_FLAC_HIDEF

#ifdef XNZ_FLAC_HIDEF
#define uint_pcm uint64_t
#define int_pcm  int64_t
#else
#define uint_pcm uint32_t
#define int_pcm  int32_t
#endif

typedef int_pcm (*xnz_flac_fixed_predictor) (int_pcm * s);

static int_pcm xnz_flac_fixed_predictor0(int_pcm * s) { return      0;                            }  //N/A
static int_pcm xnz_flac_fixed_predictor1(int_pcm * s) { return    s[0];                           }  //N/A
static int_pcm xnz_flac_fixed_predictor2(int_pcm * s) { return (2*s[0] -   s[1]);                 }  //s(n-1) + s'(n-1)
static int_pcm xnz_flac_fixed_predictor3(int_pcm * s) { return (3*s[0] - 3*s[1] +   s[2]);        }  //s(n-1) + s'(n-1) + s''(n-1)
static int_pcm xnz_flac_fixed_predictor4(int_pcm * s) { return (4*s[0] - 6*s[1] + 4*s[2] - s[3]); }  //s(n-1) + s'(n-1) + s''(n-1) + s'''(n-1)

static xnz_flac_fixed_predictor xnz_flac_fixed_predictors[5] =
{
    xnz_flac_fixed_predictor0,
    xnz_flac_fixed_predictor1,
    xnz_flac_fixed_predictor2,
    xnz_flac_fixed_predictor3,
    xnz_flac_fixed_predictor4
};


XNZ_FLAC_API XNZ_FLAC_INLINE uint64_t xnz_flac_decode_constant_subframe_samples(XNZ_FLAC_ARCHIVE* archive, xnz_bitstream* bs, xnz_flac_frame* frame, uint8_t channel, uint8_t nWastedBits, void** sampleBuffers)
{
    uint64_t subframeSample = 0;
    
    //As this is a constant subframe, the subframe only contains a single unencoded sample values
    //With a block size of 1, it contains only 1 sample(s)
    //the bitdepth of the audio is n bits, but if subframe header signals w wasted bits, only n-w bits are stored
    //if no stereo decorrelation is used, a bit depth increase for the side channel is not applicable
    
    uint32_t  nSampleBits  = frame->sampleSize - nWastedBits;
    uint32_t  nSampleBytes = frame->sampleSize/8;

    int_pcm   sourceSample = 0;
 
    if( frame->channelID > 7 && frame->channelID < 11)
    {
        nSampleBytes = sizeof(int_pcm);

        if( frame->channelID == 8) //left/side
        {
            if( channel == 1 )
            {
                nSampleBits++; //side
                //nSampleBytes = sizeof(int32_t);
            }
            //else                            //left
        }
        if( frame->channelID == 9) //right/side
        {
            if( channel == 0 )
            {
                nSampleBits++; //side
                //nSampleBytes = sizeof(int32_t);
            }
            //else                            //right
        }
        if( frame->channelID == 10) //right/side
        {
            if( channel == 1 )
            {
                nSampleBits++; //side
                //nSampleBytes = sizeof(int32_t);
            }
            //else                            //mid
        }
    }
    
    //read from bitstream that has already reversed octets
    sourceSample = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, nSampleBits))) >> (32 - nSampleBits);
    
    //fprintf(stdout, "\nconstant sample[%d]: %d\n", subframeSample, sourceSample);

    //copy constant sample to output buffer
    uint64_t i = (subframeSample * frame->nChannels + channel) * (nSampleBytes);
    char* hostBuffer = (char*)sampleBuffers[0];
    memcpy(&hostBuffer[i], &sourceSample, nSampleBytes);
    //subframeSample++;
    
    //The spec doesn't say anything about this ...
    //If decoding a constant subframe against a blocksize > 1, copy the sample for the entire block
    for( subframeSample = 1; subframeSample<frame->blockSize; subframeSample++)
    {
        uint64_t i = (subframeSample * frame->nChannels + channel) * (nSampleBytes);
        memcpy(&hostBuffer[i], &sourceSample, nSampleBytes);
    }
    
    return subframeSample;
}

XNZ_FLAC_API XNZ_FLAC_INLINE uint64_t xnz_flac_decode_verbatim_subframe_samples(XNZ_FLAC_ARCHIVE* archive, xnz_bitstream* bs, xnz_flac_frame* frame, uint8_t channel, uint8_t nWastedBits, void** sampleBuffers)
{
    uint64_t subframeSample = 0;
    
    //As this is a verbatim subframe, the subframe only contains unencoded sample values
    //With a block size of x, it contains only x sample(s)
    //the bitdepth of the audio is n bits, but if subframe header signals w wasted bits, only n-w bits are stored
    //if no stereo decorrelation is used, a bit depth increase for the side channel is not applicable
    
    uint32_t  nSampleBits  = frame->sampleSize - nWastedBits;
    uint32_t  nSampleBytes = frame->sampleSize/8;

    int_pcm   sourceSample = 0;
 
    if( frame->channelID > 7 && frame->channelID < 11)
    {
        assert(1==0);

        if( frame->channelID == 8) //left/side
        {
            if( channel == 1 ) nSampleBits++; //side
            //else                            //left
        }
        if( frame->channelID == 9) //right/side
        {
            if( channel == 0 ) nSampleBits++; //side
            //else                            //right
        }
        if( frame->channelID == 10) //mid/side
        {
            if( channel == 1 ) nSampleBits++; //side
            //else                            //mid
        }
    }
    
    //read first sample while pushing past # wasted bits
    //sourceSample = xnz_be32toh(xnz_bitstream_read_bits(bs, nWastedBits + nSampleBits)) << nWastedBits;
    //sourceSample >>= (32 - frame->sampleSize);

    //read from bitstream that has already reversed octets
    sourceSample = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, nSampleBits))) >> (32 - nSampleBits); //sourceSample = xnz_be32toh(sourceSample) >> (32 - nSampleBits);
    //sourceSample = rbytes( sourceSample );
    
    //fprintf(stdout, "\nverbatim sample[%d]: %d\n", subframeSample, sourceSample);

    //copy verbatim sample to output buffer
    uint64_t i = (subframeSample * frame->nChannels + channel) * (nSampleBytes);
    char* hostBuffer = (char*)sampleBuffers[0];
    memcpy(&hostBuffer[i], &sourceSample, nSampleBytes);
        
    for( subframeSample = 1 ; subframeSample < frame->blockSize; subframeSample++)
    {
        i = (subframeSample * frame->nChannels + channel) * (nSampleBytes);
        
        //sourceSample = xnz_be32toh(xnz_bitstream_read_bits(bs, nSampleBits)) << nWastedBits;
        //sourceSample >>= (32 - frame->sampleSize);

        sourceSample = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, nSampleBits))) >> (32 - nSampleBits); //sourceSample = xnz_be32toh(sourceSample) >> (32 - nSampleBits);
        //sourceSample = rbytes( sourceSample );
        
        //convert unencoded residual value to signed int based on bit precision (positive term + negative term)
        //sourceSample =  (int32_t)(sourceSample & ((1<<frame->sampleSize)-1)) - (int32_t)((sourceSample >> (frame->sampleSize-1)) * (1<<frame->sampleSize));
        
        //fprintf(stdout, "\nverbatim sample[%d]: %d\n", subframeSample, sourceSample);
        
        //copy verbatim sample to output buffer
        //char* hostBuffer = (char*)sampleBuffers[0];
        memcpy(&hostBuffer[i], &sourceSample, nSampleBytes);
    }
    
    return subframeSample;
}

XNZ_FLAC_API XNZ_FLAC_INLINE uint64_t xnz_flac_decode_fixed_subframe_samples(XNZ_FLAC_ARCHIVE* archive, xnz_bitstream* bs, xnz_flac_frame* frame, uint8_t channel, uint8_t subframeType, void** sampleBuffers)
{
    uint64_t subframeSample = 0;

    uint32_t predictorOrder = subframeType - 8;
    
    uint32_t  nSampleBits   = frame->sampleSize;
    uint32_t  nSampleBytes  = frame->sampleSize/8;
            
    if( frame->channelID > 7 && frame->channelID < 11)
    {
        nSampleBytes = sizeof(int_pcm);

        if( frame->channelID == 8) //left/side
        {
            if( channel == 1 )
            {
                nSampleBits++; //side
                //nSampleBytes = sizeof(int32_t);
            }
            //else                            //left
        }
        if( frame->channelID == 9) //right/side
        {
            if( channel == 0 )
            {
                nSampleBits++; //side
                //nSampleBytes = sizeof(int32_t);
            }
            //else                            //right
        }
        if( frame->channelID == 10) //right/side
        {
            if( channel == 1 )
            {
                nSampleBits++; //side
                //nSampleBytes = sizeof(int32_t);
            }
            //else                            //mid
        }
    }
    
    int_pcm sourceSample = 0;
    int_pcm sourceSamples[5] = {0};
    
    uint64_t i = 0;
    char* hostBuffer = (char*)sampleBuffers[0];
    //char* sideBuffer = hostBuffer + (frame->blockSize * frame->nChannels * nSampleBytes); //assume there is extra space in dst buffer
                                                                                          //to save extra precision byte required by side channel
    //1 read unencoded warmup samples
    switch (frame->sampleSize)
    {
        case (8):
        case (12):
        case (16):
        case (20):
        case (24):
        case (32):
        default:
        {
            for( subframeSample=0; subframeSample<predictorOrder; subframeSample++)
            {
                i = (subframeSample * frame->nChannels + channel) * (nSampleBytes);
                
                sourceSample = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, nSampleBits))) >> (32 - nSampleBits);
                sourceSample =  (int_pcm)(sourceSample & ((1<<nSampleBits)-1)) - (int_pcm)((sourceSample >> (nSampleBits-1)) * (1<<nSampleBits));
                sourceSamples[predictorOrder-1 - subframeSample] = sourceSample;     //copy sample to predictor queue
                memcpy(&hostBuffer[i], &sourceSample, nSampleBytes);                 //copy sample to output buffer

                //fprintf(stdout, "warmup sample[%d][%llu]: %d\n", channel, nWarmupSamples, sourceSample);
            }
            break;
        }
    };
            
    //read CODED RESIDUAL
    //The first two bits in a coded residual indicate which coding method is used
    //
    //      0b00          partitioned Rice code with 4-bit parameters
    //      0b01          partitioned Rice code with 5-bit parameters
    //      b10 - 0b11    reserved
    
    uint8_t ESCAPE_CODE[2] = { 0b1111, 0b11111 };
    
    //method is not a signed/unsigned number, it is just an ordering of 2 bits
    uint32_t method =  (xnz_bitstream_read_octet_bits(bs, 1) << 1 ) |  xnz_bitstream_read_octet_bits(bs, 1); assert( method < 2);
    uint32_t nRiceBits = method ? 5 : 4;
    
    //The 4 bits that directly follow the coding method bits form the PARTITION ORDER
    uint32_t partitionOrder = rbyte(xnz_bitstream_read_octet_bits(bs, 4)) >> (8-4);   //4 bits; unsigned number
    uint32_t nPartitions    = 1 << partitionOrder;

    //The number of residual samples in the first partition is equal to (block size >> partition order) - predictor order,
    //i.e. the block size divided by the number of partitions minus the predictor order.
    uint32_t nResiduals = (frame->blockSize >> partitionOrder) - predictorOrder;
    
    assert( frame->blockSize % nPartitions == 0);                     //The partition order MUST be so that the block size is evenly divisible by the number of partitions)
    assert( (frame->blockSize >> partitionOrder) > predictorOrder );  //The partition order also MUST be so that the (block size >> partition order) is larger than the predictor order

    //The rest of the coded residual consists of 2^(partition order) partitions
    //The number of residual samples in the first partition is equal to (block size >> partition order) - predictor order
    uint32_t riceParameter = rbyte(xnz_bitstream_read_octet_bits(bs, nRiceBits)) >> (8 - nRiceBits);
    uint32_t nUnencodedBits = 0;

    assert(riceParameter != ESCAPE_CODE[method]);
    
    //read rice rice coded residuals as unsigned numbers (n unary quotient + remainder pairs each corresponding to a sample)
    for( subframeSample = predictorOrder; subframeSample < frame->blockSize; subframeSample++)
    {
        uint_pcm q = 0;
        uint_pcm r = 0;
        int_pcm  z = 0;
        int_pcm  v = 0;
        sourceSample = 0;

        if( !nUnencodedBits )
        {
            //read unary quotient
            while (!xnz_bitstream_read_octet_bits(bs, 1)) q++;
            r = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, riceParameter))) >> (32 - riceParameter);
            
            z = q * (1 << riceParameter) + r;
            v = (z%2) ? -1*((z+1)>>1) : z >> 1; //map even zigzag # to positive residual value
                                                //map odd  zigzag # to negative residual value
        }
        else
        {
            v = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, nUnencodedBits))) >> (32 - nUnencodedBits);

            //convert unencoded residual value to signed int based on bit precision (positive term + negative term)
            v =  (int_pcm)(v & ((1<<nUnencodedBits)-1)) - (int_pcm)((v >> (nUnencodedBits-1)) * (1<<nUnencodedBits));
        }
        
        //restore sample value from residual value and predictor coefficients with fixed predictor of order 0-4
        sourceSample = xnz_flac_fixed_predictors[predictorOrder](sourceSamples);

        //No shift for fixed predictors, but always add residual to prediction
        //sourceSample >>= shift; fprintf(stdout, "\nsample prediction (after shift): %d\n",    sourceSample);
        sourceSample += v;
        
        //shift decoded samples for next prediction calculation (ignore predictor order 0)
        if( predictorOrder ) memmove(&sourceSamples[1], &sourceSamples[0], (predictorOrder-1) * sizeof(int_pcm));
        sourceSamples[0] = sourceSample; //insert most current decoded sample
        
        //copy sample to output buffer
        i = (subframeSample * frame->nChannels + channel) * (nSampleBytes);
        memcpy(&hostBuffer[i], &sourceSample, nSampleBytes); //little endian copy from int32_t to byte works out nicely for different sample sizes
    
        //fprintf(stdout, "q: %d\tr: %d\tz: %d\tvalue: %d\tsample: %d\n", q, r, z, v, sourceSample);
        //fprintf(stdout, "fixed sample[%d][%d]: %d\n", channel, subframeSample, sourceSample);
                
        nResiduals--;

        //open another residual partition if this isn't the last residual partition
        if( !nResiduals && nPartitions > 1 )
        {
            nUnencodedBits = 0;
            nResiduals     = (frame->blockSize >> partitionOrder); //In all other partitions after the first, the number of residual samples is equal to (block size >> partition order).

            riceParameter  = rbyte(xnz_bitstream_read_octet_bits(bs, nRiceBits)) >> (8 - nRiceBits);
            if( riceParameter == ESCAPE_CODE[method] ) nUnencodedBits = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, 5))) >> (32 - 5);
            nPartitions--;
        }
        
    }

    return subframeSample;
}

XNZ_FLAC_API XNZ_FLAC_INLINE uint64_t xnz_flac_decode_lpc_subframe_samples(XNZ_FLAC_ARCHIVE* archive, xnz_bitstream* bs, xnz_flac_frame* frame, uint8_t channel, uint8_t subframeType, void** sampleBuffers)
{
    uint64_t subframeSample   = 0;

    uint32_t c                =  0;  //coefficient index
    int_pcm  coefficients[32] = {0}; //read a max of 32 predictor coefficients from stream
        
    uint32_t predictorOrder   = subframeType - 31;
    
    uint32_t  nSampleBits     = frame->sampleSize;
    uint32_t  nSampleBytes    = frame->sampleSize/8;
            
    if( frame->channelID > 7 && frame->channelID < 11)
    {
        nSampleBytes = sizeof(int_pcm);

        if( frame->channelID == 8) //left/side
        {
            if( channel == 1 )
            {
                nSampleBits++; //side
                //nSampleBytes = sizeof(int32_t);
            }
            //else                            //left
        }
        if( frame->channelID == 9) //right/side
        {
            if( channel == 0 )
            {
                nSampleBits++; //side
                //nSampleBytes = sizeof(int32_t);
            }
            //else                            //right
        }
        if( frame->channelID == 10) //right/side
        {
            if( channel == 1 )
            {
                nSampleBits++; //side
                //nSampleBytes = sizeof(int32_t);
            }
            //else                            //mid
        }
    }
        
    int_pcm sourceSample = 0;
    int_pcm sourceSamples[32] = {0};

    uint64_t i = 0;
    char* hostBuffer = (char*)sampleBuffers[0];

    //1 read unencoded warmup samples
    switch (frame->sampleSize)
    {
        case (8):
        {
            for( subframeSample=0; subframeSample<predictorOrder; subframeSample++)
            {
                i = (subframeSample * frame->nChannels + channel) * (nSampleBytes);
                sourceSample = rbytes(xnz_bitstream_read_octet_bits(bs, frame->sampleSize));
                
                if( nSampleBits > frame->sampleSize)
                    sourceSample = (sourceSample << (nSampleBits - frame->sampleSize)) | xnz_bitstream_read_octet_bits(bs, (nSampleBits - frame->sampleSize));

                sourceSamples[predictorOrder-1 - subframeSample] = sourceSample;
                
                //fprintf(stdout, "warmup sample[%d][%llu]: %d\n", channel, subframeSample, sourceSample);

                //copy sample to output buffer
                memcpy(&hostBuffer[i], &sourceSample, nSampleBytes);
            }
            break;
        }
        case (12):
        case (16):
        case (20):
        case (24):
        case (32):
        default:
        {
            for( subframeSample=0; subframeSample<predictorOrder; subframeSample++)
            {
                i = (subframeSample * frame->nChannels + channel) * (nSampleBytes);
                
                sourceSample = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, nSampleBits))) >> (32 - nSampleBits);
                sourceSample =  (int_pcm)(sourceSample & ((1<<nSampleBits)-1)) - (int_pcm)((sourceSample >> (nSampleBits-1)) * (1<<nSampleBits));
                sourceSamples[predictorOrder-1 - subframeSample] = sourceSample;    //copy sample to predictor queue
                memcpy(&hostBuffer[i], &sourceSample, nSampleBytes);                //copy sample to output buffer

                //fprintf(stdout, "warmup sample[%d][%llu]: %d\n", channel, nWarmupSamples, sourceSample);
            }
            break;
        }
    };
        
    //retrieve predictor coefficient precision (bits-1)
    //retrieve post prediction shift magnitude (signed but always positive)
    uint32_t precision = (rbyte(xnz_bitstream_read_octet_bits(bs, 4)) >> 4) + 1;                    //4 bits; unsigned number
    int32_t  shift     =  rbyte(xnz_bitstream_read_octet_bits(bs, 5)) >> (8-5); assert(shift > -1); //5 bits; signed two's complement
    
    //read n unencoded predictor coefficients of length precision
    for( c = 0; c<predictorOrder; c++)
    {
        coefficients[c] = rbytes( xnz_be32toh(xnz_bitstream_read_octet_bits(bs, precision))) >> (32 - precision);
        
        //convert to signed int based on bit precision (positive term + negative term)
        coefficients[c] =  (int_pcm)(coefficients[c] & ((1<<precision)-1)) - (int_pcm)((coefficients[c] >> (precision-1)) * (1<<precision));
        
        fprintf(stdout, "\ncoefficients[%d] = %d", c, coefficients[c]);
    }
    
    //read CODED RESIDUAL
    //The first two bits in a coded residual indicate which coding method is used
    //
    //      0b00          partitioned Rice code with 4-bit parameters
    //      0b01          partitioned Rice code with 5-bit parameters
    //      b10 - 0b11    reserved
    
    uint8_t ESCAPE_CODE[2] = { 0b1111, 0b11111 };
    
    //method is not a signed/unsigned number, it is just an ordering of 2 bits
    uint32_t method =  (xnz_bitstream_read_octet_bits(bs, 1) << 1 ) |  xnz_bitstream_read_octet_bits(bs, 1); assert( method < 2);
    uint32_t nRiceBits = method ? 5 : 4;
    
    //The 4 bits that directly follow the coding method bits form the PARTITION ORDER
    uint32_t partitionOrder = rbyte(xnz_bitstream_read_octet_bits(bs, 4)) >> (8-4);   //4 bits; unsigned number
    uint32_t nPartitions    = 1 << partitionOrder;

    //The number of residual samples in the first partition is equal to (block size >> partition order) - predictor order,
    //i.e. the block size divided by the number of partitions minus the predictor order.
    uint32_t nResiduals = (frame->blockSize >> partitionOrder) - predictorOrder;
    
    assert( frame->blockSize % nPartitions == 0);                     //The partition order MUST be so that the block size is evenly divisible by the number of partitions)
    assert( (frame->blockSize >> partitionOrder) > predictorOrder );  //The partition order also MUST be so that the (block size >> partition order) is larger than the predictor order

    //The rest of the coded residual consists of 2^(partition order) partitions
    //The number of residual samples in the first partition is equal to (block size >> partition order) - predictor order
    uint32_t riceParameter = rbyte(xnz_bitstream_read_octet_bits(bs, nRiceBits)) >> (8 - nRiceBits);
    uint32_t nUnencodedBits = 0;

    assert(riceParameter != ESCAPE_CODE[method]);
    
    //read rice rice coded residuals as unsigned numbers (n unary quotient + remainder pairs each corresponding to a sample)
    for( subframeSample = predictorOrder; subframeSample < frame->blockSize; subframeSample++)
    {
        //Note:  zigzag will always be positive, therefore quotient and remainder will always be positive
        uint_pcm q = 0;
        uint_pcm r = 0;
        int_pcm  z = 0;
        int_pcm  v = 0;
        sourceSample = 0;

        if( !nUnencodedBits )
        {
            //read unary quotient
            while (!xnz_bitstream_read_octet_bits(bs, 1)) q++;
            r = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, riceParameter))) >> (32 - riceParameter);
            
            z = q * (1 << riceParameter) + r;
            v = (z%2) ? -1*((z+1)>>1) : z >> 1; //map even zigzag # to positive residual value
                                                //map odd  zigzag # to negative residual value
            
        }
        else
        {
            v = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, nUnencodedBits))) >> (32 - nUnencodedBits);

            //convert unencoded residual value to signed int based on bit precision (positive term + negative term)
            v =  (int_pcm)(v & ((1<<nUnencodedBits)-1)) - (int_pcm)((v >> (nUnencodedBits-1)) * (1<<nUnencodedBits));
        }
        
        //restore sample value from residual value and predictor coefficients of order n
        // 1) derive the prediction for sample at i by summing (i-predictor order) coefficients * sample at (i-predictor order)
        // 2) shift the prediction and sum with the residual value
        for (i = 0; i < predictorOrder; i++) sourceSample += coefficients[i] * sourceSamples[i];
        
        //fprintf(stdout, "\nsample prediction (prior to shift): %d\n", sourceSample);
        sourceSample >>= shift;
        //fprintf(stdout, "\nsample prediction (post shift): %d\n",    sourceSample);
        sourceSample += v;
        
        //shift decoded samples for next prediction calculation
        memmove(&sourceSamples[1], &sourceSamples[0], (predictorOrder-1) * sizeof(int_pcm));
        sourceSamples[0] = sourceSample; //insert most current decoded sample
                
        //copy sample to output buffer
        i = (subframeSample * frame->nChannels + channel) * (nSampleBytes);
        memcpy(&hostBuffer[i], &sourceSample, nSampleBytes); //little endian copy from int_pcm to byte works out nicely for different sample sizes

        //fprintf(stdout, "\nq: %d\tr: %d\tz: %d\tvalue: %d\tsample: %d\n", q, r, z, v, sourceSample);
        //fprintf(stdout, "lpc sample[%d][%hd]: %d\n", channel, subframeSample, (int16_t)sourceSample);

        nResiduals--;

        //open another residual partition if this isn't the last residual partition
        if( !nResiduals && nPartitions > 1 )
        {
            nUnencodedBits = 0;
            nResiduals = (frame->blockSize >> partitionOrder); //In all other partitions after the first, the number of residual samples is equal to (block size >> partition order).

            riceParameter = rbyte(xnz_bitstream_read_octet_bits(bs, nRiceBits)) >> (8 - nRiceBits);
            if( riceParameter == ESCAPE_CODE[method] ) nUnencodedBits = rbytes(xnz_be32toh(xnz_bitstream_read_octet_bits(bs, 5))) >> (32 - 5);
            nPartitions--;
        }
        
    }
    
    return subframeSample;
}
    
XNZ_FLAC_API XNZ_FLAC_INLINE uint64_t xnz_flac_decode_subframe_samples(XNZ_FLAC_ARCHIVE* archive, xnz_bitstream* bs, xnz_flac_frame* frame, void** sampleBuffers, unsigned long long sampleFrame)
{
    uint64_t nSubframeSamples = 0;
    uint64_t channel = 0;
    
    //read nChannel subframes
    for( channel = 0; channel < frame->nChannels; channel++)
    {
        //9.2.1.  Subframe header(s)

        //   Each subframe starts with a header.  The first bit of the header MUST
        //   be 0, followed by 6 bits describing which subframe type is used
        //   according to the following table, where v is the value of the 6 bits
        //   as an unsigned number.
        
        //read subframe header byte
        uint8_t byte         = rbyte((uint8_t)xnz_bitstream_read_octet_bits(bs, 8));

        //assert(!xnz_bitstream_read_octet_bits(bs, 1));
        //rbyte((uint8_t)xnz_bitstream_read_octet_bits(bs, 6)) >> 2;
        //for(int i=0; i < 6 ; i++) byte = (byte << 1) | xnz_bitstream_read_octet_bits(bs, 1);
            
        uint8_t subframeType = byte >> 1;
        uint8_t  nWastedBits = 0;
        uint32_t unaryBit    = 0;

        //Linear prediction subframes (see section linear predictor subframe) containing audio with a sample rate
        //less than or equal to 48000 Hz MUST have a predictor order less than or equal to 12
        if( frame->sampleRate <= 48000 && subframeType > 30) assert( subframeType - 31 <= 12);
                
        if( (nWastedBits = byte & 0x01) ) //wasted bits present
        {
            while (!xnz_bitstream_read_octet_bits(bs, 1)) nWastedBits++;
        }
        
        //spec is unclear if wasted bits would ever be present in fixed/lpc subframes
        if(subframeType > 1 && nWastedBits) assert(1==0);
        
        //switch( subframeType )
             if( subframeType == 0 ) //constant subframe
                 nSubframeSamples += xnz_flac_decode_constant_subframe_samples(archive, bs, frame, channel, nWastedBits, sampleBuffers);
        else if( subframeType == 1 ) //verbatim subframe
                 nSubframeSamples += xnz_flac_decode_verbatim_subframe_samples(archive, bs, frame, channel, nWastedBits, sampleBuffers);
        else if( subframeType > 1  && subframeType < 8 ) assert(1==0); //reserved
        else if( subframeType > 7  && subframeType < 13)
                 nSubframeSamples += xnz_flac_decode_fixed_subframe_samples(archive, bs, frame, channel, subframeType, sampleBuffers);//Subframe with a fixed  predictor of order v-8,  i.e., 0, 1, 2, 3 or 4
        else if( subframeType > 12 && subframeType < 32) assert(1==0); //reserved
        else if( subframeType > 31 && subframeType < 64) //Subframe with a linear predictor of order v-31, i.e., 1 through 32 (inclusive)
                 nSubframeSamples += xnz_flac_decode_lpc_subframe_samples(archive, bs, frame, channel, subframeType, sampleBuffers);

    }
    
    //perform interchannel decorrelation
    if( frame->channelID > 7 && frame->channelID < 11)
    {
        //assert(frame->sampleSize == 16);
        
        uint64_t  i = 0;
        
        int_pcm left, right, mid, side; //decoded      inputs
        int_pcm L, R;                   //decorrelated outputs

        uint32_t  nSampleBytes     = frame->sampleSize/8;   //# output sample bytes
        uint32_t  nSideSampleBytes = sizeof(int_pcm);       //xnz decodes all channels that require [side channel] decorrelation to int32 precision
        
        char* hostBuffer = (char*)sampleBuffers[0];
        
        if( frame->channelID == 8) //left/side
        {
            for( i=0; i < nSubframeSamples/frame->nChannels; i++)
            {
                uint64_t iL = (i * frame->nChannels + 0) * (nSampleBytes);
                uint64_t iR = (i * frame->nChannels + 1) * (nSampleBytes);
                
                uint64_t sL = (i * frame->nChannels + 0) * (nSideSampleBytes);
                uint64_t sR = (i * frame->nChannels + 1) * (nSideSampleBytes);

                memcpy(&left, &hostBuffer[sL], nSideSampleBytes);
                memcpy(&side, &hostBuffer[sR], nSideSampleBytes);
                
                L = left;
                R = left - side;
                
                memcpy(&hostBuffer[iL], &L, nSampleBytes);
                memcpy(&hostBuffer[iR], &R, nSampleBytes);

            }
        }
        
        if( frame->channelID == 9) //right/side
        {
            for( i=0; i < nSubframeSamples/frame->nChannels; i++)
            {
                uint64_t iL = (i * frame->nChannels + 0) * (nSampleBytes);
                uint64_t iR = (i * frame->nChannels + 1) * (nSampleBytes);
                
                uint64_t sL = (i * frame->nChannels + 0) * (nSideSampleBytes);
                uint64_t sR = (i * frame->nChannels + 1) * (nSideSampleBytes);

                memcpy(&side,  &hostBuffer[sL], nSideSampleBytes);
                memcpy(&right, &hostBuffer[sR], nSideSampleBytes);
                
                L = side + right;
                R = right;

                memcpy(&hostBuffer[iL], &L, nSampleBytes);
                memcpy(&hostBuffer[iR], &R, nSampleBytes);
            }
        }
        
        if( frame->channelID == 10)
        {
            for( i=0; i < nSubframeSamples/frame->nChannels; i++)
            {
                uint64_t iL = (i * frame->nChannels + 0) * (nSampleBytes);
                uint64_t iR = (i * frame->nChannels + 1) * (nSampleBytes);
                
                uint64_t sL = (i * frame->nChannels + 0) * (nSideSampleBytes);
                uint64_t sR = (i * frame->nChannels + 1) * (nSideSampleBytes);
                
                memcpy(&mid,  &hostBuffer[sL], nSideSampleBytes);
                memcpy(&side, &hostBuffer[sR], nSideSampleBytes);

                //On decoding, all mid channel samples have to be shifted left by 1 bit.
                //Also, if a side channel sample is odd, 1 has to be added to the corresponding mid channel sample after it has been shifted left by one bit.
                mid = ((int_pcm)mid) << 1;
                mid |= (side & 1); /* i.e. if 'side' is odd... */

                //To reconstruct the  left channel, the corresponding samples in the mid and side subframes are added and the result shifted right by 1 bit
                //To reconstruct the right channel, the side channel has to be subtracted from the mid channel and the result shifted right by 1 bit.
                L = (mid + side) >> 1;
                R = (mid - side) >> 1;

                //fprintf(stdout, "mid/side sample[%llu] = %d, %d\n", i, mid16, side); //.left, right);

                memcpy(&hostBuffer[iL], &L, nSampleBytes);
                memcpy(&hostBuffer[iR], &R, nSampleBytes);

            }
        }
            
    }
    
    return nSubframeSamples/frame->nChannels;
}

XNZ_FLAC_API XNZ_FLAC_INLINE uint64_t xnz_flac_decode_frame_samples(XNZ_FLAC_ARCHIVE* archive, xnz_bitstream* bs, xnz_flac_frame* frame, void** sampleBuffers, uint64_t sampleFrame)
{
    //xnz_flac_frame frame   = {0};
    uint64_t nSubframeSamples =  0;

    //Each frame MUST start on a byte boundary and starts with the 15-bit frame sync code 0b111111111111100.
    uint16_t bytes            = rbytes(xnz_be16toh(xnz_bitstream_read_octet_bits(bs, 16))); //(xnz_bitstream_read_bits(bs, 8) << 8) | (xnz_bitstream_read_bits(bs, 8));
             
    //check for audio frame header sync code
    frame->syncCode         = bytes >> 1; assert(frame->syncCode == 32764);
    
    //get blocking strategy
    frame->blockingStrategy  = bytes & 0x0001; // 0 : fixed-blocksize    stream; frame header encodes the frame number
                                               // 1 : variable-blocksize stream; frame header encodes the sample number

    //read blockSizeID + sampleRate ID from the same byte
    uint8_t byte              = rbytes(xnz_bitstream_read_octet_bits(bs, 8));
    uint8_t blockSizeID       = byte >> 4;
    uint8_t sampleRateID      = byte & 0x0F;
        
    fprintf(stdout, "xnz_flac_read_samples::blockSizeID  = %d\n", blockSizeID);
    fprintf(stdout, "xnz_flac_read_samples::sampleRateID = %d\n", sampleRateID);
    
    frame->blockSize        = 192; //assert(blockSizeID > 0);
    frame->sampleRate       = archive->streaminfo.sampleRate;

    //parse blockSizeID
    if     (blockSizeID > 1 && blockSizeID < 6)  frame->blockSize = 576 * (1<<(blockSizeID - 2));
    //else if(blockSizeID == 6)  assert(1==0);//get  8 bit (blocksize-1) from end of header
    //else if(blockSizeID == 7)  assert(1==0);//get 16 bit (blocksize-1) from end of header
    else if(blockSizeID > 7 && blockSizeID < 15) frame->blockSize = 256 * (1<<(blockSizeID-8));
    else if(blockSizeID > 14) assert(1==0);
    
    //parse sampleRateID
    if( sampleRateID > 0 && sampleRateID < 12) frame->sampleRate = FLAC_SAMPLE_RATES[sampleRateID];
    else if( sampleRateID > 11 && sampleRateID < 16) assert(1==0);
    else assert(1==0);
    
    //Read channel assignment + sample size from the same byte
    byte                 = rbytes(xnz_bitstream_read_octet_bits(bs, 8));

    uint32_t channelID   =  byte >> 4;
    uint8_t sampleSizeID = (byte >> 1) & 0x07; assert(sampleSizeID !=3); //3 is reserved
    
    fprintf(stdout, "xnz_flac_read_samples::channelID  = %d\n",   channelID);
    fprintf(stdout, "xnz_flac_read_samples::sampleSizeID = %d\n", sampleSizeID);
    
    frame->nChannels    = archive->streaminfo.nChannels;
    frame->sampleSize   = archive->streaminfo.bitsPerSample;

    //parse channelID
         if( channelID < 8)   frame->nChannels = channelID + 1;
    else if( channelID == 8)  frame->nChannels = 2; //left  + side
    else if( channelID == 9)  frame->nChannels = 2; //right + side
    else if( channelID == 10) frame->nChannels = 2; //mid + side
    else                      assert(1==0);

    if( channelID > 7 && channelID < 11)
    {
        //assert(1==0); //TO DO: handle mid+ side channel samples
        
    }
    
    frame->channelID = channelID;
    
    //parse sampleSizeID
    uint8_t FLAC_SAMPLE_RATES[8] = {archive->streaminfo.bitsPerSample, 8, 12, 0, 16, 20, 24, 32};
    frame->sampleSize = FLAC_SAMPLE_RATES[sampleSizeID];
    
    //9.1.x read any variable length frame header fields

    //Var1: Coded Number
    //Following the reserved bit (starting at the fifth byte of the frame)
    //is either a sample or a frame number, which will be referred to as the coded number.
    //When dealing with variable block size streams, the sample number of the first sample in the frame is encoded.
    //When the file contains a fixed block size stream, the frame number is encoded.
    uint32_t codedNumber = 0;
    
    //Determine number of variable length bytes that represent coded number by finding the number of leading ones
    
    uint8_t octet    = rbyte(xnz_bitstream_read_octet_bits(bs, 8));
    uint8_t nLeading = 0; uint8_t nVariableBytes  = 0;
    
    //count leading ones in first coded number octet
    for( int i=7; i > -1; i--)
    {
        if ((octet >> i) & 0x01) { nLeading++; continue; }
        break;
    }
    
    //while (!xnz_bitstream_read_octet_bits(bs, 1)) nWastedBits++;

    //if( nLeading == 0 ) codedNumber = octets[0]; //the first 127 coded numbers are stored in a single octet 0xxxxxxx
    //else //read as many bytes as leading ones
    {
        uint8_t baseBits = (8-(nLeading+1));
        uint8_t mask     = ((1 << baseBits) - 1);

        int totalBits = /* baseBits + */ (6 * nLeading); //totalBits -= baseBits;

        //push past leading bits in first octet
        codedNumber |= ((int32_t)(octet & mask)) << totalBits;
        
        //push past two bits in each remaining octet
        mask     = ((1 << 6) - 1);
        for(int i=0; i<nLeading-1; i++)
        {
            octet = rbyte(xnz_bitstream_read_octet_bits(bs, 8)); totalBits -= 6;
            codedNumber |= ((int32_t)(octet & mask)) << totalBits;
        }
        
    }

    nVariableBytes += nLeading;

    //Var 2: Uncommon Block Size
              if( blockSizeID == 6) { frame->blockSize = rbyte(xnz_bitstream_read_octet_bits(bs, 8)) + 1; nVariableBytes += 1; }
         else if( blockSizeID == 7) { frame->blockSize = rbytes(xnz_be16toh(xnz_bitstream_read_octet_bits(bs, 16))) + 1; nVariableBytes += 2; } //((xnz_bitstream_read_bits(bs, 8) << 8) | xnz_bitstream_read_bits(bs, 8)) + 1

    //Var 3: Uncommon sample rate
    
    
    //CRC-8: Frame Header CRC
    uint8_t crc8          = rbyte(xnz_bitstream_read_octet_bits(bs, 8));
    
    fprintf(stdout, "xnz_flac_read_samples::blockSize  = %d\n", frame->blockSize);
    fprintf(stdout, "xnz_flac_read_samples::sampleRate = %d\n", frame->sampleRate);
    fprintf(stdout, "xnz_flac_read_samples::nChannels  = %d\n", frame->nChannels);
    fprintf(stdout, "xnz_flac_read_samples::sampleSize = %d\n", frame->sampleSize);

    assert(bs->position % 8 == 0);
    //assert(bs->position/8 == 6 + nVariableBytes);

    //9.2.  Read Subframes

    //   Following the frame header are a number of subframes equal to the
    //   number of audio channels.  Note that as subframes contain a bitstream
    //   that does not necessarily has to be a whole number of bytes, only the
    //   first subframe always starts at a byte boundary.
    
    //reinitialize stream for reading subframes
    //a subframe consists of mandatory 1 byte for header + blockSize samples,
    //There is no way to tell how long a variable length subframe stream might be?!  Same problem as RFC1951!
    //uint32_t SUBFRAME_HEADER_SIZE = 1 + frame->blockSize * frame->sampleSize/8;
    //uint8_t * src_ptr = bs->buffer + (bs->position/8); //bs->src_ptr + 6 + nVariableBytes;
    //memset(bs, 0, sizeof(xnz_bitstream)); xnz_bitstream_attach(bs, src_ptr, SUBFRAME_HEADER_SIZE);
    nSubframeSamples += xnz_flac_decode_subframe_samples(archive, bs, frame, sampleBuffers, sampleFrame);
    
    
    //TO DO:  read crc-16 property in footer
    
    return nSubframeSamples;
}

XNZ_FLAC_API XNZ_FLAC_INLINE void xnz_flac_read_samples(XNZ_FLAC_ARCHIVE* archive, uint64_t numFramesToRead, void** sampleBuffers)
{
    //Flac Frame
    //uint32_t FRAME_HEADER_SIZE = 6 + 6; //6 mandatory bytes, plus up to 6 variable code number bytes
    xnz_flac_frame frame = {0};

    //Bitstream
    uint8_t* src_ptr = archive->audioFrames;
    uint32_t src_len = (uint32_t)(archive->file.size - (archive->audioFrames - (uint8_t*)archive->file.buffer));
    xnz_bitstream bs = {0}; memset(&bs, 0, sizeof(xnz_bitstream)); xnz_bitstream_attach(&bs, src_ptr, src_len);
    
    //index counts
    uint64_t frameIndex       = 0; //the flac frame (not the sample frame)
    uint64_t nSubframeSamples = 0; //flac refers to this as # interchannel samples
    uint64_t i                = 0;
    
    
    //decode flac frames until all n sample frames (ie interchannel samples) are decoded
    while( nSubframeSamples < archive->streaminfo.nSampleFrames)
    {
        //reattach bitstream each frame
        //memset(&bs, 0, sizeof(xnz_bitstream)); xnz_bitstream_attach(&bs, src_ptr, src_len);

        //decode subframe samples to dst buffer
        uint8_t* hostBuffer = ((uint8_t*)sampleBuffers[0]) + i;
        uint64_t subframeSamples = xnz_flac_decode_frame_samples(archive, &bs, &frame, &hostBuffer, nSubframeSamples);
        
        //increment counts
        i += (subframeSamples * frame.nChannels) * (frame.sampleSize/8);
        nSubframeSamples += subframeSamples;
        
        //burn any bits necessary to ensure frame ends on byte boundary until padding is introduced
        uint32_t paddingBits = (bs.position%8) ? xnz_bitstream_read_octet_bits(&bs, 8 - (bs.position%8)) : 0;

        //TO DO: Verify frame decoding with CRC
        uint16_t crc = xnz_bitstream_read_octet_bits(&bs, 16);

        //update bitstream source for next reattachment
        //src_ptr = bs.buffer + (bs.position / 8);
        //src_len = (uint32_t)archive->file.size - (src_ptr - (uint8_t*)archive->file.buffer);
        
        frameIndex++;
    }

 
    //To validate the MD5, we line up the samples interleaved, byte-aligned, little endian, signed two's complement.
    unsigned char digest[CC_MD5_DIGEST_LENGTH]; //16 bytes
    CC_LONG datalen = (CC_LONG)i;//(nSubframeSamples * archive->streaminfo.nChannels * archive->streaminfo.bitsPerSample/8);
    uint8_t* data   = (uint8_t*)sampleBuffers[0];

    
    //streaming
    //CC_MD5_CTX context;
    //CC_MD5_Init(&context);
    //CC_MD5_Update(&context, data, datalen);
    //CC_MD5_Final(digest, &context);

    //oneshot
    

    CC_MD5(data, datalen, digest);

    for (i=0; i<CC_MD5_DIGEST_LENGTH; ++i)
    {
       fprintf(stdout, "%.2x", digest[i]);
       assert( digest[i] == archive->streaminfo.signature[i] );
    }
    
    /*
    for (i=0; i<datalen; ++i)
    {
       fprintf(stdout, "%.2x", data[i]);
       //assert( digest[i] == archive->streaminfo.signature[i] );
    }
    */
    
    fprintf(stdout, "\n");

    //assert(1==0);
}


XNZ_FLAC_API XNZ_FLAC_INLINE void xnz_flac_close(XNZ_FLAC_ARCHIVE* archive)
{
    CRFileClose(&(archive->file));
    //xnz_file_unmap(crate->file.mFile, crate->file.buffer);
    //xnz_file_close(crate->file.fd);
}


#ifdef __cplusplus
}
#endif


#endif //_XNZ_FLAC_CODEC_H_
