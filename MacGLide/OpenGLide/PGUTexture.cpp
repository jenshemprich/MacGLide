//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*           implementation of the PGUexture class
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "PGTexture.h"
#include "PGUTexture.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PGUTexture::PGUTexture( void )
{
    for ( int i = 0; i < MAX_MM; i++ )
    {
        mm_info[ i ].valid = false;
    }

    m_free_mem = 0;
    m_free_id = 0;
    m_current_id = GR_NULL_MIPMAP_HANDLE;
}

PGUTexture::~PGUTexture( void )
{
}

GrMipMapId_t PGUTexture::AllocateMemory( GrChipID_t tmu, FxU8 odd_even_mask, 
                                         int width, int height,
                                         GrTextureFormat_t fmt, GrMipMapMode_t mm_mode,
                                         GrLOD_t smallest_lod, GrLOD_t largest_lod, 
                                         GrAspectRatio_t aspect,
                                         GrTextureClampMode_t s_clamp_mode, 
                                         GrTextureClampMode_t t_clamp_mode,
                                         GrTextureFilterMode_t minfilter_mode, 
                                         GrTextureFilterMode_t magfilter_mode,
                                         float lod_bias, FxBool trilinear )
{
    FxU32   size = 0;
    GrLOD_t lod;

    for ( lod = largest_lod; lod <= smallest_lod; lod++ )
    {
        size += PGTexture::MipMapMemRequired( lod, aspect, fmt );
    }

    size = ((size + 7) & ~7);

#ifdef OGL_UTEX
    GlideMsg( "Allocate id = %d size = %d\n", m_free_id, size );
#endif

    if ( ( m_free_id >= MAX_MM ) || 
         ( ( m_free_mem + size ) >= Textures->GetMemorySize( ) ) )
    {
#ifdef OGL_UTEX
        GlideMsg("Allocation failed\n");
#endif
        return GR_NULL_MIPMAP_HANDLE;
    }

    mm_info[ m_free_id ].tmu            = tmu;
    mm_info[ m_free_id ].odd_even_mask  = odd_even_mask;
    mm_info[ m_free_id ].width          = width;
    mm_info[ m_free_id ].height         = height;
    mm_info[ m_free_id ].format         = fmt;
    mm_info[ m_free_id ].mipmap_mode    = mm_mode;
    mm_info[ m_free_id ].lod_min        = smallest_lod;
    mm_info[ m_free_id ].lod_max        = largest_lod;
    mm_info[ m_free_id ].aspect_ratio   = aspect;
    mm_info[ m_free_id ].s_clamp_mode   = s_clamp_mode;
    mm_info[ m_free_id ].t_clamp_mode   = t_clamp_mode;
    mm_info[ m_free_id ].minfilter_mode = minfilter_mode;
    mm_info[ m_free_id ].magfilter_mode = magfilter_mode;
    mm_info[ m_free_id ].lod_bias       = lod_bias;
    mm_info[ m_free_id ].trilinear      = trilinear;
    mm_info[ m_free_id ].valid          = FXTRUE;

    mm_start[ m_free_id ] = m_free_mem;

    m_free_mem += size;

    return m_free_id++;
}

void PGUTexture::DownloadMipMap( GrMipMapId_t mmid, const void *src, const GuNccTable *table )
{
#ifdef OGL_UTEX
    GlideMsg("Download id = %d ", mmid);
#endif

    if ( ( mmid >=0 ) && ( mmid < MAX_MM ) && ( mm_info[ mmid ].valid ) )
    {
        GrTexInfo info;

        info.aspectRatio = mm_info[ mmid ].aspect_ratio;
        info.format      = mm_info[ mmid ].format;
        info.largeLod    = mm_info[ mmid ].lod_max;
        info.smallLod    = mm_info[ mmid ].lod_min;
        info.data        = (void *)src;
        if (info.format == GR_TEXFMT_YIQ_422 ||
            info.format == GR_TEXFMT_AYIQ_8422)
        {
	        memcpy(&mm_info[ mmid ].ncc_table, table, sizeof(GuNccTable));
	    }
	      
#ifdef OGL_UTEX
        {
            FxU32 size = 0;

            for ( GrLOD_t lod = info.largeLod; lod <= info.smallLod; lod++ )
            {
                size += PGTexture::MipMapMemRequired( lod, info.aspectRatio, info.format );
            }

            GlideMsg( "size = %d\n", size );
        }
#endif

        grTexDownloadMipMap( mm_info[mmid].tmu, mm_start[ mmid ], mm_info[ mmid ].odd_even_mask, &info );
    }
#ifdef OGL_UTEX
    else
    {
        GlideMsg( "failed\n" );
    }
#endif
}

