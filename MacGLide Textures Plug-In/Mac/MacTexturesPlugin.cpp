//**************************************************************
//*            MacGLide - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*               Shared Library entry points
//*
//*  MacGLideTexturesPlugin is OpenSource under LGPL license
//*          Originally made by Jens-Olaf Hemprich
//**************************************************************

#include "MacTexturesPlugin.h"

OSErr __macglide_textures_plugin_initialize(struct CFragInitBlock* initBlkPtr)
{
	OSErr err = __initialize((CFragInitBlock*) initBlkPtr);
	if (err == noErr)
	{
#ifdef OGL_PROFILING
		err = ProfilerInit(collectSummary, bestTimeBase, 666, 666);
		assert(err == noErr);
		ProfilerSetStatus(1);
#endif
	}
	return err;
}

void __macglide_textures_plugin_terminate()
{
#ifdef OGL_PROFILING
	 char* filename = "XTextures Plugin Profiler stats";
	filename[0] = strlen(&filename[1]);
	err = ProfilerDump(reinterpret_cast<unsigned char*>(filename));
	ProfilerTerm();
#endif
  __terminate();
}
