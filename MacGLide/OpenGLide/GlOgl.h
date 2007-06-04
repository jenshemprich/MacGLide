//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                      Main Header
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#ifndef __GLOGL_H__
#define __GLOGL_H__

// #define __WIN32__

#include "sdk2_glide.h"

/*
#define RDTSC(v)    __asm _emit 0x0f                \
                    __asm _emit 0x31                \
                    __asm mov FxU32 ptr v, eax      \
                    __asm mov FxU32 ptr v+4, edx
*/
#define RDTSC(v) 

#define OGL_LOG_SEPARATE        "--------------------------------------------------------\n"

#define OGL_MIN_FRAME_BUFFER    2
#define OGL_MAX_FRAME_BUFFER    16
#define OGL_MIN_TEXTURE_BUFFER  2
#define OGL_MAX_TEXTURE_BUFFER  32

#define OPENGLFOGTABLESIZE      64 * 1024

#define D1OVER255               0.003921568627451f      // 1 / 255
#define D1OVER65536             0.0000152587890625f     // 1 / 65536
#define D1OVER65535             0.000015259021896696421759365224689097f     // 1 / 65535
#define D1OVER256               0.00390625f             // 1 / 256
#define D2OVER256               0.0078125f              // 2 / 256
#define D4OVER256               0.015625f               // 4 / 256
#define D8OVER256               0.03125f                // 8 / 256

#define WBUFFERNEAR             -1.0f
#define WBUFFERFAR               0.0f
#define ZBUFFERNEAR              0.0f
#define ZBUFFERFAR              -1.0f

// Class declarations

struct BufferStruct
{
    bool                    Lock;
    GrLock_t                Type;
    GrLfbWriteMode_t        WriteMode;
    GrBuffer_t              Buffer;
    FxBool                  PixelPipeline;
    FxU16                    *Address;
};

struct TexSourceStruct
{
    FxU32       StartAddress;
    FxU32       EvenOdd;
    GrTexInfo   Info;
};

union OGLByteColor
{
    struct
    {
        FxU8    B;
        FxU8    G;
        FxU8    R;
        FxU8    A;
    };
    FxU32   C;
};

struct CombineFunction;
struct CombineArgument;

// The current glide state, valid anytime
struct GlideState
{
	GrBuffer_t              RenderBuffer;
	GrDepthBufferMode_t     DepthBufferMode;
	GrCmpFnc_t              DepthFunction;
	FxBool                  DepthBufferWritting;
	FxI16                   DepthBiasLevel;
	GrDitherMode_t          DitherMode;
	GrColor_t               ChromakeyValue;
	GrChromakeyMode_t       ChromaKeyMode;
	GrAlpha_t               AlphaReferenceValue;
	GrCmpFnc_t              AlphaTestFunction;
	FxBool                  AlphaMask;
	FxBool                  ColorMask;
	GrColor_t               ConstantColorValue;
	GrColor_t               FogColorValue;
	GrFogMode_t             FogMode;
	GrCullMode_t            CullMode;
	GrTextureClampMode_t    SClampMode;
	GrTextureClampMode_t    TClampMode;
	GrTextureFilterMode_t   MinFilterMode;
	GrTextureFilterMode_t   MagFilterMode;
	GrMipMapMode_t          MipMapMode;
	FxBool                  LodBlend;
	GrCombineFunction_t     ColorCombineFunction;
	GrCombineFactor_t       ColorCombineFactor;
	GrCombineLocal_t        ColorCombineLocal;
	GrCombineOther_t        ColorCombineOther;
	FxBool                  ColorCombineInvert;
	GrCombineFunction_t     AlphaFunction;
	GrCombineFactor_t       AlphaFactor;
	GrCombineLocal_t        AlphaLocal;
	GrCombineOther_t        AlphaOther;
	FxBool                  AlphaInvert;
	GrCombineFunction_t     TextureCombineCFunction;
	GrCombineFactor_t       TextureCombineCFactor;
	GrCombineFunction_t     TextureCombineAFunction;
	GrCombineFactor_t       TextureCombineAFactor;
	FxBool                  TextureCombineRGBInvert;
	FxBool                  TextureCombineAInvert;
	GrOriginLocation_t      OriginInformation;
	TexSourceStruct         TexSource;
	GrAlphaBlendFnc_t       AlphaBlendRgbSf;
	GrAlphaBlendFnc_t       AlphaBlendRgbDf;
	GrAlphaBlendFnc_t       AlphaBlendAlphaSf;
	GrAlphaBlendFnc_t       AlphaBlendAlphaDf;
	FxU32                   ClipMinX;
	FxU32                   ClipMaxX;
	FxU32                   ClipMinY;
	FxU32                   ClipMaxY;
	GrColorFormat_t         ColorFormat;
	FxU32                   STWHint;
	FxBool                  VRetrace;
	FxFloat                 LodBias;
	bool                    Delta0Mode;
	FxFloat                 Delta0ModeColor[4];
	FxFloat                 Gamma;
};

