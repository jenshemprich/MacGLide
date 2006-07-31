//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*               Glide framebuffer emulation
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "driversrc_declarations.h"
#include "GlideApplication.h"
#include "GlideFramebuffer.h"
#include "GlideSettings.h"

// Custom framebuffer tiles tables to minimise calls to glTexImage2D()
// by defining tiles that:
// - are empty (no color conversion takes place)
// - don't change (so they have to be downloaded to the GPU only once)
// This does not apply to drawing underlays because they are drawn with
// max tile sizes as they're usally used to draw fullscreen backgrounds

// The sum of the first column must be Glide.WindowHeigth (vertical resolution)
// The sum of all elements in the second element of each row must be Glide.WindowWidth (horizontal resolution)

// Optimised framebuffer tile layout for Carmageddon:
// Carmageddon uses only the glide resolution 640x480. The table optimised
// the outside car view. In cockpit mode the framebuffer must be rendered two times
// which makes everything much slower, but in cockpit mode this table allows
// for a fluid framerate even on slow (~450 Mhz) machines
const Framebuffer::tilesize GlideFramebuffer::tilesize_table_carmageddon[11] =
{
	{ 64, {128,128,  8,128,128, 64, 32, 16,  8,0,0,0}},
	{ 64, {128,128,128,128, 64, 64,  0,  0,  0,0,0,0}},
	{ 32, {128,128,128,128,128,  0,  0,  0,  0,0,0,0}},
	{ 32, {128,128,128,128,128,  0,  0,  0,  0,0,0,0}},
	{ 32, {128,128,128,128,128,  0,  0,  0,  0,0,0,0}},
	{ 32, {128,128,128,128,128,  0,  0,  0,  0,0,0,0}},
	{ 32, {128,128,128,128,128,  0,  0,  0,  0,0,0,0}},
	{ 32, {128,128,128,128,128,  0,  0,  0,  0,0,0,0}},
	{ 32, {128,128,128,128,128,  0,  0,  0,  0,0,0,0}},
	{ 64, {  8,128,256,128, 32,  8, 64, 16,  0,0,0,0}},
	{ 64, {  8,128,256,128, 32,  8, 64, 16,  0,0,0,0}}
};

// Optimised framebuffer tile layouts for Falcon 4.0:
// Falcon ofers a lot of screen resolutions, so we have to provide mopre than one table.
// If the table can be derived from a smaller one, it is scaled up to the appropriate
// screen size. In fact, just 640x480 and 800x600 have to be hardcoded.

// The table optimises the main cockpit view (looking forward after pressing "2" on the keyboard")
// Areas are kept as large as possible in order to avoid producing many small textures in other views.
// Static tiles with artwork only at the bottom are yield good performance as the better part of
// the tile is just read (as if the tile was totally empty) and then never gets downloaded again.
const Framebuffer::tilesize GlideFramebuffer::tilesizeTableFalcon40_640[7] =
{
	{  32, {256,128,256,  0,  0,  0,  0,  0,  0,0,0,0}},
	{ 128, {128, 64,256, 64,128,  0,  0,  0,  0,0,0,0}},
	{  64, { 32,128, 32,256, 32,128, 32,  0,  0,0,0,0}},
	{  64, { 64,128,128,128,128, 64,  0,  0,  0,0,0,0}},
	{  64, { 64,128,128,128,128, 64,  0,  0,  0,0,0,0}},
	{ 128, { 64,128,128,128,128, 64,  0,  0,  0,0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0,0}}
};

const Framebuffer::tilesize GlideFramebuffer::tilesizeTableFalcon40_800[8] =
{
	{   8, {256,256, 32,256,  0,  0,  0,  0,0,0,0,0}},
	{  16, {256,256, 32,256,  0,  0,  0,  0,0,0,0,0}},
	{ 128, {128,128,256, 32,128,128,  0,  0,0,0,0,0}},
	{ 128, {128,128,256, 32,128,128,  0,  0,0,0,0,0}},
	{  64, {128,256, 32,256,128,  0,  0,  0,0,0,0,0}},
	{  64, {128,128,128, 32,128,128,128,  0,0,0,0,0}},
	{  64, {128,128,128, 32,128,128,128,  0,0,0,0,0}},
	{ 128, {128,128,128, 32,128,128,128,  0,0,0,0,0}}
};

