#ifndef _XNZ_AIF_CODEC_H_
#define _XNZ_AIF_CODEC_H_

#include <[Pb]Audio/[Pb]Audio/cr_file.h> //CRFile
//#include "stdint.h"         //uint types

#include <xnz/xnz.h>

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
#ifndef XNZ_AIF_API
#define XNZ_AIF_API static
#endif

//inline doesn't exist in C89, __inline is MSVC specific
#ifndef XNZ_AIF_INLINE
#ifdef _WIN32
#define XNZ_AIF_INLINE __inline
#else
#define XNZ_AIF_INLINE
#endif
#endif

//__decspec doesn't exist in C89, __declspec is MSVC specific
#ifndef XNZ_AIF_DECLSPEC
#ifdef _WIN32
#define XNZ_AIF_DECLSPEC __declspec
#else
#define XNZ_AIF_DECLSPEC
#endif
#endif

//align functions are diffent on windows vs iOS, Linux, etc.
#ifndef XNZ_AIF_ALIGN
#ifdef _WIN32
#define XNZ_AIF_ALIGN(X) 
#else
#define XNZ_AIF_ALIGN(X) __attribute__ ((aligned(X)))
#endif
#endif

#ifndef XNZ_AIF_PACK_ATTRIBUTE
#ifdef _WIN32
#define XNZ_AIF_PACK_ATTRIBUTE
#pragma pack(push, 1)
#else
#define XNZ_AIF_PACK_ATTRIBUTE __attribute__ ((packed))
#endif
#endif

typedef unsigned long long uint64;

static double xnz_read_float80(uint8_t* buffer)
{
    uint32_t offset = 0;// *ref_offset;

    //80 bit floating point value according to the IEEE-754 specification and the Standard Apple Numeric Environment specification:
    //1 bit sign, 15 bit exponent, 1 bit normalization indication, 63 bit mantissa

    double sign;
    if ((buffer[0] & 0x80) == 0x00)
        sign = 1;
    else
        sign = -1;

    uint32_t b0 = (uint32_t)buffer[0];
    uint32_t b1 = (uint32_t)buffer[1];
    uint64_t b2 = *(uint64_t*)&buffer[2];

    uint32_t exponent = (((uint32_t)b0 & 0x7F) << 8) | b1;
    uint64_t mantissa = xnz_htobe64(b2); // readUInt64BE(buffer, 0 + 2);

    //If the highest bit of the mantissa is set, then this is a normalized number.
    double normalizeCorrection;
    if ((mantissa & 0x8000000000000000) != 0x00)
        normalizeCorrection = 1;
    else
        normalizeCorrection = 0;
    mantissa &= 0x7FFFFFFFFFFFFFFF;

    //offset += 10;

    //value = (-1) ^ s * (normalizeCorrection + m / 2 ^ 63) * 2 ^ (e - 16383)
    return (sign * (normalizeCorrection + (double)mantissa / ((uint64_t)1 << 63)) * pow(2, (int32_t)exponent - 16383));
}

