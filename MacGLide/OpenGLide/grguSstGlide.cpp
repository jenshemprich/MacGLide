//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                     Sst Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Glide.h"
#include "GlideApplication.h"
#include "GlideDisplay.h"
#include "GlideSettings.h"
#include "GLExtensions.h"
#include "GLRender.h"
#include "GLUtil.h"
#include "OGLTables.h"
#include "PGTexture.h"


//*************************************************
//* Returns the current Glide Version
//*************************************************
FX_ENTRY void FX_CALL
grGlideGetVersion( char version[80] )
{
#ifdef OGL_DONE
    GlideMsg( "grGlideGetVersion( --- )\n" );
#endif
    sprintf(version, "Glide 2.45 - %s %s", OpenGLideProductName, OpenGLideVersion);
}

//*************************************************
//* Initializes what is needed
//*************************************************
FX_ENTRY void FX_CALL
grGlideInit( void )
{
#ifdef OGL_DONE
    GlideMsg( "grGlideInit( )\n" );
#endif

    if ( OpenGL.GlideInit )
    {
        grGlideShutdown( );
    }
    memset( &Glide, 0, sizeof( GlideStruct ) );
    memset( &OpenGL, 0, sizeof( OpenGLStruct ) );
    Glide.ActiveVoodoo      = 0;
    Glide.State.VRetrace    = FXTRUE;
    ExternErrorFunction = NULL;
/*
#ifdef OGL_DEBUG
    RDTSC( FinalTick );
    RDTSC( InitialTick );
    Fps = FpsAux = Frame = 0;
#endif
*/
    OpenGL.GlideInit = true;
    Glide.TextureMemory = UserConfig.TextureMemorySize * 1024 * 1024;
    Textures = new PGTexture( Glide.TextureMemory );
    if ( Textures == NULL )
    {
        GlideError( "Cannot allocate enough memory for Texture Buffer in User setting, using default" );
    }
    Glide.TexMemoryMaxPosition  = (FxU32)Glide.TextureMemory;
}

//*************************************************
//* Finishes everything
//*************************************************
FX_ENTRY void FX_CALL
grGlideShutdown( void )
{
    if ( !OpenGL.GlideInit )
    {
        return;
    }
    OpenGL.GlideInit = false;

#ifdef OGL_DEBUG
    RDTSC( FinalTick );
#endif
#ifdef OGL_DONE
    GlideMsg( "grGlideShutdown()\n" );
#endif

    grSstWinClose( );
    if (Textures) delete Textures;
}

//*************************************************
//* Sets all Glide State Variables
//*************************************************
FX_ENTRY void FX_CALL
grGlideSetState( const GrState *state )
{
#ifdef OGL_PARTDONE
	GlideMsg( "grGlideSetState( --- )\n" );
#endif

	// Ensure the state size is not too large		
	assert(sizeof(GlideState) < sizeof(GrState));
		
	GlideState StateTemp;
	memcpy( &StateTemp, state, sizeof( GlideState ) );
	Glide.State.ColorFormat = StateTemp.ColorFormat;
	grRenderBuffer( StateTemp.RenderBuffer );
	grDepthBufferMode( StateTemp.DepthBufferMode );
	grDepthBufferFunction( StateTemp.DepthFunction );
	grDepthMask( StateTemp.DepthBufferWritting );
	grDepthBiasLevel( StateTemp.DepthBiasLevel );
	grDitherMode( StateTemp.DitherMode );
	grChromakeyValue( StateTemp.ChromakeyValue );
	grChromakeyMode( StateTemp.ChromaKeyMode );
	grAlphaTestReferenceValue( StateTemp.AlphaReferenceValue );
	grAlphaTestFunction( StateTemp.AlphaTestFunction );
	grColorMask( StateTemp.ColorMask, StateTemp.AlphaMask );
	grConstantColorValue( StateTemp.ConstantColorValue );
	grFogColorValue( StateTemp.FogColorValue );
	grFogMode( StateTemp.FogMode );
	grCullMode( StateTemp.CullMode );
	grTexClampMode( GR_TMU0, StateTemp.SClampMode, StateTemp.TClampMode );
	grTexFilterMode( GR_TMU0, StateTemp.MinFilterMode, StateTemp.MagFilterMode );
	grTexMipMapMode( GR_TMU0, StateTemp.MipMapMode, StateTemp.LodBlend );
	grColorCombine( StateTemp.ColorCombineFunction, StateTemp.ColorCombineFactor, 
	                StateTemp.ColorCombineLocal, StateTemp.ColorCombineOther, StateTemp.ColorCombineInvert );
	grAlphaCombine( StateTemp.AlphaFunction, StateTemp.AlphaFactor, StateTemp.AlphaLocal, StateTemp.AlphaOther, StateTemp.AlphaInvert );
	grTexCombine( GR_TMU0, StateTemp.TextureCombineCFunction, StateTemp.TextureCombineCFactor,
	              StateTemp.TextureCombineAFunction, StateTemp.TextureCombineAFactor,
	              StateTemp.TextureCombineRGBInvert, StateTemp.TextureCombineAInvert );
	grAlphaBlendFunction( StateTemp.AlphaBlendRgbSf, StateTemp.AlphaBlendRgbDf, StateTemp.AlphaBlendAlphaSf, StateTemp.AlphaBlendAlphaDf );
	grClipWindow( StateTemp.ClipMinX, StateTemp.ClipMinY, StateTemp.ClipMaxX, StateTemp.ClipMaxY );
	grSstOrigin( StateTemp.OriginInformation );
	grTexSource( GR_TMU0, StateTemp.TexSource.StartAddress, StateTemp.TexSource.EvenOdd, &StateTemp.TexSource.Info );
	grTexLodBiasValue(GR_TMU0, StateTemp.LodBias);
}

