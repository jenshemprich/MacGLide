//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                      Utility File
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Glide.h"
#include "GlideDisplay.h"
#include "GlideSettings.h"
#include "GLExtensions.h"
#include "GLUtil.h"
#include "PGTexture.h"


OSStatus aglReportWarning(void)
{
	GLenum err = aglGetError();
	if (AGL_NO_ERROR != err)
	{
		GlideMsg("AGL-Error: %s\n", reinterpret_cast<const char *>(aglErrorString(err)));
	}
	// ensure we are returning an OSStatus noErr if no error condition
	if (err == AGL_NO_ERROR)
		return noErr;
	else
		return (OSStatus) err;
}

// Gobals for initialising the display
WindowPtr pGLWindow = NULL;
structWindowInfoPtr pWindowInfo = NULL;

short SetAGLAttributes(GLint* aglAttributes, short index)
{
	// The maximun depth buffer size will be requested
	// by choosing AGL_MAXIMUM_POLICY
	if (UserConfig.DepthBufferBits < 0) UserConfig.DepthBufferBits = 0;
	if (UserConfig.DepthBufferBits > 32) UserConfig.DepthBufferBits = 32;
	GLint depthbufferbits = UserConfig.DepthBufferBits;
	// 0 triggers the maximun number of available depth
	// buffer bits to be used but at least 16
	if (depthbufferbits == 0)
	{
		depthbufferbits = 16;
	}
	aglAttributes[index++] = AGL_RGBA;
	aglAttributes[index++] = AGL_ALPHA_SIZE;
	aglAttributes[index++] = 0; // Alpha not supported on Glide Cards with depth buffering
	aglAttributes[index++] = AGL_DOUBLEBUFFER;
	// ARB_multisample doesn't seem to work in Classic,
	// but maybe someone wants to port this to Carbon...
	/*
	if (UserConfig.FullSceneAntiAliasing > 0)
	{
		aglAttributes[index++] = AGL_SAMPLE_BUFFERS_ARB;
		aglAttributes[index++] = 1;
		aglAttributes[index++] = AGL_SAMPLES_ARB;
		aglAttributes[index++] = UserConfig.FullSceneAntiAliasing > 0;
	}
	*/
	aglAttributes[index++] = AGL_NO_RECOVERY;
	if (UserConfig.DepthBufferBits == 0) aglAttributes[index++] = AGL_MAXIMUM_POLICY;
	aglAttributes[index++] = AGL_DEPTH_SIZE;
	aglAttributes[index++] = depthbufferbits;
	if (UserConfig.DisplayMode == OpenGLideDisplayMode_aglSetFullScreen)
	{
		aglAttributes[index++] = AGL_FULLSCREEN;
	}
	if (UserConfig.GapFix & OpenGLideGapFixFlag_Enabled)
	{
		aglAttributes[index++] = AGL_STENCIL_SIZE;
		aglAttributes[index++] = AGL_2_BIT;
	}
	aglAttributes[index] = AGL_NONE;
	return index;
}

