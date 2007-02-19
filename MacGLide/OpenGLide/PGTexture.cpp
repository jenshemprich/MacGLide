//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*           implementation of the PGTexture class
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "FormatConversion.h"
#include "Glide.h"
#include "GlideApplication.h"
#include "GlideSettings.h"
#include "Glextensions.h"
#include "GLRender.h"
#include "OGLTables.h"
#include "PGTexture.h"


void PGTexture::genPaletteMipmaps( FxU32 width, FxU32 height, const FxU8 *data )
{
    FxU8    buf[ 128 * 128 ];
    FxU32   mmwidth;
    FxU32   mmheight;
    FxU32   lod;
    FxU32   skip;

    mmwidth = width;
    mmheight = height;
    lod = 0;
    skip = 1;

    while ( ( mmwidth > 1 ) || ( mmheight > 1 ) )
    {
        FxU32   x, 
                y;

        mmwidth = mmwidth > 1 ? mmwidth / 2 : 1;
        mmheight = mmheight > 1 ? mmheight / 2 : 1;
        lod += 1;
        skip *= 2;

        for ( y = 0; y < mmheight; y++ )
        {
            const FxU8* in;
            FxU8* out;

            in = data + width * y * skip;
            out = buf + mmwidth * y;
            for ( x = 0; x < mmwidth; x++ )
            {
                out[ x ] = in[ x * skip ];
            }
        }

        glTexImage2D( GL_TEXTURE_2D, lod, GL_COLOR_INDEX8_EXT, mmwidth, mmheight, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, buf );
    }
}

PGTexture::PGTexture( int mem_size )
{
	m_db = new TexDB( mem_size );
	m_palette_dirty = true;
	m_valid = false;
	m_chromakey_mode = GR_CHROMAKEY_DISABLE;
	m_tex_memory_size = mem_size;
	m_memory = (FxU8*) AllocBuffer(mem_size, sizeof(FxU8));
	if (InternalConfig.EXT_Client_Storage)
	{
		// Alloc 4-times the size of the Glide buffer for texture storage
		m_textureCache = reinterpret_cast<FxU32*>(AllocBuffer(m_tex_memory_size, sizeof(FxU32)));
		// No memory is wasted here because we save the internal OpenGL copy
	}
	else
	{
		m_textureCache = NULL;
	}
	// Needed for subtexturing and internal conversions
	m_tex_temp = reinterpret_cast<FxU32*>(AllocBuffer(256 * 256, sizeof(FxU32)));
	m_ncc_select = GR_NCCTABLE_NCC0;

#ifdef OGL_DEBUG
	Num_565_Tex = 0;
	Num_565_Chromakey_Tex = 0;
	Num_1555_Tex = 0;
	Num_1555_Chromakey_Tex = 0;
	Num_4444_Tex = 0;
	Num_4444_Chromakey_Tex = 0;
	Num_332_Tex = 0;
	Num_8332_Tex = 0;
	Num_Alpha_Tex = 0;
	Num_AlphaIntensity88_Tex = 0;
	Num_AlphaIntensity44_Tex = 0;
	Num_AlphaPalette_Tex = 0;
	Num_Palette_Tex = 0;
	Num_Palette_Chromakey_Tex = 0;
	Num_Intensity_Tex = 0;
	Num_YIQ_Tex = 0;
	Num_AYIQ_Tex = 0;
	Num_Other_Tex = 0;
#endif
}

PGTexture::~PGTexture( void )
{
	FreeBuffer(m_memory);
	FreeBuffer(m_tex_temp);
	if (m_textureCache) FreeBuffer(m_textureCache);
	delete m_db;
}

