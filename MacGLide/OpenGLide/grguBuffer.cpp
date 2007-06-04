//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                    Buffer functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "FormatConversion.h"
#include "Framebuffer.h"
#include "Glide.h"
#include "GLRender.h"
#include "GLRenderUpdateState.h"
#include "GLUtil.h"

// Write colored pixels only
void gapfixSetSimpleColorState()
{
	glReportErrors("gapfixSetSimpleColorState");
	
	// Disable the cull mode
	glDisable(GL_CULL_FACE);
	// Disable clip volume hint manually to avoid recursion
	if (InternalConfig.EXT_clip_volume_hint && OpenGL.ClipVerticesEnabledState)
	{
		glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
	}
	// Blend
	glDisable(GL_BLEND);
	SetBlendState();
	// Alpha
	glDisable(GL_ALPHA_TEST);
	SetChromaKeyAndAlphaState();
	// disable depth buffer
	glDepthMask(false);
	// Enable colormask
	glColorMask(true, true, true, false);
	// Needed for displaying in-game menus
	if (Glide.State.DepthBufferMode != GR_DEPTHBUFFER_DISABLE)
	{
		glDisable(GL_DEPTH_TEST);
	}
	if (InternalConfig.EXT_secondary_color)
	{
		glDisable(GL_COLOR_SUM_EXT);
	}
	if (OpenGL.Fog && OpenGL.FogTextureUnit)
	{
		glActiveTextureARB(OpenGL.FogTextureUnit);
		if (InternalConfig.EXT_compiled_vertex_array)
		{
			glClientActiveTextureARB(OpenGL.FogTextureUnit);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(4, GL_FLOAT, 0, NULL);
		}
		glDisable(GL_TEXTURE_2D);
		OpenGL.FogTextureUnitEnabledState = false;
		SetFogModeState();
	}
	for(long unit_index = 1; unit_index >= 0; unit_index--)
	{
		glActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
		if (InternalConfig.EXT_compiled_vertex_array)
		{
			glClientActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(4, GL_FLOAT, 0, NULL);
		}
		glDisable(GL_TEXTURE_2D);
		OpenGL.ColorAlphaUnitColorEnabledState[unit_index] = false;
		OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = false;
	}
	SetTextureState();
	SetColorCombineState();
	SetAlphaCombineState();
	
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
	VERIFY_TEXTURE_ENABLED_STATE();
}

void gapfixRestoreFromSimpleColorState()
{
	glReportErrors("gapfixRestoreFromSimpleColorState");

	// Restore state
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
	// restore previous state
	if (OpenGL.DepthBufferWritting)
	{
		glDepthMask(true);
	}
	const bool colormask = Glide.State.ColorMask;
	glColorMask(colormask, colormask, colormask, Glide.State.AlphaMask);
	if (Glide.State.DepthBufferMode != GR_DEPTHBUFFER_DISABLE)
	{
		glEnable(GL_DEPTH_TEST);
	}
	// Restore colormask
	bool rgb = Glide.State.ColorMask;
	glColorMask(rgb, rgb, rgb, Glide.State.AlphaMask);
	if (InternalConfig.EXT_secondary_color)
	{
		glEnable(GL_COLOR_SUM_EXT);
	}
	// Return to normal rendering
	glStencilFunc(GL_ALWAYS, 0x02, 0x03);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
	VERIFY_TEXTURE_ENABLED_STATE();
}

//*************************************************
//* Clear buffer
//*************************************************
FX_ENTRY void FX_CALL
grBufferClear( GrColor_t color, GrAlpha_t alpha, FxU16 depth )
{
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
	GlideMsg( "grBufferClear( %d, %d, %d )\n", color, alpha, depth );
#endif
	glReportErrors("grBufferClear");

	RenderDrawTriangles( );
	s_Framebuffer.OnBeforeBufferClear();

	unsigned int Bits = 0;
	if ( Glide.State.ColorMask )
	{
		Bits = GL_COLOR_BUFFER_BIT;
		float r, g, b, a;
		ConvertColorF(color, r, g, b, a);
		if (InternalConfig.GapFix & OpenGLideGapFixFlag_Enabled)
		{
			Bits |= GL_STENCIL_BUFFER_BIT;
			if (InternalConfig.GapFix & OpenGLideGapFixFlag_Debug)
			{
				// By making the background green,
				// holes in the geometry are easy to spot
				g = 1.0f;
			}
		}
		glClearColor(r, g, b, a);
		glReportError();
	}

	if ( Glide.State.DepthBufferWritting)
	{
		glClearDepth( depth * D1OVER65535 );
		glReportError();
		Bits |= GL_DEPTH_BUFFER_BIT;
	}

	if ( ! OpenGL.Clipping )
	{
	    glClear( Bits );
	    glReportError();
	}
	else
	{
		glEnable( GL_SCISSOR_TEST );
		glClear( Bits );
		glReportError();
		glDisable( GL_SCISSOR_TEST );
	}
	
	// In Tomb Raider 1, the last column/row of framebuffer pixels contains gapfix rendering artefacts.
	// This happens, because TR1 doesn't render the last column/row. Because the gapfix actually renders
	// into this area the stencil buffer value must be set to "pixel normally renderded"
	if ((InternalConfig.GapFix & OpenGLideGapFixFlag_Enabled) && (s_GlideApplication.GetType() == GlideApplication::TombRaiderI))
	{
		// write "pixel normally rendered" stencil value in the last column/row
		gapfixSetSimpleColorState();
		if (InternalConfig.GapFix & OpenGLideGapFixFlag_Debug)
		{
			// Write Cyan colored pixels
			GLfloat clearcolor[] = 
			{
				0.0f, 1.0f, 1.0f, 0.0f
			};
			glColorMask(true, true, true, false);
			glColor4fv(clearcolor);
		}
		// Stencil already set to "pixel normally rendered"
		const GLfloat xmin = 0.0;
		const GLfloat ymin = 0.0;
		const GLfloat xmax = Glide.WindowWidth;
		const GLfloat ymax = Glide.WindowHeight;
		const GLfloat depth = 1.0f;
		// Because we're rendering a quad, one more line maybe not gap-fixed
		// Bottom
		glBegin(GL_QUADS);
			glVertex3f(xmin, ymax - 1.0, depth);
			glVertex3f(xmax, ymax - 1.0, depth);
			glVertex3f(xmax, ymax, depth);
			glVertex3f(xmin, ymax, depth);
		glEnd();
		// right
		glBegin(GL_QUADS);
			glVertex3f(xmax - 1.0, ymin, depth);
			glVertex3f(xmax - 1.0, ymax, depth);
			glVertex3f(xmax, ymax, depth);
			glVertex3f(xmax, ymin, depth);
		glEnd();
		gapfixRestoreFromSimpleColorState();
	}

	glReportError();
}