// This function can only use settings from UserConfig,
// as it's called before InternalConfig
bool InitialiseOpenGLWindow(FxU32 hwnd, int x, int y, FxU32 width, FxU32 height )
{
	glReportErrors("InitialiseOpenGLWindow");
	// Adjust the origin if the selected display resolution
	// is greater than the OpenGL resolution
	unsigned long display_width = 0;
	unsigned long display_height = 0;
	OpenGL.OriginX = 0;
	OpenGL.OriginY = 0;
	Rect rectWin = {y, x , height, width};
	// Change the screen resolution?
	if (UserConfig.DisplayMode == OpenGLideDisplayMode_DisplayManager ||
	    UserConfig.DisplayMode == OpenGLideDisplayMode_aglSetFullScreen)
	{
		OSErr err = DisplayManager_RememberPassthroughDisplay();
		if (err == noErr)
		{	
			// Avoid flicker during screen resolution switching
			#ifndef OGL_DEBUG
				DisplayManager_SetPassthroughDisplayGammaBlack();
			#endif
			long freq = UserConfig.MonitorRefreshRate;
			// sort out illegal values
			if (freq < 60) freq = 0;
			// check whether to use the refresh rate requested by the Glide application
			if (freq == 0) freq = OpenGL.Refresh;
			// Allows to play Falcon 4.0 in True Color and to play Tomb Raider Gold 1+2 with movies enabled.
			// However, Tomb Raider II < 1.03 has problems displaying Lara and other in-game objects.
			err = DisplayManager_SetGlideDisplay(width, height, freq); 
			if (err != noErr)
			{
				if (width < 640)
				{
					// Not all displays support all Glide resolutions,
					// for instance the LCD-iMacs don't support 512x384
					GlideMsg("Error: Resolution %dx%d@%dHz not found. Trying double resolution %dx%d@%dHz.\n", width, height, freq, width << 1, height << 1, freq);
					// Restore gamma first
					err = DisplayManager_SetGlideDisplay(width << 1, height << 1, freq);
					if (err == noErr)
					{
						width = width << 1;
						height = height << 1;
						OpenGL.WindowWidth = width;
						OpenGL.WindowHeight = height;
					}
				}
			}
		}
		if (err != noErr)
		{
			// If changing the screensize failed (for instance
			// when the calling app checks for available resolutions)
			// the gamma correction table must be restored or
			// we may end up with a black screen
			DisplayManager_RestorePassthroughDisplayGamma();
			return false;
		}
		if (DisplayManager_GetGlideDisplayResolution(display_width, display_height))
		{
			// Position the viewport in the middle of the screen
			OpenGL.OriginX = (display_width - width) / 2;
			OpenGL.OriginY = (display_height - height) / 2;
			rectWin.right = display_width;
			rectWin.bottom = display_height;
		}
	}
	else
	{
		if (DisplayManager_GetDesktopDisplayResolution(display_width, display_height))
		{
			// Position the window in the middle of the screen
			rectWin.left = (display_width - width) / 2;
			rectWin.top = (display_height - height) / 2;
		}
		else
		{
			rectWin.left = 40;
			rectWin.top = 60;
		}
		rectWin.right = rectWin.left + width;
		rectWin.bottom = rectWin.top + height;
	}
	// build window
	pWindowInfo = (structWindowInfoPtr) NewPtrSysClear (sizeof (structWindowInfo));
	OSStatus err = CreateNewWindow(kDocumentWindowClass, kWindowNoAttributes, &rectWin, &pGLWindow);
	if (pGLWindow)
	{
		ShowWindow (pGLWindow);
		SetPortWindowPort (pGLWindow);
		pWindowInfo->glInfo.fDraggable = false;
		pWindowInfo->glInfo.fmt = 0;
		pWindowInfo->glInfo.fAcceleratedMust = true;
		pWindowInfo->glInfo.VRAM = 0;
		pWindowInfo->glInfo.textureRAM = 0;
		SetAGLAttributes(pWindowInfo->glInfo.aglAttributes, 0);
		BuildGLFromWindow(pGLWindow, &pWindowInfo->aglContext, &pWindowInfo->glInfo, NULL);
		if (pWindowInfo->aglContext)
		{
			// This does not work reliably
			/*
			if (UserConfig.DisplayMode == OpenGLideDisplayMode_aglSetFullScreen)
			{
				long freq = UserConfig.MonitorRefreshRate;
				// sort out illegal values
				if (freq < 60) freq = 0;
				// check whether to use the refresh rate requested by the Glide application
				if (freq == 0) freq = OpenGL.Refresh;
				// aglSetDrawable(pWindowInfo->aglContext, (AGLDrawable) NULL);
				if (aglSetFullScreen(pWindowInfo->aglContext, width, height, freq, 0))
				{
					if (aglSetCurrentContext(pWindowInfo->aglContext))
					{
						aglUpdateContext(pWindowInfo->aglContext);
					}
					else
					{
						aglReportWarning();
						GlideError("aglSetCurrentContext() after aglSetFullScreen() failed\n");
						// shutdown full screen
				 		aglSetCurrentContext(pWindowInfo->aglContext);
						aglReportError();
						aglSetDrawable(pWindowInfo->aglContext, (AGLDrawable) GetWindowPort(pGLWindow));
						aglReportError();
						aglUpdateContext(pWindowInfo->aglContext);
						aglReportError();
					}
				}
				else
				{
					aglReportWarning();
					GlideError("aglSetFullScreen() failed\n");
					// shutdown full screen
			 		aglSetCurrentContext(pWindowInfo->aglContext);
					aglReportError();
					aglSetDrawable(pWindowInfo->aglContext, (AGLDrawable) GetWindowPort(pGLWindow));
					aglReportError();
					aglUpdateContext(pWindowInfo->aglContext);
					aglReportError();
				}
			}
			else
			{
		 		if (aglSetCurrentContext(pWindowInfo->aglContext))
		 		{
					aglUpdateContext(pWindowInfo->aglContext);
					aglReportError();
				}
				else
				{
					aglReportWarning();
					GlideError("aglSetCurrentContext() failed\n");
				}					
			}
			*/
			// Keep renderers in memory
			aglConfigure(AGL_RETAIN_RENDERERS, GL_TRUE);
			aglReportWarning();
			// And minimise cache size
			aglConfigure(AGL_FORMAT_CACHE_SIZE, 1);
			aglReportWarning();
			// Don't render ahead
			bool runningInClassic = strstr(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)), "GL_EXT_fog_coord") != NULL;
			if (runningInClassic)
			{
				aglEnable(pWindowInfo->aglContext, AGL_SWAP_LIMIT);
				aglReportWarning();
			}
		}
		else
		{
			aglReportWarning();
			DestroyGLFromWindow(&pWindowInfo->aglContext, &pWindowInfo->glInfo);
			GlideError("couldn't create agl context");
			return false;
		}
	}
	else
	{
		GlideError("Couldn't create window");
	}
	// clear color buffer to black to minimmise white screen flash on resolution change
	// and white top/bottoms when displaying widescreen movies on 4/3 screens
	glDrawBuffer(GL_BACK);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	GLint swap = 0;
	aglSetInteger(pWindowInfo->aglContext, AGL_SWAP_INTERVAL, &swap);
	aglSwapBuffers(pWindowInfo->aglContext);
	aglReportError();
	
	if (UserConfig.GapFix & OpenGLideGapFixFlag_Enabled)
	{
		glEnable(GL_STENCIL_TEST);
		glStencilMask(0x03);
		glClearStencil(0x00);
		glStencilFunc(GL_ALWAYS, 0x02, 0x03);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glReportError();
	}
	
	// The gamma correction value is restored in grSstWinOpen()
	// so we don't have to do anything here
	return true;
}    

