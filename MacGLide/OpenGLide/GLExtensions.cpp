//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                    OpenGL Extensions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Glide.h"
#include "GlideSettings.h"
#include "GLColorAlphaCombineEnvTables.h"
#include "GLextensions.h"
#include "GLRender.h"
#include "GLutil.h"

#include <agl.h>


enum enExtensionType
{
    OGL_EXT_UNUSED = 0,
    OGL_EXT_REQUIRED,
    OGL_EXT_DESIRED
};

struct stExtensionSupport
{
    char *          name;
    enExtensionType type;
    bool *          userVar;
    bool *          internalVar;
};

// Running in Classic ?
#ifdef OPENGLIDE_HOST_MAC
bool RunningInClassic = false;
#endif

// It is important that dummyExtVariable retains the value true, so
// we pass dummyExtVariable2 in places where the value may be altered.
bool dummyExtVariable = true;
bool dummyExtVariable2 = true;

// Env Combine fog data
static GLuint fogtexturename = 0;
static FxU8* fogalpharamp = NULL;
static GLint dummytexture = 0x00000000;

stExtensionSupport glNecessaryExt[] =
{
		// new
    { "GL_EXT_abgr",                      OGL_EXT_REQUIRED,   &dummyExtVariable,                    &dummyExtVariable2 },
    { "GL_EXT_bgra",                      OGL_EXT_REQUIRED,   &dummyExtVariable,                    &dummyExtVariable2 },
    { "GL_EXT_secondary_color",           OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.EXT_secondary_color },
    { "GL_ARB_multitexture",              OGL_EXT_DESIRED,    &UserConfig.ARB_multitexture,         &InternalConfig.ARB_multitexture },
    { "GL_ARB_texture_env_add",           OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.EXT_texture_env_add },
    { "GL_ARB_texture_env_combine",       OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.EXT_texture_env_combine },
    { "GL_ATI_texture_env_combine3",      OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.ATI_texture_env_combine3 },
    { "GL_EXT_texture_lod_bias",          OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.EXT_texture_lod_bias },
    { "GL_SGIS_generate_mipmap",          OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.EXT_SGIS_generate_mipmap },
    { "GL_SGIS_texture_edge_clamp",       OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.EXT_SGIS_texture_edge_clamp },
    { "GL_EXT_paletted_texture",          OGL_EXT_DESIRED,    &UserConfig.EXT_paletted_texture,     &InternalConfig.EXT_paletted_texture },
    { "GL_APPLE_packed_pixels",           OGL_EXT_REQUIRED,   &dummyExtVariable,                    &dummyExtVariable2 },
    { "GL_APPLE_client_storage",          OGL_EXT_DESIRED,    &UserConfig.APPLE_client_storage,       &InternalConfig.APPLE_client_storage },
    { "GL_EXT_compiled_vertex_array",     OGL_EXT_DESIRED,    &UserConfig.EXT_compiled_vertex_array,&InternalConfig.EXT_compiled_vertex_array },
    { "GL_ARB_texture_rectangle",         OGL_EXT_DESIRED,    &UserConfig.ARB_texture_rectangle,    &InternalConfig.ARB_texture_rectangle },
#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
    { "GL_EXT_fog_coord",                 OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.EXT_fog_coord },
#endif
#ifdef OPENGLIDE_SYSTEM_HAS_BLENDFUNC_SEPERATE
    { "GL_EXT_blend_func_separate",       OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.EXT_blend_func_separate },
#endif
    { "GL_EXT_texture_filter_anisotropic",OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.EXT_texture_filter_anisotropic },
    { "GL_ARB_multisample",               OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.ARB_multisample },
    { "GL_NV_multisample_filter_hint",    OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.NV_multisample_filter_hint },
    { "GL_EXT_clip_volume_hint",          OGL_EXT_DESIRED,    &UserConfig.EXT_clip_volume_hint,     &InternalConfig.EXT_clip_volume_hint },
    { "GL_APPLE_transform_hint",          OGL_EXT_DESIRED,    &dummyExtVariable,                    &InternalConfig.APPLE_transform_hint },
    { "",                                 OGL_EXT_UNUSED,     &dummyExtVariable,                    &dummyExtVariable2 }
}; 