static double xnz_double_from_extended(const unsigned char x[10])
{
    int exponent = (((x[9] << 8) | x[8]) & 0x7FFF);
    uint64 mantissa =
        ((uint64)x[7] << 56) | ((uint64)x[6] << 48) | ((uint64)x[5] << 40) | ((uint64)x[4] << 32) | 
        ((uint64)x[3] << 24) | ((uint64)x[2] << 16) | ((uint64)x[1] << 8) | (uint64)x[0];
    unsigned char d[8] = {0};
    double result;

    d[7] = x[9] & 0x80; /* Set sign. */

    if ((exponent == 0x7FFF) || (exponent == 0))
    {
        /* Infinite, NaN or denormal */
        if (exponent == 0x7FFF)
        {
            /* Infinite or NaN */
            d[7] |= 0x7F;
            d[6] = 0xF0;
        }
        else
        {
            /* Otherwise it's denormal. It cannot be represented as double. Translate as singed zero. */
            memcpy(&result, d, 8);
            return result;
        }
    }
    else
    {
        /* Normal number. */
        exponent = exponent - 0x3FFF + 0x03FF; /*< exponent for double precision. */

        if (exponent <= -52)  /*< Too small to represent. Translate as (signed) zero. */
        {
            memcpy(&result, d, 8);
            return result;
        }
        else if (exponent < 0)
        {
            /* Denormal, exponent bits are already zero here. */
        }
        else if (exponent >= 0x7FF) /*< Too large to represent. Translate as infinite. */
        {
            d[7] |= 0x7F;
            d[6] = 0xF0;
            memset(d, 0x00, 6);
            memcpy(&result, d, 8);
            return result;
        }
        else
        {
            /* Representable number */
            d[7] |= (exponent & 0x7F0) >> 4;
            d[6] |= (exponent & 0xF) << 4;
        }
    }
    /* Translate mantissa. */

    mantissa >>= 11;

    if (exponent < 0)
    {
        /* Denormal, further shifting is required here. */
        mantissa >>= (-exponent + 1);
    }

    d[0] = mantissa & 0xFF;
    d[1] = (mantissa >> 8) & 0xFF;
    d[2] = (mantissa >> 16) & 0xFF;
    d[3] = (mantissa >> 24) & 0xFF;
    d[4] = (mantissa >> 32) & 0xFF;
    d[5] = (mantissa >> 40) & 0xFF;
    d[6] |= (mantissa >> 48) & 0x0F;

    memcpy(&result, d, 8);

    printf("Result: 0x%016llx", *(uint64*)(&result) );

    return result;
}

typedef struct AIF_SAMPLE
{
    uint8_t  bytes[4];
    uint16_t shorts[2];
    uint32_t uint;
    int32_t  sint;
    float    f32;
}AIF_SAMPLE;

//EA IFF 85 chunks in precedent order
typedef const enum XNZ_AIF_CHUNK_CODE
{
    AIF_FORM = 0,
    AIF_COMM = 1,
    AIF_SSND = 2,
    AIF_MARK = 3,
    AIF_INST = 4,
    AIF_COMT = 5,

    //Text Chunks
    AIF_NAME = 6,
    AIF_AUTH = 7,
    AIF_CPRT = 8,
    AIF_ANNO = 9,

    AIF_AESD = 10,
    AIF_MIDI = 11,
    AIF_APPL = 12,
        
    AIF_MAX
};

static const uint32_t AIF_FOURCC[AIF_MAX] =
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


/*XNZ_PNG_ALIGN(4)*/ typedef struct XNZ_AIF_PACK_ATTRIBUTE xnz_aif_chunk
{
    union
    {

        uint8_t     chunkID[4];
        uint32_t    fourCC;
    };
    int32_t        chunkSize;

}xnz_aif_chunk;

#define XNZ_AIF_CHUNK_FIELDS \
XNZ_AIF_CHUNK_FIELD(xnz_aif_chunk, uint8_t,  8,  4, chunkID,   chunkID[4], "%s") \
XNZ_AIF_CHUNK_FIELD(xnz_aif_chunk, int32_t, 32,  1, chunkSize, chunkSize, "%hu") \


/***
***    IHDR STRUCT (OPCODE 'IHDR')
***/
//#pragma mark -- IHDR RECORD (OPCODE 'IHDR')

#define XNZ_AIF_FORM_FIELDS \
XNZ_AIF_FORM_FIELD(XNZ_AIF_FORM , uint8_t,  8,  4, formType, formType[4], "%s")


typedef char float_ext[10]; //80 bit IEEE Standard 754

#define XNZ_AIF_COMM_FIELDS \
XNZ_AIF_COMM_FIELD(XNZ_AIF_COMM, uint16_t,   16, 1, nChannels,     nChannels,     "%hu") \
XNZ_AIF_COMM_FIELD(XNZ_AIF_COMM, uint32_t,   32, 1, nSampleFrames, nSampleFrames, "%d") \
XNZ_AIF_COMM_FIELD(XNZ_AIF_COMM, uint16_t,   16, 1, sampleSize,    sampleSize,    "%hu") \
XNZ_AIF_COMM_FIELD(XNZ_AIF_COMM, char ,      80, 1, sampleRate,    sampleRate[10],    "%s")

