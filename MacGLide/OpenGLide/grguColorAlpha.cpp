//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                  Color and Alpha File
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Glide.h"
#include "GlideDisplay.h"
#include "GLColorAlphaCombineEnvTables.h"
#include "GLExtensions.h"
#include "GLRender.h"
#include "GLUtil.h"
#include "GLRenderUpdateState.h"
#include "OGLTables.h"
#include "PGTexture.h"

//*************************************************
//* Sets the Dithering Mode 24->16 bits
//*************************************************
FX_ENTRY void FX_CALL
grDitherMode( GrDitherMode_t mode )
{
	CHECK_STATE_CHANGED(Glide.State.DitherMode == mode);
	
#ifdef OGL_DONE
    GlideMsg( "grDitherMode( %d )\n", mode );
#endif
	glReportErrors("grDitherMode");
	
	RenderDrawTriangles( );
	Glide.State.DitherMode = mode;
	if ( mode != GR_DITHER_DISABLE )
	{
		// GR_DITHER_2x2 or GR_DITHER_4x4
		glEnable( GL_DITHER );
	}
	else
	{
		glDisable( GL_DITHER );
	}
	glReportError();
}

//*************************************************
//* Sets the Constant color
//*************************************************
FX_ENTRY void FX_CALL
grConstantColorValue( GrColor_t value )
{
	CHECK_STATE_CHANGED(Glide.State.ConstantColorValue == value);

#ifdef OGL_DONE
	GlideMsg( "grConstantColorValue( 0x%X )\n", value );
#endif

	RenderDrawTriangles();

	Glide.State.ConstantColorValue = value;
	ConvertColorF(value, 
								OpenGL.ConstantColor[ 0 ], 
								OpenGL.ConstantColor[ 1 ], 
								OpenGL.ConstantColor[ 2 ], 
								OpenGL.ConstantColor[ 3 ]);

	SetConstantColorValueState();
}

//*************************************************
//* Sets the Constant color - obsolete in 2.4
//*************************************************
FX_ENTRY void FX_CALL
grConstantColorValue4( float a, float r, float g, float b )
{
	CHECK_STATE_CHANGED(r == Glide.State.Delta0ModeColor[0] &&
	                    g == Glide.State.Delta0ModeColor[1] &&
	                    b == Glide.State.Delta0ModeColor[2] &&
	                    a == Glide.State.Delta0ModeColor[3]);

#ifdef OGL_DONE
  GlideMsg( "grConstantColorValue4( %f, %f, %f, %f )\n", a, r, g, b );
#endif

  RenderDrawTriangles();

	Glide.State.Delta0ModeColor[0] = r;
	Glide.State.Delta0ModeColor[1] = g;
	Glide.State.Delta0ModeColor[2] = b;
	Glide.State.Delta0ModeColor[3] = a;
	
	OpenGL.Delta0Color[0] = r * D1OVER255;
	OpenGL.Delta0Color[1] = g * D1OVER255;
	OpenGL.Delta0Color[2] = b * D1OVER255;
	// according to the linux driver src,
	// alpha is completely ignored
	// Glide.State.Delta0ModeColor[3] = a * D10255;
	// Thus, alpha is take from grConstantColorValue()
	OpenGL.Delta0Color[3] = OpenGL.ConstantColor[3];

	SetConstantColorValue4State();
}

//*************************************************
//* Sets the Color and Alpha mask
//*************************************************
FX_ENTRY void FX_CALL
grColorMask( FxBool rgb, FxBool a )
{
#ifdef OGL_DONE
	GlideMsg( "grColorMask( %s, %s )\n", 
	    rgb ? "TRUE" : "FALSE", a  ? "TRUE" : "FALSE" );
#endif
	glReportErrors("grColorMask");
	
	RenderDrawTriangles( );

	Glide.State.ColorMask = rgb;
	Glide.State.AlphaMask = a;
	glColorMask( rgb, rgb, rgb, a );
	glReportError();
}