// check to see if Extension is Supported
// code by Mark J. Kilgard of NVidia modified by Fabio Barros
bool OGLIsExtensionSupported(const char* extensions, const char* extension)
{
	char* where = (char *) strchr( extension, ' ' );
	if ( where || ( *extension == '\0' ) )
	{
		return false;
	}
	const char* start = extensions;
	if ( *start == '\0' )
	{
		GlideError( "No OpenGL extension supported, using all emulated.\n" );
		return false;
	}
	
	while ( true )
	{
		where = (char *)strstr( start, extension );
		if ( !where )
		{
			break;
		}
		const char* terminator = where + strlen( extension );
		if ( ( where == start ) || ( *( where - 1 ) == ' ' ) )
		{
		    if ( ( *terminator == ' ' ) || ( *terminator == '\0' ) )
			{
				return true;
			}
		}
		start = terminator;
	}
	
	// fix for Rage 128 OpenGL Engine 1.1.ATI-5.99
	// (See apple techote TN2014)
	if (strcmp(extension, "GL_APPLE_packed_pixels") == 0)
	{
		return OGLIsExtensionSupported(extensions, "GL_APPLE_packed_pixel");
	}
	// fix for Rage 128 OpenGL Engine 1.1.ATI-5.99
	// support for GL_EXT_texture_env_combine
	else if (strcmp(extension, "GL_ARB_texture_env_combine") == 0)
	{
		return OGLIsExtensionSupported(extensions, "GL_EXT_texture_env_combine");	
	}
	// also support the older GL_EXT_texture_env_add
	else if (strcmp(extension, "GL_ARB_texture_env_add") == 0)
	{
		return OGLIsExtensionSupported(extensions, "GL_EXT_texture_env_add");	
	}
	else
	{
		return false;
	}
}

void ValidateUserConfig( void )
{
	glReportErrors("ValidateUserConfig");
	
	// Copy config
	InternalConfig = UserConfig;

	GlideMsg( OGL_LOG_SEPARATE );
	GlideMsg( "** OpenGL Information **\n" );
	const unsigned char* renderer = glGetString( GL_RENDERER );
	if (renderer == NULL || strlen(reinterpret_cast<const char*>(renderer)) == 0)
	{
		GlideError("The OpenGL driver failed to report its version/vendor/renderer.\nThis may be caused by beta drivers located in thge game directory.\nThese drivers should be deleted.\n");
	}
	GlideMsg( OGL_LOG_SEPARATE );
	GlideMsg( "Vendor:      %s\n", glGetString( GL_VENDOR ) );
	GlideMsg( "Renderer:    %s\n", glGetString( GL_RENDERER ) );
	GlideMsg( "Version:     %s\n", glGetString( GL_VERSION ) );
	// Extension string is too long for the temp buffer, so we don't use GlideMsg()
	UserConfig.write_log("Available Extensions: ");
	const char* extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
	UserConfig.write_log(extensions);
	UserConfig.write_log("\n");

	GlideMsg( OGL_LOG_SEPARATE );
	GlideMsg( "OpenGL Extensions:\n" );
	GlideMsg( OGL_LOG_SEPARATE );
	int index = 0;
	while ( strlen( glNecessaryExt[ index ].name ) > 0 )
	{
		*glNecessaryExt[ index ].internalVar = false;
		switch ( glNecessaryExt[ index ].type )
		{
		case OGL_EXT_REQUIRED:
			if (!OGLIsExtensionSupported(extensions, glNecessaryExt[index].name ))
			{
				char buffer[StringBufferSize];
				sprintf(buffer, "Severe Problem: OpenGL %s extension is required for %s!", 
				        glNecessaryExt[ index ].name, OpenGLideProductName);
				GlideError(buffer);
			}
			break;
		case OGL_EXT_DESIRED:
			if (!OGLIsExtensionSupported(extensions, glNecessaryExt[index].name))
			{
				char buffer[StringBufferSize];
				sprintf(buffer, "Note: OpenGL %s extension is not supported, emulating behavior.\n", 
				        glNecessaryExt[ index ].name );
				GlideMsg(buffer);
			}
			else
			{
				if ( *glNecessaryExt[ index ].userVar )
				{
					*glNecessaryExt[ index ].internalVar = true;
					GlideMsg( "Extension %s is present and ENABLED\n", glNecessaryExt[ index ].name );
				}
				else
				{
					char buffer[StringBufferSize];
					sprintf(buffer, "Note: OpenGL %s extension is supported but disabled by user\n", 
					        glNecessaryExt[ index ].name );
					GlideMsg(buffer);
				}
			}
			break;
		case OGL_EXT_UNUSED:
			break;
		}
		++index;
	}
	GlideMsg( OGL_LOG_SEPARATE );
	GLExtensions( );
}

