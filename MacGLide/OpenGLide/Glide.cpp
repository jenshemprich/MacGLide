//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                         Main File
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Glide.h"
#include "GlideApplication.h"
#include "GlideDisplay.h"
#include "GlideFramebuffer.h"
#include "GlideSettings.h"
#include "GLextensions.h"
#include "GLUtil.h"
#include "PGTexture.h"
#include "PGUTexture.h"

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Version numbers ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

const char* OpenGLideVersion = "0.13a2";

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

// Structs
GlideStruct Glide;
OpenGLStruct OpenGL;
GlideSettingsImpl UserConfig;
GlideSettingsImpl InternalConfig;
PGTexture* Textures = NULL;
PGUTexture UTextures;
GlideFramebuffer s_Framebuffer;

// Number of Errors
unsigned long NumberOfErrors;

OSErr InitMainVariables(void)
{
	OpenGL.WinOpen = false;
	OpenGL.GlideInit = false;
	NumberOfErrors = 0;
	OSErr err = UserConfig.load();
	if (err == noErr)
	{
		// The following values must be updated right away, because the client application
		// might call grSstQueryHardware() before opening a window
		if ((UserConfig.TextureMemorySize >= OGL_MIN_TEXTURE_BUFFER) && 
		    (UserConfig.TextureMemorySize <= OGL_MAX_TEXTURE_BUFFER))
		{
			UserConfig.TextureMemorySize = UserConfig.TextureMemorySize;
		}

		if ((UserConfig.FrameBufferMemorySize >= OGL_MIN_FRAME_BUFFER) && 
		    (UserConfig.FrameBufferMemorySize <= OGL_MAX_FRAME_BUFFER))
		{
			UserConfig.FrameBufferMemorySize = UserConfig.FrameBufferMemorySize;
		}
	  
		if (UserConfig.BoardType < OpenGLideBoardType_Voodoo)
		{
			UserConfig.BoardType = OpenGLideBoardType_Voodoo;
		}
		else if (UserConfig.BoardType > OpenGLideBoardType_Voodoo2)
		{
			UserConfig.BoardType = OpenGLideBoardType_Voodoo2;
		}

		if (UserConfig.GlideTextureUnits < 1)
		{
			UserConfig.GlideTextureUnits = 1;
		}
		else if (UserConfig.GlideTextureUnits > 3)
		{
			UserConfig.GlideTextureUnits = 3;
		}
		// Detect vector unit (Altivec, SSE, etc.)
		UserConfig.VectorUnitType = GetVectorUnitType();
		if (UserConfig.VectorUnitType > OpenGLideVectorUnitType_None)
		{
			GlideMsg("Using %s vector unit\n", OpenGLideVectorUnitNames[UserConfig.VectorUnitType - 1]);
		}
		// Apply mandatory application dependent settings/patches
		GlideApplication::Type application = s_GlideApplication.GetType();
		switch (application)
		{
		case GlideApplication::Quake:
		  {
				// Quake would not display anything without fixing the memory
		  	unsigned int megabytes = 4;
				if (UserConfig.FrameBufferMemorySize > megabytes)
				{
					UserConfig.FrameBufferMemorySize = megabytes;
					GlideMsg("Decreased frmaebuffer memory size to %d megabytes to make Quake 3Dfx happy\n");
				}
		  }
		  break;
		}
		// Apply optional game specific settings
		if (UserConfig.AutoEnableGameSpecificSettings)
		{
			bool texturesmoothing = false;
			bool subtextures = false;
			bool gapfix = false;
			switch(application)
			{
			case GlideApplication::Carmageddon:
				texturesmoothing = true;
				break;
			case GlideApplication::MythTFL:
				texturesmoothing = true;
				break;
			case GlideApplication::Falcon40:
				texturesmoothing = true;
				break;
			case GlideApplication::TombRaiderI:
				texturesmoothing = true;
				subtextures = true;
				gapfix = true;
				break;
			case GlideApplication::TombRaiderII:
				// subtextures = true; // works but slow
				gapfix = true;
				break;
			default:
				break;
			}
			if (texturesmoothing || subtextures || gapfix)
			{
				GlideMsg("Auto-enabling game specific settings for %s:\n", s_GlideApplication.GetName());
			}
			// Suppress artefacts in main menu
			if (texturesmoothing)
			{
				UserConfig.TextureSmoothing = true;
				GlideMsg("TextureSmoothing enabled\n");
			}
			if (subtextures)
			{
				UserConfig.GenerateSubTextures = 8;	// Good for TR1 and TR2
				GlideMsg("Subtexture generation enabled\n");
			}
			if (gapfix)
			{
				UserConfig.GapFix = static_cast<OpenGLideGapFixFlags>(UserConfig.GapFix | (OpenGLideGapFixFlag_Enabled));
				GlideMsg("GapFix enabled\n");
			}
		}
	}
	if (err != noErr) GlideError("Failed to load user config file: Error code %d", err);
	return err;
}