void PGTexture::DownloadMipMap( FxU32 startAddress, FxU32 evenOdd, const GrTexInfo *info )
{
	const FxU32 mip_size = MipMapMemRequired(info->smallLod, 
	                                         info->aspectRatio, 
	                                         info->format);
	const FxU32 mip_offset = startAddress + TextureMemRequired(evenOdd, info);
	if ( mip_offset <= m_tex_memory_size )
	{
		memcpy( m_memory + mip_offset - mip_size, info->data, mip_size );
	}
	// Any texture based on memory crossing this range
	// is now out of date
	m_db->WipeRange( startAddress, mip_offset, 0 );

#ifdef OGL_DEBUG
	if ( info->smallLod == info->largeLod )
	{
		switch ( info->format )
		{
		case GR_TEXFMT_RGB_332:             Num_332_Tex++;                  break;
		case GR_TEXFMT_YIQ_422:             Num_YIQ_Tex++;                  break;
		case GR_TEXFMT_ALPHA_8:             Num_Alpha_Tex++;                break;
		case GR_TEXFMT_INTENSITY_8:         Num_Intensity_Tex++;            break;
		case GR_TEXFMT_ALPHA_INTENSITY_44:  Num_AlphaIntensity44_Tex++;     break;
		case GR_TEXFMT_P_8:                 m_chromakey_mode
		                                    ? Num_Palette_Chromakey_Tex++
		                                    : Num_Palette_Tex++;
		                                    break;
		case GR_TEXFMT_ARGB_8332:           Num_8332_Tex++;                 break;
		case GR_TEXFMT_AYIQ_8422:           Num_AYIQ_Tex++;                 break;
		case GR_TEXFMT_RGB_565:             m_chromakey_mode
		                                    ? Num_565_Chromakey_Tex++
		                                    : Num_565_Tex++;
		                                    break;
		case GR_TEXFMT_ARGB_1555:           m_chromakey_mode
		                                    ? Num_1555_Chromakey_Tex++
		                                    : Num_1555_Tex++;
		                                    break;
		case GR_TEXFMT_ARGB_4444:           m_chromakey_mode
		                                    ? Num_4444_Chromakey_Tex++
		                                    : Num_4444_Tex++;
		                                    break;
		case GR_TEXFMT_ALPHA_INTENSITY_88:  Num_AlphaIntensity88_Tex++;     break;
		case GR_TEXFMT_AP_88:               Num_AlphaPalette_Tex++;         break;
		case GR_TEXFMT_RSVD0:
		case GR_TEXFMT_RSVD1:
		case GR_TEXFMT_RSVD2:
		default:                            Num_Other_Tex++;                break;
		}
	}
#endif
}

void PGTexture::DownloadMipMapLevel(FxU32             startAddress,
                                    GrLOD_t           thisLod,
                                    GrLOD_t           largeLod,
                                    GrAspectRatio_t   aspectRatio,
                                    GrTextureFormat_t format,
                                    FxU32             evenOdd,
                                    void              *data)
{
	GrTexInfo info;
	info.smallLod       = thisLod;
	info.largeLod       = largeLod;
	info.aspectRatio    = aspectRatio;
	info.format         = format;
	info.data           = data;
	DownloadMipMap(startAddress, evenOdd, &info);
}

void PGTexture::DownloadMipMapLevelPartial(FxU32             startAddress,
                                           GrLOD_t           thisLod,
                                           GrLOD_t           largeLod,
                                           GrAspectRatio_t   aspectRatio,
                                           GrTextureFormat_t format,
                                           FxU32             evenOdd,
                                           void              *data,
                                           int               start,
                                           int               end)
{
	GrTexInfo info;
	info.smallLod       = thisLod;
	info.largeLod       = largeLod;
	info.aspectRatio    = aspectRatio;
	info.format         = format;
	info.data           = data;
	const FxU32 mip_size = MipMapMemRequired(info.smallLod, info.aspectRatio, info.format);
	// instead of forwarding evenOdd, GR_MIPMAPLEVELMASK_BOTH might also be appropriate
	const FxU32 mip_offset = startAddress + TextureMemRequired(evenOdd, &info);
	// Choose the rows to copy (only largeLod is supported)
	TexValues texVals;
	GetTexValues(&texVals);
	const FxU32 texel_size = info.format >= GR_TEXFMT_16BIT ? 2 : 1;
	const FxU32 startRow = texVals.width * texel_size * start;
	const FxU32 rows = texVals.width * texel_size * (end - start);
	if ( mip_offset <= m_tex_memory_size )
	{
		memcpy(m_memory + mip_offset - mip_size + startRow, info.data, rows);
	}
	// Any texture based on memory crossing this range
	// is now out of date
	m_db->WipeRange(startAddress, mip_offset, 0);	
}

void PGTexture::Source(FxU32 startAddress, FxU32 evenOdd, const GrTexInfo *info)
{
	m_startAddress = startAddress;
	m_evenOdd = evenOdd;
	m_info = *info;
	
	m_wAspect = texAspects[ info->aspectRatio ].w;
	m_hAspect = texAspects[ info->aspectRatio ].h;
	
	m_valid = ( ( startAddress + TextureMemRequired( evenOdd, info ) ) <= m_tex_memory_size );
}

void PGTexture::DownloadMipmapsToOpenGL(GLint compnum, GLint compformat, GLenum comptype, const void* texdata, TexValues& t, bool build_mipmaps)
{
	glReportErrors("DownloadMipmapsToOpenGL");
	
	glTexImage2D(GL_TEXTURE_2D, t.lod, compnum, t.width, t.height, 0, compformat, comptype, texdata);
	if (InternalConfig.Mipmapping && build_mipmaps)
	{
		gluBuild2DMipmaps(GL_TEXTURE_2D, compnum, t.width, t.height, compformat, comptype, texdata);
	}
	glReportError();
}