void GLExtensions(void)
{
	glReportErrors("GLExtensions");
	const float one = 1.0f;
	
 	#ifdef OPENGLIDE_HOST_MAC
		RunningInClassic = strstr(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)), "GL_EXT_fog_coord") != NULL;
	#endif

	GlideMsg("OpenGL configuration:\n");
	GlideMsg(OGL_LOG_SEPARATE);

 	// query the actual buffer size
 	GLint size;
	glGetIntegerv(GL_DEPTH_BITS, &size);
	glReportError();
	GlideMsg("Depthbuffer size = %d\n", size);
 	// Rendering quality seems to be better when this is left enabled for depth buffer size < 24 bits only
	// (Try spinning around at the portal in the first level of Tomb Raider Gold)
	const int disablePrecisionFixAtDepthSize = 24;
	if (size >= disablePrecisionFixAtDepthSize && InternalConfig.PrecisionFix == 1)
	{
		GlideMsg("Actual number of depthbuffer bits is >= %d - precision fix can safely be disabled\n", disablePrecisionFixAtDepthSize, size);
		InternalConfig.PrecisionFix = 0;
	}
	if (UserConfig.GapFix & OpenGLideGapFixFlag_Enabled)
	{
		glGetIntegerv(GL_STENCIL_BITS, &size);
		GlideMsg("Stencilbuffer size = %d\n", size);
	}
 	GLint MaxAnisotropyLevel;
	glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAnisotropyLevel);
	GlideMsg("Maximum level of anisotropy = %d\n", MaxAnisotropyLevel);

	// Since this a global setting, texture data must not be uploaded from temp buffers
	if (InternalConfig.APPLE_client_storage)
	{
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, true);
		glReportError();
	}

	if (InternalConfig.EXT_texture_filter_anisotropic)
	{
 		GLint MaxAnisotropyLevel;
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAnisotropyLevel);
		if (UserConfig.AnisotropylLevel > MaxAnisotropyLevel)
		{
			InternalConfig.AnisotropylLevel = MaxAnisotropyLevel;
		}
		else
		{
			InternalConfig.AnisotropylLevel = UserConfig.AnisotropylLevel;
		}
	}
	else
	{
		InternalConfig.AnisotropylLevel = 1;
	}
	
	if (InternalConfig.ARB_multisample == false)
	{
		InternalConfig.FullSceneAntiAliasing = 0;
	}
	else if (InternalConfig.FullSceneAntiAliasing > 0)
	{
		// Thanks to Frank Condello <developerNOSPAM@NOSPAMchaoticbox.com> for posting this trick at
		// http://support.realsoftware.com/listarchives/realbasic-games/2004-09/msg00273.html
		aglSetInteger(pWindowInfo->aglContext, AGL_ATI_FSAA_LEVEL, reinterpret_cast<const long*>(&InternalConfig.FullSceneAntiAliasing));
		GLenum err = aglGetError();
		if (AGL_NO_ERROR != err)
		{
			GlideError("Couldn't set ATI FSAA level: %s\n", reinterpret_cast<const char *>(aglErrorString(err)));
		}
		// Initially enable multisampling
		glEnable(GL_MULTISAMPLE_ARB);
		glReportError();
	}
	if (InternalConfig.FullSceneAntiAliasing > 0
	 &&	InternalConfig.NV_multisample_filter_hint)
	{
		glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
		glReportError();
	}
	
#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
	if (InternalConfig.EXT_fog_coord == false && InternalConfig.FogMode > OpenGLideFogEmulation_EnvCombine)
	{
		InternalConfig.FogMode = OpenGLideFogEmulation_EnvCombine;
	}
#endif

	// FogCoord extension not supported on MacOS9
#ifdef OPENGLIDE_SYSTEM_DOESN_T_HAVE_FOGCOORD
	if (InternalConfig.FogMode > OpenGLideFogEmulation_EnvCombine)
	{
		InternalConfig.FogMode = OpenGLideFogEmulation_EnvCombine;
	}
