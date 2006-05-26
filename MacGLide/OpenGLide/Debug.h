//**************************************************************
//*     OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                        Debug switches
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

//#define OGL_DEBUG
//#define OGL_PROFILING
//#define OGL_RENDER
//#define OPENGL_DEBUG
//#define OGL_DEBUG_COORDS
//#define OGL_ALL

//#define OGL_DONE
//#define OGL_PARTDONE
//#define OGL_NOTDONE
//#define OGL_COMBINE
//#define OGL_CRITICAL
//#define OGL_FRAMEBUFFER

//#define OGL_UTEX
//#define OGL_UTEX_MEM
//#define OGL_OPTIMISE_DEBUG
//#define OGL_DEBUG_GLIDE_COORDS
//#define OGL_DEBUG_OPENGL_COORDS

#ifdef OGL_DEBUG_COORDS
	#define OGL_DEBUG_GLIDE_COORDS
	#define OGL_DEBUG_OPENGL_COORDS
#endif

#ifdef OGL_RENDER
	#define OGL_DONE
	#define OGL_PARTDONE
	#define OGL_NOTDONE
	#define OGL_DEBUG
	#define OGL_CRITICAL
	#define OGL_COMBINE
	#define OGL_FRAMEBUFFER
#endif

#ifdef OGL_ALL
	#define OGL_DONE
	#define OGL_PARTDONE
	#define OGL_NOTDONE
	#define OGL_DEBUG
	#define OGL_CRITICAL
	#define OPENGL_DEBUG
	#define OGL_PROFILING
	#define OGL_UTEX
	#define OGL_COMBINE
	#define OGL_FRAMEBUFFER
	#define OGL_DEBUG_GLIDE_COORDS
	#define OGL_DEBUG_OPENGL_COORDS
#endif

#define OGL_STOP_ON_GL_ERROR