//*************************************************
//* Gets all Glide State Variables
//*************************************************
FX_ENTRY void FX_CALL
grGlideGetState( GrState *state )
{
#ifdef OGL_PARTDONE
	GlideMsg( "grGlideGetState( --- )\n" );
#endif

	// Ensure the state size is not too large		
	assert(sizeof(GlideState) < sizeof(GrState));

	memcpy( state, &Glide.State, sizeof( GlideState ) );
}

//*************************************************
FX_ENTRY void FX_CALL
grGlideShamelessPlug( const FxBool on )
{
#ifdef OGL_DONE
    GlideMsg( "grGlideShamelessPlug( %d )\n", on );
#endif
	InternalConfig.ShamelessPlug = on;
}

//*************************************************
//* Returns the number of Voodoo Boards Instaled
//*************************************************
FX_ENTRY FxBool FX_CALL
grSstQueryBoards( GrHwConfiguration *hwConfig )
{
#ifdef OGL_DONE
    GlideMsg( "grSstQueryBoards( --- )\n" );
#endif

    memset(hwConfig, 0, sizeof(GrHwConfiguration));
    hwConfig->num_sst = 1;

    return FXTRUE;
}

//*************************************************
FX_ENTRY FxBool FX_CALL
grSstWinOpen(FxU32 hwnd,
             GrScreenResolution_t res,
             GrScreenRefresh_t ref,
             GrColorFormat_t cformat,
             GrOriginLocation_t org_loc,
             int num_buffers,
             int num_aux_buffers)
{
	if (OpenGL.WinOpen)
	{
		grSstWinClose();
	}
	// Some games read from the buffer after the window has been closed
	// As a result, freeing the read buffer must be deferred until the
	// next call to grSstWinOpen() or unloading the library
	if (Glide.ReadBuffer.Address)
	{
		FreeFrameBuffer(Glide.ReadBuffer.Address);
		Glide.ReadBuffer.Address = NULL;
	}
#ifdef OGL_DONE
		GlideMsg( "grSstWinOpen( %d, %d, %d, %d, %d, %d, %d )\n",
				hwnd, res, ref, cformat, org_loc, num_buffers, num_aux_buffers );
#endif
	Glide.Resolution = res;
#ifdef OGL_DEBUG
		if ( Glide.Resolution > GR_RESOLUTION_400x300 )
		{
				GlideError( "grSstWinOpen: res = GR_RESOLUTION_NONE\n" );
				return FXFALSE;
		}
		if ( Glide.Refresh > GR_REFRESH_120Hz )
		{
				GlideError( "grSstWinOpen: Refresh Incorrect\n" );
				return FXFALSE;
		}
#endif
	Glide.WindowWidth = windowDimensions[Glide.Resolution].width;
	Glide.WindowHeight = windowDimensions[Glide.Resolution].height;
	// Set the size of the opengl window (might be different from Glide window size)
	if (UserConfig.Resolution == 0)
	{
		// Does only work if the library is loaded at startup, before the game changes the screen resolution
		if (!DisplayManager_GetDesktopDisplayResolution(OpenGL.WindowWidth, OpenGL.WindowHeight))
		{
			// Use the resuolution requested by the game
			OpenGL.WindowWidth = Glide.WindowWidth;
			OpenGL.WindowHeight = Glide.WindowHeight;
		}
	}
	else if (UserConfig.Resolution <= 16)
	{
		// multiply the original size by the resolution factor
		OpenGL.WindowWidth = Glide.WindowWidth * UserConfig.Resolution;
		OpenGL.WindowHeight = Glide.WindowHeight * UserConfig.Resolution;
	}
	else
	{
		// override the resolution
		OpenGL.WindowWidth = UserConfig.Resolution;
		// Glide games have a fixed 4/3 aspect ratio
		OpenGL.WindowHeight = UserConfig.Resolution * 3 / 4;
	}
	// Limit the display size to the max allowed screen resolution
	if (UserConfig.ResolutionCap)
	{
		OpenGL.WindowWidth = min(OpenGL.WindowWidth,
		                         UserConfig.ResolutionCap);
		OpenGL.WindowHeight = min(OpenGL.WindowHeight,
		                          UserConfig.ResolutionCap * 3 / 4);
	}
	Glide.WindowTotalPixels = Glide.WindowWidth * Glide.WindowHeight;
	Glide.Refresh = ref;
	Glide.State.ColorFormat = cformat;
	Glide.NumBuffers        = num_buffers;
	Glide.AuxBuffers        = num_aux_buffers;
	OpenGL.Refresh = windowRefresh[ Glide.Refresh ];
	OpenGL.WaitSignal = (FxU32)( 1000 / OpenGL.Refresh );
	memset(&Glide.FrameBuffer, 0, sizeof(BufferStruct));
	memset(&Glide.TempBuffer, 0, sizeof(BufferStruct));
	memset(&Glide.ReadBuffer, 0, sizeof(BufferStruct));
	// Initing OpenGL Window
	if (!InitWindow(hwnd))
	{
		return FXFALSE;
	}
	// Note: The OpenGL resolution might have changed during the call to InitWindow().
	// As a result, buffers must be allocated afterwards
	const unsigned long openglpixels = OpenGL.WindowWidth * OpenGL.WindowHeight;
	// As the lfb write format isn't known yet we must allocate a framebuffer for 32bit color formats
	// although most games will use 16bit corlor formats only 
	Glide.FrameBuffer.Address = (FxU16*) AllocFrameBuffer(Glide.WindowTotalPixels + openglpixels, 4);
	// >> 1 as the framebuffer is allocated for 32 bit color formats but the pointer is declared as a short
	Glide.TempBuffer.Address = &Glide.FrameBuffer.Address[Glide.WindowTotalPixels << 1];
	memset(Glide.FrameBuffer.Address, 0, Glide.WindowTotalPixels * sizeof(FxU32));
	memset(Glide.TempBuffer.Address, 0, openglpixels * sizeof(FxU32));
	// Prealloc readbuffer for Carmageddon, because allocating it on demand
	// (when moving the cursor in Movie mode) would produce an OutOfMemory error)
	const bool preallocateReadBuffer = s_GlideApplication.GetType() == GlideApplication::Carmageddon;
	if (preallocateReadBuffer)
	{
		Glide.ReadBuffer.Address = (FxU16*) AllocFrameBuffer(Glide.WindowTotalPixels, sizeof(FxU16));
#ifdef OPENGL_DEBUG
		GlideMsg("Allocated Readbuffer(%dx%d) at 0x%x\n",
		           Glide.WindowWidth, Glide.WindowHeight, Glide.ReadBuffer.Address);
#endif							
	}
	// Initialise the frame buffer emulation
	RenderInitialize();
	s_Framebuffer.initialise(&Glide.FrameBuffer, &Glide.TempBuffer);
	Textures->initOpenGL();
	#ifdef OGL_DONE
		GlideMsg( "----Start of grSstWinOpen()\n" );
	#endif
	// All of this should be disabled: depth buffering, fog, chroma-key, alpha blending, alpha testing
	#ifdef OPTIMISE_GLIDE_STATE_CHANGES
		// When state change optimising is enabled, passing in default values of 0
		// would not initialise the corresponding values in the OpenGL struct,
		// because the optimising code would assume that the state has already been set.
		// By writing values other than 0 to glide state variables which will
		// become 0 below, the OpenGL values are initialises as exspected.
		// For functions that are called by other functions during the
		// initialisation, like grColorCombine, explicit default values
		// are given,  (in this case by grAlphaCombine)
		// grTexClampMode
		Glide.State.SClampMode = -1;
		Glide.State.TClampMode = -1;
		// grTexMipMapMode
		Glide.State.MipMapMode = -1;
		Glide.State.LodBlend = -1;
		// grCullMode
		Glide.State.CullMode = -1;
		// grDepthMask
		Glide.State.DepthBufferWritting = -1;
		// grDepthBufferMode
		Glide.State.DepthBufferMode = -1;
		// grChromakeyValue
		Glide.State.ChromakeyValue = -1;
		// grAlphaTestReferenceValue
		Glide.State.AlphaReferenceValue = -1.0f;
		// grDepthBiasLevel
		Glide.State.DepthBiasLevel = -1;
		// grFogMode
		Glide.State.FogMode = -1;
		// grFogColorValue
		Glide.State.FogColorValue = -1;
		// grHints
		Glide.State.STWHint = -1;
		// color combine
	    Glide.State.ColorCombineFunction = GR_COMBINE_FUNCTION_LOCAL;
	    Glide.State.ColorCombineFactor = GR_COMBINE_FACTOR_LOCAL;
	    Glide.State.ColorCombineLocal = GR_COMBINE_LOCAL_CONSTANT;
	    Glide.State.ColorCombineOther = GR_COMBINE_OTHER_CONSTANT;
	    Glide.State.ColorCombineInvert = FXTRUE;
		// alpha combine
	    Glide.State.AlphaFunction = GR_COMBINE_FUNCTION_LOCAL;
	    Glide.State.AlphaFactor = GR_COMBINE_FACTOR_LOCAL;
	    Glide.State.AlphaLocal = GR_COMBINE_LOCAL_CONSTANT;
	    Glide.State.AlphaOther = GR_COMBINE_OTHER_CONSTANT;
	    Glide.State.AlphaInvert = FXTRUE;
		// Blend
		Glide.State.AlphaBlendRgbSf = -1;
		Glide.State.AlphaBlendRgbDf = -1;
		Glide.State.AlphaBlendAlphaSf = -1;
		Glide.State.AlphaBlendAlphaDf = -1;
		// tex combine
		Glide.State.TextureCombineCFunction = GR_COMBINE_FUNCTION_ZERO;
		Glide.State.TextureCombineCFactor   = GR_COMBINE_FACTOR_NONE;
		Glide.State.TextureCombineAFunction = GR_COMBINE_FUNCTION_ZERO;
		Glide.State.TextureCombineAFactor   = GR_COMBINE_FACTOR_NONE;
		Glide.State.TextureCombineRGBInvert = FXFALSE;
		Glide.State.TextureCombineAInvert   = FXFALSE;
		// Alpha function
		Glide.State.AlphaTestFunction = -1;
		Glide.State.AlphaOther = -1;
		// Chromakeying
		Glide.State.ChromaKeyMode = GR_CHROMAKEY_ENABLE;
		Glide.State.ChromakeyValue = 0xffffffff;
		// constant color value
		Glide.State.ConstantColorValue = 0;
		// lod bias
		Glide.State.LodBias = -1;
	#endif
	Glide.State.OriginInformation = org_loc;
	grClipWindow( 0, 0, Glide.WindowWidth, Glide.WindowHeight );
	grSstOrigin( org_loc );
	grTexClampMode( 0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP );
	grTexMipMapMode( 0, GR_MIPMAP_DISABLE, FXFALSE );
	grTexFilterMode( 0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR );
	grFogMode( GR_FOG_DISABLE );
	grCullMode( GR_CULL_DISABLE );
	grRenderBuffer( GR_BUFFER_BACKBUFFER );
	grAlphaTestFunction( GR_CMP_ALWAYS );
	grDitherMode( GR_DITHER_4x4 );
	// grColorCombine depends on grAlphaCombine,
	// so we have to call grAlphaCombine first
	grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
	                GR_COMBINE_FACTOR_ONE,
	                GR_COMBINE_LOCAL_NONE,
	                GR_COMBINE_OTHER_CONSTANT,
	                FXFALSE );
	grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
	                GR_COMBINE_FACTOR_ONE,
	                GR_COMBINE_LOCAL_ITERATED,
	                GR_COMBINE_OTHER_ITERATED,
	                FXFALSE );
	grTexCombine( GR_TMU0,GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
              GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE );
	grAlphaControlsITRGBLighting( FXFALSE );
	grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
	grColorMask( FXTRUE, FXFALSE );
	grDepthMask( FXFALSE );
	grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );
	grDepthBufferFunction( GR_CMP_LESS );
	// chroma-key value, alpha test reference, constant depth value,
	// constant alpha value, etc.) and pixel rendering statistic counters 
	// are initialized to 0x00.
	grChromakeyMode( GR_CHROMAKEY_DISABLE );
	grChromakeyValue( 0x00 );
	grAlphaTestReferenceValue( 0x00 );
	grDepthBiasLevel( 0x00 );
	grFogColorValue( 0x00 );
	grConstantColorValue( 0xFFFFFFFF );
	grGammaCorrectionValue( 1.6f );
	grHints( GR_HINT_STWHINT, 0 );
	grTexLodBiasValue(GR_TMU0, 0.0f);
	
	#ifdef OGL_DONE
		GlideMsg( "----End of grSstWinOpen()\n" );
	#endif

	OpenGL.WinOpen = true;
	glFinish( );

	// Show the splash screen? (code copied from the linux driver src)
	if (UserConfig.NoSplash == false)
	{
		// Obsolete because state is preserved in grSplash
		// GrState state;
		// grGlideGetState(&state);
// @todo: There seem to be several versions of the splash screen
/*
#if (GLIDE_PLATFORM & GLIDE_OS_WIN32)
    grSstOrigin( GR_ORIGIN_LOWER_LEFT );
    if (!_GlideRoot.environment.noSplash) {
      HMODULE newSplash;
      if (newSplash = LoadLibrary("3dfxsplash2.dll")) {
        FARPROC fxSplash;
        if (fxSplash = GetProcAddress(newSplash, "_fxSplash@16")) {
          fxSplash(hWnd, gc->state.screen_width,
                   gc->state.screen_height, nAuxBuffers);  
          // _GlideRoot.environment.noSplash = 1;
          UserConfig.NoSplash = true;
        } 
      }
    }
#endif // (GLIDE_PLATFORM & GLIDE_OS_WIN32)
*/
	    /* If it's still 0, then do the old one */
		if (UserConfig.NoSplash == false)
		{
			grSplash(0.0f, 0.0f, 
			        static_cast<float>(Glide.WindowWidth),
			        static_cast<float>(Glide.WindowHeight),
			        0);
			// The splash screen is displayed once, and because the
			// internal config is reinitialised each time grWinOpen()
			// is called, the value must be reset in the user config
			UserConfig.NoSplash = true;
		}
		// grGlideSetState(&state);
	}

  return FXTRUE;
}