FX_ENTRY void FX_CALL
grColorCombine( GrCombineFunction_t function, GrCombineFactor_t factor,
                GrCombineLocal_t local, GrCombineOther_t other,
                FxBool invert )
{
	glReportErrors("grColorCombine");

	CHECK_STATE_CHANGED(Glide.State.ColorCombineFunction == function
	                 && Glide.State.ColorCombineFactor == factor
	                 && Glide.State.ColorCombineLocal == local
	                 && Glide.State.ColorCombineOther == other
	                 && Glide.State.ColorCombineInvert == invert);

#if defined( OGL_DONE ) || defined( OGL_COMBINE )
	GlideMsg( "grColorCombine( %d, %d, %d, %d, %s )\n",
	function, factor, local, other, invert ? "TRUE" : "FALSE" );
#endif

	RenderDrawTriangles( );

	// May need to turn the fog texture unit on/off
	if ( Glide.State.ColorCombineInvert != invert)
	{
		SetColorInvertState();
	}
	Glide.State.ColorCombineFunction    = function;
	Glide.State.ColorCombineFactor      = factor;
	Glide.State.ColorCombineLocal       = local;
	Glide.State.ColorCombineOther       = other;
	Glide.State.ColorCombineInvert      = invert;

	// Used by the simple coloralpha model but also to determine whether textures are used
	Glide.CLocal = colorCombineTable[ factor ][ function ].local;
	Glide.COther = colorCombineTable[ factor ][ function ].other;
	if ( colorCombineTable[ factor ][ function ].alocal )
	{
		Glide.ALocal = true;
	}
	if ( colorCombineTable[ factor ][ function ].aother )
	{
		Glide.AOther = true;
	}
	ColorFunctionFunc = colorCombineTable[ factor ][ function ].func;
	ColorFactor3Func = colorCombineTable[ factor ][ function ].factorfunc;

	SetColorTextureState();
	SetTextureState();
	SetColorCombineState();

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}

inline void _grColorCombineDelta0Mode(bool delta0mode)
{
	CHECK_STATE_CHANGED(delta0mode == Glide.State.Delta0Mode);

#if defined(OGL_DONE)
	GlideMsg( "_grColorCombineDelta0Mode(%s)\n", delta0mode ? "TRUE" : "FALSE");
#endif

	Glide.State.Delta0Mode = delta0mode;
	if (delta0mode)
	{
		SetConstantColorValue4State();	
	}
	else
	{
		SetConstantColorValueState();
	}
}

//*************************************************
FX_ENTRY void FX_CALL
guColorCombineFunction( GrColorCombineFnc_t fnc )
{
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
	GlideMsg( "guColorCombineFunction( %d )\n", fnc );
#endif

	// @todo: Analyse driver source to find out about ITRGB_DELTA0
	// color alpha render modes (in /glide/src/gu.c)
	// So far it seems to ignore to iterate the vertex colors (right?) 
	/* gross hack to get ITRGB_DELTA0 modes working */
	// _grColorCombineDelta0Mode( FXFALSE );
	bool delta0mode = FXFALSE;

	switch ( fnc )
	{
	case GR_COLORCOMBINE_ZERO:                              //0x0
		grColorCombine( GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
		            GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_NONE, FXFALSE );
		break;

	case GR_COLORCOMBINE_CCRGB:                             //0x1
		grColorCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, 
		    GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FXFALSE );
		break;

	case GR_COLORCOMBINE_ITRGB:                             //0x2
		grColorCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
		            GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE );
		break;

	case GR_COLORCOMBINE_DECAL_TEXTURE:                     //0x4
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
		    GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		break;

	case GR_COLORCOMBINE_TEXTURE_TIMES_CCRGB:               //0x5
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, 
		    GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		break;

	case GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB:               //0x6
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, 
		    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		break;

	case GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_ADD_ALPHA:     //0x8
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA, GR_COMBINE_FACTOR_LOCAL, 
		    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		break;

	case GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA:               //0x9
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL_ALPHA, 
		    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		break;

	case GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA_ADD_ITRGB:     //0xa
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_LOCAL_ALPHA, 
		    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		break;

	case GR_COLORCOMBINE_TEXTURE_ADD_ITRGB:                 //0xb
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE, 
		    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		break;

	case GR_COLORCOMBINE_TEXTURE_SUB_ITRGB:                 //0xc
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, GR_COMBINE_FACTOR_ONE, 
		    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		break;

	case GR_COLORCOMBINE_DIFF_SPEC_A:                       //0xe
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_LOCAL_ALPHA, 
		    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		break;

	case GR_COLORCOMBINE_DIFF_SPEC_B:                       //0xf
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA, GR_COMBINE_FACTOR_LOCAL, 
		    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		break;

	case GR_COLORCOMBINE_ONE:                               //0x10
		grColorCombine( GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ONE,
		    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXTRUE );
		break;

	case GR_COLORCOMBINE_ITRGB_DELTA0:                      //0x3
		// _grColorCombineDelta0Mode(FXTRUE);
		delta0mode = FXTRUE;
		grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
		    GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_CONSTANT, FXFALSE);
		break;

	case GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_DELTA0:        //0x7
		// @todo: May not be correct, as the 3dfx 2.4 pgm doesn't discuss this function.
		// Using GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB with constant colors instead.
		// In the Glide-Driver src its almost the same, but _grColorCombineDelta0Mode(FXTRUE)
		// is called in advance to do something about the iterated colors. At least, this fixes,
		// the problems with colors and skidmarks in Carmageddon (Splatpack), so it can't be that bad :^)
		// _grColorCombineDelta0Mode(FXTRUE);
		delta0mode = FXTRUE;
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, 
		    GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
	  break;

	case GR_COLORCOMBINE_CCRGB_BLEND_ITRGB_ON_TEXALPHA:     //0xd
		grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL, GR_COMBINE_FACTOR_TEXTURE_ALPHA,
		    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_CONSTANT, FXFALSE);
	  break;
	}

	_grColorCombineDelta0Mode(delta0mode);
}

