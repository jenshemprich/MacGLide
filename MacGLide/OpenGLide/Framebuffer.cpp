//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                  framebuffer emulation
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Framebuffer.h"
#include "Glide.h"
#include "GlideApplication.h"
#include "GlideSettings.h"
#include "GLRenderUpdateState.h"
#include "GLColorAlphaCombineEnvTables.h"

// check if tile needs to be displayed
#define CHECK_RENDER_TILE

// Display small dots at opposite corners of rendered framebuffer tiles
// #define DEBUG_TILE_RENDERING

Framebuffer::Framebuffer()
: m_x_step_start(0)
, m_y_step_start(0)
, m_x_step_start_opaque(0)
, m_y_step_start_opaque(0)
, m_width(0)
, m_height(0)
, m_framebuffer(NULL)
, m_texbuffer(NULL)
, m_origin(GR_ORIGIN_UPPER_LEFT)
, m_glInternalFormat(-1)
, m_glFormat(-1)
, m_glType(-1)
, m_ChromaKey(0x0000)
, m_glDepth(1.0f)
, m_glAlpha(0x000000ff)
, m_format_valid(false)
, m_use_client_storage(false)
, m_must_clear_buffer(true)
, m_custom_tilesizes(NULL)
{
}

Framebuffer::~Framebuffer()
{
}

bool Framebuffer::initialise_buffers(BufferStruct* framebuffer, BufferStruct* texbuffer, FxU32 width, FxU32 height, const tilesize* tilesizetable, bool use_client_storage)
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::initialise_buffers(---, ---, %d, %d, ---, %d)\n", width, height, use_client_storage);
	#endif

	m_custom_tilesizes = tilesizetable;
	return initialise_buffers(framebuffer, texbuffer, width, height, 0, 0, use_client_storage);
}

bool Framebuffer::initialise_buffers(BufferStruct* framebuffer, BufferStruct* texbuffer, FxU32 width, FxU32 height, FxU32 x_tile, FxU32 y_tile, bool use_client_storage)
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "Framebuffer::initialise_buffers(---, ---, %d, %d, %d, %d, %d)\n", width, height, x_tile, y_tile, use_client_storage);
	#endif

	m_framebuffer = framebuffer;
	m_texbuffer = texbuffer;
	m_framebuffer->WriteMode = m_texbuffer->WriteMode = GR_LFBWRITEMODE_UNUSED;
	m_width = width;
	m_height = height;
	// find out largest texture size
	GLint tile_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tile_size);
	m_x_step_start_opaque = tile_size;
	m_y_step_start_opaque = tile_size;
	m_x_step_start = min(tile_size, x_tile);
	m_y_step_start = min(tile_size, y_tile);
	m_x_step_start = max(16, m_x_step_start);
	m_y_step_start = max(16, m_y_step_start);
	m_use_client_storage = use_client_storage;
	// The texture priority is set to minimun because
	// frame buffer textures are never used a second time
	GLfloat zero = 0.0f;
	if (m_use_client_storage)
	{
		glGenTextures( m_max_client_storage_textures, &m_tex_name[0]);
		for(int i = 0; i < m_max_client_storage_textures; i++)
		{
			glPrioritizeTextures(1, &m_tex_name[i], &zero);
		}
	}
	else
	{
		glGenTextures( 1, &m_tex_name[0]);
		glPrioritizeTextures(1, &m_tex_name[0], &zero);
	}
	for(int i = 0; i < (m_use_client_storage ? m_max_client_storage_textures : 1); i++)
	{
		glBindTexture(GL_TEXTURE_2D, m_tex_name[i]);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}
	// If a game has it's own tilesize table, use
	// the largest tiles for opaque renderings
	GLint y_step = y_tile == 0 ? m_y_step_start_opaque : m_y_step_start;
	// init default/opaque tilesize table
	int v = 0;
	int w = 0;
	for(FxU32 y = 0; y < m_height && w < MaxTiles ; y += y_step, w++)
	{
		while (m_height - y < y_step)
		{
			y_step = y_step >> 1;
		}
		m_tilesizes[w].y = y_step;
		GLint x_step = x_tile == 0 ? m_x_step_start_opaque : m_x_step_start;
		for(FxU32 x = 0, v = 0; x < m_width && v < MaxTiles; x += x_step, v++ )
		{
			while (m_width - x < x_step)
			{
				x_step = x_step >> 1;
			}
			m_tilesizes[w].x[v] = x_step;
		}
	}
	return m_width > 0 && m_height > 0 && m_x_step_start > 0 && m_y_step_start > 0 && m_format_valid;
}