//*************************************************
//* Close the graphics display device
//*************************************************
FX_ENTRY void FX_CALL
grSstWinClose( void )
{
#ifdef OGL_DONE
    GlideMsg( "grSstWinClose()\n" );
#endif
    if ( ! OpenGL.WinOpen )
    {
        return;
    }

    OpenGL.WinOpen = false;

#ifdef OGL_DEBUG
	GlideMsg( OGL_LOG_SEPARATE );
	GlideMsg( "** Debug Information **\n" );
	GlideMsg( OGL_LOG_SEPARATE );
	GlideMsg( "MaxTriangles in Frame = %d\n", OGLRender.MaxTriangles );
	GlideMsg( "MaxTriangles in Sequence = %d\n", OGLRender.MaxSequencedTriangles );
	GlideMsg( "Mean value of Triangles in Sequence = %g\n",
	           static_cast<float>(OGLRender.OverallTriangles) / static_cast<float>(OGLRender.OverallRenderTriangleCalls));
	GlideMsg( "Overall triangles = %d\n", OGLRender.OverallTriangles);
	GlideMsg( "Overall lines = %d\n", OGLRender.OverallLines);
 	GlideMsg( "Overall points = %d\n", OGLRender.OverallPoints);
	GlideMsg( OGL_LOG_SEPARATE );
	GlideMsg( "MaxZ = %f\nMinZ = %f\n", OGLRender.MaxZ, OGLRender.MinZ );
	GlideMsg( "MaxX = %f\nMinX = %f\nMaxY = %f\nMinY = %f\n", 
	          OGLRender.MaxX, OGLRender.MinX, OGLRender.MaxY, OGLRender.MinY );
	GlideMsg( "MaxS = %f\nMinS = %f\nMaxT = %f\nMinT = %f\n", 
	          OGLRender.MaxS, OGLRender.MinS, OGLRender.MaxT, OGLRender.MinT );
	GlideMsg( "MaxF = %f\nMinF = %f\n", OGLRender.MaxF, OGLRender.MinF );
	GlideMsg( "MaxR = %f\nMinR = %f\n", OGLRender.MaxR, OGLRender.MinR );
	GlideMsg( "MaxG = %f\nMinG = %f\n", OGLRender.MaxG, OGLRender.MinG );
	GlideMsg( "MaxB = %f\nMinB = %f\n", OGLRender.MaxB, OGLRender.MinR );
	GlideMsg( "MaxA = %f\nMinA = %f\n", OGLRender.MaxA, OGLRender.MinA );
	GlideMsg( OGL_LOG_SEPARATE );
	GlideMsg( "Texture Information:\n" );
	GlideMsg( "  565 = %d\n", Textures->Num_565_Tex );
	GlideMsg( " c565 = %d\n", Textures->Num_565_Chromakey_Tex );
	GlideMsg( " 1555 = %d\n", Textures->Num_1555_Tex );
	GlideMsg( "c1555 = %d\n", Textures->Num_1555_Chromakey_Tex );
	GlideMsg( " 4444 = %d\n", Textures->Num_4444_Tex );
	GlideMsg( "c4444 = %d\n", Textures->Num_4444_Chromakey_Tex );
	GlideMsg( "  332 = %d\n", Textures->Num_332_Tex );
	GlideMsg( " 8332 = %d\n", Textures->Num_8332_Tex );
	GlideMsg( "Alpha = %d\n", Textures->Num_Alpha_Tex );
	GlideMsg( " AI88 = %d\n", Textures->Num_AlphaIntensity88_Tex );
	GlideMsg( " AI44 = %d\n", Textures->Num_AlphaIntensity44_Tex );
	GlideMsg( " AP88 = %d\n", Textures->Num_AlphaPalette_Tex );
	GlideMsg( "   P8 = %d\n", Textures->Num_Palette_Tex );
	GlideMsg( "  cP8 = %d\n", Textures->Num_Palette_Chromakey_Tex );
	GlideMsg( "Inten = %d\n", Textures->Num_Intensity_Tex );
	GlideMsg( "  YIQ = %d\n", Textures->Num_YIQ_Tex );
	GlideMsg( " AYIQ = %d\n", Textures->Num_AYIQ_Tex );
	GlideMsg( "Other = %d\n", Textures->Num_Other_Tex );
	GlideMsg( OGL_LOG_SEPARATE );
#endif

	if (s_GlideApplication.GetType() == GlideApplication::FutureCop)
	{
		// Store the read buffer for read out after the window has been closed
		// (needed by FutureCop LAPD)
		// @todo: writemode, buffer and origin are hardwired,
		// so this might not work for other games
#ifdef OGL_DEBUG
		GlideMsg("----MacGLide readbuffer copy----\n");
#endif
		GrLfbInfo_t lfbinfo;
		if (grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER, GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE, &lfbinfo))
		{
			grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER);
		}