//XNZ_AIF_FMT_FIELD(XNZ_PNG_IHDR, uint16_t, 16, 1, wValidBitsPerSample, wValidBitsPerSample, "%hu") \
//XNZ_AIF_FMT_FIELD(XNZ_PNG_IHDR, uint32_t, 32, 1, dwChannelMask, dwChannelMask, "%d") \
//XNZ_AIF_FMT_FIELD(XNZ_PNG_IHDR, uint8_t,  8,  16, GUID, GUID, "%d") \

XNZ_AIF_ALIGN(1) typedef struct XNZ_AIF_PACK_ATTRIBUTE xnz_aif_comment
{
    uint32_t            timeStamp;
    short               marker;
    unsigned short      count;
    //char                text;
} xnz_aif_comment;

#define XNZ_AIF_COMT_FIELDS \
XNZ_AIF_COMT_FIELD(XNZ_AIF_COMT, uint16_t,          16,                      1, nComments,     nComments,     "%hu")

//XNZ_AIF_COMT_FIELD(XNZ_AIF_COMT, xnz_aif_comment,   sizeof(xnz_aif_comment), 1, comment, comment, "%d") \
//XNZ_AIF_COMT_FIELD(XNZ_AIF_COMT, uint16_t,   16, 1, sampleSize,    sampleSize,    "%hu") \
//XNZ_AIF_COMT_FIELD(XNZ_AIF_COMT, float_ext , 80, 1, sampleRate,    sampleRate,    "%s")


#define XNZ_AIF_CUE_FIELDS \
XNZ_AIF_CUE_FIELD(XNZ_AIF_CUE, uint32_t, 32, 1, nCuePoints, nCuePoints, "%d") \
XNZ_AIF_CUE_FIELD(XNZ_AIF_CUE, char,      8, 1, points,     points,     "%d")

#define XNZ_AIF_FACT_FIELDS \
XNZ_AIF_FACT_FIELD(XNZ_AIF_FACT, uint32_t, 32, 1, dwSampleLength, dwSampleLength, "%d")

#define XNZ_AIF_LIST_FIELDS \
XNZ_AIF_LIST_FIELD(XNZ_AIF_LIST, uint8_t,  8,  4, typeID, typeID[4], "%s") \
XNZ_AIF_LIST_FIELD(XNZ_AIF_LIST, char,     8,  1, data, data, "%d")



#define XNZ_AIF_SSND_FIELDS \
XNZ_AIF_SSND_FIELD(XNZ_AIF_SSND, uint32_t,   32,                    1, offset,    offset, "%d") \
XNZ_AIF_SSND_FIELD(XNZ_AIF_SSND, uint32_t,   32,                    1, blockSize, blockSize, "%d") \

//XNZ_AIF_SSND_FIELD(XNZ_AIF_SSND, AIF_SAMPLE, sizeof(AIF_SAMPLE)*8,  1, sample,    sample, "%d")

/*
//--- define the structure, the X macro will be expanded once per field
XNZ_AIFE_ALIGN(1) typedef struct CR_PACK_ATTRIBUTE XNZ_PNG_IHDR
{
//the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_PNG_IHDR_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_PNG_IHDR_FIELDS
#undef XNZ_PNG_IHDR_FIELD
} XNZ_PNG_IHDR;
*/

//--- define the structure, the X macro will be expanded once per field
XNZ_AIF_ALIGN(1) typedef struct XNZ_AIF_PACK_ATTRIBUTE XNZ_AIF_FORM
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_AIF_FORM_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_AIF_FORM_FIELDS
#undef  XNZ_AIF_FORM_FIELD
} XNZ_AIF_FORM;

    
XNZ_AIF_ALIGN(1) typedef struct XNZ_AIF_PACK_ATTRIBUTE XNZ_AIF_COMM
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_AIF_COMM_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_AIF_COMM_FIELDS
#undef  XNZ_AIF_COMM_FIELD
} XNZ_AIF_COMM;


XNZ_AIF_ALIGN(1) typedef struct XNZ_AIF_PACK_ATTRIBUTE XNZ_AIF_COMT
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_AIF_COMT_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
        XNZ_AIF_COMT_FIELDS
#undef  XNZ_AIF_COMT_FIELD
} XNZ_AIF_COMT;