void Framebuffer::free_buffers()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "Framebuffer::free_buffers()\n");
	#endif

	if (m_tilesizes) FreeObject(m_tilesizes);
	if (m_use_client_storage)
	{
		glDeleteTextures( m_max_client_storage_textures, &m_tex_name[0] );
	}
	else
	{
		glDeleteTextures( 1, &m_tex_name[0] );
	}
}

void Framebuffer::initialise_format(GrLfbWriteMode_t writemode)
{
	#if defined(OGL_PART_DONE) || defined(OGL_FRAMEBUFFER)
		GlideMsg("Framebuffer::initialise_format(0x%x)\n", writemode);
	#endif

	// Enlarge buffer?
	if (writemode >= GR_LFBWRITEMODE_888 &&
	    (m_framebuffer->WriteMode < GR_LFBWRITEMODE_888 || m_framebuffer->WriteMode == GR_LFBWRITEMODE_UNUSED) &&
	    m_framebuffer->Address)
	{
		// Delete existing buffer
		FreeFrameBuffer(m_framebuffer->Address);
		m_framebuffer->Address = NULL;
		m_texbuffer->Address = NULL;
	}
	// Allocate 32-bit buffer (16bit buffer has been allocated in grSstWinOpen()
	if (m_framebuffer->Address == NULL)
	{
		unsigned long openglpixels = OpenGL.WindowWidth * OpenGL.WindowHeight;
		// Framebuffer can be written to with 16bit or 32bit data
		unsigned long buffertypesize = (writemode >= GR_LFBWRITEMODE_888) ? sizeof(FxU32) : sizeof(FxU16);
		Glide.FrameBuffer.Address = (FxU16*) AllocFrameBuffer(Glide.WindowTotalPixels * buffertypesize + openglpixels * sizeof(FxU32), 1);
		Glide.TempBuffer.Address = &Glide.FrameBuffer.Address[Glide.WindowTotalPixels * buffertypesize >> 1];
		memset( Glide.FrameBuffer.Address, 0, Glide.WindowTotalPixels * buffertypesize);
		memset( Glide.TempBuffer.Address, 0, openglpixels * sizeof(FxU32));
	}
	m_framebuffer->WriteMode = writemode;
	m_glInternalFormat = 4;
	m_glFormat = GL_RGBA;
	m_glType = GL_UNSIGNED_BYTE;
	FxU16 chromakeyvalue;
	switch (writemode)
	{
	case GR_LFBWRITEMODE_565:
		chromakeyvalue = 	s_GlideApplication.GetType() == GlideApplication::Carmageddon
		                  ? 0x1f1f : 0x07ff;
		m_format_valid = true;
		break;
	case GR_LFBWRITEMODE_1555:
		chromakeyvalue = 0x03ff;
		m_format_valid = true;
		break;
	case GR_LFBWRITEMODE_888:
		chromakeyvalue = 0x7ffdfeff;
		m_format_valid = true;
		break;
	default:
		chromakeyvalue = 0x0;
		m_format_valid = false;
		break;
	}
	// When the chromakeyvalue changes, the buffer has to be cleared
	if (chromakeyvalue != m_ChromaKey)
	{
		m_ChromaKey = chromakeyvalue;
		m_must_clear_buffer = true;
	}
}

bool Framebuffer::begin_write()
{
#ifdef OGL_FRAMEBUFFER
	GlideMsg("Framebuffer::begin_write()\n");
#endif

	if (m_must_clear_buffer)
	{
		Clear();
		m_must_clear_buffer = false;
	}
	return true;
}

void Framebuffer::Clear()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "Framebuffer::Clear()\n");
	#endif

	FxU16 ChromaKey = m_ChromaKey;
	FxU32 count = m_width * m_height ;
	FxU16* framebuffer = m_framebuffer->Address;
	for ( int i = 0; i < count; i++)
	{
		framebuffer[i] = ChromaKey;
	}
}

bool Framebuffer::end_write(FxU32 alpha, GLfloat depth, bool pixelpipeline)
{
#ifdef OGL_FRAMEBUFFER
	GlideMsg("Framebuffer::end_write(%d, %f, %d)\n", alpha, depth, pixelpipeline);
#endif

	m_glDepth = depth;
	m_glAlpha = alpha;
	// if all pixels are invisible, nothing must be rendered.
	// The pixel conversion functions assume alpha is != 0 in order
	// to determine if a tile contains any pixels to be rendered.
	if (m_glAlpha == 0) return false;
	set_gl_state(pixelpipeline);
	draw(m_custom_tilesizes ? m_custom_tilesizes : m_tilesizes, pixelpipeline);
	restore_gl_state(pixelpipeline);
	return true;
}

