//**************************************************************
//*     OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                  Mac specific functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "FormatConversion.h"
#include "Glide.h"
#include "GlideApplication.h"
#include "GlideDisplay.h"
#include "GlideSettings.h"
#include "GLExtensions.h"
#include "GLRender.h"

#ifdef OGL_PROFILING
#include <profiler.h>
#endif

#include <Gestalt.h>

OSErr __macglide_initialize(struct CFragInitBlock* initBlkPtr)
{
	OSErr err = __initialize((CFragInitBlock*) initBlkPtr);
	if (err == noErr)
	{
#ifdef OGL_PROFILING
	err = ProfilerInit(collectDetailed, bestTimeBase, 666, 666); // collectDetailed, collectSummary
	assert(err == noErr);
	ProfilerSetStatus(1);
#endif
		err = UserConfig.init(s_GlideApplication.GetName()); 
		if (err == noErr)
		{
			if (!ClearAndGenerateLogFile())
			{
				GlideError("Unable to init log file: Error code %d", err);
			}
			else
			{
				InitMainVariables();
				DisplayManager_Initialise();
				err = DisplayManager_RememberDesktopDisplay();
				GlideMsg(OGL_LOG_SEPARATE);				
			}
		}
	}
	return err;
}

void __macglide_terminate()
{
#ifdef OGL_DEBUG
	GlideMsg(OGL_LOG_SEPARATE);
#endif
  grGlideShutdown();
	OSErr err = DisplayManager_RestoreDesktopDisplay();
	DisplayManager_Cleanup();
  CloseLogFile();
	if (Glide.ReadBuffer.Address)
	{
		FreeFrameBuffer(Glide.ReadBuffer.Address);
		Glide.ReadBuffer.Address = NULL;
	}

#ifdef OGL_PROFILING
	 char* filename = "XMacGLide Profiler stats";
	filename[0] = strlen(&filename[1]);
	err = ProfilerDump(reinterpret_cast<unsigned char*>(filename));
	ProfilerTerm();
#endif

  __terminate();
}

const char* OpenGLideProductName = "MacGLide";

// Allow allocation of memory at 32-byte boundaries to support G4 cacheline clearing
void* AllocSysPtr16ByteAligned(long buffersize)
{
	// Align buffers to 32byte boundaries so
	// we can use __dcbz() to clear buffers
	const int align = 32;
	const int align_mask = 0xffffffe0;
	void* buffer = NewPtrSys(buffersize + align);
	if (buffer == NULL) return NULL;
	unsigned long aligned_buffer = (reinterpret_cast<unsigned long>(buffer) + align) & align_mask;
	// remember the real location of the buffer
	(reinterpret_cast<unsigned long*>(aligned_buffer))[-1] = reinterpret_cast<unsigned long>(buffer);
	return reinterpret_cast<void*>(aligned_buffer);
}

void Free16ByteAligned(void* aligned_buffer)
{
	DisposePtr((Ptr) (static_cast<unsigned long*>(aligned_buffer))[-1]);
}

void FatalErrorMessageBox(const char* message)
{
	unsigned char buffer1[] = "XMacGLide has encountered an unrecoverable error and cannot continue. Excuse:";
	buffer1[0] = strlen(reinterpret_cast<char*>(&buffer1[1]));
	unsigned char buffer2[StringBufferSize + 1];
	strncpy(reinterpret_cast<char*>(&buffer2[1]), message, StringBufferSize -1);
	buffer2[0] = strlen(message);
	buffer2[buffer2[0] + 1] = 0x0;
	SInt16 itemhit;
	StandardAlert(kAlertStopAlert, buffer1, buffer2, NULL, &itemhit);
}

char* _strtime(char* timebuf)
{
	time_t systime;
	struct tm *currtime;
	systime = time(NULL);
	currtime = localtime(&systime);
	strftime(timebuf, 128, "%X", currtime);
	return timebuf;
}

char* _strdate(char* timebuf)
{
	time_t systime;
	struct tm *currtime;
	systime = time(NULL);
	currtime = localtime(&systime);
	strftime(timebuf, 128, "%x", currtime);
	return timebuf;
}

OpenGLideVectorUnitType GetVectorUnitType()
{
	long cpuAttributes;
	OpenGLideVectorUnitType type = OpenGLideVectorUnitType_None;
	OSErr err = Gestalt( gestaltPowerPCProcessorFeatures, &cpuAttributes );
	if(noErr == err)
	{
	    if (( 1 << gestaltPowerPCHasVectorInstructions) & cpuAttributes)
	    {
	    	type = OpenGLideVectorUnitType_Altivec;
	    }
	}
	return type;
}

