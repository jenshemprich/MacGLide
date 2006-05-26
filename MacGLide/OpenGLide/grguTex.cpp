//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                  Glide Texture Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "GlideSettings.h"
#include "GLextensions.h"
#include "GLRender.h"
#include "GLRenderUpdateState.h"
#include "PGTexture.h"
#include "PGUTexture.h"


// Functions

//*************************************************
//* Return the lowest start address for texture downloads
//*************************************************
FX_ENTRY FxU32 FX_CALL
grTexMinAddress( GrChipID_t tmu )
{
#ifdef OGL_DONE
    GlideMsg( "grTexMinAddress( %d ) = 0\n", tmu );
#endif

    return (FxU32) 0;
}

//*************************************************
//* Return the highest start address for texture downloads
//*************************************************
FX_ENTRY FxU32 FX_CALL
grTexMaxAddress( GrChipID_t tmu )
{
#ifdef OGL_DONE
    GlideMsg( "grTexMaxAddress( %d ) = %lu\n", tmu, Glide.TexMemoryMaxPosition );
#endif

    return (FxU32)( Glide.TexMemoryMaxPosition );
}

//*************************************************
//* Specify the current texture source for rendering
//*************************************************
FX_ENTRY void FX_CALL
grTexSource( GrChipID_t tmu,
             FxU32      startAddress,
             FxU32      evenOdd,
             GrTexInfo  *info )
{
#ifdef OPTIMISE_GLIDE_STATE_CHANGES
	const GrTexInfo* current = Textures->GetCurrentTexInfo();
#endif
	CHECK_STATE_CHANGED(Glide.State.TexSource.StartAddress == startAddress &&
	                    Glide.State.TexSource.EvenOdd == evenOdd &&
	                    current->aspectRatio == info->aspectRatio &&
	                    current->format == info->format &&
	                    current->largeLod == info->largeLod &&
	                    current->smallLod == info->smallLod);

#ifdef OGL_PARTDONE
	GlideMsg("grTexSource( %d, 0x%x, 0x%x, (%u, %u, ar=%u, fmt=%u) )\n",
	         tmu, startAddress, evenOdd,
	         info->smallLod, info->largeLod, info->aspectRatio, info->format);
#endif

	// We're emulating tmu0 only
	if ( tmu != GR_TMU0 )
	{
		return;
	}

#ifdef OGL_DEBUG
	if((startAddress & 7 ) != 0 || startAddress < 0 || startAddress > Glide.TexMemoryMaxPosition)
	{
		if (startAddress != GR_NULL_MIPMAP_HANDLE)
		{
			GlideMsg("Warning: grTexSource(%d, %d, %d, xxx) invalid startAddress\n", tmu, startAddress, evenOdd);
		}
	}
#endif
		
	RenderDrawTriangles();
	Glide.State.TexSource.StartAddress = startAddress;
	Glide.State.TexSource.EvenOdd = evenOdd;
	Textures->Source( startAddress, evenOdd, info );    
}

//*************************************************
//* Return the texture memory consumed by a texture
//*************************************************
FX_ENTRY FxU32 FX_CALL
grTexTextureMemRequired( FxU32 dwEvenOdd, GrTexInfo *texInfo )
{
#ifdef OGL_DONE
    GlideMsg( "grTexTextureMemRequired( %u, --- )\n", dwEvenOdd );
#endif

    return Textures->TextureMemRequired( dwEvenOdd, texInfo );
}

//*************************************************
//* Return the texture memory consumed by a texture
//*************************************************
FX_ENTRY void FX_CALL
grTexDownloadMipMap( GrChipID_t tmu,
                     FxU32      startAddress,
                     FxU32      evenOdd,
                     GrTexInfo  *info )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadMipMap(%d, 0x%x, 0x%x, (%u, %u, ar=%u, fmt=%u) )\n",
      tmu, startAddress, evenOdd,
      info->smallLod, info->largeLod, info->aspectRatio, info->format);
#endif

    if ( tmu != GR_TMU0 )
    {
        return;
    }

    RenderDrawTriangles( );

    info->smallLod = info->largeLod;
    Textures->DownloadMipMap( startAddress, evenOdd, info );
}