//*************************************************
//* Sets the Alpha Test Reference Value
//*************************************************
FX_ENTRY void FX_CALL
grAlphaTestReferenceValue( GrAlpha_t value )
{
	CHECK_STATE_CHANGED(Glide.State.AlphaReferenceValue == value);

#ifdef OGL_DONE
	GlideMsg( "grAlphaTestReferenceValue( %d )\n", value );
#endif

	RenderDrawTriangles( );

	Glide.State.AlphaReferenceValue = value;
	OpenGL.AlphaReferenceValue = value * D1OVER255;
	// Only call if the state needs to be changed
#ifdef OPTIMISE_OPENGL_STATE_CHANGES
	// alpha reference value applies only when the following conditons are true
	if ((!OpenGL.ChromaKey || !OpenGL.Texture || OpenGL.Blend)
	 && Glide.State.AlphaOther == GR_COMBINE_OTHER_TEXTURE
	 && Glide.State.AlphaTestFunction != GR_CMP_ALWAYS)
#endif
		SetChromaKeyAndAlphaState();
}

//*************************************************
//* Sets the Alpha Test Function
//*************************************************
FX_ENTRY void FX_CALL
grAlphaTestFunction( GrCmpFnc_t function )
{
	CHECK_STATE_CHANGED(Glide.State.AlphaTestFunction == function);

#ifdef OGL_DONE
	GlideMsg( "grAlphaTestFunction( %d )\n", function );
#endif

	RenderDrawTriangles( );

	Glide.State.AlphaTestFunction = function;

	// We can do this just because we know the constant values for both OpenGL and Glide
	// To port it to anything else than OpenGL we NEED to change this code
	OpenGL.AlphaTestFunction = GL_NEVER + function;

#ifdef OPTIMISE_OPENGL_STATE_CHANGES
	if ((Glide.State.AlphaOther == GR_COMBINE_OTHER_TEXTURE)
	 && (!OpenGL.ChromaKey || OpenGL.Blend || !OpenGL.Texture))
#endif
	{
		SetChromaKeyAndAlphaState();
	}
}