#ifdef OGL_DEBUG
		GlideMsg(OGL_LOG_SEPARATE);
#endif
	}
	
	Textures->cleanupOpenGL();
	GLExtensionsCleanup();
	RenderFree();
	FinaliseOpenGLWindow();

	if (Glide.FrameBuffer.Address)
	{
		FreeFrameBuffer(Glide.FrameBuffer.Address);
		Glide.FrameBuffer.Address = NULL;
		Glide.TempBuffer.Address = NULL;
	}
	// Freeing the readbuffer is be deferred until
	// reopening the window or unloading the library
}

//*************************************************
//* Returns the Hardware Configuration
//*************************************************
FX_ENTRY FxBool FX_CALL
grSstQueryHardware( GrHwConfiguration *hwconfig )
{
#ifdef OGL_DONE
	GlideMsg( "grSstQueryHardware( --- )\n" );
#endif

	hwconfig->num_sst = 1;
	hwconfig->SSTs[0].type = UserConfig.BoardType;

	switch (hwconfig->SSTs[0].type)
	{
	case GR_SSTTYPE_VOODOO:
		hwconfig->SSTs[0].sstBoard.VoodooConfig.fbRam = UserConfig.FrameBufferMemorySize;
		// GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS and GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS
		// modes are not available in revision 1 of the Pixelfx chip
		hwconfig->SSTs[0].sstBoard.VoodooConfig.fbiRev = 2;
		hwconfig->SSTs[0].sstBoard.VoodooConfig.nTexelfx = UserConfig.GlideTextureUnits;
		hwconfig->SSTs[0].sstBoard.VoodooConfig.sliDetect = FXFALSE;
		for(int tmu = 0; tmu < UserConfig.GlideTextureUnits; tmu++)
		{
			hwconfig->SSTs[tmu].sstBoard.VoodooConfig.tmuConfig[0].tmuRev = 1;
			hwconfig->SSTs[tmu].sstBoard.VoodooConfig.tmuConfig[0].tmuRam = UserConfig.TextureMemorySize;
		}
		break;
	case GR_SSTTYPE_SST96:
		hwconfig->SSTs[0].sstBoard.SST96Config.fbRam = UserConfig.FrameBufferMemorySize;
		hwconfig->SSTs[0].sstBoard.SST96Config.nTexelfx = UserConfig.GlideTextureUnits;
		hwconfig->SSTs[0].sstBoard.SST96Config.tmuConfig.tmuRev = 1;
		hwconfig->SSTs[0].sstBoard.SST96Config.tmuConfig.tmuRam = UserConfig.TextureMemorySize;
		break;
	case GR_SSTTYPE_AT3D:
		hwconfig->SSTs[0].sstBoard.AT3DConfig.rev = 1; // whatsoever
		break;
	case GR_SSTTYPE_Voodoo2:
		hwconfig->SSTs[0].sstBoard.Voodoo2Config.fbRam = UserConfig.FrameBufferMemorySize;
		// GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS and GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS
		// modes are not available in revision 1 of the Pixelfx chip
		hwconfig->SSTs[0].sstBoard.Voodoo2Config.fbiRev = 2;
		hwconfig->SSTs[0].sstBoard.Voodoo2Config.nTexelfx = UserConfig.GlideTextureUnits;
		hwconfig->SSTs[0].sstBoard.Voodoo2Config.sliDetect = FXFALSE;
		for(int tmu = 0; tmu < UserConfig.GlideTextureUnits; tmu++)
		{
			hwconfig->SSTs[tmu].sstBoard.Voodoo2Config.tmuConfig[0].tmuRev = 1;
			hwconfig->SSTs[tmu].sstBoard.Voodoo2Config.tmuConfig[0].tmuRam = UserConfig.TextureMemorySize;
		}
		break;
	}
	return FXTRUE;
}