//*************************************************
FX_ENTRY void FX_CALL
grTexDownloadMipMapLevel( GrChipID_t        tmu,
                          FxU32             startAddress,
                          GrLOD_t           thisLod,
                          GrLOD_t           largeLod,
                          GrAspectRatio_t   aspectRatio,
                          GrTextureFormat_t format,
                          FxU32             evenOdd,
                          void              *data )
{
#ifdef OGL_PARTDONE
	GlideMsg( "grTexDownloadMipMapLevel( %d, 0x%x, %d, %d, %d, %d, 0x%x, --- )\n",
	    tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd );
#endif

	if ( ( tmu != GR_TMU0 ) || ( thisLod != largeLod ) )
	{
		return;
	}
	Textures->DownloadMipMapLevel(startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data);
}

//*************************************************
FX_ENTRY void FX_CALL
grTexDownloadMipMapLevelPartial( GrChipID_t        tmu,
                                 FxU32             startAddress,
                                 GrLOD_t           thisLod,
                                 GrLOD_t           largeLod,
                                 GrAspectRatio_t   aspectRatio,
                                 GrTextureFormat_t format,
                                 FxU32             evenOdd,
                                 void              *data,
                                 int               start,
                                 int               end )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadMipMapLevelPartial( %d, 0x%x, %d, %d, %d, fmt=%d, 0x%x, ---, %d, %d )\n",
        tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, start, end );
#endif
    if ( ( tmu != GR_TMU0 ) || ( thisLod != largeLod ) )
    {
        return;
    }

    Textures->DownloadMipMapLevelPartial(startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data, start, end);
}

//*************************************************
//* Set the texture map clamping/wrapping mode
//*************************************************
FX_ENTRY void FX_CALL
grTexClampMode( GrChipID_t tmu,
                GrTextureClampMode_t s_clampmode,
                GrTextureClampMode_t t_clampmode )
{
	CHECK_STATE_CHANGED( Glide.State.SClampMode == s_clampmode
	                  && Glide.State.TClampMode == t_clampmode);

#ifdef OGL_DONE
	GlideMsg( "grTexClampMode( %d, %d, %d )\n",
			tmu, s_clampmode, t_clampmode );
#endif

	if ( tmu != GR_TMU0 )
	{
		return;
	}

	RenderDrawTriangles( );

	Glide.State.SClampMode = s_clampmode;
	Glide.State.TClampMode = t_clampmode;

	switch ( s_clampmode )
	{
	case GR_TEXTURECLAMP_CLAMP:     OpenGL.SClampMode = InternalConfig.EXT_SGIS_texture_edge_clamp ? GL_CLAMP_TO_EDGE : GL_CLAMP;   break;
	case GR_TEXTURECLAMP_WRAP:      OpenGL.SClampMode = GL_REPEAT;  break;
	}
	switch ( t_clampmode )
	{
	case GR_TEXTURECLAMP_CLAMP:     OpenGL.TClampMode = InternalConfig.EXT_SGIS_texture_edge_clamp ? GL_CLAMP_TO_EDGE : GL_CLAMP;   break;
	case GR_TEXTURECLAMP_WRAP:      OpenGL.TClampMode = GL_REPEAT;  break;
	}
}

