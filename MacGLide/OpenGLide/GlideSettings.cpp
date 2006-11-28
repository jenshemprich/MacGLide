//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                   OpenGLide Settings File
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "GlOgl.h"
#include "GlideSettings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Version numbers ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

const char* GlideSettings::OpenGLidePreferencesVersion = "0.13";

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

GlideSettings::GlideSettings()
: m_FileBuffer(NULL)
, m_FileBufferSize(0)
{
	defaults();
}

GlideSettings::~GlideSettings(void)
{
}

bool GlideSettings::get(const char* setting, const char** value)
{
	static char buffer[StringBufferSize];
	buffer[0] = 0x0;
	bool found = false;
	const char* v = m_FileBuffer;
	while(!found && v)
	{
		v = strstr(v, setting);
		if (v)
		{
			int s = strlen(setting);
			// At the beginning of the line or tabbed and followed by '='
			if (v[-1] < 0x20 && v[s] == '=')
			{
				// found, point to begin of setting
				v = v + s + 1;
				// copy til end of line
				sscanf(v, "%255s", buffer);
				found = true;
			}
		}
	}
	*value = buffer;
	if (!found)
	{
		GlideMsg("Setting '%s' not found, using default value\n", setting);
	}
	return found;
}

bool GlideSettings::get(const char* setting, unsigned long* value)
{
	const char* v;
	if (get(setting, &v))
	{
		*value = atol(v);
		return true;
	}
	else
	{
		// keep default value
		return false;
	}
}

bool GlideSettings::get(const char* setting, float* value)
{
	const char* v;
	if (get(setting, &v))
	{
		*value = atof(v);
		return true;
	}
	else
	{
		// keep default value
		return false;
	}
}

bool GlideSettings::get(const char* setting, bool* value)
{
	const char* v;
	if (get(setting, &v))
	{
		*value = atol(v) != 0;
		return true;
	}
	else
	{
		// keep default value
		return false;
	}
}

void GlideSettings::defaults()
{
#ifdef OGL_DEBUG
	GlideMsg("Initialising to internal default settings...\n");
#endif
	// These are the default values that will end up in a new configuration file.
	// OpenGL extensions that can be disabled via the config file must be set to
	// true in order to become enabled in ValidateUserConfig()
	Resolution                     = 1;
	MonitorRefreshRate             = 0;
	DepthBufferBits                = 0;
	FullSceneAntiAliasing          = 0;
	GammaBias                      = -0.3f;
	TextureSmoothing               = false;
	DisplayMode = OpenGLideDisplayMode_DisplayManager;
	#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
		FogMode                      = OpenGLideFogEmulation_FogCoord;
	#else
		FogMode                      = OpenGLideFogEmulation_EnvCombine;
	#endif
	ColorAlphaRenderMode           = OpenGLideColorAlphaRenderMode_Automatic;
	PrecisionFix                   = 1;
	/*
	// Not bad
	GapFix = static_cast<OpenGLideGapFixFlags>(OpenGLideGapFixFlag_Enabled |
	                                           OpenGLideGapFixFlag_IncircleOr |
	                                           OpenGLideGapFixFlag_DepthFactor);
	GapFixParam1 = 3.0f;
	GapFixParam2 = 10.0f;
	GapFixParam3 = 3.0f;
	GapFixDepthFactor = 3.3f;
	*/
	/*
	// But this one is better
	GapFix = static_cast<OpenGLideGapFixFlags>(OpenGLideGapFixFlag_Enabled |
	                                           OpenGLideGapFixFlag_VertexLengthSecondRadius |
	                                           OpenGLideGapFixFlag_DepthFactor);
	GapFixParam1 = 3.0f;
	GapFixParam2 = 25.0f;
	GapFixParam3 = 0.9f;
	GapFixDepthFactor = 3.3f;
	*/
	// @todo: Let's make this even more better
	GapFix = static_cast<OpenGLideGapFixFlags>(/*OpenGLideGapFixFlag_Enabled |*/
	                                           OpenGLideGapFixFlag_VertexLengthSecondRadius |
	                                           OpenGLideGapFixFlag_DepthFactor);
	GapFixParam1 = 3.0f;
	GapFixParam2 = 25.0f;
	GapFixParam3 = 0.9f;
	GapFixDepthFactor = 3.3f;
	
	GenerateSubTextures            = 0;
	Mipmapping                     = true;
	AnisotropylLevel               = 16;
	IgnorePaletteChange            = false;
	ARB_multitexture               = true;
	EXT_paletted_texture           = true;
	EXT_texture_env_add            = true;
	EXT_texture_env_combine        = true;
	ATI_texture_env_combine3       = true;
#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
	EXT_fog_coord                  = true;
#endif
#ifdef OPENGLIDE_SYSTEM_HAS_BLENDFUNC_SEPERATE
	EXT_blend_func_separate        = true;
#endif
  EXT_texture_lod_bias           = true;
  EXT_secondary_color            = true;
	EXT_SGIS_generate_mipmap       = true;
	EXT_SGIS_texture_edge_clamp    = true;
  EXT_Client_Storage             = true;
	EXT_compiled_vertex_array      = true;
	EXT_texture_filter_anisotropic = true;
	ARB_multisample                = true;
	NV_multisample_filter_hint     = true;
	EXT_clip_volume_hint           = true;
	APPLE_transform_hint           = true;
	TextureMemorySize              = 16;
	FrameBufferMemorySize          = 8;
	EnableFrameBufferOverlays      = true;
	EnableFrameBufferUnderlays     = true;
	FramebufferIgnoreUnlock        = false;
	PedanticFrameBufferEmulation   = false;
	BoardType                      = static_cast<OpenGLideBoardType>(GR_SSTTYPE_VOODOO);
	GlideTextureUnits              = 0;
	NoSplash                       = false;
	ShamelessPlug                  = false;
	UseApplicationSpecificSettings = false;
	AutoEnableGameSpecificSettings = true;
	VectorUnitType                 = OpenGLideVectorUnitType_None;
}