void GlideFramebuffer::scaleTilesizeTable(const Framebuffer::tilesize* in, Framebuffer::tilesize* out, int factor)
{
	GLint y_step;
	int w = 0;
	for(FxU32 y = 0; y < Glide.WindowHeight && w < MaxTiles ; y += y_step, w++)
	{
		y_step = in[w].y;
		out[w].y = y_step * factor;
		GLint x_step;
		int v = 0;
		for(FxU32 x = 0; x < Glide.WindowWidth && v < MaxTiles; x += x_step, v++ )
		{
			x_step = in[w].x[v];
			out[w].x[v] = x_step * factor;
		}
	}
}

Framebuffer::tilesize GlideFramebuffer::tilesizeTableFalcon40Scaled[Framebuffer::MaxTiles] =
{
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}},
	{   0, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0}}
};

GlideFramebuffer::GlideFramebuffer()
: m_write_opaque(false)
, m_must_write(false)
, m_renderbuffer_changed(false)
, m_renderbuffer_changed_for_read(false)
, m_depth(0.0f)
, m_alpha(0x000000ff)
, m_chromakeyvalue_changed(false)
, m_chromakeyvalue_new(0x0)
, m_bufferclearcalls(0)
{
}

GlideFramebuffer::~GlideFramebuffer()
{
}

void GlideFramebuffer::initialise(BufferStruct* framebuffer, BufferStruct* texbuffer)
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::initialise(---, ---)\n");
	#endif

	m_grLfbLockWriteMode[0] = GR_LFBWRITEMODE_UNUSED;
	m_grLfbLockWriteMode[1] = GR_LFBWRITEMODE_UNUSED;
	m_grLfbLockWriteMode[2] = GR_LFBWRITEMODE_UNUSED;
	m_grLfbLockWriteMode[3] = GR_LFBWRITEMODE_UNUSED;
	m_grLfbLockWriteMode[4] = GR_LFBWRITEMODE_UNUSED;
	m_grLfbLockWriteMode[5] = GR_LFBWRITEMODE_UNUSED;
	GlideApplication::Type application = s_GlideApplication.GetType();
	if (application == GlideApplication::Falcon40 && Glide.WindowWidth == 640)
	{
		initialise_buffers(framebuffer, texbuffer,
		                   Glide.WindowWidth, Glide.WindowHeight,
		                   tilesizeTableFalcon40_640);
	}
	else if (application == GlideApplication::Falcon40 && Glide.WindowWidth == 800)
	{
		initialise_buffers(framebuffer, texbuffer,
		                   Glide.WindowWidth, Glide.WindowHeight,
		                   tilesizeTableFalcon40_800);
	}
	else if (application == GlideApplication::Falcon40 && Glide.WindowWidth == 1280)
	{
		scaleTilesizeTable(tilesizeTableFalcon40_640,
		                   tilesizeTableFalcon40Scaled,
		                   2);
		initialise_buffers(framebuffer, texbuffer,
		                   Glide.WindowWidth, Glide.WindowHeight,
		                   tilesizeTableFalcon40Scaled);
	}
	else if (application == GlideApplication::Falcon40 && Glide.WindowWidth == 1600)
	{
		scaleTilesizeTable(tilesizeTableFalcon40_800,
		                   tilesizeTableFalcon40Scaled,
		                   2);
		initialise_buffers(framebuffer, texbuffer,
		                   Glide.WindowWidth, Glide.WindowHeight,
		                   tilesizeTableFalcon40Scaled);
	}
	else if (application == GlideApplication::Carmageddon && Glide.WindowWidth == 640 && Glide.WindowHeight == 480)
	{
		initialise_buffers(framebuffer, texbuffer,
		                   Glide.WindowWidth, Glide.WindowHeight,
		                   tilesize_table_carmageddon);
	}
	else
	{
		initialise_buffers(framebuffer, texbuffer, Glide.WindowWidth, Glide.WindowHeight, 128, 128);
	}
	framebuffer->WriteMode = GR_LFBWRITEMODE_UNUSED;
	texbuffer->WriteMode = GR_LFBWRITEMODE_UNUSED;
}

