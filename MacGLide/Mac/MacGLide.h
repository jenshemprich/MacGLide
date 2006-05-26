//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*               Shared Library entry points
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#pragma once

// library entry points
#pragma export on
extern "C" OSErr __macglide_initialize(struct CFragInitBlock* initBlkPtr);
extern "C" void __macglide_terminate();
extern "C" OSErr __initialize(struct CFragInitBlock* initBlkPtr);
extern "C" void __terminate();
#pragma export off

// Not declared in MSL
char* _strtime(char* tmpbuf);
char* _strdate(char* tmpbuf);

// OpenGL extension data not available in MacOS 9 but
// provided by the MacOS X OpenGL driver in Classic

// GL_APPLE_client_storage
#define GL_UNPACK_CLIENT_STORAGE_APPLE 0x85B2

// GL_ARB_multisample
// aglChoosePixelFormat
#define AGL_SAMPLE_BUFFERS_ARB 55
#define AGL_SAMPLES_ARB 56
// aglSetInteger
#define AGL_ATI_FSAA_LEVEL 510

// GL_NV_multisample_filter_hint
#define GL_MULTISAMPLE_FILTER_HINT_NV 0x8534

// GL_ARB_texture_env_combine
#define GL_SUBTRACT_ARB 0x84E7

// GL_ATI_texture_env_combine3
#define GL_MODULATE_ADD_ATI 0x8744

// Swap limit
#define AGL_SWAP_LIMIT 203