bool Framebuffer::end_write(FxU32 alpha)
{
#ifdef OGL_DONE
	GlideMsg("Framebuffer::end_write(%d)\n", alpha);
#endif

	// draw frame buffer
	// @todo: Depth should OpenGL.ZNear, but that breaks overlays in Myth
	FxBool result = end_write(alpha, 0.0, false);
	return result;
}

bool Framebuffer::end_write()
{
#ifdef OGL_FRAMEBUFFER
	GlideMsg( "Framebuffer::end_write( )\n" );
#endif

	return end_write(0x000000ff);
}

bool Framebuffer::end_write_opaque()
{
#ifdef OGL_FRAMEBUFFER
	GlideMsg("Framebuffer::end_write_opaque()\n");
#endif

	// @todo: Depth should OpenGL.ZNear, but that breaks overlays in Myth
	return end_write(0x000000ff, 0.0, false);
}

void Framebuffer::set_gl_state(bool pixelpipeline)
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "Framebuffer::set_gl_state(%d)\n", pixelpipeline);
	#endif

	glReportErrors("Framebuffer::set_gl_state");
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
	// Disable the cull mode
	glDisable(GL_CULL_FACE);
	// Disable clip volume hint manually to avoid recursion
	if (InternalConfig.EXT_clip_volume_hint && OpenGL.ClipVerticesEnabledState)
	{
		glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
	}
	
	if (pixelpipeline)
	{
		if (OpenGL.ColorAlphaUnit2)
		{
			// Pixelpipeline support for env cobine based rendering:
			// Framebuffer pixels must be routed through the coloralpha unit
			// as if they were produced by the vertex iterators without an 
			// additional GL texture unit -> source must be changed accordingly
			m_bRestoreColorCombine = false;
			if (Glide.State.ColorCombineLocal == GR_COMBINE_LOCAL_ITERATED)
			{
				Glide.State.ColorCombineLocal = GR_COMBINE_LOCAL_PIXELPIPELINE;
				m_bRestoreColorCombine = true;
			}
			if (Glide.State.ColorCombineOther == GR_COMBINE_OTHER_ITERATED)
			{
				Glide.State.ColorCombineOther = GR_COMBINE_OTHER_PIXELPIPELINE;
				m_bRestoreColorCombine = true;
			}
			if (m_bRestoreColorCombine)	SetColorCombineState();
			m_bRestoreAlphaCombine = false;
			if (Glide.State.AlphaLocal == GR_COMBINE_LOCAL_ITERATED)
			{
				Glide.State.AlphaLocal = GR_COMBINE_LOCAL_PIXELPIPELINE;
				m_bRestoreAlphaCombine = true;
			}
			if (Glide.State.AlphaOther == GR_COMBINE_OTHER_ITERATED)
			{
				Glide.State.AlphaOther = GR_COMBINE_OTHER_PIXELPIPELINE;
				m_bRestoreAlphaCombine = true;
			}
			if (m_bRestoreAlphaCombine)	SetAlphaCombineState();
			// Update the opengl state for the pixel pipeline
			RenderUpdateState();
			// If the write mode doesn't provide alpha then m_glAlpha is used
			// as the constant alpha value, and we can use the alpha test
			// to mask out chromakey pixels
			switch (m_framebuffer->WriteMode)
			{
			case GR_LFBWRITEMODE_565:
			case GR_LFBWRITEMODE_888:
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_EQUAL, m_glAlpha * D1OVER255);
				glReportError();
				break;
			}
		}
		// Set the origin with clipping
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if (m_origin == GR_ORIGIN_LOWER_LEFT)
		{
			glOrtho(Glide.State.ClipMinX, Glide.State.ClipMaxX, 
			        Glide.State.ClipMinY, Glide.State.ClipMaxY, 
			        OpenGL.ZNear, OpenGL.ZFar);
			glViewport(OpenGL.OriginX + OpenGL.ClipMinX,
			           OpenGL.OriginY + OpenGL.ClipMinY, 
			           OpenGL.ClipMaxX - OpenGL.ClipMinX, 
			           OpenGL.ClipMaxY - OpenGL.ClipMinY); 
		}
		else
		{
			glOrtho(Glide.State.ClipMinX, Glide.State.ClipMaxX, 
			        Glide.State.ClipMaxY, Glide.State.ClipMinY, 
			        OpenGL.ZNear, OpenGL.ZFar);
			glViewport(OpenGL.OriginX + OpenGL.ClipMinX,
			           OpenGL.OriginY + OpenGL.WindowHeight - OpenGL.ClipMaxY, 
			           OpenGL.ClipMaxX - OpenGL.ClipMinX, 
			           OpenGL.ClipMaxY - OpenGL.ClipMinY); 
		}
		// The scissor rectangle is not reset, because scissor mode
		// is only enabled when clearing the buffer
		glMatrixMode(GL_MODELVIEW);
		glReportError();
	}
	else
	{
		// disable blend
		glDisable(GL_BLEND);
		// disable depth buffer
		glDepthMask(false);
		// Enable colormask
		glColorMask( true, true, true, false);
		// Needed for displaying in-game menus
		if (Glide.State.DepthBufferMode != GR_DEPTHBUFFER_DISABLE)
		{
			glDisable(GL_DEPTH_TEST);
		}
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0);
		// @todo: use the same parameters as for chroma keying
		//  in order to save a state change on restore
		// glAlphaFunc(GL_GEQUAL, 0.5);
		glReportError();
		if (InternalConfig.EXT_secondary_color)
		{
			glDisable(GL_COLOR_SUM_EXT);
			glReportError();
		}
		// Reset the clipping window
		// and set the origin
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if (m_origin == GR_ORIGIN_LOWER_LEFT)
		{
			glOrtho(0, Glide.WindowWidth,
			        0, Glide.WindowHeight,
			        OpenGL.ZNear, OpenGL.ZFar);
			glViewport(OpenGL.OriginX,
			           OpenGL.OriginY,
			           OpenGL.WindowWidth,
			           OpenGL.WindowHeight); 
		}
		else
		{
			glOrtho(0, Glide.WindowWidth,
			        Glide.WindowHeight, 0,
			        OpenGL.ZNear, OpenGL.ZFar);
			glViewport(OpenGL.OriginX,
			           OpenGL.OriginY,
			           OpenGL.WindowWidth,
			           OpenGL.WindowHeight); 
		}
		// The scissor rectangle is not changed, because scissor mode
		// is only enabled when clearing the buffer
		glMatrixMode(GL_MODELVIEW);
		glReportError();
		
		// Disable fog
		bool disable_fog_texture_unit = OpenGL.FogTextureUnit;
		if (disable_fog_texture_unit)
		{
			glActiveTextureARB(OpenGL.FogTextureUnit);
			if (InternalConfig.EXT_compiled_vertex_array)
			{
				glClientActiveTextureARB(OpenGL.FogTextureUnit);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glClientActiveTextureARB(OpenGL.ColorAlphaUnit1);
			}
			glDisable(GL_TEXTURE_2D);
		}
		if (OpenGL.Fog &&
		    InternalConfig.FogMode != OpenGLideFogEmulation_None &&
		    InternalConfig.FogMode != OpenGLideFogEmulation_EnvCombine)
		{
			glDisable(GL_FOG);
		}
		glReportError();
		
		// enable framebuffer texture unit
		if (OpenGL.ColorAlphaUnit2)
		{
			bool disable_coloralpha_texture_unit_2 = OpenGL.ColorAlphaUnitColorEnabledState[1] || OpenGL.ColorAlphaUnitAlphaEnabledState[1];
			if (disable_coloralpha_texture_unit_2)
			{
				glActiveTextureARB(OpenGL.ColorAlphaUnit2);
				glDisable(GL_TEXTURE_2D);
			}
			if (disable_fog_texture_unit || disable_coloralpha_texture_unit_2) glActiveTextureARB(OpenGL.ColorAlphaUnit1);
			if (!OpenGL.ColorAlphaUnitColorEnabledState[0] && !OpenGL.ColorAlphaUnitAlphaEnabledState[0])
			{
				glEnable(GL_TEXTURE_2D);
			}
		}
		else
		{
			if (disable_fog_texture_unit) glActiveTextureARB(OpenGL.ColorAlphaUnit1);
			if (OpenGL.Texture == false) glEnable(GL_TEXTURE_2D);
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glReportError();
	}
}

