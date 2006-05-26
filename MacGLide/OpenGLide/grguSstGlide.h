//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                     Sst Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#ifndef GRGUSSTGLIDE_H_
#define GRGUSSTGLIDE_H_

// functions not defined in the SDK but part of either 
// - earlier versions of Glide 2.4
// - Part of the GLide 3 interface
#ifdef __cplusplus
extern "C" {
#endif
	FX_ENTRY FxBool FX_CALL grSstControlMode(GrControl_t mode);
#ifdef __cplusplus
}
#endif

#endif /*GRGUSSTGLIDE_H_*/