void grTexFilterMode_update(GrChipID_t tmu,
                            GrTextureFilterMode_t minfilter_mode,
                            GrTextureFilterMode_t magfilter_mode)
{
	if ( tmu != GR_TMU0 )
	{
		return;
	}

	RenderDrawTriangles( );

	Glide.State.MinFilterMode = minfilter_mode;
	Glide.State.MagFilterMode = magfilter_mode;
	// At least on Rage 128 OpenGL Engine 1.1.ATI-5.99, GL_NEAREST_MIPMAP_LINEAR
	// and GL_NEAREST_MIPMAP_NEAREST seem to cause textures to not be displayed
	// As a result, mipmaps are only available on renderers with EXT_SGIS_generate_mipmap
	FxBool mipmapmode = InternalConfig.Mipmapping; // && InternalConfig.EXT_SGIS_generate_mipmap;
	switch ( minfilter_mode )
	{
	case GR_TEXTUREFILTER_POINT_SAMPLED:
	    if ( ( Glide.State.MipMapMode != GR_MIPMAP_DISABLE ) && mipmapmode)
	    {
				if ( Glide.State.LodBlend )
				{
					OpenGL.MinFilterMode = GL_NEAREST_MIPMAP_LINEAR;
				}
				else
				{
					OpenGL.MinFilterMode = GL_NEAREST_MIPMAP_NEAREST;
				}
	    }
	    else
	    {
				OpenGL.MinFilterMode = GL_NEAREST;
	    }
	    break;

	case GR_TEXTUREFILTER_BILINEAR:
		if (mipmapmode)
		{
			if ( Glide.State.LodBlend )
			{
				OpenGL.MinFilterMode = GL_LINEAR_MIPMAP_LINEAR;
			}
			else
			{
				OpenGL.MinFilterMode = GL_LINEAR_MIPMAP_NEAREST;
			}
		}
		else
		{
			OpenGL.MinFilterMode = GL_LINEAR;
		}
		break;
	}
	switch ( magfilter_mode )
	{
	case GR_TEXTUREFILTER_POINT_SAMPLED:
		OpenGL.MagFilterMode = GL_NEAREST;
		break;
	case GR_TEXTUREFILTER_BILINEAR:
		OpenGL.MagFilterMode = GL_LINEAR;
		break;
	}
}

//*************************************************
//* Set the texture Min/Mag Filter
//*************************************************
FX_ENTRY void FX_CALL
grTexFilterMode( GrChipID_t tmu,
                 GrTextureFilterMode_t minfilter_mode,
                 GrTextureFilterMode_t magfilter_mode)
{
	if (InternalConfig.TextureSmoothing) return;
	CHECK_STATE_CHANGED(Glide.State.MinFilterMode == minfilter_mode
	                 && Glide.State.MagFilterMode == magfilter_mode);
	                  
#ifdef OGL_PARTDONE
	GlideMsg("grTexFilterMode( %d, %d, %d )\n",
	         tmu, minfilter_mode, magfilter_mode);
#endif

	grTexFilterMode_update(tmu, minfilter_mode, magfilter_mode);
}


//*************************************************
//* Set the texture MipMap Mode
//*************************************************
FX_ENTRY void FX_CALL
grTexMipMapMode( GrChipID_t     tmu, 
                 GrMipMapMode_t mode,
                 FxBool         lodBlend )
{
	if (InternalConfig.TextureSmoothing) return;
	CHECK_STATE_CHANGED(Glide.State.MipMapMode == mode
	                 && Glide.State.LodBlend == lodBlend);
	                  
#ifdef OGL_PARTDONE
	GlideMsg("grTexMipMapMode( %d, %d, %d )\n",
	         tmu, mode, lodBlend );
#endif

	if ( tmu != GR_TMU0 )
	{
		return;
	}

	Glide.State.MipMapMode = mode;
	Glide.State.LodBlend = lodBlend;

	grTexFilterMode_update(tmu,
	                       Glide.State.MinFilterMode,
                         Glide.State.MagFilterMode);
}

//*************************************************
//* Returns the memory occupied by a texture
//*************************************************
FX_ENTRY FxU32 FX_CALL
grTexCalcMemRequired( GrLOD_t lodmin, GrLOD_t lodmax,
                      GrAspectRatio_t aspect, GrTextureFormat_t fmt )
{
#ifdef OGL_DONE
    GlideMsg( "grTexCalcMemRequired( %d, %d, %d, %d )\n",
        lodmin, lodmax, aspect, fmt );
#endif

    static GrTexInfo texInfo;
    texInfo.aspectRatio = aspect;
    texInfo.format      = fmt;
    texInfo.largeLod    = lodmax;
    texInfo.smallLod    = lodmin;

    return Textures->TextureMemRequired( 0, &texInfo );
}