const char* OpenGLideVectorUnitNames[] =
{
	"PowerPC AltiVec"
};

//////////////////////////////////////////////////////////////
// Big endian conversion functions
//////////////////////////////////////////////////////////////

// Note about anisotropy and chromakeying:
// If anisotropy is enabled, some textured objects contain
// chromakey-colored pixels.
// This happens because chromakey pixels keep their color values,
// and only the alpha value is set to 0. If the texture is rendered
// with anisotropy enabled (also happened in the framebuffer emulation
// with linear texture filter),  the chromakeyed pixels are somehow
// combined with a neighboring (non-chromakey colored) pixel.
// This seems to change the alpha value and as a result, the pixels
// pass the alpha test and are rendered. 
// To minimise the resulting artefacts, the color of the chroma-keyed pixel
// is set to the color of the neighboring pixel.

// Used by MacGLide for lossy conversions
void Convert565Kto5551(const FxU32* Buffer1, FxU32 key, FxU32* Buffer2, FxU32 NumberOfPixels )
{
	// Convert565Kto5551((unsigned long*) Src, (unsigned long*) Dst, NumberOfPixels);
	// This functions processes 2 pixels at a time, there is no problem in
	// passing odd numbers or a number less than 2 for the pixels, but
	// the buffers should be large enough
	FxU32 key1 = key & 0xffff;
	FxU32 key2 = key << 16;
	while ( NumberOfPixels > 0)
	{
	  	FxU32 src1 = *Buffer1 && 0xffff;
	  	FxU32 dest;
	  	if (key1 == src1)
	  	{
	  		dest = 0;
	  	}
	  	else
	  	{
	  		dest = src1;
	  	}
	  	FxU32 src2 = (*Buffer1 && 0xffff0000);
	  	if (key2 != src2)
	  	{
	  		dest = dest | src2;
	  	}
		*Buffer2++ = (   (dest) & 0xFFC0FFC0 ) |
		             ( ( (dest) & 0x001F001F ) << 1 ) |
		                          0x00010001;
	    Buffer1++;
	    NumberOfPixels -= 2;
	}
}

void Convert565Kto8888(const FxU16* src, FxU16 key, FxU32* dst, FxU32 pixels)
{
	// Scan for first non-chromakey pixel
	// in order to find to the starting value
	// for the chromakey_replacement 
	const register FxU32 mask_pixel_r = 0x0000f800;
	const register FxU32 mask_pixel_g = 0x000007e0;
	const register FxU32 mask_pixel_b = 0x0000001f;
	register FxU16 chromakey_replacement_565 = 0x0000;
	for(const FxU16* p = src; p < (src + pixels); p++)
	{
		chromakey_replacement_565 = *p;
		if (chromakey_replacement_565 != key) break;
	}
	register FxU32 rgb_mask_8888 = 0xffffff00;
	register FxU32 chromakey_replacement_8888 = 0x00000000;
	chromakey_replacement_8888 = (0x00000000 | // A
			                          (chromakey_replacement_565 & mask_pixel_b) <<  11  | // B
			                          (chromakey_replacement_565 & mask_pixel_g) <<  13  | // G
			                          (chromakey_replacement_565 & mask_pixel_r) <<  16);  // R
	// Start the conversion
	register FxU16 pixel_565;
	register FxU32 pixel_8888;
	while (pixels)
	{
		pixel_565 = *src++;
		if (pixel_565 == key)
		{
			// Minimise anisotropy artefacts
			pixel_8888 = chromakey_replacement_8888;		
		}
		else
		{
			pixel_8888 = (0x000000ff | // A
			              (pixel_565 & mask_pixel_b) <<  11  | // B
			              (pixel_565 & mask_pixel_g) <<  13  | // G
			              (pixel_565 & mask_pixel_r) <<  16);   // R
			chromakey_replacement_8888 = pixel_8888 & rgb_mask_8888;
		}
		*dst++ = pixel_8888;
		pixels--;
	}
}

void ConvertA8toAP88(const FxU8 *Buffer1, FxU16 *Buffer2, FxU32 Pixels )
{
	while ( Pixels )
	{
		*Buffer2 = ( ( ( *Buffer1 ) << 8 ) | ( *Buffer1 ) );
	    Buffer1++;
	    Buffer2++;
	    Pixels--;
	}
}

