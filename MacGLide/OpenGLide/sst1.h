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

FX_ENTRY FxU32* FX_CALL sst1InitMapBoard(FxU32 param1);
FX_ENTRY FxBool FX_CALL sst1InitGamma(FxU32* param1, double param2);
FX_ENTRY FxBool FX_CALL grSstOpen(GrScreenResolution_t res,
                                  GrScreenRefresh_t refresh,
                                  GrColorFormat_t cFormat,
                                  GrOriginLocation_t locateOrigin,
                                  GrSmoothingMode_t smoothMode,
                                  int numBuffers);

#ifdef __cplusplus
}
#endif