GlideSettings::IOErr GlideSettings::read_settings()
{
#ifdef OGL_DEBUG
	GlideMsg("Reading values...\n");
#endif
	GlideSettings::IOErr err = noErr;
	unsigned long value;
	get("DisplayMode", &value);
	DisplayMode = static_cast<OpenGLideDisplayMode>(value);
	get("Resolution", &Resolution);
	get("MonitorRefreshRate", &MonitorRefreshRate);
	get("DepthBufferBits", &DepthBufferBits);
	get("Mipmapping", &Mipmapping);
	get("AnisotropylLevel", &AnisotropylLevel);
	get("FullSceneAntiAliasing", &FullSceneAntiAliasing);
	get("GammaBias", &GammaBias);
	get("TextureSmoothing", &TextureSmoothing);
	get("FogMode", &value);
	FogMode = static_cast<OpenGLideFogEmulation>(value);
	get("ColorAlphaRenderMode", &value);
	ColorAlphaRenderMode = static_cast<OpenGLideColorAlphaRenderMode>(value);
	get("EnablePrecisionFix", &PrecisionFix);
	get("GapFix", &value);
	GapFix = static_cast<OpenGLideGapFixFlags>(value);
	get("GapFixParam1", &GapFixParam1);
	get("GapFixParam2", &GapFixParam2);
	get("GapFixParam3", &GapFixParam3);
	get("GapFixDepthFactor", &GapFixDepthFactor);
	get("GenerateSubTextures", &GenerateSubTextures);
	get("IgnorePaletteChange", &IgnorePaletteChange);
	get("TextureMemorySize", &TextureMemorySize);
	get("FrameBufferMemorySize", &FrameBufferMemorySize);
	get("EnableFrameBufferOverlays", &EnableFrameBufferOverlays);
	get("EnableFrameBufferUnderlays", &EnableFrameBufferUnderlays);
	get("FramebufferIgnoreUnlock", &FramebufferIgnoreUnlock);
	get("PedanticFrameBufferEmulation", &PedanticFrameBufferEmulation);
	get("CompiledVertexArray", &EXT_compiled_vertex_array);
	get("EnableMultiTextureEXT", &ARB_multitexture);
	get("EnablePaletteEXT", &EXT_paletted_texture);
	get("EXT_clip_volume_hint", &EXT_clip_volume_hint);
	get("BoardType", &value);
	BoardType = static_cast<OpenGLideBoardType>(value);
	get("GlideTextureUnits", &GlideTextureUnits);
	get("NoSplash", &NoSplash);
	get("ShamelessPlug", &ShamelessPlug);
	get("UseApplicationSpecificSettings", &UseApplicationSpecificSettings);
	get("AutoEnableGameSpecificSettings", &AutoEnableGameSpecificSettings);
	return err;
}