void GlideFramebuffer::OnBufferLockStartWrite(GrLock_t dwType, GrBuffer_t dwBuffer, GrLfbWriteMode_t dwWriteMode, GrOriginLocation_t dwOrigin, FxBool bPixelPipeline)
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnBufferLockStartWrite(%d)\n", dwOrigin);
	#endif

	// Finish current write if buffer properties are about to change
	if (m_must_write &&
	    (m_framebuffer->WriteMode != dwWriteMode ||
	     m_framebuffer->PixelPipeline != bPixelPipeline ||
	     m_origin != dwOrigin ||
	     (m_chromakeyvalue_changed && bPixelPipeline)))
	{
		// @todo: If the condition would trigger the frame buffer write
		// in Falcon 4.0 (which it doesn't), parts of the cockpit overlay
		// would appear twice or at unexpected places. This suggests there
		// might be a bug in here (in conjunction with clipping??).
		// Also be careful regarding performance issues.
		WriteFrameBuffer(m_framebuffer->PixelPipeline);
		m_must_write = false;
	}
	// Change format?
	if (m_framebuffer->WriteMode != dwWriteMode)
	{
		if (dwWriteMode == GR_LFBWRITEMODE_565 ||
		    dwWriteMode == GR_LFBWRITEMODE_1555 ||
		    dwWriteMode == GR_LFBWRITEMODE_888)
		{
			initialise_format(dwWriteMode);
		}
		// The chromakey value is not flagged as changed here,
		// because the initialise_format() function just changes
		// the current value, not the new one
	}
	// Apply new values
	m_grLfbLockWriteMode[m_framebuffer->Buffer] = dwWriteMode;
	m_framebuffer->Lock          = true;
	m_framebuffer->Type          = dwType;
	m_framebuffer->WriteMode     = dwWriteMode;
	m_framebuffer->Buffer        = dwBuffer;
	m_framebuffer->PixelPipeline = bPixelPipeline;
	m_origin = dwOrigin;
	// Update chromakeyvalue for pixel pipeline
	if (m_chromakeyvalue_changed && m_framebuffer->PixelPipeline)
	{
		// Apply changes to the frame buffer
		SetChromaKeyValue(m_chromakeyvalue_new);
		m_chromakeyvalue_changed = false;
		m_must_clear_buffer = true;
		begin_write();
	}
	else if (m_must_write == false)
	{
		// If we don't have to flush the framebuffer, then no write
		// operation is in progress and we must start a new one
		// (and clear the buffer if necessary)
		begin_write();
	}
	m_must_write = true;
}

void GlideFramebuffer::OnBufferUnlockEndWrite(GrBuffer_t dwBuffer)
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnBufferUnlockEndWrite(%d)\n", dwBuffer);
	#endif

	// continuing to write into the framebuffer is toggeled by not setting back
	// the buffer write mode flag. As a result, we don't have to query for the
	// FramebufferIgnoreUnlock elsewhere (only if we want to defer drawing).
	if (!InternalConfig.FramebufferIgnoreUnlock)
	{
		// reset buffer write mode to indicate the buffer is no more in use
		m_grLfbLockWriteMode[dwBuffer] = GR_LFBWRITEMODE_UNUSED;
	}
	
	// When the pixel pipeline is enabled, the buffer must be written right away,
	// in order to avoid commands like grColorCombine() etc. changing the
	// pixel pipeline state before the buffer will eventtually be written out.
	if (m_framebuffer->PixelPipeline)
	{
		WriteFrameBuffer(true);
		// This write is finished
		m_must_write = false;
	}	
	// If the pixelpipeline is disabled, writing the frame buffer is defered
	// until the next call to OnRenderDrawTriangles() or OnBeforeBufferSwap()
	// m_must_write is not set to "true" here because it has already been set
	// in OnBufferLockStartWrite()
}

