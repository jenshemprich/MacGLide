//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                     Other Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#ifndef GRGUMISC_H_
#define GRGUMISC_H_

typedef void (*GLIDEERRORFUNCTION)( const char *string, FxBool fatal );
extern GLIDEERRORFUNCTION ExternErrorFunction;

// functions not defined in the SDK but part of either 
// - earlier versions of Glide 2.4
// - Part of the GLide 3 interface
#ifdef __cplusplus
extern "C" {
#endif
	FX_ENTRY void FX_CALL guMovieStart(void);
	FX_ENTRY void FX_CALL guMovieStop(void);
	FX_ENTRY void FX_CALL guMovieSetName(const char *name);
	FX_ENTRY void FX_CALL grParameterData(FxU32 param, FxU32 components, FxU32 type, FxI32 offset);
	FX_ENTRY FxU32 FX_CALL guEndianSwapFxU16s(FxU32 value);
#ifdef __cplusplus
}
#endif

#endif /*GRGUMISC_H_*/