//*************************************************
//* Selects which Voodoo Board is Active
//*************************************************
FX_ENTRY void FX_CALL
grSstSelect( int which_sst )
{
#ifdef OGL_DONE
    GlideMsg( "grSstSelect( %d )\n", which_sst );
#endif
    // Nothing Needed Here but...
    Glide.ActiveVoodoo = which_sst;
}

//*************************************************
//* Returns the Screen Height
//*************************************************
FX_ENTRY FxU32 FX_CALL
grSstScreenHeight( void )
{
#ifdef OGL_DONE
    GlideMsg( "grSstScreenHeight()\n" );
#endif

    return Glide.WindowHeight;
}

//*************************************************
//* Returns the Screen Width
//*************************************************
FX_ENTRY FxU32 FX_CALL
grSstScreenWidth( void )
{
#ifdef OGL_DONE
    GlideMsg( "grSstScreenWidth()\n" );
#endif

    return Glide.WindowWidth;
}

//*************************************************
//* Sets the Y Origin
//*************************************************
FX_ENTRY void FX_CALL
grSstOrigin( GrOriginLocation_t  origin )
{
#ifdef OGL_DONE
    GlideMsg( "grSstSetOrigin( %d )\n", origin );
#endif

		glReportErrors("grSstOrigin");
		
    RenderDrawTriangles( );
    Glide.State.OriginInformation = origin;
    glMatrixMode( GL_PROJECTION );
    glReportError();
    glLoadIdentity( );
    glReportError();
    switch ( origin )
    {
    case GR_ORIGIN_LOWER_LEFT:
	      glOrtho(Glide.State.ClipMinX, Glide.State.ClipMaxX, 
	              Glide.State.ClipMinY, Glide.State.ClipMaxY, 
	              OpenGL.ZNear, OpenGL.ZFar);
		    glReportError();
	      glViewport(OpenGL.OriginX + OpenGL.ClipMinX,
	                 OpenGL.OriginY +  OpenGL.ClipMinY, 
	                 OpenGL.ClipMaxX - OpenGL.ClipMinX, 
	                 OpenGL.ClipMaxY - OpenGL.ClipMinY);
		    glReportError();
		    glScissor(OpenGL.OriginX + OpenGL.ClipMinX,
		              OpenGL.OriginY + OpenGL.ClipMinY,
	    	          OpenGL.ClipMaxX - OpenGL.ClipMinX,
	    			      OpenGL.ClipMaxY - OpenGL.ClipMinY);	                  
		    glReportError();
        break;
    case GR_ORIGIN_UPPER_LEFT:
	      glOrtho(Glide.State.ClipMinX, Glide.State.ClipMaxX, 
	              Glide.State.ClipMaxY, Glide.State.ClipMinY, 
	              OpenGL.ZNear, OpenGL.ZFar);
		    glReportError();
	      glViewport(OpenGL.OriginX + OpenGL.ClipMinX,
	                 OpenGL.OriginY +  OpenGL.WindowHeight - OpenGL.ClipMaxY, 
	                 OpenGL.ClipMaxX - OpenGL.ClipMinX, 
	                 OpenGL.ClipMaxY - OpenGL.ClipMinY);
		    glReportError();
		    glScissor(OpenGL.OriginX + OpenGL.ClipMinX,
		              OpenGL.OriginY + OpenGL.WindowHeight - OpenGL.ClipMaxY, 
	                OpenGL.ClipMaxX - OpenGL.ClipMinX,
	                OpenGL.ClipMaxY - OpenGL.ClipMinY);
		    glReportError();
        break;
    }
    glMatrixMode( GL_MODELVIEW );
	  glReportError();
    grCullMode( Glide.State.CullMode );
}