bool InitWindow(FxU32 hwnd)
{
	if (InitialiseOpenGLWindow(hwnd, 0, 0, OpenGL.WindowWidth, OpenGL.WindowHeight) == false)
	{
		return FXFALSE;
	}
	// Initialise some basic OpenGL() settings
	InitOpenGL();
	// Continue initialising OpenGL with processing
	// the user config and setup OpenGL extensions
	ValidateUserConfig();

	GlideMsg( OGL_LOG_SEPARATE );
	GlideMsg( "Configuration:\n");
	GlideMsg( OGL_LOG_SEPARATE );
	char sEnabled[] = "enabled";
	char sDisabled[] = "disabled";
	const char* boardnames[4] = {"Voodoo", "Voodoo Rush", "AT3D", "Voodoo 2"};
	GlideMsg( "Board type = %s\n", boardnames[InternalConfig.BoardType]);
	GlideMsg( "Number of Texture Units = %d\n", InternalConfig.GlideTextureUnits );
	GlideMsg( "Texture Memory Size = %d Mb\n", InternalConfig.TextureMemorySize );
	GlideMsg( "Frame Buffer Memory Size = %d Mb\n", InternalConfig.FrameBufferMemorySize );
	GlideMsg( "Display method = %d\n", InternalConfig.DisplayMode);
	// display resolution
	GlideMsg("Display resolution = %dx%d\n", OpenGL.WindowWidth, OpenGL.WindowHeight);
	if (InternalConfig.MonitorRefreshRate > 0) 
	{
		GlideMsg( "Monitor Refresh Rate = %d\n", InternalConfig.MonitorRefreshRate );
	}
	GlideMsg("Mipmapping %s\n", InternalConfig.Mipmapping ? sEnabled : sDisabled );
	if (InternalConfig.AnisotropylLevel >= 2 && InternalConfig.EXT_texture_filter_anisotropic)
	{
		GlideMsg("Selected level of anisotropy = %d\n", InternalConfig.AnisotropylLevel);
	}
	else
	{
		GlideMsg("Anisotropic texture filtering disabled\n");
	}
	if (InternalConfig.FullSceneAntiAliasing == 0)
	{
		GlideMsg("FullSceneAntiAliasing disabled\n");
	}
	else
	{
		GlideMsg("FullSceneAntiAliasing enabled, using %d samples\n", InternalConfig.FullSceneAntiAliasing);
	}
	GlideMsg("TextureSmoothing = %s\n", InternalConfig.TextureSmoothing ? sEnabled : sDisabled );
	GlideMsg("Precision Fix = %s\n", InternalConfig.PrecisionFix ? sEnabled : sDisabled );
	bool gapfix_enabled = InternalConfig.GapFix & OpenGLideGapFixFlag_Enabled;
	GlideMsg("GapFix = %s\n", gapfix_enabled ? sEnabled : sDisabled);
	if (gapfix_enabled)
	{
		GlideMsg(" Debug = %s\n", InternalConfig.GapFix & OpenGLideGapFixFlag_Debug ? sEnabled : sDisabled);
		GlideMsg("              IncircleFilterOr = %s\n", InternalConfig.GapFix & OpenGLideGapFixFlag_IncircleOr ? sEnabled : sDisabled);
		GlideMsg("             IncircleFilterAnd = %s\n", InternalConfig.GapFix & OpenGLideGapFixFlag_IncircleAnd ? sEnabled : sDisabled);
		GlideMsg("    IncircleFilterSecondRadius = %s\n", InternalConfig.GapFix & OpenGLideGapFixFlag_IncircleSecondRadius ? sEnabled : sDisabled);
		GlideMsg("VertexLengthFilterSecondRadius = %s\n", InternalConfig.GapFix & OpenGLideGapFixFlag_VertexLengthSecondRadius ? sEnabled : sDisabled);
		GlideMsg("                1st radius = %g\n", InternalConfig.GapFixParam1);
		if (InternalConfig.GapFix & OpenGLideGapFixFlag_VertexLengthSecondRadius)
		{
			GlideMsg("             Vertex Length = %g\n", InternalConfig.GapFixParam2);	
		}
		else
		{
			GlideMsg("     triangle isoscelesness= %g\n", InternalConfig.GapFixParam2);
		}
		if ((InternalConfig.GapFix & OpenGLideGapFixFlag_IncircleSecondRadius) || (InternalConfig.GapFix & OpenGLideGapFixFlag_VertexLengthSecondRadius))
		{
			GlideMsg("                2nd radius = %g\n", InternalConfig.GapFixParam3);	
		}
		if (InternalConfig.GapFix & OpenGLideGapFixFlag_DepthFactor)
		{
			GlideMsg("              Depth Factor = %g\n", InternalConfig.GapFixDepthFactor);	
		}
	}
	GlideMsg("Generate subtextures = %s", InternalConfig.GenerateSubTextures ? sEnabled : sDisabled );
	if (InternalConfig.GenerateSubTextures) GlideMsg(" (gridsize = %d)", InternalConfig.GenerateSubTextures);
	GlideMsg("\n");
	GlideMsg("Framebuffer overlays = %s\n", InternalConfig.EnableFrameBufferOverlays ? sEnabled : sDisabled );
	GlideMsg("Framebuffer underlays = %s\n", InternalConfig.EnableFrameBufferUnderlays ? sEnabled : sDisabled );
	if (InternalConfig.FramebufferIgnoreUnlock) GlideMsg( "Ignore buffer unlocks = %s\n", InternalConfig.FramebufferIgnoreUnlock ? sEnabled : sDisabled);
	if (InternalConfig.PedanticFrameBufferEmulation) GlideMsg( "Pedantic Framebuffer emulation = %s\n", InternalConfig.PedanticFrameBufferEmulation ? sEnabled : sDisabled);
	GlideMsg(OGL_LOG_SEPARATE);

#ifdef OGL_DEBUG
	GlideMsg("GlideState size = %d\n", sizeof(GlideState));
	GlideMsg("GrState size = %d\n", sizeof(GrState));
	GlideMsg(OGL_LOG_SEPARATE);
#endif
	GlideMsg("** Glide Calls **\n" );
	GlideMsg(OGL_LOG_SEPARATE);

	return true;
}