//*************************************************
//* Download a subset of an NCC table or color palette
//*************************************************
FX_ENTRY void FX_CALL
grTexDownloadTablePartial( GrChipID_t   tmu,
                           GrTexTable_t type, 
                           void        *data,
                           int          start,
                           int          end )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadTablePartial( %d, %d, ---, %d, %d )\n",
        tmu, type, start, end );
#endif

    if ( tmu != GR_TMU0 )
    {
        return;
    }

    RenderDrawTriangles( );

    Textures->DownloadTable( type, (FxU32*)data, start, end + 1 - start );
}

//*************************************************
//* download an NCC table or color palette
//*************************************************
FX_ENTRY void FX_CALL
grTexDownloadTable( GrChipID_t   tmu,
                    GrTexTable_t type, 
                    void         *data )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadTable( %d, %d, --- )\n", tmu, type );
#endif

    if ( tmu != GR_TMU0 )
    {
        return;
    }

    RenderDrawTriangles( );

    Textures->DownloadTable( type, (FxU32*)data, 0, 256 );
}

//*************************************************
FX_ENTRY void FX_CALL
grTexLodBiasValue( GrChipID_t tmu, float bias )
{
	glReportErrors("grTexLodBiasValue");

#ifdef OGL_NOTDONE
	GlideMsg( "grTexLodBiasValue( %d, %d )\n", tmu, bias );
#endif

	if (tmu != GR_TMU0)
	{
		return;
	}

	CHECK_STATE_CHANGED(bias == Glide.State.LodBias);
	RenderDrawTriangles();

	Glide.State.LodBias = bias;
	if ( InternalConfig.EXT_texture_lod_bias )
	{
		if (OpenGL.ColorAlphaUnit2)
		{
			for(int unit_index = 1; unit_index >= 0; unit_index--)
			{
				glActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
				glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias);
			}
		}
		else
		{
			glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias );
		}
	}
	
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}

//*************************************************
FX_ENTRY void FX_CALL
grTexNCCTable( GrChipID_t tmu, GrNCCTable_t NCCTable )
{
#ifdef OGL_DONE
	GlideMsg( "grTexNCCTable( %d, %u )\n", tmu, NCCTable );
#endif

	// Ignoring TMU as we are only emulating Glide and assuming a well behaviored program
	if ( tmu != GR_TMU0 )
	{
		return;
	}
	
	RenderDrawTriangles();
	Textures->NCCTable( NCCTable );
}

//*************************************************
FX_ENTRY void FX_CALL 
grTexDetailControl( GrChipID_t tmu,
                    int lod_bias,
                    FxU8 detail_scale,
                    float detail_max )
{
	// CHECK_STATE_CHANGED(...)

#ifdef OGL_NOTDONE
    GlideMsg( "grTexDetailControl( %d, %d, %d, %-4.2f )\n",
        tmu, lod_bias, detail_scale, detail_max );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL 
grTexMultibase( GrChipID_t tmu,
                FxBool     enable )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grTexMultibase( %d, %d )\n", tmu, enable );
#endif
	// Ignoring TMU as we are only emulating Glide and assuming a well behaviored program
    if ( tmu != GR_TMU0 )
    {
        return;
    }
}