void Framebuffer::restore_gl_state(bool pixelpipeline)
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "Framebuffer::restore_gl_state(%d)\n", pixelpipeline);
	#endif

	glReportErrors("Framebuffer::restore_gl_state");
	// Restore the cull mode
	switch (Glide.State.CullMode)
	{
	case GR_CULL_DISABLE:
		break;
	case GR_CULL_NEGATIVE:
	case GR_CULL_POSITIVE:
		glEnable(GL_CULL_FACE);
		break;
	}
	if (InternalConfig.EXT_clip_volume_hint && OpenGL.ClipVerticesEnabledState)
	{
		glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_NICEST);
	}
	
	// Restore the clipping window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
	{
		glOrtho(Glide.State.ClipMinX, Glide.State.ClipMaxX, 
							Glide.State.ClipMinY, Glide.State.ClipMaxY, 
							OpenGL.ZNear, OpenGL.ZFar);
		glViewport(OpenGL.OriginX + OpenGL.ClipMinX,
		           OpenGL.OriginY + OpenGL.ClipMinY, 
		           OpenGL.ClipMaxX - OpenGL.ClipMinX, 
		           OpenGL.ClipMaxY - OpenGL.ClipMinY); 
	}
	else
	{
		glOrtho(Glide.State.ClipMinX, Glide.State.ClipMaxX, 
							Glide.State.ClipMaxY, Glide.State.ClipMinY, 
							OpenGL.ZNear, OpenGL.ZFar);
		glViewport(OpenGL.OriginX + OpenGL.ClipMinX,
		           OpenGL.OriginY + OpenGL.WindowHeight - OpenGL.ClipMaxY, 
		           OpenGL.ClipMaxX - OpenGL.ClipMinX, 
		           OpenGL.ClipMaxY - OpenGL.ClipMinY);
	}
	// The scissor rectangle is not reset, because scissor mode
	// is only enabled when clearing the buffer
	glMatrixMode( GL_MODELVIEW );
	glReportError();
	
	if (pixelpipeline)
	{
		if (OpenGL.ColorAlphaUnit2)
		{
			// restore current values
			if (m_bRestoreColorCombine)
			{
				if (Glide.State.ColorCombineLocal == GR_COMBINE_LOCAL_PIXELPIPELINE)	Glide.State.ColorCombineLocal = GR_COMBINE_LOCAL_ITERATED;
				if (Glide.State.ColorCombineOther == GR_COMBINE_OTHER_PIXELPIPELINE)	Glide.State.ColorCombineOther = GR_COMBINE_OTHER_ITERATED;
				SetColorCombineState();
			}
			if(m_bRestoreAlphaCombine)
			{
				if (Glide.State.AlphaLocal == GR_COMBINE_LOCAL_PIXELPIPELINE)	Glide.State.AlphaLocal = GR_COMBINE_LOCAL_ITERATED;
				if (Glide.State.AlphaOther == GR_COMBINE_OTHER_PIXELPIPELINE)	Glide.State.AlphaOther = GR_COMBINE_LOCAL_ITERATED;
				SetAlphaCombineState();
			}
		}
		switch (m_framebuffer->WriteMode)
		{
		case GR_LFBWRITEMODE_565:
		case GR_LFBWRITEMODE_888:
			SetChromaKeyAndAlphaState();
			break;
		}
	}
	else
	{
		// restore previous state
		if (OpenGL.DepthBufferWritting )
		{
			glDepthMask( true );
		}
		if (Glide.State.DepthBufferMode != GR_DEPTHBUFFER_DISABLE)
		{
			glEnable( GL_DEPTH_TEST );
		}
		// Restore colormask
		bool rgb = Glide.State.ColorMask;
		glColorMask(rgb, rgb, rgb, Glide.State.AlphaMask);

		if ( OpenGL.Blend )
		{
			glEnable( GL_BLEND );
		}
		if ( InternalConfig.EXT_secondary_color )
		{
			glEnable( GL_COLOR_SUM_EXT );
		}
		glReportError();
		// Enable fog?
		bool enable_fog_texture_unit = OpenGL.FogTextureUnit &&
		    ((OpenGL.Fog && InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine) ||
		     Glide.State.ColorCombineInvert ||
		     Glide.State.AlphaInvert);
		if (enable_fog_texture_unit)
		{
			glActiveTextureARB(OpenGL.FogTextureUnit);
			glEnable(GL_TEXTURE_2D);
			// We're not using glDrawArrays to render the frame buffer,
			// but without disabling the client state the next texture drawn
			// by RenderDrawTriangles would get the wrong coordinates.
			// Can be observed in Carmageddon: The sky texture is rendered "too high"
			if (InternalConfig.EXT_compiled_vertex_array)
			{
				glClientActiveTextureARB(OpenGL.FogTextureUnit);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glClientActiveTextureARB(OpenGL.ColorAlphaUnit1);
			}
			if (OpenGL.ColorAlphaUnit2 == 0)
			{
				glActiveTextureARB(OpenGL.ColorAlphaUnit1);
			}
		}
		if (OpenGL.Fog &&
		    InternalConfig.FogMode != OpenGLideFogEmulation_None &&
		    InternalConfig.FogMode != OpenGLideFogEmulation_EnvCombine)
		{
			glEnable(GL_FOG);
		}
		glReportError();
		
		if (OpenGL.ColorAlphaUnit2)
		{
			bool enable_coloralpha_texture_unit_2 = OpenGL.ColorAlphaUnitColorEnabledState[1] || OpenGL.ColorAlphaUnitAlphaEnabledState[1];
			if (enable_coloralpha_texture_unit_2)
			{
				glActiveTextureARB(OpenGL.ColorAlphaUnit2);
				glEnable(GL_TEXTURE_2D);
			}
			if (enable_fog_texture_unit || enable_coloralpha_texture_unit_2) glActiveTextureARB(OpenGL.ColorAlphaUnit1);
			if (!OpenGL.ColorAlphaUnitColorEnabledState[0] && !OpenGL.ColorAlphaUnitAlphaEnabledState[0])
			{
				glDisable(GL_TEXTURE_2D);
			}
			// Restore the previous texture environment
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		}
		else
		{
			if (OpenGL.Texture == false)
			{
				glDisable(GL_TEXTURE_2D);
			}
			// Restore the previous texture environment
			SetColorCombineState();
		}
		glReportError();
		// This must be a forced update because GlideState changes of ChromaKeyMode
		// that don't change the corresponding GL-state are filtered out
		ForceChromaKeyAndAlphaStateUpdate();
	}

	glReportError();
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}