/*
void GlideFramebuffer::OnBufferNumPending()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnBufferNumPending()\n");
	#endif

	// Defer writing the frame buffer until the next call to
	// OnRenderDrawTriangles() or OnBeforeBufferSwap().
	// Only the back buffer is checked as front and backbuffer locks
	// share the same Glide buffer.
	if (BackBufferIsLocked())
	{
		// Update: This should be obsolete as the
		// flag is set in OnAfterBufferSwap()
		// m_must_write = true;
	}
}
*/

/*
void GlideFramebuffer::OnSetIdle()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnSetIdle()\n");
	#endif

	// Defer writing the frame buffer until the next call to
	// OnRenderDrawTriangles() or OnBeforeBufferSwap().
	// Only the back buffer is checked as front and backbuffer locks
	// share the same Glide buffer.
	if (BackBufferIsLocked())
	{
		// Disabled in favour of better frame rates
		// Uncomment if PedanticFrameBufferEmulation
		// reveals otherwise unrendered graphics
		// Update: This should be obsolete as the
		// flag is set in OnAfterBufferSwap()
		// m_must_write = true;
	}
}
*/

void GlideFramebuffer::OnClipWindow()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnClipWindow()\n");
	#endif

	// When the clip window changes, and the pixel pipeline is enabled
	// and the clip window applies to framebuffer writes, so the current
	// contents of the frame buffer must be rendered.
	// @todo: This must be reconsidered because it triggers all the time
	if (m_framebuffer->PixelPipeline)
	{
		WriteFrameBuffer(true);
		// Keep writing as the frame buffer may still be locked
		m_must_write = m_framebuffer->Lock;
		// @todo: Think this is obsolete as the case is catched by OnChromaKeyValueChanged()
		if (m_chromakeyvalue_changed)
		{
			SetChromaKeyValue(m_chromakeyvalue_new);
			m_chromakeyvalue_changed = false;
			// Fill the framebuffer with the new chromakey value
			m_must_clear_buffer = true;
		}
		if (m_must_write)
		{
			begin_write();
		}
	}
}

void GlideFramebuffer::OnBeforeBufferClear()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnBufferClear()\n");
	#endif

	// Render Carma retro mirror
	if (s_GlideApplication.GetType() == GlideApplication::Carmageddon &&
	    !InternalConfig.PedanticFrameBufferEmulation)
	{
		// ignore solid colored backgrounds (happens in some levels)
		if (Glide.State.ColorMask == false)
		{
			m_bufferclearcalls++;
			if (m_bufferclearcalls == 2)
			{
				// Trigger  write in order to render cockpit layer before the retro mirror 3D-view
#ifdef OGL_FRAMEBUFFER
		GlideMsg( "Triggering  write in order to render cockpit layer before the retro mirror 3D-view\n");
#endif
				m_must_write = true;
			}
			else if (m_write_opaque == false)
			{
				// This is be needed to render the 3D-View in the map screen
				// in levels without a background texture. In these levels, the
				// background of the 3d_View is cleared by writing a fog colored
				// rectangle to the framebuffer, and this data must be rendered
				// before the 3D-View (or it will be deleted before buffer swap)
#ifdef OGL_FRAMEBUFFER
		GlideMsg( "Triggering  write in order to make 3D-View in map mode visible\n");
#endif
				m_must_write = true;
			}
			else if (m_framebuffer->Address[Glide.WindowTotalPixels >> 1] == GetChromaKeyValue())
			{
				// In Carmageddon, an opaque background is written to the frame buffer
				// as the background for the Car Damage screen. By peeking just one pixel
				// the framebuffer opaque render call can be avoided whilst in race mode.
				// This triggers in levels with a background texture.
#ifdef OGL_FRAMEBUFFER
			GlideMsg( "Detected background texture - skipping framebuffer underlay write\n");
#endif
				m_write_opaque = false;
				m_must_write = false;
			}
		}
		else
		{
			// In levels without background texture, the background is cleared with
			// grBufferClear() during the race, and we can skip writing the underlays
#ifdef OGL_FRAMEBUFFER
			GlideMsg( "Detected opaque background - Skipping framebuffer underlay write in order to improve frame rate\n");
#endif
			m_write_opaque = false;
			m_must_write = false;
		}
	}
}