inline void ncc_convert_FxI9_2_FxI16(FxI16* ncc_xRGB)
{
	if (*ncc_xRGB & 0x100)
		*ncc_xRGB |= 0xff00;
}

void PGTexture::DownloadTable( GrTexTable_t type, const FxU32 *data, int first, int count )
{
	if ( type == GR_TEXTABLE_PALETTE )
	{
		for ( int i = count - 1; i >= 0; i-- )
		{
			// Convert palette entry from ARGB to RGBA
			const FxU32& src = data[ i ];
			m_palette[ first + i ] = ((src & 0x00ffffff) << 8) | // RGB
			                         ((src & 0xff000000) >> 24); // A
		}
		m_palette_dirty = true;
	}
	else
	{
		// GR_TEXTABLE_NCC0 or GR_TEXTABLE_NCC1
		GuNccTable *ncc = &(m_ncc[ type ]);
		memcpy( ncc, data, sizeof( GuNccTable ) );
		for (int i = 0; i < 4; i++ )
		{
			ncc_convert_FxI9_2_FxI16(&ncc->iRGB[ i ][ 0 ]);
			ncc_convert_FxI9_2_FxI16(&ncc->iRGB[ i ][ 1 ]);
			ncc_convert_FxI9_2_FxI16(&ncc->iRGB[ i ][ 2 ]);
			ncc_convert_FxI9_2_FxI16(&ncc->qRGB[ i ][ 0 ]);
			ncc_convert_FxI9_2_FxI16(&ncc->qRGB[ i ][ 1 ]);
			ncc_convert_FxI9_2_FxI16(&ncc->qRGB[ i ][ 2 ]);
		 }
	}
}