//*************************************************
//* Swaps front and back Buffer
//*************************************************
FX_ENTRY void FX_CALL
grBufferSwap(int swap_interval)
{
#if defined( OGL_DONE ) || defined( OGL_COMBINE )
    GlideMsg( "grBufferSwap( %d )\n", swap_interval );
#endif
	glReportErrors("grBufferSwap");

	RenderDrawTriangles();

#ifdef OGL_DEBUG
	static float Temp = 1.0f;
	if (OGLRender.FrameTriangles > OGLRender.MaxTriangles)
	{
		OGLRender.MaxTriangles = OGLRender.FrameTriangles;
	}
	OGLRender.FrameTriangles = 0;
#endif

	// Flush the framebuffer
	s_Framebuffer.OnBeforeBufferSwap();

	if (InternalConfig.GapFix & OpenGLideGapFixFlag_Enabled)
	{
		gapfixSetSimpleColorState();
		glStencilFunc(GL_NOTEQUAL, 0x02, 0x03);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glReportError();
		GLfloat clearcolor[] = 
		{
			0.0f, 0.0f, 0.0f, 0.0f
		};
		if (InternalConfig.GapFix & OpenGLideGapFixFlag_Enabled)
		{
			if (InternalConfig.GapFix & OpenGLideGapFixFlag_Debug)
			{
				// By making the gaps red, they're easily to spot
				clearcolor[0] = 1.0f;
			}
		}
		// @todo: Turn off clipping
		const FxU32 xmin = 0;
		const FxU32 ymin = 0;
		const GlideApplication::Type application = s_GlideApplication.GetType();
		const FxU32 xmax = Glide.WindowWidth - ((application == GlideApplication::TombRaiderI) ? 1 : 0);
		const FxU32 ymax = Glide.WindowHeight - ((application == GlideApplication::TombRaiderI) ? 1 : 0);
		const GLfloat depth = 1.0f;
		glBegin(GL_QUADS);
			glColor4fv(clearcolor);
			glVertex3f(xmin, ymin, depth);
			glColor4fv(clearcolor);
			glVertex3f(xmax, ymin, depth);
			glColor4fv(clearcolor);
			glVertex3f(xmax, ymax, depth);
			glColor4fv(clearcolor);
			glVertex3f(xmin, ymax, depth);
		glEnd();

		gapfixRestoreFromSimpleColorState();
	}
	
	// @todo: setting the swap interval doesn't seem to work
	// - the 3Dfx splash animation appears a bit to fast
	//   and using higher values doesn't work
	// - happens in Classic as well as in OS9 native
	GLint swap = swap_interval;
	aglSetInteger(pWindowInfo->aglContext, AGL_SWAP_INTERVAL, &swap);
	aglReportError();
#if defined (OGL_DONE) || defined (OGL_PARTDONE) || defined (OGL_NOTDONE) || defined(OGL_OPTIMISE_DEBUG)
	GlideMsg("Calling aglSwapBuffers() with swapinterval %d\n", swap);
#endif
 	// swap buffers
	aglSwapBuffers(pWindowInfo->aglContext);
	s_Framebuffer.OnAfterBufferSwap();

#ifdef OGL_DEBUG
//    RDTSC( FinalTick );
//    Temp = (float)(FinalTick - InitialTick);
//    FpsAux += Temp;
//    Frame++;
//    RDTSC( InitialTick );
#endif
}

//*************************************************
//* Return the number of queued buffer swap requests
//* Always 0, never pending
//*************************************************
FX_ENTRY int FX_CALL
grBufferNumPending( void )
{
#ifdef OGL_DONE
	GlideMsg( "grBufferNumPending( ) = 0\n" );
#endif

	RenderDrawTriangles();
	// s_Framebuffer.OnBufferNumPending();
	return 0; 
}

//*************************************************
//* Defines the Buffer to Render
//*************************************************
FX_ENTRY void FX_CALL
grRenderBuffer( GrBuffer_t dwBuffer )
{
#ifdef OGL_DONE
	GlideMsg( "grRenderBuffer( %d )\n", dwBuffer );
#endif
	glReportErrors("grRenderBuffer");

	RenderDrawTriangles( );
	Glide.State.RenderBuffer = dwBuffer;
	// Valid parameters are only FRONT and BACK ( 0x0 and 0x1 )
	OpenGL.RenderBuffer = GL_FRONT + dwBuffer;
	glDrawBuffer( OpenGL.RenderBuffer );
	glReportError();
}