void FinaliseOpenGLWindow(void)
{
	s_Framebuffer.Cleanup();
	if (UserConfig.DisplayMode == OpenGLideDisplayMode_DisplayManager ||
	    UserConfig.DisplayMode == OpenGLideDisplayMode_aglSetFullScreen)
	{
		#ifndef OGL_DEBUG
			DisplayManager_SetGlideDisplayGammaBlack();
		#endif
	}
	if (pWindowInfo)
	{
		// DeleteFontGL (pWindowInfo->fontList);
		// must clean up failure to do so will result in an Unmapped Memory Exception
		DestroyGLFromWindow(&pWindowInfo->aglContext, &pWindowInfo->glInfo);
		DisposePtr ((Ptr) pWindowInfo);
		pWindowInfo = NULL;
	}
	if (pGLWindow)
	{
		DisposeWindow (pGLWindow);
		pGLWindow = NULL;
	}
	// Cleanup display manager?
	if (UserConfig.DisplayMode == OpenGLideDisplayMode_DisplayManager ||
		  UserConfig.DisplayMode == OpenGLideDisplayMode_aglSetFullScreen)
	{
		DisplayManager_RestorePassthroughDisplay();
	}
}

void HideOpenGLWindow()
{
	if (UserConfig.DisplayMode == OpenGLideDisplayMode_DisplayManager ||
		  UserConfig.DisplayMode == OpenGLideDisplayMode_aglSetFullScreen)
	{
#ifndef OGL_DEBUG
		DisplayManager_SetGlideDisplayGammaBlack();
#endif
	}
	// Hide window by moving it offscreen, allowing some extra offset to hide the bar
	MoveWindow(pGLWindow, OpenGL.WindowWidth + 64, OpenGL.WindowHeight + 64, FALSE);
	// Actually HideWIndow() should be working, but it blacks out
	// the Movies in Tomb Raider Gold
	// HideWindow(pGLWindow);
	aglUpdateContext(pWindowInfo->aglContext);
	aglReportError();
	if (UserConfig.DisplayMode == OpenGLideDisplayMode_DisplayManager ||
		  UserConfig.DisplayMode == OpenGLideDisplayMode_aglSetFullScreen)
	{
		DisplayManager_RestorePassthroughDisplay();
		aglUpdateContext(pWindowInfo->aglContext);
		aglReportError();
	}
}