bool PGTexture::MakeReady(TTextureStruct* tex_coords, unsigned long number_of_triangles)
{
	glReportErrors("PGTexture::MakeReady");
	if( ! m_valid )
	{
		return false;
	}

	FxU32 test_hash = 0;
	FxU32 wipe_hash = 0;
	bool palette_changed  = false;
	bool use_mipmap_ext = InternalConfig.Mipmapping && InternalConfig.EXT_SGIS_generate_mipmap;
	bool use_two_textures = false;
	bool* pal_change_ptr = NULL;
	const FxU8* data = m_memory + m_startAddress;
	FxU32 size = TextureMemRequired(m_evenOdd, &m_info);

	TexValues texVals;
	GetTexValues(&texVals);
	switch (m_info.format)
	{
	case GR_TEXFMT_P_8:
		ApplyKeyToPalette();
		// todo: Would work with anisotropic filtering if chromakeying was disabled
		if (InternalConfig.EXT_paletted_texture && InternalConfig.AnisotropylLevel < 2)
		{
			//
			// OpenGL's mipmap generation doesn't seem
			// to handle paletted textures.
			//
			use_mipmap_ext = false;
			pal_change_ptr = &palette_changed;
		}
		else
		{
			wipe_hash = m_palette_hash;
		}
		test_hash = m_palette_hash;
		break;
	case GR_TEXFMT_AP_88:
		ApplyKeyToPalette();
		if (InternalConfig.EXT_paletted_texture && InternalConfig.ARB_multitexture)
		{
			use_mipmap_ext   = false;
			pal_change_ptr   = &palette_changed;
			// Two textures can only be used with the simple coloralpha engine
			// and if there are enough texture units left to support fog
			use_two_textures = (OpenGL.ColorAlphaUnit2 == 0) && (OpenGL.FogTextureUnit >= GL_TEXTURE1_ARB);
		}
		else
		{
			wipe_hash = m_palette_hash;
		}
		test_hash = m_palette_hash;
		break;
	}
	// Look if we already have an OpenGL texture to match this
	TexDB::Record* texture_record;
	SubTexCoord_t subtexcoords_struct;
	const bool generate_sub_textures = InternalConfig.GenerateSubTextures &&
	                                   OpenGL.SClampMode != GL_REPEAT &&
	                                   OpenGL.TClampMode != GL_REPEAT;
	SubTexCoord_t* subtexcoords = generate_sub_textures ? &subtexcoords_struct : NULL;
	if (subtexcoords)
	{
		assert(number_of_triangles == 1);
		GLfloat min;
		GLfloat max;
		const unsigned int grid_snap = 0xffffffff - (InternalConfig.GenerateSubTextures -1);
		// Get Glide pixel coordinates (prepared in RenderAddTriangle())
		const GLfloat as = tex_coords->as;
		const GLfloat bs = tex_coords->bs;
		const GLfloat cs = tex_coords->cs;
		max = as;
		min = bs;
		if (max < min)
		{
			max = bs;
			min = as;
		}
		if (max < cs)
		{
			max = cs;
		}
		else if (min > cs)
		{
			min = cs;
		}
		subtexcoords->smin = min;
		subtexcoords->smax = max;
		subtexcoords->left = min;
		// snap to grid to allow using the left/top values as keys for caching
		subtexcoords->left &= grid_snap; // snap to 8 pixel grid
		subtexcoords->smin = subtexcoords->left;
		// same for t
		const GLfloat at = tex_coords->at;
		const GLfloat bt = tex_coords->bt;
		const GLfloat ct = tex_coords->ct;
		max = at;
		min = bt;
		if (max < min)
		{
			max = bt;
			min = at;
		}
		if (max < ct)
		{
			max = ct;
		}
		else if (min > ct)
		{
			min = ct;
		}
		subtexcoords->tmin = min;
		subtexcoords->tmax = max;
		subtexcoords->top = min;
		subtexcoords->top &= grid_snap; // snap to 8 pixel grid
		subtexcoords->tmin = subtexcoords->top;
		// Calculate width and height of the texture
		subtexcoords->width = subtexcoords->smax - subtexcoords->left;
		subtexcoords->height = subtexcoords->tmax - subtexcoords->top;
		subtexcoords->texImageWidth = max(InternalConfig.GenerateSubTextures, PowerOfTwoCeiling(subtexcoords->width));
		subtexcoords->texImageHeight = max(InternalConfig.GenerateSubTextures, PowerOfTwoCeiling(subtexcoords->height));
		texture_record = texture_record = m_db->Find(m_startAddress, &m_info, test_hash, pal_change_ptr, subtexcoords);
		// At this point the subtexcoord texsize might have been updated (by Find())
		// Calculate texture coordinates for rendering the subtexture
		// 1. scale a->s to texture pixel s-space: as = a->tmuvtx[ 0 ].sow / atmuoow;
		// 2. shift as to origin: as_s = as - subtexcoords.smin;
		// 3. scale as_s to opengl texture space as_s *= wAspect / subtexcoords.width;
		// 4. scale as_s to opengl subtexture space: * subtexcoords.width / subtexcoords.texImageWidth
		// 5. undo dividing by atmoow and scale with maxoow to fit texture edge into object edges
		// tex_coords->aoow is actually maxoow * aoow what is what we want
		const GLfloat s_factor = 1.0f / subtexcoords->texImageWidth;
		tex_coords->as = (as - subtexcoords->smin) * s_factor * tex_coords->aoow;
		tex_coords->bs = (bs - subtexcoords->smin) * s_factor * tex_coords->boow;
		tex_coords->cs = (cs - subtexcoords->smin) * s_factor * tex_coords->coow;
		const GLfloat t_factor = 1.0f / subtexcoords->texImageHeight;
		tex_coords->at = (at - subtexcoords->tmin) * t_factor * tex_coords->aoow; 
		tex_coords->bt = (bt - subtexcoords->tmin) * t_factor * tex_coords->boow;
		tex_coords->ct = (ct - subtexcoords->tmin) * t_factor * tex_coords->coow;
#ifdef OGL_UTEX
			GlideMsg("glTexCoords(%g,%g)-(%g,%g)-(%g,%g), ",
			         tex_coords->as,
			         tex_coords->at,
			         tex_coords->bs,
			         tex_coords->bt,
			         tex_coords->cs,
			         tex_coords->ct);
			GlideMsg("Subtex: min/max(%g,%g)-(%g,%g),",
			         subtexcoords->smin,
			         subtexcoords->smax,
			         subtexcoords->tmin,
			         subtexcoords->tmax);
			GlideMsg(" w/h(%g,%g), texture(%d,%d)-(%d,%d)\n",
			         subtexcoords->width,
			         subtexcoords->height,
			         subtexcoords->left,
			         subtexcoords->top,
			         subtexcoords->texImageWidth,
			         subtexcoords->texImageHeight);
#endif
	}
	else
	{
		texture_record = texture_record = m_db->Find(m_startAddress, &m_info, test_hash, pal_change_ptr, NULL);
	}
#ifdef OGL_DEBUG
	if (InternalConfig.EXT_paletted_texture == false && palette_changed)
	{
		GlideMsg("Performance Warning: PGTexture palette change for 0x%x causes texture regeneration\n", m_startAddress);
	}
#endif

	const bool enable_coloralpha_texture_unit_1 = OpenGL.ColorAlphaUnit2 == NULL ||
	                                        OpenGL.ColorAlphaUnitColorEnabledState[0] ||
	                                        OpenGL.ColorAlphaUnitAlphaEnabledState[0];
	const bool enable_coloralpha_texture_unit_2 = OpenGL.ColorAlphaUnitColorEnabledState[1] ||
	                                        OpenGL.ColorAlphaUnitAlphaEnabledState[1];
	if (texture_record)
	{
		if (palette_changed && InternalConfig.EXT_paletted_texture)
		{
			glColorTable(GL_TEXTURE_2D, GL_RGBA, 256, GL_RGBA, GL_UNSIGNED_BYTE, m_palette);
			glReportError();
		}
		GLint texnum2;
		if (OpenGL.ColorAlphaUnit2 && enable_coloralpha_texture_unit_2)
		{
			texnum2 = texture_record->texNum;
		}
		else if (use_two_textures)
		{
			texnum2 = texture_record->tex2Num;
		}
		else
		{
			texnum2 = 0;
		}
		if (texnum2)
		{
			glActiveTextureARB(OpenGL.ColorAlphaUnit1 + 1);
			glBindTexture(GL_TEXTURE_2D, texnum2);
			// Only needed if the two textures are different
			if (use_two_textures)
			{
				if (texture_record->SClampMode != OpenGL.SClampMode)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_record->SClampMode);
				}
				if (texture_record->TClampMode != OpenGL.TClampMode)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_record->TClampMode);
				}
				if (InternalConfig.TextureSmoothing == false)
				{
					if (texture_record->MinFilterMode != OpenGL.MinFilterMode)
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_record->MinFilterMode);
					}
					if (texture_record->MagFilterMode != OpenGL.MagFilterMode)
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_record->MagFilterMode);
					}
				}
			}
			glActiveTextureARB(OpenGL.ColorAlphaUnit1);
			glReportError();
		}
		if (enable_coloralpha_texture_unit_1)
		{
			glBindTexture(GL_TEXTURE_2D, texture_record->texNum);
			if (texture_record->SClampMode != OpenGL.SClampMode)
			{
				texture_record->SClampMode = OpenGL.SClampMode;
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL.SClampMode);
			}
			if (texture_record->TClampMode != OpenGL.TClampMode)
			{
				texture_record->TClampMode = OpenGL.TClampMode;
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL.TClampMode);
			}
			if (InternalConfig.TextureSmoothing == false)
			{
				if (texture_record->MinFilterMode != OpenGL.MinFilterMode)
				{
					texture_record->MinFilterMode = OpenGL.MinFilterMode;
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGL.MinFilterMode);
				}
				if (texture_record->MagFilterMode != OpenGL.MagFilterMode)
				{
					texture_record->MagFilterMode = OpenGL.MagFilterMode;
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGL.MagFilterMode);
				}
			}
			glReportError();
		}
	}
	else
	{
		// Any existing textures crossing this memory range
		// is unlikely to be used, so remove the OpenGL version
		// of them
		if (wipe_hash)
		{
			// If we don't have GL_PALETTE_EXT, the texture must be rebuilt
			// after the palette has changed. Any existing textures must be deleted.
			// This is because Find() above was looking for a texture with
			// the current palette hash and might not have found it, although
			// a texture with a different palette exists.
			// @todo: Find doesn't return handles to TexDB records, so we have
			// to search again every time a paletted texture is used -> optimise.
			if (subtexcoords)
			{
				// Fall back to non-subtexture behaviour and just
				//  wipe the range if the texture doesn't exist at all
				if (m_db->Find(m_startAddress, &m_info, test_hash, pal_change_ptr, NULL) == NULL)
				{
					m_db->WipeRange(m_startAddress, m_startAddress + size, wipe_hash);
				}
			}
		}
		else
		{
			// Nothing needs to be done because if it's not a paletted texture,
			// the range has been wiped while downloading the texture
			// m_db->WipeRange( m_startAddress, m_startAddress + size, wipe_hash);
		}
		// Add the new texture to the data base
		TexDB::TextureMode texturemode = subtexcoords ? (use_two_textures ? TexDB::SubTextureTwo : TexDB::SubTextureOne) : (use_two_textures ? TexDB::Two : TexDB::One);
		texture_record = m_db->Add(m_startAddress, m_startAddress + size, &m_info, test_hash, texturemode, subtexcoords);
		texture_record->SClampMode = OpenGL.SClampMode;
		texture_record->TClampMode = OpenGL.TClampMode;
		texture_record->MinFilterMode = OpenGL.MinFilterMode;
		texture_record->MagFilterMode = OpenGL.MagFilterMode;
		glBindTexture( GL_TEXTURE_2D, texture_record->texNum);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL.SClampMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL.TClampMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGL.MinFilterMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGL.MagFilterMode);
		const unsigned long anisotropy_level = InternalConfig.AnisotropylLevel;
		const bool enable_mipmaps = InternalConfig.Mipmapping;
		// Write only if enabled in config
		if(InternalConfig.AnisotropylLevel > 1)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy_level);
		}
		if(use_mipmap_ext)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, enable_mipmaps);
		}
		GLint texnum2;
		if (OpenGL.ColorAlphaUnit2 && enable_coloralpha_texture_unit_2)
		{
			texnum2 = texture_record->texNum;
		}
		else if (use_two_textures)
		{
			texnum2 = texture_record->tex2Num;
		}
		else
		{
			texnum2 = 0;
		}
		if (texnum2)
		{
			glActiveTextureARB(OpenGL.ColorAlphaUnit1 + 1);
			glBindTexture(GL_TEXTURE_2D, texnum2);
			// Only needed if the two textures are different
			if (use_two_textures)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL.SClampMode);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL.TClampMode);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGL.MinFilterMode);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGL.MagFilterMode);
				if(InternalConfig.AnisotropylLevel > 1)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy_level);
				}
				if (use_mipmap_ext)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, enable_mipmaps);
				}
			}
			glActiveTextureARB(OpenGL.ColorAlphaUnit1);
		}
		glReportError();	

		// use client storage to avoid OpenGL-internal copy 
		// OpenGL may still make a copy if the color format isn't supported natively
		// by the graphics card but all xto8888 conversions should benefit from this
		// @todo: Not true for OSX, but for now we stay compatible to native OS9
		const bool useClientStorage = InternalConfig.EXT_Client_Storage /* && !subtexcoords */;
		if (subtexcoords)
		{
			// EXT_Client_Storage doesn't explicitely forbid to adjust pixel unpack :^)
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, subtexcoords->left);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, subtexcoords->top);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, texVals.width);
			// Must update texVals.width, texVals.height to new texture size
			texVals.width = subtexcoords->texImageWidth;
			texVals.height = subtexcoords->texImageHeight;
		}
		FxU32* texBuffer;
		// Which buffer
		if (useClientStorage)
		{
			texBuffer = &m_textureCache[m_startAddress];
			// glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, true);
			// glReportError();		
		}
		else
		{
			texBuffer = m_tex_temp;
		}
		// Convert Glide texture data to format understood by OpenGL
		switch (m_info.format)
		{
		case GR_TEXFMT_RGB_565:
			if (m_chromakey_mode)
			{
				// Read about anisotropy and chromakey issues in macFormatConversions.cpp
				Convert565Kto8888((FxU16*)data, m_chromakey_value_565, texBuffer, texVals.nPixels);
				DownloadMipmapsToOpenGL(4, GL_RGBA, GL_UNSIGNED_BYTE, m_tex_temp, texVals, !use_mipmap_ext);          	
			}
			else
			{
				DownloadMipmapsToOpenGL(3, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data, texVals, !use_mipmap_ext);
			}
			break;
		case GR_TEXFMT_ARGB_4444:
			DownloadMipmapsToOpenGL(4, GL_BGRA_EXT, GL_UNSIGNED_SHORT_4_4_4_4_REV, data, texVals, !use_mipmap_ext);
			break;
		case GR_TEXFMT_ARGB_1555:
			if (m_chromakey_mode)
			{
				// Read about anisotropy and chromakey issues in macFormatConversions.cpp
				Convert1555Kto8888((FxU16*) data, m_chromakey_value_1555, texBuffer, texVals.nPixels);
				DownloadMipmapsToOpenGL(4, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer, texVals, !use_mipmap_ext);          	
			}
			else
			{
				DownloadMipmapsToOpenGL(4, GL_BGRA_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV, data, texVals, !use_mipmap_ext);
			}
			break;
		case GR_TEXFMT_P_8:
			// Read about anisotropy and chromakey issues in macFormatConversions.cpp
			if ( InternalConfig.EXT_paletted_texture && InternalConfig.AnisotropylLevel < 2)
			{
				glColorTable(GL_TEXTURE_2D, GL_RGBA, 256, GL_RGBA, GL_UNSIGNED_BYTE, m_palette);
				glTexImage2D( GL_TEXTURE_2D, texVals.lod, GL_COLOR_INDEX8_EXT, 
				              texVals.width, texVals.height, 0, 
				              GL_COLOR_INDEX, GL_UNSIGNED_BYTE, data );
				if (InternalConfig.Mipmapping && !use_mipmap_ext)
				{
					genPaletteMipmaps(texVals.width, texVals.height, data);
				}
				glReportError();
			}
			else
			{
				if (InternalConfig.AnisotropylLevel >= 2)
				{
					// minimise anisotropy artefacts
					ConvertP8Kto8888(data, m_chromakey_value_8888, texBuffer, texVals.nPixels, m_palette);
					DownloadMipmapsToOpenGL(4, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer, texVals, !use_mipmap_ext);
				}
				else
				{
					ConvertP8to8888(data, texBuffer, texVals.nPixels, m_palette);
					DownloadMipmapsToOpenGL(4, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer, texVals, !use_mipmap_ext);
				}
			}
			break;
	    case GR_TEXFMT_AP_88:
			if ( use_two_textures )
			{
				FxU32 *texBuffer2 = texBuffer + 256 * 128;
				glColorTable(GL_TEXTURE_2D, GL_RGBA, 256, GL_RGBA, GL_UNSIGNED_BYTE, m_palette);
				SplitAP88((FxU16*) data, (FxU8*) texBuffer, (FxU8*) texBuffer2, texVals.nPixels);
				glTexImage2D(GL_TEXTURE_2D, texVals.lod, GL_COLOR_INDEX8_EXT, 
				             texVals.width, texVals.height, 0, 
				             GL_COLOR_INDEX, GL_UNSIGNED_BYTE, texBuffer);
				if (InternalConfig.Mipmapping && !use_mipmap_ext)
				{
				    genPaletteMipmaps(texVals.width, texVals.height, (FxU8*) texBuffer);
				}
				glActiveTextureARB(OpenGL.ColorAlphaUnit1 + 1);
				DownloadMipmapsToOpenGL( GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, texBuffer2, texVals, !use_mipmap_ext);
				glActiveTextureARB(OpenGL.ColorAlphaUnit1);
				glReportError();
			}
			else
			{
				ConvertAP88to8888((FxU16*) data, texBuffer, texVals.nPixels, m_palette );
				DownloadMipmapsToOpenGL(4, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer, texVals, !use_mipmap_ext);
			}
			break;
		case GR_TEXFMT_ALPHA_8:
			ConvertA8toAP88((FxU8*)data, (FxU16*) texBuffer, texVals.nPixels);
			DownloadMipmapsToOpenGL(2, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texBuffer, texVals, !use_mipmap_ext);
			// @todo: The statement below breaks the overlay texts in Myth TFL.
			// As a result, this optimsation has been undone for now
			// DownloadMipmapsToOpenGL(1, GL_ALPHA, GL_UNSIGNED_BYTE, data, texVals, !use_mipmap_ext);
			break;
		case GR_TEXFMT_ALPHA_INTENSITY_88:
			DownloadMipmapsToOpenGL(2, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data, texVals, !use_mipmap_ext);
			break;
		case GR_TEXFMT_INTENSITY_8:
			DownloadMipmapsToOpenGL(1, GL_LUMINANCE, GL_UNSIGNED_BYTE, data, texVals, !use_mipmap_ext);
			break;
		case GR_TEXFMT_ALPHA_INTENSITY_44:
			// @todo: untested
			// DownloadMipmapsToOpenGL(GL_LUMINANCE4_ALPHA4, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data, &texVals);
			// @todo: untested
			ConvertAI44toAP88((FxU8*)data, (FxU16*)texBuffer, texVals.nPixels);
			DownloadMipmapsToOpenGL(2, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texBuffer, texVals, !use_mipmap_ext);
			/*
			ConvertAI44toAP88((FxU8*) data, (FxU16*) texBuffer, texVals.nPixels );
			glTexImage2D(GL_TEXTURE_2D, texVals.lod, 2, texVals.width, texVals.height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texBuffer);
			if (InternalConfig.Mipmapping && !use_mipmap_ext)
			{
			gluBuild2DMipmaps(GL_TEXTURE_2D, 2, texVals.width, texVals.height, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texBuffer);
			}
			glReportError();
			*/
			break;
		case GR_TEXFMT_8BIT: //GR_TEXFMT_RGB_332
			DownloadMipmapsToOpenGL(3, GL_RGB, GL_UNSIGNED_BYTE_3_3_2_EXT, data, texVals, !use_mipmap_ext);
			break;
		case GR_TEXFMT_16BIT: //GR_TEXFMT_ARGB_8332:
			Convert8332to8888( (FxU16*)data, texBuffer, texVals.nPixels );
			DownloadMipmapsToOpenGL(4, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer, texVals, !use_mipmap_ext);
			break;
		case GR_TEXFMT_YIQ_422:
			ConvertYIQto8888((FxU8*) data, texBuffer, texVals.nPixels, &(m_ncc[m_ncc_select]));
			// @todo: Should just be RGB in order to apply constant alpha, shouldn't it?
			DownloadMipmapsToOpenGL(4, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer, texVals, !use_mipmap_ext);
			break;
		case GR_TEXFMT_AYIQ_8422:
			ConvertAYIQto8888((FxU16*) data, texBuffer, texVals.nPixels, &(m_ncc[m_ncc_select]));
			DownloadMipmapsToOpenGL(4, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer, texVals, !use_mipmap_ext);
			break;
	    case GR_TEXFMT_RSVD0:
	    case GR_TEXFMT_RSVD1:
	    case GR_TEXFMT_RSVD2:
	    default:
			GlideMsg("Error: grTexDownloadMipMapLevel - Unsupported format(%d)\n", m_info.format);
			memset(texBuffer, 255, texVals.nPixels * 2);
			DownloadMipmapsToOpenGL(1, GL_LUMINANCE, GL_UNSIGNED_BYTE, texBuffer, texVals, !use_mipmap_ext);
			break;
		}
		// Cleanup
		// if (useClientStorage)
		// {
		// 	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, false);
		// 	glReportError();		
		// }
		if (subtexcoords)
		{
			// restore default values
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}
	}

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
	return use_two_textures;
}