GlideSettings::IOErr GlideSettings::load()
{
#ifdef OGL_DEBUG
	GlideMsg( OGL_LOG_SEPARATE );
	GlideMsg("Loading settings from preferences directory...\n");
#endif
	const char* prefs_version = OpenGLidePreferencesVersion;
	GlideSettings::IOErr err = read_defaults();
	if (err == noErr)
	{
		const char* version_complaint = "Unable to read version information from %s settings - using default values\n";
		const char* version;
		bool success = get("Version", &version);
		if (success)
		{
			GlideMsg("Settings file version = %s\n", version);
		}
		if (success && strcmp(version, prefs_version) == 0)
		{
			err = read_settings();
		}
		else if (!success)
		{
			GlideMsg(version_complaint, "default");
		}
		else
		{
			GlideMsg("Configuration outdated - Creating new default settings %s...\n", prefs_version);
			defaults();
			err = create_defaults();
			if (err == noErr)
			{
				err = saveSettings();
			}
		}
		if (err == noErr)
		{
			if (isApplicationSpecific() && UseApplicationSpecificSettings)
			{
				err = read();
				if (err == noErr)
				{
					success = get("Version", &version);
					if (success && strcmp(version, prefs_version) == 0)
					{
						bool use_app_specific_settings = false;
						success = get("UseApplicationSpecificSettings", &use_app_specific_settings);
						if (use_app_specific_settings)
						{
							err = read_settings();
						}
					}
					else if (!success)
					{
						GlideMsg(version_complaint, "application specific");
					}
					else
					{
						GlideMsg("Configuration outdated - Creating new application specific settings %s...\n", prefs_version);
						defaults();
						err = create();
						if (err == noErr)
						{
							err = saveSettings();
						}
					}
				}
			}
		}
	}
	return err;
}