//*************************************************
FX_ENTRY void FX_CALL
grTexMultibaseAddress( GrChipID_t       tmu,
                       GrTexBaseRange_t range,
                       FxU32            startAddress,
                       FxU32            evenOdd,
                       GrTexInfo        *info )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grTexMultibaseAddress( %d, %d, %lu, %lu, --- )\n",
        tmu, range, startAddress, evenOdd );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grTexCombine( GrChipID_t tmu,
              GrCombineFunction_t rgb_function,
              GrCombineFactor_t rgb_factor, 
              GrCombineFunction_t alpha_function,
              GrCombineFactor_t alpha_factor,
              FxBool rgb_invert,
              FxBool alpha_invert )
{
	CHECK_STATE_CHANGED( Glide.State.TextureCombineCFunction == rgb_function
	                  && Glide.State.TextureCombineCFactor   == rgb_factor
	                  && Glide.State.TextureCombineAFunction == alpha_function
	                  && Glide.State.TextureCombineAFactor   == alpha_factor
	                  && Glide.State.TextureCombineRGBInvert == rgb_invert
	                  && Glide.State.TextureCombineAInvert   == alpha_invert);
	
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
    GlideMsg( "grTexCombine( %d, %d, %d, %d, %d, %d, %d )\n",
        tmu, rgb_function, rgb_factor, alpha_function, alpha_factor, 
        rgb_invert, alpha_invert );
#endif

	// Ignoring TMU as we are only emulating Glide and assuming a well behaviored program
	if ( tmu != GR_TMU0 )
	{
		return;
	}

	RenderDrawTriangles( );

	if (Glide.State.TextureCombineCFunction != rgb_function ||
	    Glide.State.TextureCombineCFactor != rgb_factor)
	{
		Glide.State.TextureCombineCFunction = rgb_function;
		Glide.State.TextureCombineCFactor   = rgb_factor;
		SetColorTextureState();
		SetTextureState();
	}
	if (Glide.State.TextureCombineAFunction != alpha_function ||
	    Glide.State.TextureCombineAFactor != alpha_factor)
	{
		Glide.State.TextureCombineAFunction = alpha_function;
		Glide.State.TextureCombineAFactor   = alpha_factor;
		SetAlphaTextureState();
		SetTextureState();
	}
	if (Glide.State.TextureCombineRGBInvert != rgb_invert)
	{
		Glide.State.TextureCombineRGBInvert = rgb_invert;
		SetColorCombineState();
		SetAlphaCombineState();
	}
	if (Glide.State.TextureCombineAInvert != alpha_invert)
	{
		Glide.State.TextureCombineAInvert = alpha_invert;
		SetAlphaCombineState();
	}
}

//*************************************************
FX_ENTRY void FX_CALL
grTexCombineFunction( GrChipID_t tmu, GrTextureCombineFnc_t func )
{
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
	GlideMsg( "grTexCombineFunction( %d, %d )\n", tmu, func );
#endif

	// Ignoring TMU as we are only emulating Glide and assuming a well behaviored program
	if ( tmu != GR_TMU0 )
	{
		return;
	}

	switch ( func )
	{
	case GR_TEXTURECOMBINE_ZERO:            // 0x00 per component
		grTexCombine( tmu, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO,
			GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO, FXFALSE, FXFALSE );
		break;

	case GR_TEXTURECOMBINE_DECAL:           // Clocal decal texture
		grTexCombine( tmu, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
			GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE );
		break;

	case GR_TEXTURECOMBINE_OTHER:           // Cother pass through
		grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
			GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
		break;

	case GR_TEXTURECOMBINE_ADD:             // Cother + Clocal additive texture
		grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE,
			GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
		break;

	case GR_TEXTURECOMBINE_MULTIPLY:        // Cother * Clocal modulated texture
		grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
			GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, FXFALSE, FXFALSE );
		break;
	
	case GR_TEXTURECOMBINE_SUBTRACT:        // Cother – Clocal subtractive texture
		grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, GR_COMBINE_FACTOR_ONE,
			GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
		break;
	
	case GR_TEXTURECOMBINE_ONE:             // 255 0xFF per component
		grTexCombine( tmu, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO,
			GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO, FXTRUE, FXTRUE );
		break;

//  case GR_TEXTURECOMBINE_DETAIL:          // blend (Cother, Clocal) detail textures with detail on selected TMU
//  case GR_TEXTURECOMBINE_DETAIL_OTHER:    // blend (Cother, Clocal) detail textures with detail on neighboring TMU
//  case GR_TEXTURECOMBINE_TRILINEAR_ODD:   // blend (Cother, Clocal) LOD blended textures with odd levels on selected TMU
//  case GR_TEXTURECOMBINE_TRILINEAR_EVEN:  // blend (Cother, Clocal) LOD blended textures with even levels on selected TMU
//      break;
    }
}