FxU32 PGTexture::LodOffset( FxU32 evenOdd, const GrTexInfo *info )
{
    FxU32   total = 0;
    GrLOD_t i;

    for( i = info->largeLod; i < info->smallLod; i++ )
    {
        total += MipMapMemRequired( i, info->aspectRatio, info->format );
    }

    total = ( total + 7 ) & ~7;

    return total;
}

FxU32 PGTexture::TextureMemRequired( FxU32 evenOdd, const GrTexInfo *info )
{
    //
    // If the format is one of these:
    // GR_TEXFMT_RGB_332, GR_TEXFMT_YIQ_422, GR_TEXFMT_ALPHA_8
    // GR_TEXFMT_INTENSITY_8, GR_TEXFMT_ALPHA_INTENSITY_44, GR_TEXFMT_P_8
    // Reduces the size by 2
    //
    return nSquareTexLod[ info->format < GR_TEXFMT_16BIT ][ info->aspectRatio ][ info->largeLod ][ info->smallLod ];
}

FxU32 PGTexture::MipMapMemRequired( GrLOD_t lod, GrAspectRatio_t aspectRatio, GrTextureFormat_t format )
{
	//
	// If the format is one of these:
	// GR_TEXFMT_RGB_332, GR_TEXFMT_YIQ_422, GR_TEXFMT_ALPHA_8
	// GR_TEXFMT_INTENSITY_8, GR_TEXFMT_ALPHA_INTENSITY_44, GR_TEXFMT_P_8
	// Reduces the size by 2
	//
	return nSquareLod[ format >= GR_TEXFMT_16BIT ][ aspectRatio ][ lod ];
}