void PGUTexture::DownloadMipMapLevel( GrMipMapId_t mmid, GrLOD_t lod, const void **src )
{
#ifdef OGL_NOTDONE
	GlideMsg("PGUTexture::DownloadMipMapLevel(...)\n");
#endif
}

void PGUTexture::MemReset( void )
{
#ifdef OGL_UTEX
    GlideMsg("Reset\n");
#endif

    for ( int i = 0; i < MAX_MM; i++ )
    {
        mm_info[ i ].valid = false;
    }

    m_free_mem = 0;
    m_free_id = 0;
    m_current_id = GR_NULL_MIPMAP_HANDLE;
}

void PGUTexture::Source( GrMipMapId_t id )
{
	if ( ( id >=0 ) && ( id < MAX_MM ) && ( mm_info[ id ].valid ) )
	{
		GrTexInfo info;
		
		info.aspectRatio = mm_info[ id ].aspect_ratio;
		info.format      = mm_info[ id ].format;
		info.largeLod    = mm_info[ id ].lod_max;
		info.smallLod    = mm_info[ id ].lod_min;
		
		int tmu = mm_info[ id ].tmu;
		grTexSource( tmu, mm_start[ id ], mm_info[ id ].odd_even_mask, &info );
		grTexFilterMode( tmu, mm_info[ id ].minfilter_mode, mm_info[ id ].magfilter_mode );
		grTexMipMapMode( tmu, mm_info[ id ].mipmap_mode, mm_info[ id ].trilinear );
		grTexClampMode( tmu, mm_info[ id ].s_clamp_mode, mm_info[ id ].t_clamp_mode );
		grTexLodBiasValue( tmu, mm_info[ id ].lod_bias );
		// Download the ncc table
		// @todo: only necessary when the format uses the table?
		if ((info.format == GR_TEXFMT_YIQ_422) ||
	        (info.format == GR_TEXFMT_AYIQ_8422))
		{
			// @todo: the linux driver utilises both tables and optimises downloads
			// MacGLide only uses gr* functions whereas
			// the linux driver has extra internal state for gu* functions
			// -> No download, no texture recreation (see gutex.c)
			//  /* Which table should we use? */
			//  table = gc->tmu_state[tmu].next_ncc_table;
			//  /* Download NCC table */ // using internal function
			//  _grTexDownloadNccTable( tmu, table, &mminfo->ncc_table, 0, 11 );
			//  /* Set the mmid so we known it's down there */
			//  gc->tmu_state[tmu].ncc_mmids[table] = mmid;
			//  /* Set the state to know which table was the LRA */
			//  gc->tmu_state[tmu].next_ncc_table =
			//    (table == 0 ? 1 : 0);
			grTexDownloadTable(tmu, GR_NCCTABLE_NCC0, &mm_info[ id ].ncc_table);
		}
		m_current_id = id;
	}
#ifdef OGL_UTEX
	else
	{
		GlideMsg( "TexSource failed\n" );
	}
#endif
}

GrMipMapInfo *PGUTexture::GetMipMapInfo( GrMipMapId_t mmid )
{
    return ( ( mmid >= 0 ) && ( mmid < MAX_MM ) && 
             ( mm_info[mmid].valid ) ? &( mm_info[ mmid ] ) : NULL );
}

FxBool PGUTexture::ChangeAttributes( GrMipMapId_t mmid, int width, int height, 
                                     GrTextureFormat_t fmt, GrMipMapMode_t mm_mode, 
                                     GrLOD_t smallest_lod, GrLOD_t largest_lod, 
                                     GrAspectRatio_t aspect, GrTextureClampMode_t s_clamp_mode, 
                                     GrTextureClampMode_t t_clamp_mode, 
                                     GrTextureFilterMode_t minFilterMode, 
                                     GrTextureFilterMode_t magFilterMode )
{
#ifdef OGL_NOTDONE
	GlideMsg("PGUTexture::ChangeAttributes(...)\n");
#endif

    return FXFALSE;
}

GrMipMapId_t PGUTexture::GetCurrentMipMap( GrChipID_t tmu ) const
{
    return m_current_id;
}

FxU32 PGUTexture::MemQueryAvail( GrChipID_t tmu ) const
{
    return Textures->GetMemorySize( ) - m_free_mem;
}