/*
XNZ_AIF_ALIGN(1) typedef struct XNZ_AIF_PACK_ATTRIBUTE XNZ_AIF_CUE
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_AIF_CUE_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_AIF_CUE_FIELDS
#undef  XNZ_AIF_CUE_FIELD
} XNZ_AIF_CUE;

XNZ_AIF_ALIGN(1) typedef struct XNZ_AIF_PACK_ATTRIBUTE XNZ_AIF_FACT
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_AIF_FACT_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_AIF_FACT_FIELDS
#undef  XNZ_AIF_FACT_FIELD
} XNZ_AIF_FACT;


XNZ_AIF_ALIGN(1) typedef struct XNZ_AIF_PACK_ATTRIBUTE XNZ_AIF_LIST
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_AIF_LIST_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_AIF_LIST_FIELDS
#undef  XNZ_AIF_LIST_FIELD
} XNZ_AIF_LIST;

*/

XNZ_AIF_ALIGN(1) typedef struct XNZ_AIF_PACK_ATTRIBUTE XNZ_AIF_SSND
{
    uint8_t     chunkID[4];
    uint32_t    chunkSize;

    //the compiler will expand each field in the CR_FLT_HEADER_RECORD_FIELDS macro to define the fields of the struct, supercool!
#define XNZ_AIF_SSND_FIELD(xnz_flt_record_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) datatype varDeclaration;
    XNZ_AIF_SSND_FIELDS
#undef  XNZ_AIF_SSND_FIELD
} XNZ_AIF_SSND;


#pragma mark -- SERIALIZE



/***
 ***    UNIVERSAL SERIALIZE FUNCTION FOR ALL RECORDS DATA FIELDS PARSED BY X-MACROS
 ***
 ***    Converts FLT Record Big Endian Data Types to host format (e.g. Intel = Little Endian) using system level conversion calls
 ***/
static void xnz_aif_chunk_field_serialize(void* data, unsigned int elementSizeInBits, unsigned int numElements)
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
static void xnz_aif_comm_chunk_serialize(void* chunk_data)
{
#define XNZ_AIF_CHUNK_FIELD(xnz_chunk_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) \
        xnz_aif_chunk_field_serialize(&(((xnz_chunk_data*)chunk_data)->varName), dataTypeSizeInBits, numElements);
        XNZ_AIF_CHUNK_FIELDS
#undef  XNZ_AIF_CHUNK_FIELD


#define XNZ_AIF_COMM_FIELD(xnz_chunk_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) \
        xnz_aif_chunk_field_serialize(&(((xnz_chunk_data*)chunk_data)->varName), dataTypeSizeInBits, numElements);
        XNZ_AIF_COMM_FIELDS
#undef  XNZ_AIF_COMM_FIELD

}

//Serialize specific record types;  X-MACROS are used to iterate over each field in each flt record struct with minimal code
static void xnz_aif_ssnd_chunk_serialize(void* chunk_data)
{
#define XNZ_AIF_CHUNK_FIELD(xnz_chunk_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) \
        xnz_aif_chunk_field_serialize(&(((xnz_chunk_data*)chunk_data)->varName), dataTypeSizeInBits, numElements);
        XNZ_AIF_CHUNK_FIELDS
#undef  XNZ_AIF_CHUNK_FIELD


#define XNZ_AIF_SSND_FIELD(xnz_chunk_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) \
        xnz_aif_chunk_field_serialize(&(((xnz_chunk_data*)chunk_data)->varName), dataTypeSizeInBits, numElements);
        XNZ_AIF_SSND_FIELDS
#undef  XNZ_AIF_SSND_FIELD

}



/***
 ***    PRINT FUNCTIONS FOR ALL [SUPPORTED] RECORD TYPES
 ***/
#pragma mark -- PRINT


 /***
  ***    UNIVERSAL PRINT FUNCTION FOR ALL RECORD DATA FIELDS PARSED BY X-MACROS
  ***/
static void xnz_aif_chunk_field_print(char* formatString, char* catString, char* fieldName, void* data, unsigned int elementSizeInBits, unsigned int numElements/*, char * formatSpecifier */, bool floatingPoint)
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