GlideSettings::IOErr GlideSettings::saveSettings()
{
#ifdef OGL_DEBUG
	GlideMsg("Saving...\n");
#endif
	put("Configuration File for MacGLide");
	put();
	put();
	put("Current version number of the settings file:" );
	put("Version", OpenGLidePreferencesVersion);
	put();
	put("Enable to apply application specific settings:");
	put("UseApplicationSpecificSettings", UseApplicationSpecificSettings);
	put();
	put("Video settings");
	put();
	put("Fullscreen display:");
	put("0 for windowed mode, 1 for fullscreen display");
	put("DisplayMode", static_cast<unsigned long>(DisplayMode));
	put();
	put("Display resolution:");
	put("1-8: Multiplier for Glide resolution as requested by the");
	put("game/application. Any value equal or greater than 512:");
	put("fixed x resolution (y is computed for a 4/3 aspect ratio)");
	put("Resolution", Resolution);
	put();
	put("Refresh rate of the monitor:");
	put("0 to use the Glide refresh rate as requested by the game/");
	put("application. Any other value equal or greater than 60(Hz):");
	put("Use fixed refresh rate");
	put("MonitorRefreshRate", MonitorRefreshRate);
	put();
	put("Gamma bias value for the Glide display:");
	put("Most games adjust the gamma correction of the display.");
	put("For certain monitors and LCD display the resulting gamma");
	put("correction might be too bright (probably because the");
	put("3Dfx-Hardware was never supposed to work with LCD displays).");
	put("Although most games provide a way to adjust the gamma");
	put("correction via their configuration dialog, it might be");
	put("necessary to set a reasonable default bias value.");
	put("Reasonable bias values are -1.0 to 1.0.");
	put("GammaBias", GammaBias);
	put();
	put("Mip-Mapping: 0 to disable, 1 to enable mipmapped textures");
	put("Mipmapping", Mipmapping);
	put();
	put("Anisotropic filtering:");
	put("0 or 1 to disable, 2, 4, 8, 16 to specify the quality");
	put("level of anisotropic filtering");
	put("AnisotropylLevel", AnisotropylLevel);
	put();
	put("Fullscene antialiasing:");
	put("0 to disable, any other value to set number of samples to use");
	put("for antialiasing (currently this works with ATI cards only)");
	put("FullSceneAntiAliasing", FullSceneAntiAliasing);
	put();
	put();
	put("Game specific settings (usually set automatically per application");
	put();
	put("Enable game specific settings automatically:");
	put("0 to disable, 1 to automatically certain enable game specific settings");
	put("The settings in this section don't work with all games. To avoid");
	put("problems, these settings are usually set individually for each game.");
	put("AutoEnableGameSpecificSettings", AutoEnableGameSpecificSettings);
	put();
	put("Texture smoothing:");
	put("0 to disable, 1 to enable texture filtering. Overrides Glide");
	put("texture filter settings in order to suppress blocky pixels.");
	put("Some games don't like these filter settings to be changed, for");
	put("instance horizontal and vertical gaps may appear in texture-based");
	put("menu screens");
	put("TextureSmoothing", TextureSmoothing);
	put();
	put("Texture artefact optimisation for Tomb Raider 1 & 2:");
	put("Tomb Raider joins a number of texture images within a");
	put("larger texture object. In conjunction with ansisotropic");
	put("filtering and mipmapping this cause artefacts at the");
	put("edges of single tiles/triangles.");
	put("0 to disable, n (power of 2) to place each texture image");
	put("into a seperate OpenGL texture object. The value determines");
	put("the grid points and the minimal size of the generated");
	put("OpenGL textures.");
	put("GenerateSubTextures", GenerateSubTextures);
	put();
	put("Vertex gapfix optimisation for Tomb Raider 1 & 2:");
	put("Reduces gaps between tiles in the tomb raider series");
	put("(best observed in TR1, but TR2 is also affected)");
	put("A combination of the following values:");
	put("1=Enable, 2=Debug, 4=DepthFactor, 16=IncircleOr,");
	put("32=IncircleAnd, 64=IncircleSecondRadius,");
	put("128=VertexLengthSecondRadius");
	put("GapFix", static_cast<unsigned long>(GapFix));
	put();
	put("Minimum pixel incircle of a triangle for applying the gapfix");
	put("GapFixParam1", GapFixParam1);
	put();
	put("Minimum value of the isoscelesness or the maximum vertex");
	put("length of a triangle for applying the gapfix");
	put("(an isosceles triangle has a value of 10)");
	put("GapFixParam2", GapFixParam2);
	put();
	put("Second minimum incircle of a triangle for applying the gapfix,");
	put(" this one is independent from the isoscelesness/vertex length");
	put("GapFixParam3", GapFixParam3);
	put();
	put("Multiplies the incircles/isoscelesness with the vertex depth to");
	put("allow applying the gapfix to smaller triangles in the background");
	put("GapFixDepthFactor", GapFixDepthFactor);
	put();
	put();
	put("Various settings that affect the rendering quality.");
	put();
	put("Fog emulation method:");
	put("0 to disable, 1 for simple, 2 for complete fog emulation");
	put("FogMode", static_cast<unsigned long>(FogMode));
	put();
	put("The renderer to be used to emulate the Glide color and alpha");
	put("combine unit. 0 for automatic selection, 1 for simple (0.10),");
	put("2 for enhanced, and 3 for enhanced ATI optimised emulation");
	put("ColorAlphaRenderMode", static_cast<unsigned long>(ColorAlphaRenderMode));
	put();
	put("Number of bits to use for the depth buffer:");
	put("0: automatic (attempts to use the largest possible value)");
	put("16, 24, 32: use the specified number of bits for the depth buffer");
	put("DepthBufferBits", DepthBufferBits);
	put();
	put("Depth precision fix: attempts to avoid depth precision");
	put("problems on graphic cards with only 16bit-zbuffer depth");
	put("0 = always off, 1 = automatic, 2 = always on");
	put("EnablePrecisionFix", PrecisionFix);
	put();
	put();
	put("Various debug settings");
	put();
	put("IgnorePaletteChange", IgnorePaletteChange);
	put("EnableFrameBufferOverlays", EnableFrameBufferOverlays);
	put("EnableFrameBufferUnderlays", EnableFrameBufferUnderlays);
	put("FramebufferIgnoreUnlock", FramebufferIgnoreUnlock);
	put("PedanticFrameBufferEmulation", PedanticFrameBufferEmulation);
	put("CompiledVertexArray", EXT_compiled_vertex_array);
	put("EnableMultiTextureEXT", ARB_multitexture);
	put("EnablePaletteEXT", EXT_paletted_texture);
	put("EXT_clip_volume_hint", EXT_clip_volume_hint);
	put();
	put();
	put("Glide hardware settings");
	put();
	putv("Texture Memory goes from %d to %d", OGL_MIN_TEXTURE_BUFFER, OGL_MAX_TEXTURE_BUFFER);
	put("TextureMemorySize", TextureMemorySize);
	put();
	putv("Frame Buffer Memory goes from %d to %d", OGL_MIN_FRAME_BUFFER, OGL_MAX_FRAME_BUFFER);
	put("FrameBufferMemorySize", FrameBufferMemorySize);
	put();
	put("The hardware we're pretending to be emulating");
	put("(This has no effect on the emulation, but games may change");
	put("their behaviour/features depending on this setting)");
	put("0 = Voodoo, 1 = SST-96, 2 = AT3D, 3 = Voodoo 2");
	put("BoardType", static_cast<unsigned long>(BoardType));
	put();
	put("The number of emulated Glide texture units (TMUs)");
	put("0 = as many as possible, 1-3 number of emulated TMUs");
	put("GlideTextureUnits", GlideTextureUnits);
	put();
	put("0 to display the cool splash animation on startup,");
	put("or 1 to supress the annoying splash animation");
	put("NoSplash", NoSplash);
	put();
	put("Shows the 3Dfx logo in the lower right corner of the screen");
	put("ShamelessPlug", ShamelessPlug);
	GlideSettings::IOErr err = close();
	return err;
}