bool Framebuffer::draw(const tilesize* tilesizetable, bool pixelpipeline)
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "Framebuffer::draw(---, %d)\n", pixelpipeline);
	#endif

	glReportErrors("Framebuffer::draw()");
	GLint x_step;
	GLint y_step;
	bool init_second_textureunit = pixelpipeline && OpenGL.ColorAlphaUnit2;
	int texturenameindex = 0;
	FxU32* texbuffer = reinterpret_cast<FxU32*>(m_texbuffer->Address);
	if (m_use_client_storage)
	{
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, true);
		glReportError();
	}
	FxU32 x;
	FxU32 y;
	y = 0;
	for(int w = 0; y < m_height && w < MaxTiles; w++, y += y_step)
	{
		y_step = tilesizetable[w].y;
		x = 0;
		for(int v = 0; x < m_width && v < MaxTiles; v++, x += x_step)
		{
			x_step = tilesizetable[w].x[v];
			if (createTextureData(texbuffer, x, y, x_step, y_step))
			{
				// Obsolete, enabled before calling draw()
				GLint texturename;
				if (m_use_client_storage)
				{
					// Multiple textures are used to keep the gl pipeline busy
					texturename = m_tex_name[texturenameindex];
					if (texturenameindex == m_max_client_storage_textures -1)
					{
						glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE , false);
						glReportError();
					}
					else if (texturenameindex < m_max_client_storage_textures -1)
					{
						texturenameindex++;
					}
				}
				else
				{
					// Just one texture is used to minimise memory usage
					// @todo: Could be further optimised by using multiple texture names
					// (Like if CLIENT_STORAGE_APPLE is present) and deleting obsolete
					// textures > texturenameindex after rendering the current framebuffer.
					texturename = m_tex_name[0];
				}
				glBindTexture(GL_TEXTURE_2D, texturename);
				#ifdef DEBUG_TILE_RENDERING
					((long*) texbuffer)[0] = 0x00ff00ff;
					((long*) texbuffer)[x_step * y_step - 1] = 0x00ff00ff;
				#endif
				glTexImage2D(GL_TEXTURE_2D, 0, m_glInternalFormat, x_step, y_step, 0, m_glFormat, m_glType, texbuffer);
				if (init_second_textureunit)
				{
					glActiveTextureARB(OpenGL.ColorAlphaUnit2);
					glBindTexture(GL_TEXTURE_2D, texturename);
					glActiveTextureARB(OpenGL.ColorAlphaUnit1);
				}
				glReportError();
				static struct
				{
					const GLfloat bl[4];
					const GLfloat br[4];
					const GLfloat tr[4];
					const GLfloat tl[4];
				}
				texcoords =
				{
					{0.0, 0.0, 1.0, 1.0},
					{1.0, 0.0, 1.0, 1.0},
					{1.0, 1.0, 1.0, 1.0},
					{0.0, 1.0, 1.0, 1.0}
				};
				glBegin(GL_QUADS);
					// counter clockwise
					glColor3f(1.0, 1.0, 1.0);
					glTexCoord4fv(&texcoords.bl[0]);
					if (init_second_textureunit)
					{
						glMultiTexCoord4fvARB(OpenGL.ColorAlphaUnit2, &texcoords.bl[0]);
					}
					glVertex3f(x, y, m_glDepth);
					glColor3f(1.0, 1.0, 1.0);
					glTexCoord4fv(&texcoords.br[0]);
					if (init_second_textureunit)
					{
						glMultiTexCoord4fvARB(OpenGL.ColorAlphaUnit2, &texcoords.br[0]);
					}
					glVertex3f(x + x_step, y, m_glDepth);
					glColor3f(1.0, 1.0, 1.0);
					glTexCoord4fv(&texcoords.tr[0]);
					if (init_second_textureunit)
					{
						glMultiTexCoord4fvARB(OpenGL.ColorAlphaUnit2, &texcoords.tr[0]);
					}
					glVertex3f(x + x_step, y + y_step, m_glDepth);
					glColor3f(1.0, 1.0, 1.0);
					glTexCoord4fv(&texcoords.tl[0]);
					if (init_second_textureunit)
					{
						glMultiTexCoord4fvARB(OpenGL.ColorAlphaUnit2, &texcoords.tl[0]);
					}
					glVertex3f(x, y + y_step , m_glDepth);
				glEnd();
				glReportError();
				// use the next texbuffer location
				if (m_use_client_storage && texturenameindex != m_max_client_storage_textures -1)
				{
					texbuffer += x_step * y_step;
				}
			}
		}
	}
	if (m_use_client_storage && texturenameindex < m_max_client_storage_textures - 1)
	{
 		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE , false);
		glReportError();
 	}

	s_Framebuffer.SetRenderBufferChanged();
	return y == m_height && x == m_width; 
}