//*************************************************
FX_ENTRY void FX_CALL
grAlphaBlendFunction( GrAlphaBlendFnc_t rgb_sf,   GrAlphaBlendFnc_t rgb_df,
                      GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df )
{
	glReportErrors("grAlphaBlendFunction");
	
	CHECK_STATE_CHANGED(Glide.State.AlphaBlendRgbSf == rgb_sf
	                 && Glide.State.AlphaBlendRgbDf == rgb_df
	                 && Glide.State.AlphaBlendAlphaSf == alpha_sf
	                 && Glide.State.AlphaBlendAlphaDf == alpha_df);

#ifdef OGL_PARTDONE
	GlideMsg( "grAlphaBlendFunction( %d, %d, %d, %d )\n",
			rgb_sf, rgb_df, alpha_sf, alpha_df );
#endif

#ifndef OGL_PARTDONE
#ifdef OGL_NOTDONE
	// Unhandled glBlendFuncSeperateEXT emulation error
	if (Glide.State.AlphaBlendRgbSf != Glide.State.AlphaBlendAlphaSf
		|| Glide.State.AlphaBlendRgbDf != Glide.State.AlphaBlendAlphaDf)
	{
		GlideMsg( "grAlphaBlendFunction( %d, %d, %d, %d ): emulation for sepearate alpha blend function not implemented\n",
				rgb_sf, rgb_df, alpha_sf, alpha_df );
	}
#endif
#endif

	RenderDrawTriangles( );

	Glide.State.AlphaBlendRgbSf     = rgb_sf;
	Glide.State.AlphaBlendRgbDf     = rgb_df;
	Glide.State.AlphaBlendAlphaSf   = alpha_sf;
	Glide.State.AlphaBlendAlphaDf   = alpha_df;

	switch ( rgb_sf )
	{
	case GR_BLEND_ZERO:                 OpenGL.SrcBlend = GL_ZERO;                  break;
	case GR_BLEND_ONE:                  OpenGL.SrcBlend = GL_ONE;                   break;
	case GR_BLEND_DST_COLOR:            OpenGL.SrcBlend = GL_DST_COLOR;             break;
	case GR_BLEND_ONE_MINUS_DST_COLOR:  OpenGL.SrcBlend = GL_ONE_MINUS_DST_COLOR;   break;
	case GR_BLEND_SRC_ALPHA:            OpenGL.SrcBlend = GL_SRC_ALPHA;             break;
	case GR_BLEND_ONE_MINUS_SRC_ALPHA:  OpenGL.SrcBlend = GL_ONE_MINUS_SRC_ALPHA;   break;
	case GR_BLEND_DST_ALPHA:            OpenGL.SrcBlend = GL_DST_ALPHA;             break;
	case GR_BLEND_ONE_MINUS_DST_ALPHA:  OpenGL.SrcBlend = GL_ONE_MINUS_DST_ALPHA;   break;
	case GR_BLEND_ALPHA_SATURATE:       OpenGL.SrcBlend = GL_SRC_ALPHA_SATURATE;    break;

#ifdef OGL_DEBUG
	default:
		GlideMsg( "grAlphaBlendFunction: Unknown RGB source blend factor.\n" );
		OpenGL.SrcBlend = GL_ONE;
		break;
#endif
	}

	switch ( rgb_df )
	{
	case GR_BLEND_ZERO:                 OpenGL.DstBlend = GL_ZERO;                  break;
	case GR_BLEND_ONE:                  OpenGL.DstBlend = GL_ONE;                   break;
	case GR_BLEND_SRC_COLOR:            OpenGL.DstBlend = GL_SRC_COLOR;             break;
	case GR_BLEND_ONE_MINUS_SRC_COLOR:  OpenGL.DstBlend = GL_ONE_MINUS_SRC_COLOR;   break;
	case GR_BLEND_SRC_ALPHA:            OpenGL.DstBlend = GL_SRC_ALPHA;             break;
	case GR_BLEND_ONE_MINUS_SRC_ALPHA:  OpenGL.DstBlend = GL_ONE_MINUS_SRC_ALPHA;   break;
	case GR_BLEND_DST_ALPHA:            OpenGL.DstBlend = GL_DST_ALPHA;             break;
	case GR_BLEND_ONE_MINUS_DST_ALPHA:  OpenGL.DstBlend = GL_ONE_MINUS_DST_ALPHA;   break;
	case GR_BLEND_PREFOG_COLOR:         OpenGL.DstBlend = GL_ONE;                   break;

#ifdef OGL_DEBUG
	default:
		GlideMsg( "grAlphaBlendFunction: Unknown RGB destination blend factor.\n" );
		OpenGL.DstBlend = GL_ZERO;
		break;
#endif
	}

	switch ( alpha_sf )
	{
	case GR_BLEND_ZERO:                 OpenGL.SrcAlphaBlend = GL_ZERO;                 break;
	case GR_BLEND_ONE:                  OpenGL.SrcAlphaBlend = GL_ONE;                  break;
	case GR_BLEND_DST_COLOR:            OpenGL.SrcAlphaBlend = GL_DST_COLOR;            break;
	case GR_BLEND_ONE_MINUS_DST_COLOR:  OpenGL.SrcAlphaBlend = GL_ONE_MINUS_DST_COLOR;  break;
	case GR_BLEND_SRC_ALPHA:            OpenGL.SrcAlphaBlend = GL_SRC_ALPHA;            break;
	case GR_BLEND_ONE_MINUS_SRC_ALPHA:  OpenGL.SrcAlphaBlend = GL_ONE_MINUS_SRC_ALPHA;  break;
	case GR_BLEND_DST_ALPHA:            OpenGL.SrcAlphaBlend = GL_DST_ALPHA;            break;
	case GR_BLEND_ONE_MINUS_DST_ALPHA:  OpenGL.SrcAlphaBlend = GL_ONE_MINUS_DST_ALPHA;  break;
	case GR_BLEND_ALPHA_SATURATE:       OpenGL.SrcAlphaBlend = GL_SRC_ALPHA_SATURATE;   break;
	}

	switch ( alpha_df )
	{
	case GR_BLEND_ZERO:                 OpenGL.DstAlphaBlend = GL_ZERO;                 break;
	case GR_BLEND_ONE:                  OpenGL.DstAlphaBlend = GL_ONE;                  break;
	case GR_BLEND_SRC_COLOR:            OpenGL.DstAlphaBlend = GL_SRC_COLOR;            break;
	case GR_BLEND_ONE_MINUS_SRC_COLOR:  OpenGL.DstAlphaBlend = GL_ONE_MINUS_SRC_COLOR;  break;
	case GR_BLEND_SRC_ALPHA:            OpenGL.DstAlphaBlend = GL_SRC_ALPHA;            break;
	case GR_BLEND_ONE_MINUS_SRC_ALPHA:  OpenGL.DstAlphaBlend = GL_ONE_MINUS_SRC_ALPHA;  break;
	case GR_BLEND_DST_ALPHA:            OpenGL.DstAlphaBlend = GL_DST_ALPHA;            break;
	case GR_BLEND_ONE_MINUS_DST_ALPHA:  OpenGL.DstAlphaBlend = GL_ONE_MINUS_DST_ALPHA;  break;
	case GR_BLEND_PREFOG_COLOR:         OpenGL.DstAlphaBlend = GL_ONE;                  break;
	}

	OpenGL.Blend = !(( rgb_sf == GR_BLEND_ONE ) && ( rgb_df == GR_BLEND_ZERO ));
	SetTextureState();
	SetBlendState();
	
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}

