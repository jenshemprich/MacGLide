
#ifndef __OGL_TABLES_H__
#define __OGL_TABLES_H__

#include "glogl.h"
#include "GLRender.h"

typedef struct 
{
    float h;
    float w;
} OGLAspect;

typedef struct 
{
    int width;
    int height;
} OGLWindow;

typedef struct
{
    int width;
    int height;
    int numPixels;
} OGLTexInfo;

typedef struct
{
    bool                local;
    bool                other;
    ALPHAFACTORFUNCPROC func;
} OGLAlphaTable;

typedef struct
{
    bool                local;
    bool                other;
    bool                alocal;
    bool                aother;
    COLORFACTORFUNCPROC factorfunc;
    COLORFUNCTIONPROC   func;
} OGLColorTable;

// OGLTextureTables.cpp
extern const OGLAspect texAspects[ 7 ];
extern const OGLTexInfo texInfo[ 7 ][ 9 ];
extern const FxU32 nSquareLod[ 2 ][ 7 ][ 9 ];
extern const FxU32 nSquareTexLod[ 2 ][ 7 ][ 9 ][ 9 ];

// OGLColorAlphaTables.cpp
extern const OGLAlphaTable alphaCombineTable[ 14 ][ 17 ];
extern const OGLColorTable colorCombineTable[ 14 ][ 17 ];

// OGLFogTables.cpp
extern const FxU32 intStartEnd[ GR_FOG_TABLE_SIZE + 1 ];
extern const FxU32 intEndMinusStart[ GR_FOG_TABLE_SIZE ];
extern const float tableIndexToW[ GR_FOG_TABLE_SIZE ];

// OGLMiscTables.cpp
extern const OGLWindow windowDimensions[ 16 ];
extern const int windowRefresh[ 9 ];

#endif
