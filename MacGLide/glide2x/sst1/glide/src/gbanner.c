/*
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
**
** $Header: /cvsroot/macglide/MacGLide/glide2x/sst1/glide/src/gbanner.c,v 1.2 2005/07/20 20:45:56 jens-olaf Exp $
** $Log: gbanner.c,v $
** Revision 1.2  2005/07/20 20:45:56  jens-olaf
** Commented out unwanted code instead of using #define statements
**
** Revision 1.1  2005/07/09 21:28:54  jens-olaf
** Added splash screen and shameless plug
**
** Revision 1.1.1.1  1999/12/07 21:48:52  joseph
** Initial checkin into SourceForge.
**
 * 
 * 9     5/02/97 2:08p Pgj
 * screen_width/height now FxU32
 * 
 * 8     3/16/97 2:24a Jdt
 * Fixed bug.  Didn't initialize info.
 * 
 * 7     3/12/97 11:51p Jdt
 * Watcom warning.
 * 
 * 6     3/12/97 4:20p Jdt
 * Fixed for VG96 and optimized SST-1
 * 
 * 5     2/26/97 11:55a Jdt
 * Updated banner for new lfb api
 * 
 * 4     12/23/96 1:37p Dow
 * chagnes for multiplatform glide
**
*/

/*
#include <3dfx.h>
#include <glidesys.h>

#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"
*/

// OpenGLide specific
#include "../../../driversrc_defines.h"

#ifdef GLIDE_PLUG
#include "banner.inc"
#endif

/* display the translucent 3Dfx powerfield logo */
void
_grShamelessPlug( void )
{
// #ifdef GLIDE_PLUG
  GrState state;
  GrLfbInfo_t info;

  GR_BEGIN_NOFIFOCHECK("_grShamelessPlug",80);
  GDBG_INFO_MORE((gc->myLevel,"()\n"));

// #if ( GLIDE_PLATFORM & GLIDE_HW_SST1 ) 
  grGlideGetState( &state );
  grDisableAllEffects();

  grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                 GR_COMBINE_FACTOR_ONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_TEXTURE, FXFALSE );
  grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                 GR_COMBINE_FACTOR_ONE,
                 GR_COMBINE_LOCAL_NONE,
                 GR_COMBINE_OTHER_TEXTURE,
                 FXFALSE );
  grAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
                       GR_BLEND_ONE_MINUS_SRC_ALPHA,
                       GR_BLEND_ZERO, GR_BLEND_ZERO );
  grClipWindow( 0, 0,
               Glide.WindowWidth - 1,
               Glide.WindowHeight - 1); 
  grDepthMask(FXFALSE);
  grDepthBufferFunction(GR_CMP_ALWAYS);
  grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );
  
  grChromakeyValue( 0x0000 );
  grChromakeyMode( GR_CHROMAKEY_ENABLE );
  grLfbConstantAlpha( (FxU8) 90);
  grLfbWriteColorFormat(GR_COLORFORMAT_ARGB);

  /* Attempt to lock with pixpipe enabled */
  info.size = sizeof( info );
  if ( grLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER,
                  GR_LFBWRITEMODE_565,GR_ORIGIN_UPPER_LEFT, 
                  FXTRUE, &info ) ) {
      FxU32 *dstData;
      FxU32  dstJump;
      FxU32 *srcData;
      FxI32  srcJump;
      FxU32  srcScanlineLength;
      FxU32  scrWidth  = Glide.WindowWidth;
      FxU32  scrHeight = Glide.WindowHeight;

      FxU32  scanline;

      /* Draw Banner in lower right of screen */
      if ( scrWidth  < (FxU32)banner_width  ) return;
      if ( scrHeight < (FxU32)banner_height ) return;

      dstData = static_cast<unsigned long*>(info.lfbPtr);
      dstData = (FxU32*)( ((char*)dstData) + 
                          (info.strideInBytes*((scrHeight-1)-banner_height)) +
                          ((scrWidth-banner_width)<<1) );
      dstJump = ((info.strideInBytes >> 1) - banner_width)>>1;
      srcData = (FxU32*)&banner_data[banner_width*(banner_height-1)];
      srcScanlineLength = banner_width>>1;
      srcJump = (-banner_width);

      for( scanline = 0; scanline < (FxU32)banner_height; scanline++ ) {
          FxU32 *end = srcData + srcScanlineLength;
          while( srcData < end ) *dstData++ = *srcData++;
          dstData += dstJump;
          srcData += srcJump;
      }

      grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER );
  }
  grGlideSetState( &state );
/*
#elif ( GLIDE_PLATFORM & GLIDE_HW_SST96 )
  FXUNUSED( state );
  info.size = sizeof( info );
  if ( grLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER,
                  GR_LFBWRITEMODE_565,GR_ORIGIN_UPPER_LEFT, 
                  FXFALSE, &info ) ) {

      FxU16 *dstData;
      FxU32  dstJump;
      FxU16 *srcData;
      FxI32  srcJump;
      FxU32  srcScanlineLength;
      FxU32  scrWidth  = gc->state.screen_width;
      FxU32  scrHeight = gc->state.screen_height;

      FxU32  scanline;

      // Draw Banner in lower right of screen
      if ( scrWidth  < (FxU32)banner_width  ) return;
      if ( scrHeight < (FxU32)banner_height ) return;

      dstData = info.lfbPtr;
      dstData = (FxU16*)( ((char*)dstData) + 
                          (info.strideInBytes*((scrHeight-1)-banner_height)) +
                          ((scrWidth-banner_width)<<1) );
      dstJump = ((info.strideInBytes >> 1) - banner_width);
      srcData = (FxU16*)&banner_data[banner_width*(banner_height-1)];
      srcScanlineLength = banner_width;
      srcJump = (-banner_width)*2;

      for( scanline = 0; scanline < (FxU32)banner_height; scanline++ ) {
          FxU16 *end = srcData + srcScanlineLength;
          while( srcData < end ) {
              if ( *srcData )
                  *dstData = *srcData;
              dstData++;
              srcData++;
          }
          dstData += dstJump;
          srcData += srcJump;
      }
      
      grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER );
  }

#else
#  error "Shameless Plug Unimplemented on this Hardware"  
#endif
*/

  // GR_END();
// #endif
} /* _grShamelessPlug */