static void xnz_aif_comm_chunk_print(void* chunk_data)
{
    char formatString[256];
    char catString[256];

#define XNZ_AIF_COMM_FIELD(xnz_chunk_data, datatype, dataTypeSizeInBits, numElements, varName, varDeclaration, stringFormat) \
       sprintf (formatString, "\n\t%s = %s", #varName, stringFormat); \
       sprintf(catString, ", %s", stringFormat); \
       xnz_aif_chunk_field_print(formatString, catString, #varName, &(((xnz_chunk_data*)chunk_data)->varName), dataTypeSizeInBits, numElements, strncmp(#datatype, "float", 5) == 0 || strncmp(#datatype, "double", 6) == 0 );
       XNZ_AIF_COMM_FIELDS
#undef XNZ_AIF_COMM_FIELD

}





static void xnz_aif_chunk_print(xnz_aif_chunk* chunk)
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





typedef struct XNZ_AIF_ARCHIVE
{
    union {
        struct {
            union {
                XNZ_AIF_FORM* form;
                void* buffer;
            };
            size_t size;
        };
        CRFile file;
    };

    //xnz_aif_chunk* chunks;

    XNZ_AIF_COMM comm;
    XNZ_AIF_COMT comt;

    //XNZ_AIF_CUE* cue;
    //XNZ_AIF_LIST* list;
    //XNZ_AIF_FACT* fact;
    
    XNZ_AIF_SSND ssnd;

    char* samples;

}XNZ_AIF_ARCHIVE;

#ifdef _WIN32
#pragma pack(pop)   //end struct single byte packing
#endif

XNZ_AIF_API XNZ_AIF_INLINE void   xnz_aif_open(XNZ_AIF_ARCHIVE* archive, char* pngFilePath);
//XNZ_AIF_API XNZ_AIF_INLINE size_t xnz_aif_decode_blocks(XNZ_PNG_IMAGE* archive, uint8_t* dst_buf, size_t dst_size);
XNZ_AIF_API XNZ_AIF_INLINE void   xnz_aif_close(XNZ_AIF_ARCHIVE* archive);


XNZ_AIF_API XNZ_AIF_INLINE void xnz_aif_parse_chunks(XNZ_AIF_ARCHIVE* archive)
{


    xnz_aif_chunk* chunk = NULL;// {0};
    unsigned int  numChunks = 0;
    unsigned int  chunkDataLength = 0;

    size_t bytesRead = 0; size_t padBytes = 0;
    char* bufferPosition = archive->file.buffer;


    fprintf(stderr, "sizeof(XNZ_AIF_FORM) == %zu\n", sizeof(XNZ_AIF_FORM));
    fprintf(stderr, "sizeof(xnz_aif_chunk) == %zu\n", sizeof(xnz_aif_chunk));

    int32_t formChunkSize = xnz_be32toh(archive->form->chunkSize);
    assert(archive->form->formType[0] == 'A');
    assert(archive->form->formType[1] == 'I');
    assert(archive->form->formType[2] == 'F');
    assert(archive->form->formType[3] == 'F');

    /*
    archive->fmt = (XNZ_AIF_FMT*)( archive->file.buffer + sizeof(XNZ_AIF_RIFF) );

    assert(archive->fmt->chunkID[0] == 'f');
    assert(archive->fmt->chunkID[1] == 'm');
    assert(archive->fmt->chunkID[2] == 't');
    assert(archive->fmt->chunkID[3] == ' ');

    fprintf(stderr, "XNZ_AIF_FMT Chunk ID: %.4s\n",      archive->fmt->chunkID);
    fprintf(stderr, "XNZ_AIF_FMT Chunk Size: %d\n",      archive->fmt->chunkSize);
    fprintf(stderr, "XNZ_AIF_FMT wFormatTag: %hu\n",     archive->fmt->wFormatTag);
    fprintf(stderr, "XNZ_AIF_FMT nChannels: %hu\n",      archive->fmt->nChannels);
    fprintf(stderr, "XNZ_AIF_FMT nSamplesPerSec: %u\n",  archive->fmt->nSamplesPerSec);
    fprintf(stderr, "XNZ_AIF_FMT nAvgBytesPerSec: %u\n", archive->fmt->nAvgBytesPerSec);
    fprintf(stderr, "XNZ_AIF_FMT nBlockAlign: %hu\n",    archive->fmt->nBlockAlign);
    fprintf(stderr, "XNZ_AIF_FMT wBitsPerSample: %hu\n", archive->fmt->wBitsPerSample);

    //if no compressed data there will be no fact chunk
    //archive->fact = (XNZ_AIF_FACT*)(archive->file.buffer + sizeof(XNZ_AIF_RIFF) + sizeof(XNZ_AIF_FMT));
    //assert(archive->fact->chunkID[0] == 'f');
    //assert(archive->fact->chunkID[1] == 'a');
    //assert(archive->fact->chunkID[2] == 'c');
    //assert(archive->fact->chunkID[3] == 't');

    archive->data = (XNZ_AIF_DATA*)(archive->file.buffer + sizeof(XNZ_AIF_RIFF) + sizeof(XNZ_AIF_FMT));

    assert(archive->data->chunkID[0] == 'd');
    assert(archive->data->chunkID[1] == 'a');
    assert(archive->data->chunkID[2] == 't');
    assert(archive->data->chunkID[3] == 'a');

    fprintf(stderr, "XNZ_AIF_DATA Chunk ID: %.4s\n", archive->data->chunkID);
    fprintf(stderr, "XNZ_AIF_DATA Chunk Size: %d\n", archive->data->chunkSize);
    */

    bytesRead += sizeof(XNZ_AIF_FORM);
    bufferPosition += sizeof(XNZ_AIF_FORM);

    xnz_aif_chunk chunks[10];

    //read the header record
    while (bytesRead < archive->file.size)
    {
        //read 8 bytes into the struct containing the opcode and chunk length (in bytes) as words, respectively
        //fread(&record, 4, 1, file);
        //memcpy(&chunk, bufferPosition, sizeof(xnz_aif_chunk)); //bufferPosition += sizeof(xnz_aif_chunk);

        chunk = (xnz_aif_chunk*)bufferPosition;
        //chunks[0] = *chunk;
        //chunks[0].chunkSize = xnz_be32toh(chunks[0].chunkSize);

        //convert 16 bits from big endian to little endian
        //record.opcode = cr_be16toh(record.opcode);//(( (record.opcode & 0xff)<<8 | (record.opcode & 0xff00)>>8 ));
        //record.length = cr_be16toh(record.length);//(( (record.length & 0xff)<<8 | (record.length & 0xff00)>>8 ));

        //OpCodeCount[record.opcode - 1]++;

        chunkDataLength = xnz_be32toh(chunk->chunkSize) + sizeof(xnz_aif_chunk);
        
        //if (chunk->chunkSize % 2 == 1) chunkDataLength += 1;

        switch (chunk->fourCC)
        {

        case (MAKEFOURCC('C','O','M','M')):
            memcpy(&archive->comm, chunk, sizeof(XNZ_AIF_COMM)); xnz_aif_comm_chunk_serialize(&archive->comm); break;
            //archive->comm = (XNZ_AIF_COMM*)chunk; break;
        case (MAKEFOURCC('S','S','N','D')):
        {
            memcpy(&archive->ssnd, chunk, sizeof(XNZ_AIF_SSND)); xnz_aif_ssnd_chunk_serialize(&archive->ssnd);
            if (archive->ssnd.chunkSize % 2 == 1) { chunkDataLength += 1; padBytes++; }
            archive->samples = (char*)chunk + sizeof(XNZ_AIF_SSND);
            break;
        }

        /*
        case (MAKEFOURCC('f', 'm', 't', ' ')):
            archive->fmt = (XNZ_AIF_FMT*)chunk; break;
        case (MAKEFOURCC('c', 'u', 'e', ' ')):
            archive->cue = (XNZ_AIF_CUE*)chunk; break;
        case (MAKEFOURCC('L', 'I', 'S', 'T')):
            archive->list = (XNZ_AIF_LIST*)chunk; break;
        case (MAKEFOURCC('f', 'a', 'c', 't')):
            archive->fact = (XNZ_AIF_FACT*)chunk; break;

        */

        default:
            fprintf(stderr, "xnz_aif_parse_chunks::Unhandled AIF Chunk: %.4s\n", chunk->chunkID);
        }

        //seek forward to start of next record
        //fseek(file, recordDataLength, SEEK_CUR);
        bufferPosition += chunkDataLength;
        //long currentByte = ftell(file);
        bytesRead += chunkDataLength;// chunk.chunkSize;
        numChunks++;
    }

    assert(bytesRead - sizeof(XNZ_AIF_FORM) - padBytes == formChunkSize - 4);
}

XNZ_AIF_API XNZ_AIF_INLINE void xnz_aif_open(XNZ_AIF_ARCHIVE* archive, char* wavFilePath)
{

    if (wavFilePath)
    {
        //1 OPEN THE GBL OR GLTF FILE FOR READING AND GET FILESIZE USING LSEEK
        archive->file.fd = cr_file_open(wavFilePath);
        archive->file.size = cr_file_size(archive->file.fd);
        archive->file.path = wavFilePath;

        fprintf(stderr, "\nxnz_aif_open::WAV File Size =  %lu bytes\n", archive->file.size);

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

    //3  READ THE PNG (.png) file signature
    archive->form = (XNZ_AIF_FORM*)archive->file.buffer;

    assert(archive->form->chunkID[0] == 'F');
    assert(archive->form->chunkID[1] == 'O');
    assert(archive->form->chunkID[2] == 'R');
    assert(archive->form->chunkID[3] == 'M');

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

    xnz_aif_parse_chunks(archive);


    //archive->samples = &archive->ssnd.sample;

}

XNZ_AIF_API XNZ_AIF_INLINE void xnz_aif_read_samples(XNZ_AIF_ARCHIVE* archive, unsigned long long numFramesToRead, void** sampleBuffers)
{
    switch (archive->comm.sampleSize)
    {
        case (16):
        {
            //int16_t** shortBuffers = (int16_t**)archive->samples;
            int16_t* shortSamplesL = (int16_t*)archive->samples;// shortBuffers[0];
            //int16_t* shortSamplesR = (int16_t*)(player->sourceAudioFile.samples[1]);

            //dest
            int16_t* hostBuffer = (int16_t*)sampleBuffers[0];

            for (int channel = 0; channel < archive->comm.nChannels; channel++)
            {
                for (int frame = 0; frame < archive->comm.nSampleFrames; frame++)
                {
                    hostBuffer[frame * archive->comm.nChannels + channel] = xnz_be16toh(shortSamplesL[frame * archive->comm.nChannels + channel]);

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

            for (int channel = 0; channel < archive->comm.nChannels; channel++)
            {
                for (int frame = 0; frame < archive->comm.nSampleFrames; frame++)
                {
                    uint64_t i = (frame * archive->comm.nChannels + channel) * 3;
                    //memcpy(&hostBuffer[i], &sourceSamples[i], 3);
                    hostBuffer[i + 0] = sourceSamples[i + 2];
                    hostBuffer[i + 1] = sourceSamples[i + 1];
                    hostBuffer[i + 2] = sourceSamples[i + 0];
                }
            }
            break;

        }

        case (32):
        {

            //int16_t** shortBuffers = (int16_t**)archive->samples;
            int32_t* intSamples = (int32_t*)archive->samples;// shortBuffers[0];
            //int16_t* shortSamplesR = (int16_t*)(player->sourceAudioFile.samples[1]);

            //dest
            int32_t* hostBuffer = (int32_t*)sampleBuffers[0];

            for (int channel = 0; channel < archive->comm.nChannels; channel++)
            {
                for (int frame = 0; frame < archive->comm.nSampleFrames; frame++)
                {
                    uint64_t i = (frame * archive->comm.nChannels + channel);// *4;
                    hostBuffer[i] = xnz_be32toh(intSamples[i]);
                }
            }
            break;

        }


        default:
            assert(1 == 0);
    }
}


XNZ_AIF_API XNZ_AIF_INLINE void xnz_aif_close(XNZ_AIF_ARCHIVE* archive)
{
    CRFileClose(&(archive->file));
    //xnz_file_unmap(crate->file.mFile, crate->file.buffer);
    //xnz_file_close(crate->file.fd);
}


#ifdef __cplusplus
}
#endif


#endif //_XNZ_AIF_CODEC_H_