//*************************************************
FX_ENTRY void FX_CALL
grAlphaCombine( GrCombineFunction_t function, GrCombineFactor_t factor,
                GrCombineLocal_t local, GrCombineOther_t other,
                FxBool invert )
{
	glReportErrors("grAlphaCombine");

	CHECK_STATE_CHANGED( Glide.State.AlphaFunction == function
	                  && Glide.State.AlphaFactor == factor
	                  && Glide.State.AlphaLocal == local
	                  && Glide.State.AlphaOther == other
	                  && Glide.State.AlphaInvert == invert);

#if defined( OGL_DONE ) || defined( OGL_COMBINE )
	GlideMsg( "grAlphaCombine( %d, %d, %d, %d, %s )\n",
		function, factor, local, other, invert ? "TRUE" : "FALSE" );
#endif

  RenderDrawTriangles();

	// If OPTIMISE_OPENGL_STATE_CHANGES is not defined,
	// the update will always be executed
#ifdef OPTIMISE_OPENGL_STATE_CHANGES
	// When the local or other alpha changes,
	// the color combine state may need an update
	bool bUpdateColorCombineState = (OpenGL.ColorAlphaUnit2 &&
	                                 ((local != Glide.State.AlphaLocal
	                                    && (Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_LOCAL_ALPHA
	                                     || Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA))
	                                || (other != Glide.State.AlphaOther
	                                    && (Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_OTHER_ALPHA
	                                     || Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA))
	                                 ));
#endif
	// May need to turn the fog texture unit on/off
	if ( Glide.State.AlphaInvert != invert)
	{
		SetAlphaInvertState();
	}

	Glide.State.AlphaFunction = function;
	Glide.State.AlphaFactor = factor;
	Glide.State.AlphaLocal = local;
	Glide.State.AlphaOther = other;
	Glide.State.AlphaInvert = invert;

	// Used by the simple coloralpha model but also to determine whether textures are used
	Glide.ALocal = alphaCombineTable[ factor ][ function ].local;
	Glide.AOther = alphaCombineTable[ factor ][ function ].other;
	AlphaFactorFunc = alphaCombineTable[ factor ][ function ].func;

	SetAlphaTextureState();
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
	SetTextureState();

#ifdef OPTIMISE_OPENGL_STATE_CHANGES
	if (bUpdateColorCombineState)
	{
#endif
		if (OpenGL.ColorAlphaUnit2) SetColorCombineState();
#ifdef OPTIMISE_OPENGL_STATE_CHANGES
	}
#endif

	// Update chroma key and alpha because both
	// Blend and Texture state might have changed
#ifdef OPTIMISE_OPENGL_STATE_CHANGES
	if (OpenGL.ChromaKey && OpenGL.Texture)
	{
		// Without blend, changing the alpha combine has no effect when chromakeying is enabled
		if (OpenGL.Blend)
		{
			SetChromaKeyAndAlphaState();
			SetAlphaCombineState();
		}
	}
	else
	{
		// If chromakeying is disabled, then everything is already setup correctly,
		// and we have just to update the alpha combine setting
		SetAlphaCombineState();
	}
#else
	SetChromaKeyAndAlphaState();
	SetAlphaCombineState();
#endif
	
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}

