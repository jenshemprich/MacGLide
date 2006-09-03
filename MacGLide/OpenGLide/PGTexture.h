//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                PGTexture Class Definition
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#ifndef __PGTEXTURE_H__
#define __PGTEXTURE_H__

#include "TexDB.h"

struct TTextureStruct;

class PGTexture  
{
	struct TexValues
	{
		GrLOD_t lod;
		FxU32 width;
		FxU32 height;
		FxU32 nPixels;
	};
	unsigned int PowerOfTwoCeiling(unsigned int x);
public:
	void NCCTable( GrNCCTable_t tab );
	static FxU32 LodOffset( FxU32 evenOdd, const GrTexInfo *info );
	static FxU32 MipMapMemRequired( GrLOD_t lod, GrAspectRatio_t aspectRatio, 
	                                GrTextureFormat_t format );
	static FxU32 TextureMemRequired( FxU32 evenOdd, const GrTexInfo *info );
	static void genPaletteMipmaps( FxU32 width, FxU32 height, const FxU8 *data );
	void ChromakeyMode( GrChromakeyMode_t mode );
	void ChromakeyValue( GrColor_t value );
	inline float GetHAspect() const {return m_hAspect;}
	inline float GetWAspect() const	{return m_wAspect;}
	inline const GrTexInfo* GetCurrentTexInfo()  const {return &m_info;}
	inline void Clear() {m_db->Clear();}
	inline void initOpenGL() {m_db->initOpenGL();}
	inline void cleanupOpenGL() {m_db->cleanupOpenGL();}
	bool MakeReady(TTextureStruct* tex_coords = NULL, unsigned long number_of_triangles = 0);
	void DownloadTable( GrTexTable_t type, const FxU32 *data, int first, int count );
	void Source( FxU32 startAddress, FxU32 evenOdd, const GrTexInfo *info );
	void DownloadMipMap( FxU32 startAddress, FxU32 evenOdd, const GrTexInfo *info );
	void DownloadMipMapLevel(FxU32             startAddress,
                           GrLOD_t           thisLod,
                           GrLOD_t           largeLod,
                           GrAspectRatio_t   aspectRatio,
                           GrTextureFormat_t format,
                           FxU32             evenOdd,
                           void              *data);
	void DownloadMipMapLevelPartial(FxU32             startAddress,
                                  GrLOD_t           thisLod,
                                  GrLOD_t           largeLod,
                                  GrAspectRatio_t   aspectRatio,
                                  GrTextureFormat_t format,
                                  FxU32             evenOdd,
                                  void              *data,
                                  int               start,
                                  int               end);
	FxU32 GetMemorySize( void ) const;
	PGTexture( int mem_size );
	virtual ~PGTexture();

#ifdef OGL_DEBUG
	int Num_565_Tex;
	int Num_565_Chromakey_Tex;
	int Num_1555_Tex;
	int Num_1555_Chromakey_Tex;
	int Num_4444_Tex;
	int Num_4444_Chromakey_Tex;
	int Num_332_Tex;
	int Num_8332_Tex;
	int Num_Alpha_Tex;
	int Num_AlphaIntensity88_Tex;
	int Num_AlphaIntensity44_Tex;
	int Num_AlphaPalette_Tex;
	int Num_Palette_Tex;
	int Num_Palette_Chromakey_Tex;
	int Num_Intensity_Tex;
	int Num_YIQ_Tex;
	int Num_AYIQ_Tex;
	int Num_Other_Tex;
#endif

private:
	void ApplyKeyToPalette( void );
	void GetTexValues( TexValues *tval ) const;
	void DownloadMipmapsToOpenGL(GLint compnum, GLint compformat, GLenum comptype, const void* texdata, TexValues& t, bool build_mipmaps);
	FxU32           m_tex_memory_size;
	bool            m_palette_dirty;
	FxU32           m_palette_hash;
	TexDB *         m_db;
	GrChromakeyMode_t m_chromakey_mode;
	FxU32           m_chromakey_value_8888;
	FxU16           m_chromakey_value_565;
	FxU16           m_chromakey_value_1555;
	float           m_wAspect;
	float           m_hAspect;
	FxU32*          m_tex_temp;
	FxU32*          m_textureCache;
	bool            m_valid;
	FxU8 *          m_memory;
	FxU32           m_startAddress;
	FxU32           m_evenOdd;
	GrTexInfo       m_info;
	FxU32           m_palette[ 256 ];
	GrNCCTable_t    m_ncc_select;
	GuNccTable      m_ncc[2];
};

extern PGTexture *Textures;

#endif