#endif

	// check for other extensions needed by fog emulation
	if (InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine
	 && InternalConfig.EXT_texture_env_combine == false)
	{
		GlideMsg( "Enhanced fog emulation disabled because GL_ARB_texture_env_combine is missing or disabled\n");
		InternalConfig.FogMode = OpenGLideFogEmulation_Simple;
	}
	else if (InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine
	 && InternalConfig.ARB_multitexture == false)
	{
		GlideMsg( "Enhanced fog emulation disabled because GL_ARB_multitexture is missing or disabled\n");
		InternalConfig.FogMode = OpenGLideFogEmulation_Simple;
	}
	
	// Setup texture units
	if (InternalConfig.GlideTextureUnits > 1)
	{
		GlideMsg( "Only one texure unit is currently supported. Additional units are ignored.\n");
	}
	if (InternalConfig.ARB_multitexture)
	{
		GLint number_of_texture_units = 1;
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &number_of_texture_units);
		GlideMsg("MultiTexture Units = %x\n", number_of_texture_units);
		if (InternalConfig.ColorAlphaRenderMode >= OpenGLideColorAlphaRenderMode_Unknown)
		{
			InternalConfig.ColorAlphaRenderMode = OpenGLideColorAlphaRenderMode_Automatic;
		}
		if (InternalConfig.ColorAlphaRenderMode > OpenGLideColorAlphaRenderMode_Simple
		 && InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine
		 && number_of_texture_units <= 2)
		{
	    GlideMsg( "Coloralpha render mode reset to simple in favour of enhanced fog emulation\n");
			InternalConfig.ColorAlphaRenderMode = OpenGLideColorAlphaRenderMode_Simple;
		}
		// Setup coloralpha texture units
		OpenGL.ColorAlphaUnit1 = GL_TEXTURE0_ARB;
		if ((number_of_texture_units > 2 && InternalConfig.EXT_texture_env_combine
			|| number_of_texture_units == 2 && InternalConfig.FogMode != OpenGLideFogEmulation_EnvCombine)
			&& (InternalConfig.ColorAlphaRenderMode == OpenGLideColorAlphaRenderMode_Automatic
			 || InternalConfig.ColorAlphaRenderMode > OpenGLideColorAlphaRenderMode_Simple))
		{
			OpenGL.ColorAlphaUnit2 = GL_TEXTURE1_ARB;
			OpenGL.FogTextureUnit = GL_TEXTURE2_ARB;
			// All texture units are enabled all the time
			glActiveTextureARB(OpenGL.ColorAlphaUnit1);
			glEnable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glActiveTextureARB(OpenGL.ColorAlphaUnit2);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glReportError();
			// Secondary color not neded in the enhanced color alpha rendering model
			InternalConfig.EXT_secondary_color = false;
			if (InternalConfig.ColorAlphaRenderMode == OpenGLideColorAlphaRenderMode_Automatic)
			{
				if (InternalConfig.ATI_texture_env_combine3)
				{
					// With GL_ATI_texture_env_combine3, more combine functions can be modeled as
					// exactly as the original combine functions. They can also be rendered more
					// often with one texture unit, which leaves more space for correct rendering
					// of textures that use both chromakeying and alpha blending together.
					InternalConfig.ColorAlphaRenderMode = OpenGLideColorAlphaRenderMode_EnvCombine3_ATI;
					OpenGL.ColorCombineFunctions = ColorCombineFunctionsEnvCombine3ATI;
					OpenGL.AlphaCombineFunctions = AlphaCombineFunctionsEnvCombine3ATI;
				}
				else if (InternalConfig.EXT_texture_env_combine)
				{
					// Standard setup, works with most graphics cards and provides accurate
					// results in most cases
					InternalConfig.ColorAlphaRenderMode = OpenGLideColorAlphaRenderMode_EnvCombine_ARB;
					OpenGL.ColorCombineFunctions = ColorCombineFunctionsEnvCombineARB;
					OpenGL.AlphaCombineFunctions = AlphaCombineFunctionsEnvCombineARB;
				}
			}
			if (InternalConfig.ColorAlphaRenderMode == OpenGLideColorAlphaRenderMode_EnvCombine3_ATI)
			{
				OpenGL.ColorCombineFunctions = ColorCombineFunctionsEnvCombine3ATI;
				OpenGL.AlphaCombineFunctions = AlphaCombineFunctionsEnvCombine3ATI;
			}
			else if (InternalConfig.ColorAlphaRenderMode == OpenGLideColorAlphaRenderMode_EnvCombine_ARB)
			{
				OpenGL.ColorCombineFunctions = ColorCombineFunctionsEnvCombineARB;
				OpenGL.AlphaCombineFunctions = AlphaCombineFunctionsEnvCombineARB;
			}
		}
		else
		{
			OpenGL.ColorAlphaUnit2 = 0;
			OpenGL.FogTextureUnit = number_of_texture_units > 2 ? GL_TEXTURE2_ARB : GL_TEXTURE1_ARB;
			InternalConfig.ColorAlphaRenderMode = OpenGLideColorAlphaRenderMode_Simple;
		}
	}
	else
	{
		OpenGL.ColorAlphaUnit1 = GL_TEXTURE0_ARB;
		OpenGL.ColorAlphaUnit2 = 0;
		OpenGL.FogTextureUnit = 0;
		InternalConfig.ColorAlphaRenderMode = OpenGLideColorAlphaRenderMode_Simple;
	}
	
	// Print out coloralpha render mode
	char* coloralpharendermodename;
	switch (InternalConfig.ColorAlphaRenderMode)
	{
	case OpenGLideColorAlphaRenderMode_Simple:
		coloralpharendermodename = "MultiTextureSimple";
		break;
	case OpenGLideColorAlphaRenderMode_EnvCombine_ARB:
		coloralpharendermodename = "EnvironmentCombineARB";
		break;
	case OpenGLideColorAlphaRenderMode_EnvCombine3_ATI:
		coloralpharendermodename = "EnvironmentCombine3ATI";
		break;
	default:
		assert(false);
		break;
	}
	GlideMsg( "Coloralpha rendermode = %s\n", coloralpharendermodename);

	if (OpenGL.FogTextureUnit == 0 && InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine)
	{
		InternalConfig.FogMode = OpenGLideFogEmulation_Simple;
	}

	if (InternalConfig.EXT_secondary_color && OpenGL.ColorAlphaUnit2 == 0)
	{
		glEnable(GL_COLOR_SUM_EXT);
		glReportError();
	}

	if (InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine)
	{
		fogalpharamp = static_cast<unsigned char*> (AllocBuffer(256, 1));
		if (fogalpharamp)
		{
			for(unsigned int i = 0; i < 256; i++)
			{
				fogalpharamp[i] = 255 - i;
			}
		}
		else
		{
			InternalConfig.FogMode = OpenGLideFogEmulation_Simple;
		}
	}

	if (OpenGL.ColorAlphaUnit2 || InternalConfig.FogMode != OpenGLideFogEmulation_EnvCombine)
	{
		glGenTextures(1, &OpenGL.DummyTextureName);
		glPrioritizeTextures(1, &OpenGL.DummyTextureName, &one);
		glActiveTextureARB(OpenGL.ColorAlphaUnit1);
		glBindTexture(GL_TEXTURE_2D, OpenGL.DummyTextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, InternalConfig.EXT_SGIS_texture_edge_clamp ? GL_CLAMP_TO_EDGE : GL_CLAMP); // GL_REPEAT would cause randomly
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, InternalConfig.EXT_SGIS_texture_edge_clamp ? GL_CLAMP_TO_EDGE : GL_CLAMP); // fogged triangles in the foreground
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (InternalConfig.AnisotropylLevel >= 2)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
		}
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &dummytexture);
		glReportError();
	}
	
