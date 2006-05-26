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

#include "GlideFramebuffer.h"
#include "GlideSettings_FSp.h"
typedef GlideSettingsFSp GlideSettingsImpl;

#ifdef OPENGLIDE_HOST_MAC
	#include "MacGlide.h"
#endif

// Pointer to version string
extern const char* OpenGLideVersion;

// Product name of the library
extern const char* OpenGLideProductName;
 
// extern double ClockFreq;
extern GlideStruct Glide; // Glide Internal
extern OpenGLStruct OpenGL; // OpenGL equivalents
extern GlideSettingsImpl UserConfig;
extern GlideSettingsImpl InternalConfig;

// GLide
OSErr InitMainVariables();
bool InitWindow( FxU32 hwnd );
void InitOpenGL( void );
bool ClearAndGenerateLogFile( void );
void CloseLogFile( void );

// Memory management
void* AllocFrameBuffer(long buffersize, long buffertypesize);
void FreeFrameBuffer(void* address);
void* AllocBuffer(long buffersize, long buffertypesize);
void FreeBuffer(void* address);
void* AllocObject(long buffersize);
void FreeObject(void* buffer);
// Platform specific
void* AllocSysPtr16ByteAligned(long buffersize);
void Free16ByteAligned(void* aligned_buffer);

// framebuffer emulation
extern GlideFramebuffer s_Framebuffer;

// endianess
inline void swaplong(void* l)
{
	unsigned char* v = reinterpret_cast<unsigned char*>(l);
	unsigned char n[4];
	n[0] = v[3];	n[1] = v[2];	n[2] = v[1];	n[3] = v[0];
	*reinterpret_cast<unsigned long*>(l) = *reinterpret_cast<unsigned long*>(n);
}

inline void swapshort(void* s)
{
	unsigned char* v = reinterpret_cast<unsigned char*>(s);
	unsigned char n[2];
	n[0] = v[1];	n[1] = v[0];
	*reinterpret_cast<unsigned short*>(s) = *reinterpret_cast<unsigned short*>(n);
}

inline void swaplong(void* d, const void* l)
{
	const unsigned char* v = reinterpret_cast<const unsigned char*>(l);
	unsigned char n[4];
	n[0] = v[3];	n[1] = v[2];	n[2] = v[1];	n[3] = v[0];
	*reinterpret_cast<unsigned long*>(d) = *reinterpret_cast<unsigned long*>(n);
}

inline void swapshort(void* d, const void* s)
{
	const unsigned char* v = reinterpret_cast<const unsigned char*>(s);
	unsigned char n[2];
	n[0] = v[1];	n[1] = v[0];
	*reinterpret_cast<unsigned short*>(d) = *reinterpret_cast<unsigned short*>(n);
}

#define max(X,Y) ( (X) > (Y) ? (X) : (Y) )
#define min(X,Y) ( (X) < (Y) ? (X) : (Y) )

// size of temporary buffers for string processing
const int StringBufferSize = 256;

void GlideMsg(const char *message, ...);
void GlideError(const char *message, ...);

extern void FatalErrorMessageBox(const char* message);

// error handling and debugging
#ifdef OPENGL_DEBUG
	OSStatus glReportError_impl(const char* __glide_functionname);
	#ifdef OGL_STOP_ON_GL_ERROR
		#define glReportError() assert(GL_NO_ERROR == glReportError_impl(__glide_functionname));
	#else
		#define glReportError() glReportError_impl(__glide_functionname);
	#endif
	#define glReportErrors(name) const char* __glide_functionname = name
#else
	#define glReportError()
	#define glReportErrors(name)
#endif

// Optimising the code
#ifdef OPTIMISE_GLIDE_STATE_CHANGES
	#define CHECK_STATE_CHANGED(x) if (x) return
#else
	#define CHECK_STATE_CHANGED(x)
#endif

// Veryfying state
#if defined(OPTIMISE_OPENGL_STATE_CHANGES) && defined(OPENGL_DEBUG)
	bool VerifyActiveTextureUnit_impl(GLint x, const char* functionname);
#ifdef OGL_STOP_ON_GL_ERROR
	#define VERIFY_ACTIVE_TEXTURE_UNIT(x) assert(VerifyActiveTextureUnit_impl(x, __glide_functionname));
#else
	#define VERIFY_ACTIVE_TEXTURE_UNIT(x) VerifyActiveTextureUnit_impl(x, __glide_functionname);
#endif
#else
	#define VERIFY_ACTIVE_TEXTURE_UNIT(x)
#endif

#if defined(OPTIMISE_OPENGL_STATE_CHANGES) && defined(OPENGL_DEBUG)
	bool VerifyTextureEnabledState_impl(const char* functionname);
#ifdef OGL_STOP_ON_GL_ERROR
	#define VERIFY_TEXTURE_ENABLED_STATE() assert(VerifyTextureEnabledState_impl(__glide_functionname));
#else
	#define VERIFY_TEXTURE_ENABLED_STATE() VerifyTextureEnabledState_impl(__glide_functionname);
#endif
#else
	#define VERIFY_TEXTURE_ENABLED_STATE()
#endif
