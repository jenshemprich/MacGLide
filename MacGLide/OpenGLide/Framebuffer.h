//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                  framebuffer emulation
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#pragma once

class Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();
	static const int MaxTiles = 12;
	struct tilesize {GLint y; GLint x[MaxTiles];};
public:
	bool initialise_buffers(BufferStruct* framebuffer, BufferStruct* texbuffer, FxU32 width, FxU32 height, const tilesize* tilesizetable);
	bool initialise_buffers(BufferStruct* framebuffer, BufferStruct* texbuffer, FxU32 width, FxU32 height, FxU32 x_tile, FxU32 y_tile);
	void free_buffers();
	void initialise_format(GrLfbWriteMode_t format);
	bool begin_write();
	bool end_write();
	bool end_write(FxU32 alpha);
	bool end_write(FxU32 alpha, GLfloat depth, bool pixelpipeline);
	bool end_write_opaque();
	inline FxU16 GetChromaKeyValue() {return m_ChromaKey.Scalar;};
	inline void SetChromaKeyValue(FxU16 chromakey)
	{
#ifdef __ALTIVEC__
		for(int i = 0; i < 8; i++)
		{
			(&m_ChromaKey.Scalar)[i] = chromakey;
		}
#else
		m_ChromaKey.Scalar = chromakey;
#endif
	}
protected:
	void Clear();
	bool draw(const tilesize* tilesizetable, bool pixelpipeline);
	bool drawCompiledVertexArrays(const tilesize* tilesizetable, int vertexarrayindex, int tilecount, bool pixelpipeline);
	int buildVertexArrays(const tilesize* tilesizetable, int vertexarrayindex);
	void set_gl_state(bool pixelpipeline);
	void restore_gl_state(bool pixelpipeline);
	// Pixel conversion
	enum TileUpdateState
	{
		TileUpdateState_TileEmpty = false,
		TileUpdateState_TileDownloadToGPU = 1, // true,
		TileUpdateState_TileDrawOnly = -1 // -true
	};
	static const int s_maxTiles = MaxTiles * MaxTiles;
	GLuint m_textureNames[s_maxTiles];
	inline TileUpdateState createTextureData(FxU32* texbuffer, FxU32 x, FxU32 y, FxU32 x_step, FxU32 y_step, int checksumIndex);
	inline TileUpdateState Convert565Kto8888(FxU16* buffer1, FxU32* buffer2, register FxU32 width, register FxU32 height, register FxU32 stride);
#ifdef __ALTIVEC__
	inline TileUpdateState Convert565Kto8888_AV(FxU16* buffer1, FxU32* buffer2, register FxU32 width, register FxU32 height, register FxU32 stride, int checksumIndex);
	vector unsigned long m_tileChecksums[s_maxTiles];
#endif
	inline TileUpdateState Convert1555Kto8888(FxU16* buffer1, register FxU32* buffer2, FxU32 register width, register FxU32 height, register FxU32 stride);
	inline TileUpdateState ConvertARGB8888Kto8888(FxU32* buffer1, register FxU32* buffer2, FxU32 register width, register FxU32 height, register FxU32 stride);
	bool m_useRectangleARB;
	bool m_must_clear_buffer;
	// Format
	GrOriginLocation_t m_origin;
	GLint m_glInternalFormat;
	GLint m_glFormat;
	GLint m_glType;
	bool m_format_valid;
	BufferStruct* m_framebuffer;
	BufferStruct* m_texbuffer;
	// Dimensions
	FxU32 m_width;
	FxU32 m_height;
	GLint m_x_step_start;
	GLint m_y_step_start;
	GLint m_x_step_start_opaque;
	GLint m_y_step_start_opaque;
	// Tiles
	inline int getTileCount() const;
	tilesize m_tilesizes[MaxTiles];
	int m_tilesizesCount;
	int m_tilesizesVertexArrayIndex;
	const tilesize* m_custom_tilesizes;
	int m_customtilesizesCount;
	int m_customtilesizesVertexArrayIndex;
	// Color, depth and alpha
	GLfloat m_glDepth;
	union 
	{
		FxU32 Scalar;
#ifdef __ALTIVEC__
	vector unsigned long Vector;
#endif
	} m_glAlpha;
	// Pixelpipeline
	bool m_bRestoreColorCombine;
	bool m_bRestoreAlphaCombine;
private:
	union 
	{
		FxU16 Scalar;
#ifdef __ALTIVEC__
	vector bool short Vector;
#endif
	} m_ChromaKey;
};
