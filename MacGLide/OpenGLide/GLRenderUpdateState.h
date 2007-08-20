//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*       updates the GL state before rendering vertices
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "GlOgl.h"
#include "GlideApplication.h"

extern bool s_bUpdateTextureState;
extern bool s_bUpdateFogModeState;
extern bool s_bUpdateFogColorState;
extern bool s_bUpdateBlendState;
extern bool s_bUpdateChromaKeyAndAlphaState;
extern bool s_bUpdateColorCombineState;
extern bool s_bUpdateAlphaCombineState;
extern bool s_bUpdateColorInvertState;
extern bool s_bUpdateAlphaInvertState;
extern bool s_bUpdateConstantColorValueState;
extern bool s_bUpdateConstantColorValue4State;

extern bool s_bForceChromaKeyAndAlphaStateUpdate;

// Forward declarations for inline functions 
// (Deferred inlining)
inline void SetChromaKeyAndAlphaState();
inline void SetAlphaCombineState();
inline void SetTextureState();

inline void ForceChromaKeyAndAlphaStateUpdate()
{
	s_bForceChromaKeyAndAlphaStateUpdate = true;
}

inline void SetColorTextureState()
{
	if ((Glide.COther &&
	       (Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE)
	    && (Glide.State.TextureCombineCFunction != GR_COMBINE_FUNCTION_ZERO))
	 || ((Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA)
	  || (Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA)
	  || (Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB)))
	{
		OpenGL.ColorTexture = true;
	}
	else
	{
		OpenGL.ColorTexture = false;
	}
	SetTextureState();

#ifdef OPENGL_DEBUG
    GlideMsg( "OpenGL.ColorTexture = %d\n", OpenGL.ColorTexture);
#endif
}

inline void SetAlphaTextureState()
{
	if (OpenGL.ColorAlphaUnit2)
	{
		if (( Glide.AOther &&
		       (Glide.State.AlphaOther == GR_COMBINE_OTHER_TEXTURE)
		    && (Glide.State.TextureCombineAFunction != GR_COMBINE_FUNCTION_ZERO))
		 || ((Glide.State.AlphaFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA)
		  || (Glide.State.AlphaFactor == GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA)
		  || (Glide.State.AlphaFactor == GR_COMBINE_FACTOR_TEXTURE_RGB)))
		{
			OpenGL.AlphaTexture = true;
		}
		else
		{
			OpenGL.AlphaTexture = false;
		}
	}
	else
	{
		if ( Glide.AOther && ( Glide.State.AlphaOther == GR_COMBINE_OTHER_TEXTURE ) )
		{
			OpenGL.AlphaTexture = true;
		}
		else
		{
			OpenGL.AlphaTexture = false;
		}
	}
	SetTextureState();

#ifdef OPENGL_DEBUG
    GlideMsg( "OpenGL.AlphaTexture = %d\n", OpenGL.AlphaTexture);
#endif
}

inline void SetTextureState()
{
	s_bUpdateTextureState = true;
	bool opengl_texture = (OpenGL.ColorTexture || (OpenGL.Blend && OpenGL.AlphaTexture))
		&& Glide.State.TexSource.StartAddress != GR_NULL_MIPMAP_HANDLE;
	OpenGL.Texture = opengl_texture;
	// When the texture state changes, chromakeying is also affected
	SetChromaKeyAndAlphaState();

#ifdef OPENGL_DEBUG
    GlideMsg( "OpenGL.Texture = %d\n", OpenGL.Texture);
#endif
}

inline void SetBlendState()
{
	s_bUpdateBlendState = true;
#ifdef OPENGL_DEBUG
    GlideMsg( "OpenGL.Blend = %d\n", OpenGL.Blend);
#endif
}

inline void SetFogModeState()
{
	s_bUpdateFogModeState = true;
#ifdef OPENGL_DEBUG
    GlideMsg( "OpenGL.Fog = %d\n", OpenGL.Fog);
#endif
}

inline void SetFogColorState()
{
	s_bUpdateFogColorState = true;
}

inline void SetChromaKeyAndAlphaState()
{
	s_bUpdateChromaKeyAndAlphaState = true;
#ifdef OPENGL_DEBUG
    GlideMsg( "OpenGL.ChromaKey = %d\n", OpenGL.ChromaKey);
#endif
	if (OpenGL.ColorAlphaUnit2)
	{
		// Need to change alpha combine because the alpha channel
		// in non-blended chromakey textures is a special case
		SetAlphaCombineState();
	}
}

inline void SetColorCombineState()
{
	s_bUpdateColorCombineState = true;
}

inline void SetAlphaCombineState()
{
	s_bUpdateAlphaCombineState = true;
}

inline void SetColorInvertState()
{
	s_bUpdateColorInvertState = true;
	SetFogModeState();
#ifdef OPENGL_DEBUG
    GlideMsg( "Glide.State.ColorCombineInvert = %d\n", Glide.State.ColorCombineInvert);
#endif
}

inline void SetAlphaInvertState()
{
	s_bUpdateAlphaInvertState = true;
	SetFogModeState();
#ifdef OPENGL_DEBUG
    GlideMsg( "Glide.State.AlphaInvert = %d\n", Glide.State.AlphaInvert);
#endif
}

inline void SetConstantColorValueState()
{
 s_bUpdateConstantColorValueState = true;
}

inline void SetConstantColorValue4State()
{
 s_bUpdateConstantColorValue4State = true;
}

// Also called from FrameBuffer class
extern void SetClipVerticesState_Update(bool clip_vertices);

// Needs to be called before adding a triangle, line or point
inline void SetClipVerticesState(bool clip)
{
	const bool clip_vertices = clip || OpenGL.Clipping;
	// Triggers very seldomly (because grDrawXXXWithClip() is used very seldom)
	// As a result this method is designed for cause the least cost when not being triggered
	if (clip_vertices != OpenGL.ClipVerticesEnabledState)
	{
		SetClipVerticesState_Update(clip_vertices);
	}
}

extern void RenderUpdateState();
