//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*            pre-glide22 buffer access interface
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "grLfb.h"

// These are the functions for the obsolete linear frame buffer interface (pre 2.2)
FX_ENTRY void grLfbBegin(void)
{
	// Todo: implment linear framebuffer interface
	assert(false);
}

FX_ENTRY void grLfbBypassMode(GrLfbBypassMode_t mode)
{
	// Todo: implment linear framebuffer interface
	assert(false);
}

FX_ENTRY void GrLfbEnd()
{
	// Todo: implment linear framebuffer interface
	assert(false);
}

FX_ENTRY const FxU32* grLfbGetReadPtr(GrBuffer_t buffer)
{
	// Todo: implment linear framebuffer interface
	assert(false);
	return NULL;
}

FX_ENTRY void* grLfbGetWritePtr(GrBuffer_t buffer)
{
	// Todo: implment linear framebuffer interface
	assert(false);
	return NULL;
}

FX_ENTRY void* grLfbOrigin(GrOriginLocation_t origin)
{
	// Todo: implment linear framebuffer interface
	assert(false);
	return NULL;
}

FX_ENTRY void grLfbWriteMode(GrLfbWriteMode_t mode)
{
	// Todo: implment linear framebuffer interface
	assert(false);
}