//*************************************************
FX_ENTRY void FX_CALL
grAlphaControlsITRGBLighting( FxBool enable )
{
#ifdef OGL_NOTDONE
    GlideMsg("grAlphaControlsITRGBALighting( %s )\n", enable ? "TRUE" : "FALSE" );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
guAlphaSource( GrAlphaSource_t dwMode )
{
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
    GlideMsg( "guAlphaSource( %d )\n", dwMode );
#endif

    switch ( dwMode )
    {
        case GR_ALPHASOURCE_CC_ALPHA:                               //0x00
            grAlphaCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, 
                GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FXFALSE );
            break;

        case GR_ALPHASOURCE_ITERATED_ALPHA:                         //0x01
            grAlphaCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, 
                GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE );
            break;

        case GR_ALPHASOURCE_TEXTURE_ALPHA:                          //0x02
            grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
            break;

        case GR_ALPHASOURCE_TEXTURE_ALPHA_TIMES_ITERATED_ALPHA:     //0x03
            grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, 
                GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE ); 
            break;
    }
}

//*************************************************
//* Sets the ChromaKey Value for comparision
//*************************************************
FX_ENTRY void FX_CALL
grChromakeyValue( GrColor_t value )
{
	CHECK_STATE_CHANGED(Glide.State.ChromakeyValue == value);
	
#ifdef OGL_PARTDONE
    GlideMsg( "grChromakeyValue( 0x%X )\n", value );
#endif

    RenderDrawTriangles( );

    ConvertColor4B( value, OpenGL.ChromaColor.C );
    Textures->ChromakeyValue( OpenGL.ChromaColor.C );

    Glide.State.ChromakeyValue = value;
    s_Framebuffer.OnChromaKeyValueChanged();
}

//*************************************************
//* Sets the ChromaKey Mode
//*************************************************
FX_ENTRY void FX_CALL
grChromakeyMode(GrChromakeyMode_t mode)
{
	CHECK_STATE_CHANGED(Glide.State.ChromaKeyMode == mode);

#ifdef OGL_PARTDONE
	GlideMsg( "grChromakeyMode( %s )\n", mode ? "TRUE" : "FALSE" );
#endif

	RenderDrawTriangles();
	Textures->ChromakeyMode(mode);
	Glide.State.ChromaKeyMode = mode;
	if (mode == GR_CHROMAKEY_ENABLE)
	{
		OpenGL.ChromaKey = true;
	}
	else
	{
		OpenGL.ChromaKey = false;
	}
	// State changes are detected in RenderUpdateState()
}

//*************************************************
FX_ENTRY void FX_CALL
grGammaCorrectionValue( float value )
{
#ifdef OGL_DONE
    GlideMsg( "grGammaCorrectionValue( %f )\n", value );
#endif

  Glide.State.Gamma = value;
	DisplayManager_SetGlideDisplayGamma(value);
}