void GlideFramebuffer::OnLfbReadRegion()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnLfbReadRegion()\n");
	#endif

	if (s_GlideApplication.GetType() == GlideApplication::Carmageddon)
	{
#ifdef OPENGL_DEBUG
		GlideMsg("Framebuffer unlocks will be ignored from now on in order to display the framebuffer's content\n");
#endif

		// Carmageddon unlocks the write buffer when moving the cursor
		// in Movie mode, causing its contents not being displayed anymore.
		// To avoid this, we must ignore the unlock.
		InternalConfig.FramebufferIgnoreUnlock = true;
	}
}

void GlideFramebuffer::OnRenderDrawTriangles()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnRenderDrawTriangles()\n");
	#endif

	// determine whether to write the underlays
	if (m_write_opaque && m_must_write)
	{
		if (InternalConfig.EnableFrameBufferUnderlays)
		{
			// write underlays
			end_write_opaque();
		}
		else
		{
			// clear the buffer in order to prevent drawing the underlays
			// as an overlay in a subsequent write
			m_must_clear_buffer = true;
		}
	}
	else if (InternalConfig.PedanticFrameBufferEmulation || m_must_write)
	{
		WriteFrameBuffer(m_framebuffer->PixelPipeline);
	}
	// Need to start a new write
	if (InternalConfig.PedanticFrameBufferEmulation)
	{
		if (m_chromakeyvalue_changed && m_framebuffer->PixelPipeline)
		{
			SetChromaKeyValue(m_chromakeyvalue_new);
			m_chromakeyvalue_changed = false;
			// Fill the framebuffer with the new chromakey value
			m_must_clear_buffer = true;
		}
		begin_write();
	}
	else
	{
		m_must_write = false;
	}
	// Once a triangle has been drawn, the framebuffer
	// data must be considered as overlay graphics
	m_write_opaque = false;
}

void GlideFramebuffer::OnBeforeBufferSwap()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnBeforeBufferSwap()\n");
	#endif

	// apply the framebuffer changes?
	if (InternalConfig.EnableFrameBufferOverlays)
	{
		// There's no check against the underlay flag here, because if no triangles
		// have been drawn, then the display is rendered via the frame buffer only,
		// and in that case, there's no such thing like under- and overlay.
		if (m_write_opaque && m_must_write && !m_renderbuffer_changed)
		{
			end_write_opaque();
		}
		else if (m_must_write)
		{
			WriteFrameBuffer(m_framebuffer->PixelPipeline);
		}
		else if (BackBufferIsLocked())
		{
			WriteFrameBuffer(m_framebuffer->PixelPipeline);
		}
		m_must_write = false;
		// optionally render the 3Dfx powerfield logo overlay on top of the frame
		if (InternalConfig.ShamelessPlug)
		{
			// @todo: For apps that lock the frame buffer accross buffer swaps
			// (for instance Carmageddon) the current state must be saved...
			_grShamelessPlug();
			if (m_must_write)
			{
				WriteFrameBuffer(m_framebuffer->PixelPipeline);
			}
			// ... and restored after rendering the shameless plug
		}
	}
	else
	{
		m_must_clear_buffer = true;
	}
	m_must_write = false;
}