//*************************************************
//* Return the amount of unallocated texture memory on a Texture Mapping Unit
//*************************************************
FX_ENTRY FxU32 FX_CALL 
guTexMemQueryAvail( GrChipID_t tmu )
{
#ifdef OGL_PARTDONE
    GlideMsg( "guTexMemQueryAvail( %d ) = %u\n", tmu, UTextures.MemQueryAvail( tmu ) );
#endif

	// Ignoring TMU as we are only emulating Glide and assuming a well behaviored program
	if ( tmu != GR_TMU0 )
	{
		return 0;
	}

    return UTextures.MemQueryAvail( tmu );
}

//*************************************************
FX_ENTRY void FX_CALL
guTexCombineFunction( GrChipID_t tmu, GrTextureCombineFnc_t func )
{
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
    GlideMsg( "guTexCombineFunction( %d, %d )\n", tmu, func );
#endif

	// Ignoring TMU as we are only emulating Glide and assuming a well behaviored program
	if ( tmu != GR_TMU0 )
	{
		return;
	}
	
	switch ( func )
	{
	case GR_TEXTURECOMBINE_ZERO:            // 0x00 per component
		grTexCombine( tmu, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO,
			GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO, FXFALSE, FXFALSE );
		break;

	case GR_TEXTURECOMBINE_DECAL:           // Clocal decal texture
		grTexCombine( tmu, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
			GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE );
		break;

	case GR_TEXTURECOMBINE_OTHER:           // Cother pass through
		grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
			GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
		break;

	case GR_TEXTURECOMBINE_ADD:             // Cother + Clocal additive texture
		grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE,
			GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
		break;

	case GR_TEXTURECOMBINE_MULTIPLY:        // Cother * Clocal modulated texture
		grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
			GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, FXFALSE, FXFALSE );
		break;

	case GR_TEXTURECOMBINE_SUBTRACT:        // Cother – Clocal subtractive texture
		grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, GR_COMBINE_FACTOR_ONE,
			GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
		break;

	case GR_TEXTURECOMBINE_ONE:             // 255 0xFF per component
		grTexCombine( tmu, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO,
			GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO, FXTRUE, FXTRUE );
		break;

//  case GR_TEXTURECOMBINE_DETAIL:          // blend (Cother, Clocal) detail textures with detail on selected TMU
//  case GR_TEXTURECOMBINE_DETAIL_OTHER:    // blend (Cother, Clocal) detail textures with detail on neighboring TMU
//  case GR_TEXTURECOMBINE_TRILINEAR_ODD:   // blend (Cother, Clocal) LOD blended textures with odd levels on selected TMU
//  case GR_TEXTURECOMBINE_TRILINEAR_EVEN:  // blend (Cother, Clocal) LOD blended textures with even levels on selected TMU
//      break;
    }
}

//*************************************************
FX_ENTRY GrMipMapId_t FX_CALL 
guTexGetCurrentMipMap( GrChipID_t tmu )
{
#ifdef OGL_DONE
    GlideMsg( "guTexGetCurrentMipMap( %d ) = %d\n", tmu, UTextures.GetCurrentMipMap( tmu ) );
#endif

	// Ignoring TMU as we are only emulating Glide and assuming a well behaviored program
	if ( tmu != GR_TMU0 )
	{
		return GR_NULL_MIPMAP_HANDLE;
	}

    return UTextures.GetCurrentMipMap( tmu );
}

//*************************************************
FX_ENTRY FxBool FX_CALL 
guTexChangeAttributes( GrMipMapId_t mmid,
                       int width, int height,
                       GrTextureFormat_t fmt,
                       GrMipMapMode_t mm_mode,
                       GrLOD_t smallest_lod, GrLOD_t largest_lod,
                       GrAspectRatio_t aspect,
                       GrTextureClampMode_t s_clamp_mode,
                       GrTextureClampMode_t t_clamp_mode,
                       GrTextureFilterMode_t minFilterMode,
                       GrTextureFilterMode_t magFilterMode )
{
#ifdef OGL_DONE
    GlideMsg( "guTexChangeAttributes( %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d )\n",
        mmid, width, height, fmt, mm_mode, smallest_lod, largest_lod, aspect,
        s_clamp_mode, t_clamp_mode, minFilterMode, magFilterMode );
#endif

    return UTextures.ChangeAttributes( mmid, width, height, fmt, mm_mode,
        smallest_lod, largest_lod, aspect, s_clamp_mode, t_clamp_mode, 
        minFilterMode, magFilterMode );
}

