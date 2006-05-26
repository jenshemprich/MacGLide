//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                pre-glide24 sst1 functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#pragma once

#include "sdk2_3dfx.h"
#include "sdk2_glide.h"

// prototypes for obsolete sst1-board functions

#ifdef __cplusplus
extern "C" {
#endif

// These are the functions for the obsolete linear frame buffer interface (pre 2.2)
FX_ENTRY void grLfbBegin(void);
FX_ENTRY void grLfbBypassMode(GrLfbBypassMode_t mode);
FX_ENTRY void GrLfbEnd();
FX_ENTRY const FxU32* grLfbGetReadPtr(GrBuffer_t buffer);
FX_ENTRY void* grLfbGetWritePtr(GrBuffer_t buffer);
FX_ENTRY void* grLfbOrigin(GrOriginLocation_t origin);
FX_ENTRY void grLfbWriteMode(GrLfbWriteMode_t mode);

#ifdef __cplusplus
}
#endif