void GlideFramebuffer::OnAfterBufferSwap()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnAfterBufferSwap()\n");
	#endif

	m_must_write = BackBufferIsLocked() || m_framebuffer->Lock;
	m_write_opaque = true;
	m_renderbuffer_changed = false;
	m_renderbuffer_changed_for_read = true;
	m_bufferclearcalls = 0;
	begin_write();
}

void GlideFramebuffer::WriteFrameBuffer(bool pixelpipeline)
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::WriteFrameBuffer(%d)\n", pixelpipeline);
	#endif

	// apply the framebuffer changes?
	if (InternalConfig.EnableFrameBufferOverlays)
	{
		// Only the back buffer is supported because games like Carmageddon
		// temporarily lock and unlock the front buffer while holding the lock
		// to the back buffer. Becasue he current implementation of the framebuffer
		// emulation doesn't support locks to multiple buffers, data is written to
		// the current buffer (which is usally the back buffer).
		if (pixelpipeline)
		{
			end_write(m_alpha, m_depth, pixelpipeline);
		}
		else
		{
			end_write();
		}
	}
	else
	{
		m_must_clear_buffer = true;
	}
}

void GlideFramebuffer::Cleanup()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::Cleanup()\n");
	#endif

	free_buffers();
}

void GlideFramebuffer::CopyFrameBuffer(FxU16* targetbuffer)
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::CopyFrameBuffer( 0x%x)\n", targetbuffer);
	#endif

	FxU16 chromakey = GetChromaKeyValue();
	FxU16* framebuffer = m_framebuffer->Address;
	FxU32 count = m_width * m_height;
	for (FxU16* end = framebuffer + count; framebuffer < end; framebuffer++, targetbuffer++)
	{
		FxU16 pixel = *framebuffer;
		if (pixel != chromakey) *targetbuffer = pixel;
	}
}

FxU16 GlideFramebuffer::GetChromaKeyValue16(GrColor_t chromakeyvalue)
{
	FxU16 chromakeyvalue16;
	if (m_framebuffer->WriteMode == GR_LFBWRITEMODE_565)
	{
		chromakeyvalue16 = static_cast<FxU16>((chromakeyvalue & 0x00F80000) >> 8 |
		                                      (chromakeyvalue & 0x0000FC00) >> 5 |
		                                      (chromakeyvalue & 0x000000F8) >> 3);
	}
	else
	{
		chromakeyvalue16 = 0x0;
	}
	return chromakeyvalue16;
}

void GlideFramebuffer::OnChromaKeyValueChanged()
{
	#ifdef OGL_FRAMEBUFFER
		GlideMsg( "GlideFrameBuffer::OnChromaKeyColorChanged()\n");
	#endif

	FxU16 chromakeyvalue = GetChromaKeyValue16(Glide.State.ChromakeyValue);
	m_chromakeyvalue_changed = GetChromaKeyValue() != chromakeyvalue;
	m_chromakeyvalue_new = chromakeyvalue;
	if (m_must_write)
	{
		if (m_chromakeyvalue_changed && m_framebuffer->PixelPipeline)
		{
			// Flush the current contents of the framebuffer
			WriteFrameBuffer(m_framebuffer->PixelPipeline);
			SetChromaKeyValue(m_chromakeyvalue_new);
			m_chromakeyvalue_changed = false;
			// Fill the framebuffer with the new chromakey value
			m_must_clear_buffer = true;
			begin_write();
		}
	}
}

void GlideFramebuffer::SetAlpha(FxU32 alpha)
{
	if (m_must_write && m_framebuffer->PixelPipeline & m_alpha != alpha)
	{
		WriteFrameBuffer(m_framebuffer->PixelPipeline);
	}
	m_alpha = alpha;
};

void GlideFramebuffer::SetDepth(GLfloat depth)
{
	if (m_must_write && m_framebuffer->PixelPipeline && m_depth != depth)
	{
		WriteFrameBuffer(m_framebuffer->PixelPipeline);
	}
	m_depth = depth;
};