GlideSettings::IOErr GlideSettings::put()
{
	return put_raw("\n");
}

GlideSettings::IOErr GlideSettings::put(const char* string)
{
	GlideSettings::IOErr err = put_raw("# ");
	if (err == noErr)
	{
		err = put_raw(string);
		if (err == noErr)
		{
			put();
		}
	}
	return err;
}

GlideSettings::IOErr GlideSettings::putv(const char* format, ...)
{
	va_list(args);
	va_start(args, format);
	char buffer[StringBufferSize];
	vsnprintf(buffer, StringBufferSize, format, args );
	GlideSettings::IOErr err = put(buffer);
	va_end(args);
	return err;
}

GlideSettings::IOErr GlideSettings::put(unsigned long value)
{
	GlideSettings::IOErr err = noErr;
	char buffer[StringBufferSize];
	if (snprintf(buffer, StringBufferSize, "%d", value) > 0)
	{
		err = put_raw(buffer);
	}
	return err;
}

GlideSettings::IOErr GlideSettings::put(const char* setting, const char* value)
{
	GlideSettings::IOErr err = put_raw(setting);
	if (err == noErr)
	{
		err = put_raw("=");
		if (err == noErr)
		{
			err = put_raw(value);
			if (err == noErr)
			{
				err = put_raw("\n");
			}
		}
	}
	return err;
}

GlideSettings::IOErr GlideSettings::put(const char* setting, unsigned long value)
{
	GlideSettings::IOErr err = put_raw(setting);
	if (err == noErr)
	{
		err = put_raw("=");
		if (err == noErr)
		{
			char buffer[StringBufferSize];
			if (snprintf(buffer, StringBufferSize, "%d", value) > 0)
			{
				err = put_raw(buffer);
			}
			if (err == noErr)
			{
				err = put_raw("\n");
			}
		}
	}
	return err;
}

GlideSettings::IOErr GlideSettings::put(const char* setting, float value)
{
	GlideSettings::IOErr err = put_raw(setting);
	if (err == noErr)
	{
		err = put_raw("=");
		if (err == noErr)
		{
			char buffer[StringBufferSize];
			if (snprintf(buffer, StringBufferSize, "%g", value) > 0)
			{
				err = put_raw(buffer);
			}
			if (err == noErr)
			{
				err = put_raw("\n");
			}
		}
	}
	return err;
}

GlideSettings::IOErr GlideSettings::put(const char* setting, bool value)
{
	return put(setting, value ? "1" : "0");
}

GlideSettings::IOErr GlideSettings::save()
{
	IOErr err = noErr;
	if (isApplicationSpecific())
	{
		err = create();
	}
	else
	{
		err = create_defaults();
	}
	if (err == noErr)
	{
		err = saveSettings();
	}
	close();
	return noErr;
}