void* AllocFrameBuffer(long buffersize, long buffertypesize)
{
	void* buffer = AllocSysPtr16ByteAligned(buffersize * buffertypesize);
	if (buffer == NULL)
	{
		GlideError("Could NOT allocate sufficient memory for framebuffer... Sorry.");
		exit( -1 );
	}
	return buffer;
}

void FreeFrameBuffer(void* buffer)
{
	Free16ByteAligned(buffer);
}

void* AllocBuffer(long buffersize, long buffertypesize)
{
	void* buffer = AllocSysPtr16ByteAligned(buffersize * buffertypesize);
	if (buffer == NULL)
	{
		GlideError("Could NOT allocate sufficient memory for buffer... Sorry.");
		exit( -1 );
	}
	return buffer;
}

void FreeBuffer(void* buffer)
{
	Free16ByteAligned(buffer);
} 


void* AllocObject(long buffersize)
{
	// Causes classic to crash
	void* buffer = AllocSysPtr16ByteAligned(buffersize);
	if (buffer == NULL)
	{
		GlideError("Could NOT allocate sufficient memory for object... Sorry.");
		exit( -1 );
	}
	return buffer;
}

void FreeObject(void* buffer)
{
	Free16ByteAligned(buffer);
}

// error handling
#ifdef OPENGL_DEBUG
OSStatus glReportError_impl (const char* __glide_functionname)
{
	char* errortext = NULL;
	GLenum err = glGetError();
	switch (err)
	{
		case GL_NO_ERROR:
			break;
		case GL_INVALID_ENUM:
			errortext = "Invalid enumeration";
			break;
		case GL_INVALID_VALUE:
			errortext ="Invalid value";
			break;
		case GL_INVALID_OPERATION:
			errortext = "Invalid operation";
			break;
		case GL_STACK_OVERFLOW:
			errortext = "Stack overflow";
			break;
		case GL_STACK_UNDERFLOW:
			errortext = "Stack underflow";
			break;
		case GL_OUT_OF_MEMORY:
			errortext = "Out of memory";
			break;
		default:
			errortext = "Unknown error code";
			break;
	}
	if (errortext)
	{
		GlideMsg ("Function %s, GL Error: %s\n", __glide_functionname, errortext);	
	}
	// ensure we are returning an OSStatus noErr if no error condition
	if (err == GL_NO_ERROR)
		return noErr;
	else
		return (OSStatus) err;
}
#endif