//*************************************************
FX_ENTRY GrMipMapInfo * FX_CALL 
guTexGetMipMapInfo( GrMipMapId_t mmid )
{
#ifdef OGL_DONE
    GlideMsg( "guTexGetMipMapInfo( ) = 0x%p\n" );
#endif

    return UTextures.GetMipMapInfo( mmid );
}

//*************************************************
FX_ENTRY void FX_CALL 
guTexMemReset( void )
{
#ifdef OGL_PARTDONE
    GlideMsg( "guTexMemReset( )\n" );
#endif

    UTextures.MemReset( );
    Textures->Clear( );
}

//*************************************************
FX_ENTRY void FX_CALL 
guTexDownloadMipMapLevel( GrMipMapId_t mmid, GrLOD_t lod, const void **src )
{
#ifdef OGL_DONE
    GlideMsg( "guTexDownloadMipMapLevel( %d, %d, 0x%p )\n", mmid, lod, src );
#endif

    UTextures.DownloadMipMapLevel( mmid, lod, src );
}

//*************************************************
FX_ENTRY void FX_CALL 
guTexDownloadMipMap( GrMipMapId_t mmid, const void *src, const GuNccTable *table )
{
#ifdef OGL_DONE
    GlideMsg( "guTexDownloadMipMap( %d, 0x%p, 0x%p )\n", mmid, src, table );
#endif

    UTextures.DownloadMipMap( mmid, src, table );
}

//*************************************************
FX_ENTRY GrMipMapId_t FX_CALL 
guTexAllocateMemory( GrChipID_t tmu,
                     FxU8 odd_even_mask,
                     int width, int height,
                     GrTextureFormat_t fmt,
                     GrMipMapMode_t mm_mode,
                     GrLOD_t smallest_lod, GrLOD_t largest_lod,
                     GrAspectRatio_t aspect,
                     GrTextureClampMode_t s_clamp_mode,
                     GrTextureClampMode_t t_clamp_mode,
                     GrTextureFilterMode_t minfilter_mode,
                     GrTextureFilterMode_t magfilter_mode,
                     float lod_bias,
                     FxBool trilinear )
{
#ifdef OGL_DONE
    GlideMsg( "guTexAllocateMemory( %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d )\n",
        tmu, odd_even_mask, width, height, fmt, mm_mode, smallest_lod, largest_lod, aspect,
        s_clamp_mode, t_clamp_mode, minfilter_mode, magfilter_mode, lod_bias, trilinear );
#endif

	// Ignoring TMU as we are only emulating Glide and assuming a well behaviored program
	if ( tmu != GR_TMU0 )
	{
		return GR_NULL_MIPMAP_HANDLE;
	}

    return UTextures.AllocateMemory( tmu, odd_even_mask, width, height, fmt, mm_mode,
        smallest_lod, largest_lod, aspect, s_clamp_mode, t_clamp_mode,
        minfilter_mode, magfilter_mode, lod_bias, trilinear );
}

//*************************************************
FX_ENTRY void FX_CALL 
guTexSource( GrMipMapId_t id )
{
	// Works because the current id isn't changed in UTextures.Source
	// when id is GR_NULL_MIPMAP_HANDLE (todo: this might be a bug)
	CHECK_STATE_CHANGED(id == GR_NULL_MIPMAP_HANDLE || id == UTextures.GetCurrentID());

#ifdef OGL_DONE
    GlideMsg( "guTexSource( %d )\n", id );
#endif

    RenderDrawTriangles();

    UTextures.Source( id );
}

//*************************************************
FX_ENTRY FxU16 * FX_CALL
guTexCreateColorMipMap( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guTexCreateColorMipMap( ) = NULL\n" );
#endif

    return NULL;
}

