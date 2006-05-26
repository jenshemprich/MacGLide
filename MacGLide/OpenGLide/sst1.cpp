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

#include "sst1.h"

// Called by Quake 1 3dfx on the Mac 
// (resolves error message: sst1InitMapBoard nicht gefunden)
//
// See also: http://www.xlr8yourmac.com/feedback/voodoo2gen.html
//	¥ 	Quake 1.09.x does not work. A programmer friend of mine says
//      this is because Quake makes a call to an obsolete Glide function
//      (sstInitMapBoard or similar call) which I hear was purposefully
//      left out of these relatively up-to-date Glide 2.x drivers ...
// Parameters from
// http://home.y3m.net/horde/chora/co.php/homeworld/src/rgl/glide3/include/sst1init3.h?sbt=3&ord=1&r=1.1.1.1

FX_ENTRY FxU32* FX_CALL sst1InitMapBoard(FxU32 param1)
{
#ifdef OGL_NOTDONE
    GlideMsg("sst1InitMapBoard(%d)\n", param1);
#endif

	return 0;
}

FX_ENTRY FxBool FX_CALL sst1InitGamma(FxU32* param1, double param2)
{
#ifdef OGL_NOTDONE
    GlideMsg("sst1InitGamma(%d, %g)\n", param1, param2);
#endif

	return FXTRUE;
}

FX_ENTRY FxBool FX_CALL grSstOpen(GrScreenResolution_t res,
                                  GrScreenRefresh_t refresh,
                                  GrColorFormat_t cFormat,
                                  GrOriginLocation_t locateOrigin,
                                  GrSmoothingMode_t smoothMode,
                                  int numBuffers)
{
#ifdef OGL_DONE
    GlideMsg("grSstOpen( %d, %d, %d, %d, %d, %d, %d)\n", res, refresh, cFormat, locateOrigin, smoothMode, numBuffers);
#endif
	return grSstWinOpen( 0, res, refresh, cFormat, locateOrigin, numBuffers, 0);
}