#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
	if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
	{
		glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
		glFogf(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, 0.0f);
		glFogf(GL_FOG_END, 1.0f);
		glReportError();
	}
	else
#endif
	if (InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine)
	{
		GrFog_t fogtable[GR_FOG_TABLE_SIZE];
		guFogGenerateLinear(fogtable, 0.0f, 1.0f);
		grFogTable(fogtable);
		GlideMsg( "Using env-combine fog emulation\n");
		// Setup fog texture unit
		glActiveTextureARB(OpenGL.FogTextureUnit);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		// The color part is used for the fog function (see RenderUpdateState.cpp)
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_INTERPOLATE_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
		// The alpha is not blended with the fog alpha because this
		// would also fog otherwise invisible parts of the texture
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
		glGenTextures(1, &fogtexturename);
		glPrioritizeTextures(1, &fogtexturename, &one);
		glBindTexture(GL_TEXTURE_2D, fogtexturename);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, InternalConfig.EXT_SGIS_texture_edge_clamp ? GL_CLAMP_TO_EDGE : GL_CLAMP); // GL_REPEAT would cause randomly
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, InternalConfig.EXT_SGIS_texture_edge_clamp ? GL_CLAMP_TO_EDGE : GL_CLAMP); // fogged triangles in the foreground
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (InternalConfig.AnisotropylLevel >= 2)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, fogalpharamp);
		glReportError();  
	}
	else if (InternalConfig.FogMode == OpenGLideFogEmulation_Simple)
	{
		// Provide a reasonable default for application that use fog,
		// but don't set the fog table (like Carmageddon)
		// Adjusted to camageddon Main Street Track (compared with Race Track Preview image)
		// and to Coastal Carnage Track so that objects become colored right after they pop up
		// when popup distance option is far (fog is set to black in this level)

		// This doesn't seem to work with MacOS9 OpenGL drivers (beasue of version < 1.3 ?).
		// The whole scene is too dark. However, it works well in the Classic environment
		// (which actually uses the OpenGL driver provided by MacOS X)
			
		// When the fog table is not set as below, the whole scene seems to be a little to dark,
		// whereas when it's set, the foreground looks as brighht as in the software rendrerer.

		GrFog_t fogtable[GR_FOG_TABLE_SIZE];
		// The values below come from comparing screenshots between GeForce 2mx
		// (MacOS X OpenGL 1.3 driver) and the software renderer version of Carmageddon
		guFogGenerateLinear(fogtable, 0.9985f, 1.0f);
		grFogTable(fogtable);
		GlideMsg( "Using simple fog emulation\n");
	}
	else
	{
		InternalConfig.FogMode = OpenGLideFogEmulation_None;
		// These default values must be provied even if fog is
		// disabled or the shadows in Carmageddon Splatpack,
		// Meltdown Alley level will have the fog color
		// (occures on MacOS 9, Nvidia OpenGL 1.22)
		glFogf( GL_FOG_MODE, GL_LINEAR );
		glReportError();
		glFogf( GL_FOG_START, 1.0f );
		glReportError();
		glFogf( GL_FOG_END, 0.0f );
		glReportError();
		glDisable(GL_FOG);
		GlideMsg( "Fog emulation disabled\n");
	}
	
	if (InternalConfig.FogMode != OpenGLideFogEmulation_EnvCombine && OpenGL.DummyTextureName && OpenGL.FogTextureUnit)
	{
		// Setup fog texture unit as inverter (for inverting the color alpha output)
		glActiveTextureARB(OpenGL.FogTextureUnit);
		glBindTexture(GL_TEXTURE_2D, OpenGL.DummyTextureName);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
		glReportError();
		OpenGL.FogTextureUnitEnabledState = true;
	}
	else
	{
		OpenGL.FogTextureUnitEnabledState = false;
	}
	// End of texture unit setup
	glActiveTextureARB(OpenGL.ColorAlphaUnit1);

	if (InternalConfig.EXT_paletted_texture)
	{
		GlideMsg( "Using Palette Extension\n" );
	}
	