inline bool Framebuffer::Convert565Kto8888(FxU16* buffer1, register FxU32* buffer2, register FxU32 width, register FxU32 height, register FxU32 stride)
{
	// Process two pixels at once
	const register unsigned long chromakey1 = m_ChromaKey << 16;
	const register unsigned long chromakey2 = m_ChromaKey;
	const register unsigned long chromakey12 = chromakey1 | chromakey2;
	width = width >> 1;
	stride = stride >> 1;
	register unsigned long pixel;
	register unsigned long* stop;
	register unsigned jump = width + stride;
	register unsigned long* src = reinterpret_cast<unsigned long*>(buffer1);
	// check if tile must be processed in advance
	// to avoid useless writes to main memory
	// The tile should at least fit into the second level cache
	// so reading it again wouldn't hurt as much as doing needless writes
	register unsigned long h = height;
	stop = &src[width];
	do
	{
		do
		{
			pixel = *src++;
			if (pixel != chromakey12) goto create_8888_texture;
		}	while (src != stop);
		src += stride;
		stop += jump;
	}	while (--h);
	return false;
create_8888_texture:
	const register unsigned long alpha = m_glAlpha;
	const register unsigned long mask_pixel1 = 0xffff0000;
	const register unsigned long mask_pixel2 = 0x0000ffff;
	const register unsigned long mask_pixel1_r = 0xf8000000;
	const register unsigned long mask_pixel1_g = 0x07e00000;
	const register unsigned long mask_pixel1_b = 0x001f0000;
	const register unsigned long mask_pixel2_r = 0x0000f800;
	const register unsigned long mask_pixel2_g = 0x000007e0;
	const register unsigned long mask_pixel2_b = 0x0000001f;
	src = reinterpret_cast<unsigned long*>(buffer1);
	stop = &src[width];
	do
	{
		do
		{
			// GL_RGBA
			pixel = *src;
			if (pixel == chromakey12)
			{
				*buffer2++ = 0;
				*buffer2++ = 0;
			}
			else
			{
				*src = chromakey12;
				if ( (pixel & mask_pixel1) == chromakey1)
				{
					*buffer2++ = 0;
				}
				else
				{
					*buffer2++ = ( alpha                         | // A
					             ( pixel & mask_pixel1_b ) >>  5 | // B
					             ( pixel & mask_pixel1_g ) >>  3 | // G
					             ( pixel & mask_pixel1_r ));       // R
				}
				if ( (pixel & mask_pixel2) == chromakey2)
				{
					*buffer2++ = 0;
				}
				else
				{
					*buffer2++ = ( alpha                           | // A
					             ( pixel & mask_pixel2_b ) << 11   | // B
					             ( pixel & mask_pixel2_g ) << 13   | // G
					             ( pixel & mask_pixel2_r ) << 16);   // R
				}
			}
			src++;
		} while (src != stop);
		src += stride;
		stop += jump;
	} while (--height);
	return true; 
}