#if defined(OPTIMISE_OPENGL_STATE_CHANGES) || defined(OGL_DEBUG) || defined(OPENGL_DEBUG)
bool VerifyActiveTextureUnit_impl(GLint x, const char* functionname)
{
	glReportErrors("VerifyActiveTextureUnit_impl");
	GLint y;
	glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &y);
	glReportError();
	bool bVerified = x == y;
	if (!bVerified)
	{
		GlideMsg("Warning: %s() active texture unit is GL_TEXTURE%d_ARB, but should be GL_TEXTURE%d_ARB\n", functionname, y - GL_TEXTURE0_ARB, x - GL_TEXTURE0_ARB);
	}
	glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE_ARB, &y);
	glReportError();
	bVerified = x == y;
	if (!bVerified)
	{
		GlideMsg("Warning: %s() client active texture unit is GL_TEXTURE%d_ARB, but should be GL_TEXTURE%d_ARB\n", functionname, y - GL_TEXTURE0_ARB, x - GL_TEXTURE0_ARB);
	}
	return bVerified;
}
#endif

#if defined(OPTIMISE_OPENGL_STATE_CHANGES) || defined(OGL_DEBUG) || defined(OPENGL_DEBUG)
bool VerifyTextureEnabledState_impl(const char* functionname)
{
	glReportErrors("VerifyActiveTextureUnit_impl");

	// emember state
	GLint current;
	glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &current);
	glReportError();
	bool bVerified = true;
	if (OpenGL.ColorAlphaUnit2)
	{
		for(long unit_index = 1; unit_index >= 0; unit_index--)
		{
			glActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
			bool bState = glIsEnabled(GL_TEXTURE_2D);
			if (bState != (OpenGL.ColorAlphaUnitColorEnabledState[unit_index] ||
			               OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index]))
			{
				GlideMsg("Warning: texture unit GL_TEXTURE%d_ARB is %s in %s()\n", OpenGL.ColorAlphaUnit1 + unit_index - GL_TEXTURE0_ARB, bState ? "enabled" : "disabled", functionname);
				bVerified = false;
			}
			bState = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
			if (bState != (OpenGL.ColorAlphaUnitColorEnabledState[unit_index] ||
			               OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index]))
			{
				GlideMsg("Warning: texcoord array for unit GL_TEXTURE%d_ARB is %s in %s()\n", OpenGL.ColorAlphaUnit1 + unit_index - GL_TEXTURE0_ARB, bState ? "enabled" : "disabled", functionname);
				bVerified = false;
			}
			glReportError();
		}
	}
	// Restore previous state
	glActiveTextureARB(current);
	return bVerified;
}
#endif

//*************************************************
//* Initializes OpenGL
//*************************************************
void InitOpenGL( void )
{
	OpenGL.ZNear = ZBUFFERNEAR;
	OpenGL.ZFar = ZBUFFERFAR;
	OpenGL.ClipMinX = 0;
	OpenGL.ClipMinY = 0;
	OpenGL.ClipMaxX = OpenGL.WindowWidth;
	OpenGL.ClipMaxY = OpenGL.WindowHeight;
	// Glide texture data is aligned at 8 byte boundaries
	int alignment = 8;
	glPixelStorei( GL_PACK_ALIGNMENT, alignment);
	glPixelStorei( GL_UNPACK_ALIGNMENT, alignment);
}

void GlideMsg(const char* message, ... )
{
	va_list(args);
	va_start(args, message);

	char buffer[StringBufferSize];

	vsnprintf(buffer, StringBufferSize, message, args);
	UserConfig.write_log(buffer);
	va_end(args);
}

void GlideError(const char *message, ... )
{
	va_list(args);
	va_start(args, message);

	char buffer[StringBufferSize];

	NumberOfErrors++;
	vsnprintf(buffer, StringBufferSize, message, args);
	GlideMsg(buffer);

	va_end(args);
    
	grGlideShutdown();
	if (UserConfig.DisplayMode == OpenGLideDisplayMode_DisplayManager)
	{
		DisplayManager_RestoreDesktopDisplay();
	}
	FatalErrorMessageBox(buffer);
	exit(0);
}


bool ClearAndGenerateLogFile( void )
{
	UserConfig.create_log();
	char buffer[32];
	GlideMsg(OGL_LOG_SEPARATE );
	GlideMsg("%s Log File\n", OpenGLideProductName);
	GlideMsg(OGL_LOG_SEPARATE );
	GlideMsg("***** %s %s *****\n", OpenGLideProductName, OpenGLideVersion);
	GlideMsg(OGL_LOG_SEPARATE );
	_strdate(buffer);
	GlideMsg("Date: %s\n", buffer);
	_strtime(buffer);
	GlideMsg("Time: %s\n", buffer);
	GlideMsg("Application: %s\n", s_GlideApplication.GetName());
	return true;
}

void CloseLogFile( void )
{
	char tmpbuf[ 128 ];
	GlideMsg( OGL_LOG_SEPARATE );
	_strtime( tmpbuf );
	GlideMsg( "Time: %s\n", tmpbuf );
	GlideMsg( OGL_LOG_SEPARATE );
}