void Convert8332to8888(const FxU16 *Buffer1, FxU32 *Buffer2, FxU32 Pixels )
{
	FxU32 R, G, B, A;
	for ( FxU32 i = Pixels; i > 0; i-- )
	{
		A = ( ( ( *Buffer1 ) >> 8 ) & 0xFF );
		R = ( ( ( *Buffer1 ) >> 5 ) & 0x07 ) << 5;
		G = ( ( ( *Buffer1 ) >> 2 ) & 0x07 ) << 5;
		B = (   ( *Buffer1 ) & 0x03 ) << 6;
		*Buffer2 = ( R << 24 ) | ( G << 16 ) | ( B << 8 ) | A;

		Buffer1++;
		Buffer2++;
  }
}

void ConvertP8to8888(const FxU8 *Buffer1, FxU32 *Buffer2, FxU32 Pixels, FxU32 *palette )
{
  while ( Pixels-- )
  {
  	*Buffer2++ = palette[ *Buffer1++ ];
  }
}

void ConvertP8Kto8888(const FxU8 *Buffer1, FxU32 Key, FxU32 *Buffer2, FxU32 Pixels, FxU32 *palette )
{
	// Scan for first non-chromakey pixel
	// in order to find to the starting value
	// for the chromakey_replacement 
	register FxU32 chromakey_replacement = 0;
	register FxU32 chromakey = Key;
	register FxU32 alpha_mask = 0xffffff00;
	for(const FxU8* b = Buffer1; b < (Buffer1 + Pixels); b++)
	{
		chromakey_replacement = (palette[*b] & alpha_mask);
		if (chromakey_replacement != chromakey) break;
	}
	// Start the conversion
	register FxU32 current_pixel = 0;
  while ( Pixels-- )
  {
  	current_pixel = palette[*Buffer1++];
  	if (current_pixel == chromakey)
  	{
	  	*Buffer2 = chromakey_replacement;
	  }
	  else
	  {
	  	*Buffer2 = current_pixel;
	  	chromakey_replacement = current_pixel & alpha_mask;
	  }
	  Buffer2++;
  }
}

void ConvertAI44toAP88(const FxU8 *Buffer1, FxU16 *Buffer2, FxU32 Pixels )
{
  for ( FxU32 i = Pixels; i > 0; i-- )
  {
    *Buffer2 = ( ( ( ( *Buffer1 ) & 0xF0 ) << 8 ) | ( ( ( *Buffer1 ) & 0x0F ) << 4 ) );
    Buffer2++;
    Buffer1++;
  }
}

void ConvertAP88to8888(const FxU16 *Buffer1, FxU32 *Buffer2, FxU32 Pixels, FxU32 *palette )
{
  FxU32   RGB, 
            A;
  for ( FxU32 i = Pixels; i > 0; i-- )
  {
    RGB = ( palette[ *Buffer1 & 0x00ff ] & 0xffffff00 );
    A = *Buffer1 >> 8;
    *Buffer2 =  A  | RGB;
    Buffer1++;
    Buffer2++;
  }
}

void ConvertYIQto8888(const FxU8 *in, FxU32 *out, FxU32 Pixels, GuNccTable *ncc )
{
  FxI32   R;
  FxI32   G;
  FxI32   B;
  for ( FxU32 i = Pixels; i > 0; i-- )
  {
    R = ncc->yRGB[ *in >> 4 ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 0 ]
                              + ncc->qRGB[ ( *in      ) & 0x3 ][ 0 ];

    G = ncc->yRGB[ *in >> 4 ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 1 ]
                              + ncc->qRGB[ ( *in      ) & 0x3 ][ 1 ];

    B = ncc->yRGB[ *in >> 4 ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 2 ]
                              + ncc->qRGB[ ( *in      ) & 0x3 ][ 2 ];

		// Clamp values
    R = ( ( R < 0 ) ? 0 : ( ( R > 255 ) ? 255 : R ) );
    G = ( ( G < 0 ) ? 0 : ( ( G > 255 ) ? 255 : G ) );
    B = ( ( B < 0 ) ? 0 : ( ( B > 255 ) ? 255 : B ) );
    
    *out = ( (R << 24) | ( G << 16 ) | ( B << 8 ) | 0x000000ff );
    in++;
    out++;
  }
}