//*************************************************
FX_ENTRY void FX_CALL
grSstPerfStats( GrSstPerfStats_t * pStats )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSstPerfStats\n" );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grSstResetPerfStats( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSstResetPerfStats( )\n" );
#endif
}

//*************************************************
FX_ENTRY FxU32 FX_CALL 
grSstVideoLine( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSstVideoLine( )\n" );
#endif

    return 0;
}

//*************************************************
FX_ENTRY FxBool FX_CALL 
grSstVRetraceOn( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSstVRetraceOn( )\n" );
#endif

    return Glide.State.VRetrace;
}

//*************************************************
FX_ENTRY FxBool FX_CALL 
grSstIsBusy( void )
{ 
#ifdef OGL_NOTDONE
    GlideMsg( "grSstIsBusy( )\n" ); 
#endif

    return FXFALSE; 
}

//*************************************************
FX_ENTRY FxBool FX_CALL
grSstControl( GrControl_t code )
{ 
#ifdef OGL_PARTDONE
	GlideMsg( "grSstControl( %lu )\n", code ); 
#endif
	if (code == GR_CONTROL_ACTIVATE)
	{
		RestoreOpenGLWindow();
	}
	else if (code == GR_CONTROL_DEACTIVATE)
	{
		HideOpenGLWindow();
	}
	else if (code == GR_CONTROL_RESIZE)
	{
	}
	else if (code == GR_CONTROL_MOVE)
	{
	}
	return code; 
}

