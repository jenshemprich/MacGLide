//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                  OpenGLide Settings File
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#pragma once

#include "sdk2_glide.h"

enum OpenGLideDisplayMode
{
	OpenGLideDisplayMode_Window = 0,
	OpenGLideDisplayMode_DisplayManager = 1,
	OpenGLideDisplayMode_aglSetFullScreen = 2
};

enum OpenGLideFogEmulation
{
	OpenGLideFogEmulation_None = 0,
	OpenGLideFogEmulation_Simple = 1,
	OpenGLideFogEmulation_EnvCombine = 2
#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
	,
	OpenGLideFogEmulation_FogCoord = 3
#endif
};

enum OpenGLideColorAlphaRenderMode
{
	OpenGLideColorAlphaRenderMode_Automatic = 0,
	OpenGLideColorAlphaRenderMode_Simple = 1,
	OpenGLideColorAlphaRenderMode_EnvCombine_ARB = 2,
	OpenGLideColorAlphaRenderMode_EnvCombine3_ATI = 3,
	OpenGLideColorAlphaRenderMode_Unknown = 4
};

enum OpenGLideBoardType
{
	OpenGLideBoardType_Voodoo = GR_SSTTYPE_VOODOO,
	OpenGLideBoardType_VoodooRush = GR_SSTTYPE_SST96,
	OpenGLideBoardType_AT3D = GR_SSTTYPE_AT3D,
	OpenGLideBoardType_Voodoo2 = GR_SSTTYPE_Voodoo2
};

enum OpenGLideGapFixFlags
{
	OpenGLideGapFixFlag_Disabled = 0x00,
	OpenGLideGapFixFlag_Enabled = 0x01,
	OpenGLideGapFixFlag_Debug = 0x02,
	OpenGLideGapFixFlag_DepthFactor = 0x04,
	OpenGLideGapFixFlag_IncircleOr = 0x10,
	OpenGLideGapFixFlag_IncircleAnd = 0x20,
	OpenGLideGapFixFlag_IncircleSecondRadius = 0x40,
	OpenGLideGapFixFlag_VertexLengthSecondRadius = 0x80
};

enum OpenGLideVectorUnitType
{
	OpenGLideVectorUnitType_None = 0x00,
	OpenGLideVectorUnitType_Altivec = 0x01
};

struct ConfigStruct
{
	OpenGLideDisplayMode DisplayMode;
	unsigned long Resolution;
	unsigned long ResolutionCap;
	unsigned long MonitorRefreshRate;
	unsigned long DepthBufferBits;
	unsigned long FullSceneAntiAliasing;
	float GammaBias;
	bool TextureSmoothing;
	OpenGLideFogEmulation FogMode;
	OpenGLideColorAlphaRenderMode ColorAlphaRenderMode;
	unsigned long PrecisionFix;
	OpenGLideGapFixFlags GapFix;
	float GapFixParam1;
	float GapFixParam2;
	float GapFixParam3;
	float GapFixDepthFactor;
	unsigned long GenerateSubTextures;
	unsigned long TextureMemorySize;
	unsigned long FrameBufferMemorySize;
	unsigned long AnisotropylLevel;
	bool Mipmapping;
	bool IgnorePaletteChange;
	bool EXT_secondary_color;
	bool ARB_multitexture;
	bool EXT_texture_env_add;
	bool EXT_texture_env_combine;
	bool ATI_texture_env_combine3;
	bool EXT_texture_lod_bias;
	bool EXT_paletted_texture;
	bool EXT_SGIS_generate_mipmap;
	bool EXT_SGIS_texture_edge_clamp;
	bool EXT_Client_Storage;
	bool EXT_compiled_vertex_array;
#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
	bool EXT_fog_coord;
#endif
#ifdef OPENGLIDE_SYSTEM_HAS_BLENDFUNC_SEPERATE
	bool EXT_blend_func_separate;
#endif
	bool EXT_texture_filter_anisotropic;
	bool ARB_multisample;
	bool NV_multisample_filter_hint;
	bool EXT_clip_volume_hint;
	bool APPLE_transform_hint;
	bool EnableFrameBufferOverlays;
	bool EnableFrameBufferUnderlays;
	bool FramebufferIgnoreUnlock;
	bool PedanticFrameBufferEmulation;
	OpenGLideBoardType BoardType;
	unsigned long GlideTextureUnits;
	bool NoSplash;
	bool ShamelessPlug;
	bool UseApplicationSpecificSettings;
	bool AutoEnableGameSpecificSettings;
	OpenGLideVectorUnitType VectorUnitType;
};

class GlideSettings : public ConfigStruct
{
protected:
	static const char* OpenGLidePreferencesVersion;
public:
	GlideSettings();
	virtual ~GlideSettings(void);
public:
	typedef int IOErr;
	virtual IOErr init(const char* application)=0;
	IOErr load();
	IOErr save();
	virtual IOErr create_log()=0;
	virtual IOErr write_log(const char* message)=0;
	virtual IOErr create_defaults()=0;
	virtual IOErr create()=0;
protected:
	void defaults();
	virtual IOErr read_defaults()=0;
	virtual IOErr read()=0;
	IOErr read_settings();
	IOErr saveSettings();
	bool get(const char* setting, const char** value);
	bool get(const char* setting, unsigned long* value);
	bool get(const char* setting, float* value);
	bool get(const char* setting, bool* value);
	virtual IOErr put_raw(const char* string)=0;
	virtual IOErr close()=0;
	IOErr put();
	IOErr put(const char* string);
	IOErr putv(const char* format, ...);
	IOErr put(unsigned long  value);
	IOErr put(const char* setting, const char* value);
	IOErr put(const char* setting, unsigned long value);
	IOErr put(const char* setting, float value);
	IOErr put(const char* setting, bool value);
	char* m_FileBuffer;
	long m_FileBufferSize;
};
