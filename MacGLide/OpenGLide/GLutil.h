//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                   OpenGL Utility File
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Carbon_SetupGL.h"
#include "Carbon_SetupDSP.h"
#include "Carbon_Error_Handler.h"
#include <agl.h>

struct structWindowInfo
{
	structGLWindowInfo glInfo;
	AGLContext aglContext;
	GLuint fontList;
	char strContext [256];
};

typedef struct structWindowInfo structWindowInfo;
typedef struct structWindowInfo* structWindowInfoPtr;

extern WindowPtr pGLWindow;
extern structWindowInfoPtr pWindowInfo;

OSStatus aglReportWarning(void);

// General opengl utility functions
bool InitialiseOpenGLWindow( FxU32 hwnd, int x, int y, FxU32 width, FxU32 height );
void FinaliseOpenGLWindow( void );
void HideOpenGLWindow();
void RestoreOpenGLWindow();

void ConvertColor4B( GrColor_t GlideColor, FxU32 &C );
void ConvertColorB( GrColor_t GlideColor, FxU8 &R, FxU8 &G, FxU8 &B, FxU8 &A );
void ConvertColorF( GrColor_t GlideColor, float &R, float &G, float &B, float &A );
GrColor_t ConvertConstantColor( float R, float G, float B, float A );