inline bool Framebuffer::Convert1555Kto8888(FxU16* buffer1, register FxU32* buffer2, register FxU32 width, register FxU32 height, register FxU32 stride)
{
	// Process two pixels at once
	register unsigned long pixel;
	register unsigned long x;
	register unsigned long* src = reinterpret_cast<unsigned long*>(buffer1);
	const unsigned long null = 0x00000000;
	register unsigned long dstpixel = null;
	const register unsigned long chromakey1 = m_ChromaKey << 16;
	const register unsigned long chromakey2 = m_ChromaKey;
	const register unsigned long chromakey12 = chromakey1 | chromakey2;
	const register unsigned long alpha = m_glAlpha;
	const register unsigned long mask_pixel1 = 0xffff0000;
	const register unsigned long mask_pixel2 = 0x0000ffff;
	const register unsigned long mask_pixel1_r = 0x7c000000;
	const register unsigned long mask_pixel1_g = 0x03e00000;
	const register unsigned long mask_pixel1_b = 0x001f0000;
	const register unsigned long mask_pixel2_r = 0x00007c00;
	const register unsigned long mask_pixel2_g = 0x000003e0;
	const register unsigned long mask_pixel2_b = 0x0000001f;
	width >>= 1;
	stride >>= 1;
	do
	{
		x = width;
		do
		{
			// GL_RGBA
			pixel = *src;
			if (pixel == chromakey12)
			{
				*buffer2++ = null;
				*buffer2++ = null;
			}
			else
			{
				*src = chromakey12;
				if ( (pixel & mask_pixel1) == chromakey1)
				{
					*buffer2++ = null;
				}
				else
				{
					dstpixel = ( alpha                           | // A
					           ( pixel & mask_pixel1_b ) >>  5   | // B
					           ( pixel & mask_pixel1_g ) >>  2   | // G
					           ( pixel & mask_pixel1_r ) <<  1);   // R
					*buffer2++ = dstpixel;
				}
				if ( (pixel & mask_pixel2) == chromakey2)
				{
					*buffer2++ = null;
				}
				else
				{
					dstpixel = ( alpha                           | // A
					           ( pixel & mask_pixel2_b ) << 11   | // B
					           ( pixel & mask_pixel2_g ) << 14   | // G
					           ( pixel & mask_pixel2_r ) << 17);   // R
					*buffer2++ = dstpixel;
				}
			}
			src++;
		}	while (--x);
		src += stride;
	} while (--height);
	return dstpixel != null; 
}

