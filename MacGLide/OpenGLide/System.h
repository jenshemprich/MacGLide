//**************************************************************
//*            OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*        file for including all platform specific stuff
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#pragma once

#if defined(__MWERKS__)
	#if defined(macintosh)
		#define OPENGLIDE_HOST_MAC
		// Codewarrior specific
		#define PP_Target_Carbon 0
		#define PP_Target_Classic (!PP_Target_Carbon)
		#define TARGET_API_MAC_CARBON PP_Target_Carbon
		#define TARGET_API_MAC_OS8 PP_Target_Classic
// #  else if /* Carbon or MachO version */
// #    define OPENGLIDE_HOST_MAC
// #  endif /* !macintosh */
	#else /* !macintosh */
		#error "Unknown MetroWerks target platform"
	#endif /* !macintosh */
#endif

// Mac specific stuff
#ifdef OPENGLIDE_HOST_MAC
	// use plain ASCII only
	#define __NO_WIDE_CHAR
	// headers
	#include <MacHeaders.h>
	// platform specific settings
	// macos 9 doesn't provide linkage to fogcoord functions
	#define OPENGLIDE_DOESN_T_HAVE_FOGCOORD
	// macos 9 doesn't provide linkage to blendfunc seperate function
	#define OPENGLIDE_DOESN_T_HAVE_BLENDFUNC_SEPERATE
	// C and C++ standard calling convention (used on win32)
	#define __cdecl
	// #define __stdcall
	#define __fastcall
#else 
	#define OPENGLIDE_SYSTEM_HAS_FOGCOORD
	#define OPENGLIDE_SYSTEM_HAS_BLENDFUNC_SEPERATE
#endif


// Glide SDK
#include "sdk2_3dfx.h"
// This is used by CodeWarrior to just export the
// symbols needed for the 3DfxGlideLib2.x library 
#if defined(__MWERKS__)
	#pragma export on
#endif
	#include "sdk2_glide.h"
	#include "sdk2_glideutl.h"
#if defined(__MWERKS__)
	#pragma export off
#endif
#include "sdk2_glidesys.h"
#include "sdk2_sst1vid.h"

// Export some more functions needed by pre glide 2.4 games 
// (for instance Quake 3dfx on the Mac)
#ifdef OPENGLIDE_HOST_MAC
	#if defined(__MWERKS__)
		#pragma export on
	#endif
	#include "grguMisc.h"
	#include "grguSstGlide.h"
	#include "grLfb.h"
	#include "sst1.h"
	#if defined(__MWERKS__)
		#pragma export off
	#endif
#endif