#ifdef OPENGLIDE_HOST_MAC
	if (InternalConfig.EXT_compiled_vertex_array)
	{
		// Compiled vertex array crashes on MacOS9 (at least on my system)
		// so if anybody who would like to investigate this...
		if (!RunningInClassic)
		{
			InternalConfig.EXT_compiled_vertex_array = false;
			GlideMsg( "Compiled vertex arrays have been disabled (works in Classic only)\n" );    
		}
	}
#endif

	if (InternalConfig.TextureSmoothing)
	{
		OpenGL.MinFilterMode = InternalConfig.Mipmapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
		OpenGL.MagFilterMode = GL_LINEAR;
	}

	if (InternalConfig.EXT_clip_volume_hint)
	{
		glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
		OpenGL.ClipVerticesEnabledState = false;
	}

	if (InternalConfig.APPLE_transform_hint)
	{
		glHint(GL_TRANSFORM_HINT_APPLE, GL_NICEST);
	}

	if (InternalConfig.EXT_SGIS_generate_mipmap)
	{
		glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
	}

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}

void GLExtensionsCleanup()
{
	// cleanup fog texture
	if (InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine)
	{
		if (fogalpharamp)
		{
			FreeBuffer(fogalpharamp);
			glReportErrors("RenderFree");
			glDeleteTextures(1, &fogtexturename);
			glReportError();
		}
	}
	if (OpenGL.DummyTextureName)
	{
		glDeleteTextures(1, &OpenGL.DummyTextureName);
	}
}