// Additional internal Glide state information
struct GlideStruct
{
    int                     ActiveVoodoo;
    // Frame Buffer Stuff
    int                     WindowWidth;
    int                     WindowHeight;
    int                     WindowTotalPixels;
    GrScreenResolution_t    Resolution;
    GrScreenRefresh_t       Refresh;
    int                     NumBuffers;
    int                     AuxBuffers;
    // States and Constants
    FxU8                    FogTable[ GR_FOG_TABLE_SIZE + 1 ];
    FxU32                   TexMemoryMaxPosition;
    bool                    CLocal;
    bool                    COther;
    bool                    ALocal;
    bool                    AOther;
    GlideState              State;
    BufferStruct            TempBuffer;
    BufferStruct            FrameBuffer;
    BufferStruct            ReadBuffer;
    FxU32                   TextureMemory;
};

// The current OpenGL state
// Vadility of members:
// - session static - these don't change during the lifetime of the OpenGL context
// - up to date - they are always up to date
// - forthcoming up to date on the next call to RenderUpdateState()
//
// @todo: Some state variables are considered not until the next call to RenderUpdateState
// while the others reflect the actual state  
struct OpenGLStruct
{
	// Session static
	bool                    GlideInit;
	bool                    WinOpen;
	FxU32                   OriginX;
	FxU32                   OriginY;
	unsigned long           WindowWidth;
	unsigned long           WindowHeight;
	GLint                   ColorAlphaUnit1;
	GLint                   ColorAlphaUnit2;
	GLuint                  DummyTextureName;
	GLint                   FogTextureUnit;
	int                     MultiTextureTMUs;
	GLuint                  Refresh;
	FxU32                   WaitSignal;
	// up to date on the next call to RenderUpdateState()
	bool                    Blend;
	bool                    ChromaKey;
	bool                    ClipVerticesEnabledState;
	bool                    Fog;
	GLfloat                 FogColor[4];
	const CombineFunction*  ColorCombineFunctions;
	const CombineFunction*  AlphaCombineFunctions;
	const CombineArgument*  ColorCombineArguments[5];
	const CombineArgument*  AlphaCombineArguments[5];
	GLenum                  SrcBlend;
	GLenum                  DstBlend;
	GLenum                  SrcAlphaBlend;
	GLenum                  DstAlphaBlend;
	// always up to date	
	FxU32                   ClipMinX;
	FxU32                   ClipMaxX;
	FxU32                   ClipMinY;
	FxU32                   ClipMaxY;
	bool                    Clipping;
	GLfloat                 AlphaReferenceValue;
	GLenum                  AlphaTestFunction;
	GLboolean               DepthBufferWritting;
	GLfloat                 DepthBiasLevel;
	GLenum                  DepthFunction;
	int                     DepthBufferType;
	GLfloat                 ZNear;
	GLfloat                 ZFar;
	GLenum                  RenderBuffer;
	GLenum                  SClampMode;
	GLenum                  TClampMode;
	GLenum                  MinFilterMode;
	GLenum                  MagFilterMode;
	GLfloat                 Gamma;
	GLfloat                 ConstantColor[4];
	GLfloat                 Delta0Color[4];
	bool                    ColorAlphaUnitColorEnabledState[2];
	bool                    ColorAlphaUnitAlphaEnabledState[2];
	OGLByteColor            ChromaColor;
	bool                    ColorTexture;
	bool                    AlphaTexture;
	bool                    Texture;
	bool                    FogTextureUnitEnabledState; // @todo: currently not used
	FxU8                    FogTable[OPENGLFOGTABLESIZE];
};

#endif