void RestoreOpenGLWindow()
{
	// Change the screen resolution?
	if (UserConfig.DisplayMode == OpenGLideDisplayMode_DisplayManager ||
	    UserConfig.DisplayMode == OpenGLideDisplayMode_aglSetFullScreen)
	{
		OSErr err = DisplayManager_RememberPassthroughDisplay();
		if (err == noErr)
		{	
			// Avoid flicker during screen resolution switching
#ifndef OGL_DEBUG
			DisplayManager_SetPassthroughDisplayGammaBlack();
#endif
		}
	}
	DisplayManager_RestoreGlideDisplay();
	// 0,0 is only correct in fullscreen modes where the OpenGL window matches the
	// Glide screen resolution.
	MoveWindow(pGLWindow, 0, 0, FALSE);
	// Actually HideWIndow() should be working, but it blacks out the
	// Movies in Tomb Raider Gold
	// ShowWindow(pGLWindow);
	aglUpdateContext(pWindowInfo->aglContext);
	aglReportError();
	SelectWindow(pGLWindow);
	aglUpdateContext(pWindowInfo->aglContext);
	aglReportError();
	// @todo: Sometimes textures in Descent look wierd
	// Clearing the textues shouldn't be necessary,
	// so updating the context might not be enough.
	// to restore the textures (they look wierd after showing he window)
	// Textures->Clear();
	DisplayManager_SetGlideDisplayGamma(Glide.State.Gamma);

}

void ConvertColorB( GrColor_t GlideColor, FxU8 &R, FxU8 &G, FxU8 &B, FxU8 &A )
{
    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        A = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        R = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        G = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        B = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        A = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        B = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        G = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        R = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        R = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        G = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        B = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        A = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        B = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        G = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        R = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        A = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;
    }
}

GrColor_t ConvertConstantColor( float R, float G, float B, float A )
{
    GrColor_t r = (GrColor_t) R;
    GrColor_t g = (GrColor_t) G;
    GrColor_t b = (GrColor_t) B;
    GrColor_t a = (GrColor_t) A;

    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        return ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | r;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        return ( r << 24 ) | ( g << 16 ) | ( b << 8 ) | a;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        return ( b << 24 ) | ( g << 16 ) | ( r << 8 ) | a;
    }

    return 0;
}

void ConvertColorF( GrColor_t GlideColor, float &R, float &G, float &B, float &A )
{
    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        A = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        R = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        B = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        A = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        B = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        R = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        R = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        B = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        A = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        B = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        R = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        A = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;
    }
}