void ConvertAYIQto8888(const FxU16 *in, FxU32 *out, FxU32 Pixels, GuNccTable *ncc)
{
  FxI32   R;
  FxI32   G;
  FxI32   B;
  for ( FxU32 i = Pixels; i > 0; i-- )
  {
    R = ncc->yRGB[ ( *in >> 4 ) & 0xf ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 0 ]
                                        + ncc->qRGB[ ( *in      ) & 0x3 ][ 0 ];

    G = ncc->yRGB[ ( *in >> 4 ) & 0xf ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 1 ]
                                        + ncc->qRGB[ ( *in      ) & 0x3 ][ 1 ];

    B = ncc->yRGB[ ( *in >> 4 ) & 0xf ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 2 ]
                                        + ncc->qRGB[ ( *in      ) & 0x3 ][ 2 ];

    R = ( ( R < 0 ) ? 0 : ( ( R > 255 ) ? 255 : R ) );
    G = ( ( G < 0 ) ? 0 : ( ( G > 255 ) ? 255 : G ) );
    B = ( ( B < 0 ) ? 0 : ( ( B > 255 ) ? 255 : B ) );

    *out = ( (R <<24) | ( G << 16 ) | ( B << 8 ) | ( /*0x000000ff &*/ ( *in >> 8 ) ) );

    in++;
    out++;
  }
}

void SplitAP88(const FxU16 *ap88, FxU8 *index, FxU8 *alpha, FxU32 pixels )
{
  for ( FxU32 i = pixels; i > 0; i-- )
  {
    *alpha++ = ( *ap88 >> 8 );
    *index++ = ( *ap88++ & 0xff );
  }
}

// Color conversion functions: The windows version converts colors to ARGB,
// whereas the mac version converts colors to RGBA (no extension needed)
void ConvertColor4B( GrColor_t GlideColor, FxU32 &C )
{
    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        C = ( ( GlideColor & 0x00ffffff ) << 8 ) |
            ( ( GlideColor & 0xff000000 ) >> 24 );
        break;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        C = ( ( GlideColor & 0xff000000 ) >> 24 ) |
            ( ( GlideColor & 0x00ff0000 ) >>  8 ) |
            ( ( GlideColor & 0x0000ff00 ) <<  8 ) |
            ( ( GlideColor & 0x000000ff ) << 24 );
        break;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        C = GlideColor;
        break;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        C = ( ( GlideColor & 0xFF000000 ) >> 24 ) |
            ( ( GlideColor & 0x0000ff00 ) << 16 ) |
            ( ( GlideColor & 0x0ff000ff ) );
        break;
    }
}

void Convert1555Kto8888(const FxU16* src, FxU16 key, FxU32* dst, FxU32 pixels)
{
	// Scan for first non-chromakey pixel
	// in order to find to the starting value
	// for the chromakey_replacement 
	const register FxU32 mask_pixel_r = 0x00007c00;
	const register FxU32 mask_pixel_g = 0x000003e0;
	const register FxU32 mask_pixel_b = 0x0000001f;
	const register FxU32 mask_pixel_a = 0x00008000;
	register FxU16 chromakey_replacement_1555 = 0x0000;
	register FxU16 chromakey = key & 0x7fff;
	for(const FxU16* p = src; p < (src + pixels); p++)
	{
		chromakey_replacement_1555 = *p;
		if (chromakey_replacement_1555 != chromakey) break;
	}
	register FxU32 rgb_mask_8888 = 0xffffff00;
	register FxU32 chromakey_replacement_8888 = 0x00000000;
	chromakey_replacement_8888 = (0x00000000 | // A
			                          (chromakey_replacement_1555 & mask_pixel_b) <<  11  | // B
			                          (chromakey_replacement_1555 & mask_pixel_g) <<  14  | // G
			                          (chromakey_replacement_1555 & mask_pixel_r) <<  17);  // R
	// Start the conversion
	register FxU16 pixel_1555;
	register FxU32 pixel_8888;
	while (pixels)
	{
		pixel_1555 = *src++;
		if ((pixel_1555 & 0x7fff) == chromakey)
		{
			// Minimise anisotropy artefacts
			pixel_8888 = chromakey_replacement_8888;		
		}
		else
		{
			pixel_8888 = (((pixel_1555 & mask_pixel_a) ? 0x000000ff : 0x00000080) | // A
			              (pixel_1555 & mask_pixel_b) <<  11  | // B
			              (pixel_1555 & mask_pixel_g) <<  14  | // G
			              (pixel_1555 & mask_pixel_r) <<  17);   // R
			chromakey_replacement_8888 = pixel_8888 & rgb_mask_8888;
		}
		*dst++ = pixel_8888;
		pixels--;
	}
}
