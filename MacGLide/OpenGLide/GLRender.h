//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                     Render Header
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#ifndef __GLRENDER_H__
#define __GLRENDER_H__

//**************************************************************
// Structs
//**************************************************************

struct GlVertex
{
	GLfloat x, y, z;
	GLfloat s, t, q, oow;
	GLfloat r, g, b, a;
	GLfloat r2, g2, b2, a2;
	GLfloat f;
};

struct Triangle
{
	GlVertex a, b, c;
};

struct ColorStruct
{
	GLfloat r, g, b, a;
};

struct TColorStruct
{
	GLfloat ar, ag, ab, aa;
	GLfloat br, bg, bb, ba;
	GLfloat cr, cg, cb, ca;
};

struct TVertexStruct
{
	GLfloat ax, ay, az, aw;
	GLfloat bx, by, bz, bw;
	GLfloat cx, cy, cz, cw;
};

struct TTextureStruct
{
	GLfloat as, at, aq, aoow;
	GLfloat bs, bt, bq, boow;
	GLfloat cs, ct, cq, coow;
};

struct TFogStruct
{
	GLfloat af;
	GLfloat bf;
	GLfloat cf;
};

struct RenderStruct
{
	static const int RenderBufferSize = 512;
	int FrameBufferTrianglesStart;
	TColorStruct *TColor;
	TColorStruct *TColor2;
	TVertexStruct *TVertex;
	TTextureStruct *TTexture;
	TFogStruct *TFog;
	long NumberOfTriangles;
	bool UseEnvCombineFog;
	bool BufferLocked;
	FxU32 BufferLockedStart;
	FxU32 BufferStart;
#ifdef OGL_DEBUG
	float MinX, MinY, MinZ, MinW;
	float MaxX, MaxY, MaxZ, MaxW;
	float MinS, MinT, MaxS, MaxT;
	float MinR, MinG, MinB, MinA;
	float MaxR, MaxG, MaxB, MaxA;
	float MaxF, MinF;
	unsigned long FrameTriangles, MaxTriangles, MaxSequencedTriangles;
	unsigned long OverallTriangles, OverallRenderTriangleCalls;
	unsigned long OverallLines;
	unsigned long OverallPoints;
#endif
};

typedef float (*ALPHAFACTORFUNCPROC)( float LocalAlpha, float OtherAlpha );
typedef void  (*COLORFACTORFUNCPROC)( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha );
typedef void  (*COLORFUNCTIONPROC)( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );

//**************************************************************
// Function Prototypes
//**************************************************************

// Main Render functions
void RenderInitialize( void );
void RenderFree( void );
void RenderAddTriangle( const GrVertex *a, const GrVertex *b, const GrVertex *c, bool unsnap );
void RenderAddLine( const GrVertex *a, const GrVertex *b, bool unsnap );
void RenderAddPoint( const GrVertex *a, bool unsnap );
void RenderDrawTriangles( void );

extern RenderStruct	OGLRender;

void RenderInitialize( void );
void RenderFree( void );

inline void RenderUnlockArrays()
{
	glReportErrors("RenderUnlockArrays");

	if (OGLRender.BufferLocked)
	{
		glUnlockArraysEXT();
		glReportError();
		OGLRender.BufferLocked = false;
	}
};


// Main Render variables
// extern RenderStruct		    OGLRender;
extern ALPHAFACTORFUNCPROC  AlphaFactorFunc;
extern COLORFACTORFUNCPROC  ColorFactor3Func;
extern COLORFUNCTIONPROC    ColorFunctionFunc;

// Prototypes for the color combining
float AlphaFactorZero( float LocalAlpha, float OtherAlpha );
float AlphaFactorLocal( float LocalAlpha, float OtherAlpha );
float AlphaFactorOther( float LocalAlpha, float OtherAlpha );
float AlphaFactorOneMinusLocal( float LocalAlpha, float OtherAlpha );
float AlphaFactorOneMinusOther( float LocalAlpha, float OtherAlpha );
float AlphaFactorOne( float LocalAlpha, float OtherAlpha );

void ColorFactor3Zero( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha );
void ColorFactor3Local( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha );
void ColorFactor3LocalAlpha( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha );
void ColorFactor3OneMinusLocal( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha );
void ColorFactor3OneMinusLocalAlpha( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha );
void ColorFactor3OtherAlpha( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha );
void ColorFactor3OneMinusOtherAlpha( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha );
void ColorFactor3One( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha );

void ColorFunctionZero( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );
void ColorFunctionLocal( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );
void ColorFunctionLocalAlpha( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );
void ColorFunctionScaleOther( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );
void ColorFunctionScaleOtherAddLocal( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );
void ColorFunctionScaleOtherAddLocalAlpha( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );
void ColorFunctionScaleOtherMinusLocal( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );
void ColorFunctionScaleOtherMinusLocalAddLocal( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );
void ColorFunctionScaleOtherMinusLocalAddLocalAlpha( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );
void ColorFunctionMinusLocalAddLocal( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );
void ColorFunctionMinusLocalAddLocalAlpha( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other );

#endif