void PGTexture::GetTexValues( TexValues * tval ) const
{
	tval->width = texInfo[ m_info.aspectRatio ][ m_info.largeLod ].width;
	tval->height = texInfo[ m_info.aspectRatio ][ m_info.largeLod ].height;
	tval->nPixels = texInfo[ m_info.aspectRatio ][ m_info.largeLod ].numPixels;
	tval->lod = 0;
}

void PGTexture::ChromakeyValue( GrColor_t value )
{
	m_chromakey_value_8888 = (value & 0xffffff00); // RGB, Alpha ommited
	m_chromakey_value_565  = (FxU16) (( value & 0xF8000000 ) >> 16 |
	                                  ( value & 0x00FC0000 ) >> 13 |
	                                  ( value & 0x0000F800 ) >> 11 );
	m_chromakey_value_1555 = (FxU16) (( value & 0xf8000000 ) >> 17 |
	                                  ( value & 0x00f80000 ) >> 14 |
	                                  ( value & 0x0000f800 ) >> 11 );
	m_palette_dirty = true;
}

void PGTexture::ChromakeyMode( GrChromakeyMode_t mode )
{
    m_chromakey_mode = mode;
    m_palette_dirty = true;
}

void PGTexture::ApplyKeyToPalette( void )
{
	FxU32   hash;
	int     i;
	if ( m_palette_dirty )
	{
	    hash = 0;
	      {
	        for ( i = 0; i < 256; i++ )
	        {
	            if ( ( m_chromakey_mode )
	              && ( ( m_palette[i] & 0xffffff00 ) == m_chromakey_value_8888))
	            {
	               m_palette[i] &= 0xffffff00;
	            }
	            else
	            {
	               m_palette[i] |= 0x000000ff;
	            }
	            hash = ( ( hash << 5 ) | ( hash >> 27 ) );
	            hash += ( InternalConfig.IgnorePaletteChange
	                      ? ( m_palette[ i ] & 0x000000ff  )
	                      : m_palette[ i ]);
	        }
	      }
	    m_palette_hash = hash;
	    m_palette_dirty = false;
	}
}

void PGTexture::NCCTable( GrNCCTable_t tab )
{
    switch ( tab )
    {
    case GR_NCCTABLE_NCC0:
    case GR_NCCTABLE_NCC1:
        m_ncc_select = tab;
    }
}

FxU32 PGTexture::GetMemorySize( void ) const
{
    return m_tex_memory_size;
}

unsigned int PGTexture::PowerOfTwoCeiling(unsigned int x)
{
	for(unsigned int potc= 1; potc < 0x0fffffff; potc <<= 1)
	{
		if (potc >= x) return potc;
	}
	return 0;
}