//*************************************************
FX_ENTRY FxBool FX_CALL
grSstControlMode( GrControl_t mode )
{ 
#ifdef OGL_PARTDONE
    GlideMsg( "grSstControlMode( %d )\n", mode );
#endif

    switch ( mode )
    {
    case GR_CONTROL_ACTIVATE:
		RestoreOpenGLWindow();
        break;
    case GR_CONTROL_DEACTIVATE:
		HideOpenGLWindow();
        break;
    case GR_CONTROL_RESIZE:
    case GR_CONTROL_MOVE:
        break;
    }

    return FXTRUE; 
}

//*************************************************
//* Return the Value of the graphics status register
//*************************************************
FX_ENTRY FxU32 FX_CALL 
grSstStatus( void )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grSstStatus( )\n" );
#endif

//    FxU32 Status = 0x0FFFF43F;
    FxU32 Status = 0x0FFFF03F;
    
    // Vertical Retrace
    Status      |= ( ! Glide.State.VRetrace ) << 6;

    return Status;
// Bits
// 5:0      PCI FIFO free space (0x3F = free)
// 6        Vertical Retrace ( 0 = active, 1 = inactive )
// 7        PixelFx engine busy ( 0 = engine idle )
// 8        TMU busy ( 0 = engine idle )
// 9        Voodoo Graphics busy ( 0 = idle )
// 11:10    Displayed buffer ( 0 = buffer 0, 1 = buffer 1, 2 = auxiliary buffer, 3 = reserved )
// 27:12    Memory FIFO ( 0xFFFF = FIFO empty )
// 30:28    Number of swap buffers commands pending
// 31       PCI interrupt generated ( not implemented )
}

//*************************************************
//* Returns when Glides is Idle
//*************************************************
FX_ENTRY void FX_CALL
grSstIdle( void )
{
#ifdef OGL_DONE
	GlideMsg( "grSetIdle( )\n" );
#endif

	glReportErrors("grSetIdle");

	RenderDrawTriangles( );
	glFlush( );
	glFinish( );
	glReportError();
}