inline bool Framebuffer::ConvertARGB8888Kto8888(FxU32* buffer1, register FxU32* buffer2, register FxU32 width, register FxU32 height, register FxU32 stride)
{
	// Process two pixels at once
	const register unsigned long chromakey = m_ChromaKey || (m_ChromaKey << 16);
	register unsigned long pixel;
	register unsigned long* stop;
	register unsigned jump = width + stride;
	register unsigned long* src = buffer1;
	// check if tile must be processed in advance
	// to avoid useless writes to main memory
	// The tile should at least fit into the second level cache
	// so reading it again wouldn't hurt as much as doing needless writes
	register unsigned long h = height;
	stop = &src[width];
	do
	{
		do
		{
			pixel = *src++;
			if (pixel != chromakey) goto create_8888_texture;
		}	while (src != stop);
		src += stride;
		stop += jump;
	}	while (--h);
	return false;
create_8888_texture:
	const register unsigned long alpha = m_glAlpha;
	src = buffer1;
	stop = &src[width];
	do
	{
		do
		{
			// GL_RGBA
			pixel = *src;
			if (pixel == chromakey)
			{
				*buffer2++ = 0;
			}
			else
			{
				*src = chromakey;
				*buffer2++ = (pixel << 8) | alpha;
			}
			src++;
		} while (src != stop);
		src += stride;
		stop += jump;
	}	while (--height);
	return true; 
}

inline bool Framebuffer::createTextureData(FxU32* texbuffer, FxU32 x, FxU32 y, FxU32 x_step, FxU32 y_step)
{
	FxU32 stride = (m_width - x_step);
	FxU32 index = x + y * m_width;
	if (m_framebuffer->WriteMode == GR_LFBWRITEMODE_565)
	{
		return Convert565Kto8888(&m_framebuffer->Address[index], texbuffer, x_step, y_step, stride);
	}
	else if (m_framebuffer->WriteMode == GR_LFBWRITEMODE_1555)
	{
		return Convert1555Kto8888(&m_framebuffer->Address[index], texbuffer, x_step, y_step, stride);
	}
	else if (m_framebuffer->WriteMode == GR_LFBWRITEMODE_888)
	{
		FxU32* framebuffer = &reinterpret_cast<FxU32*>(m_framebuffer->Address)[index];
		return ConvertARGB8888Kto8888(framebuffer, texbuffer, x_step, y_step, stride);
	}
	else
	{
		return FALSE;
	}
}