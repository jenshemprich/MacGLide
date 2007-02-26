//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*               Linear Frame Buffer Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Glide.h"
#include "GlideApplication.h"
#include "FormatConversion.h"
#include "GLExtensions.h"
#include "GLRender.h"
#include "GLUtil.h"


//*************************************************
FX_ENTRY FxBool FX_CALL
grLfbLock( GrLock_t dwType, 
           GrBuffer_t dwBuffer, 
           GrLfbWriteMode_t dwWriteMode,
           GrOriginLocation_t dwOrigin, 
           FxBool bPixelPipeline, 
           GrLfbInfo_t *lfbInfo )
{
#if defined(OGL_CRITICAL) || defined(OGL_FRAMEBUFFER)
	GlideMsg("grLfbLock( %d, %d, %d, %d, %d, 0x%x )\n",
	           dwType, dwBuffer, dwWriteMode, dwOrigin, bPixelPipeline, lfbInfo); 
#endif

#ifdef OGL_NOTDONE
	if (dwWriteMode != GR_LFBWRITEMODE_565
	 && dwWriteMode != GR_LFBWRITEMODE_1555
	 && dwWriteMode != GR_LFBWRITEMODE_888
	 && dwWriteMode != GR_LFBWRITEMODE_ANY)
	{
		GlideMsg("grLfbLock() - dwWriteMode %d not supported\n", dwWriteMode);
	}
#endif

	glReportErrors("grLfbLock");

	dwWriteMode = (dwWriteMode == GR_LFBWRITEMODE_ANY) ? GR_LFBWRITEMODE_565 : dwWriteMode;
	lfbInfo->strideInBytes = Glide.WindowWidth * (dwWriteMode >= GR_LFBWRITEMODE_888 ? 4 : 2);
	lfbInfo->writeMode = dwWriteMode;
	lfbInfo->origin = dwOrigin;
	if ( dwType & GR_LFB_WRITE_ONLY )
	{
		lfbInfo->lfbPtr = Glide.FrameBuffer.Address;
		s_Framebuffer.OnBufferLockStartWrite(dwType, dwBuffer, dwWriteMode, dwOrigin, bPixelPipeline);
	}
	else if (OpenGL.WinOpen == false && Glide.ReadBuffer.Address)
	{
		BufferStruct* targetbuffer = Glide.ReadBuffer.Address ? &Glide.ReadBuffer : &Glide.TempBuffer;
		lfbInfo->lfbPtr = targetbuffer->Address;
		lfbInfo->origin = GR_ORIGIN_UPPER_LEFT;
	}
	else
	{
		// finish rendering
		RenderDrawTriangles();
		glFinish();
		glReportError();
		// Alloc readbuffer
		if (Glide.ReadBuffer.Address == NULL)
		{
			Glide.ReadBuffer.Address = (FxU16*) AllocFrameBuffer(Glide.WindowTotalPixels, sizeof(FxU16));
#ifdef OPENGL_DEBUG
							GlideMsg("Allocated Readbuffer(%dx%d) at 0x%x\n",
							           Glide.WindowWidth, Glide.WindowHeight, Glide.ReadBuffer.Address);
#endif
		}
		// select main memory buffers
		// @todo Remove conditions for NULL ReadBuffer (usage of FrameBuffer as source is wrong because it might be 16bit only)
		BufferStruct* targetbuffer = &Glide.ReadBuffer;
		const BufferStruct* sourcebuffer = &Glide.TempBuffer;
		if (s_Framebuffer.GetRenderBufferChangedForRead() == true)
		{
			// select buffer size
			const unsigned long bufferwidth = OpenGL.WindowWidth;
			const unsigned long bufferheight = OpenGL.WindowHeight;
			glReadBuffer(dwBuffer == GR_BUFFER_BACKBUFFER ? GL_BACK : GL_FRONT);
			glReportError();
			const bool scale = bufferwidth != Glide.WindowWidth || bufferheight != Glide.WindowHeight;
			GLint glWriteMode;
			switch (dwWriteMode)
			{
			case GR_LFBWRITEMODE_1555:	glWriteMode = GL_UNSIGNED_SHORT_1_5_5_5_REV;	break;
			case GR_LFBWRITEMODE_565:	glWriteMode = GL_UNSIGNED_SHORT_5_6_5;	break;
			default:	glWriteMode = GL_UNSIGNED_SHORT_5_6_5;	break;
			}
			// The read buffer is sized for Glide pixels. As a result we cannot be used it when the read buffer has to be resized
			void* destination = (!scale && dwOrigin == GR_ORIGIN_LOWER_LEFT) ? targetbuffer->Address : sourcebuffer->Address;
#ifdef OPENGL_DEBUG
							GlideMsg("Calling glReadPixels(%d, %d, 0x%x)\n", bufferwidth, bufferheight, destination);
#endif							
			glReadPixels(0, 0, 
			             bufferwidth, bufferheight,
			             (dwWriteMode == GR_LFBWRITEMODE_1555) ? GL_RGBA : GL_RGB,
			             glWriteMode, 
		               destination);
			glReportError();
#ifdef OPENGL_DEBUG
			if (scale)
							GlideMsg("Scaling to (%d, %d)  from 0x%x to 0x%x\n",
							         Glide.WindowWidth, Glide.WindowHeight, sourcebuffer->Address, targetbuffer->Address);
#endif							
			if (dwOrigin == GR_ORIGIN_UPPER_LEFT)
			{
				// When the OpenGL resolution differs from the Glide resolution,
				// the content of the read buffer must be scaled
				if (scale)
				{
					// size/copy from OpenGL-sized buffer to Glide-sized buffer?
					const FxU16* src;
					FxU16* dst = targetbuffer->Address;
					const int xratio = (bufferwidth << 16) / (Glide.WindowWidth);
					const int yratio = (bufferheight << 16) / (Glide.WindowHeight);
					int u;
					int v = 0;
					int x;
					int y;
					for(y = 0; y < Glide.WindowHeight; y++)
					{
						src = sourcebuffer->Address + (bufferheight -1 - (v >> 16)) * bufferwidth;
						u = 0;
						for(x = 0;x < Glide.WindowWidth; x++)
						{
							*dst++ = src[u >> 16];
							u += xratio;
						}
						v += yratio;
					}
				}
				else
				{
#ifdef OPENGL_DEBUG
							GlideMsg("Copying/Vertical mirroring pixels to destination buffer from 0x%x to 0x%x\n",
							           sourcebuffer->Address, targetbuffer->Address);
#endif							
					// Swap pixels during copy from temp to read buffer
					for ( int j = 0; j < Glide.WindowHeight; j++ )
					{
						memcpy(targetbuffer->Address + (j * Glide.WindowWidth),
						       sourcebuffer->Address + ((bufferheight - 1 - j) * bufferwidth),
						       2 * Glide.WindowWidth);
					}
				}
			}
			else if (scale) // GR_ORIGIN_LOWER_LEFT
			{
				// size/copy from OpenGL-sized buffer to Glide-sized buffer?
				if (scale)
				{
					// Copy and scale
					const FxU16* src;
					FxU16* dst = targetbuffer->Address;
					const int xratio = (bufferwidth << 16) / Glide.WindowWidth;
					const int yratio = (bufferheight << 16) / Glide.WindowHeight;
					int u;
					int v = 0;
					int x;
					int y;
					for(y = 0; y < Glide.WindowHeight; y++)
					{
						src = sourcebuffer->Address + (v >> 16) * bufferwidth;
						u = 0;
						for(x = 0;x < Glide.WindowWidth; x++)
						{
							*dst++ = src[u >> 16];
							u += xratio;
						}
						v += yratio;
					}
				}
				else
				{
#ifdef OPENGL_DEBUG
							GlideMsg("Copying/Vertical mirroring not necessary - pixels already in the right orientation\n");
#endif							
				}
			}
			// Update with current framebuffer pixels not yet written to vram
			// (Obmitting causes incomplete screenshots in Falcon 4.0)
			s_Framebuffer.CopyFrameBuffer(targetbuffer->Address);
			s_Framebuffer.ResetRenderBufferChangedForRead();
		}
		// Update buffer struct
		targetbuffer->Lock            = true;
		targetbuffer->Type            = dwType;
		targetbuffer->Buffer          = dwBuffer;
		targetbuffer->WriteMode       = dwWriteMode;
		targetbuffer->PixelPipeline   = bPixelPipeline;
		lfbInfo->lfbPtr = targetbuffer->Address;
	}
	return FXTRUE;
}

//*************************************************
FX_ENTRY FxBool FX_CALL
grLfbUnlock( GrLock_t dwType, GrBuffer_t dwBuffer )
{
#if defined(OGL_CRITICAL) || defined(OGL_FRAMEBUFFER)
	GlideMsg("grLfbUnlock( %d, %d )\n", dwType, dwBuffer ); 
#endif

	if ( dwType & GR_LFB_WRITE_ONLY )
	{
		if ( ! Glide.FrameBuffer.Lock )
		{
			return FXFALSE;
		}
		s_Framebuffer.OnBufferUnlockEndWrite(dwBuffer);
		Glide.FrameBuffer.Lock = false;
		return FXTRUE;
	}
	else
	{
		BufferStruct* targetbuffer = Glide.ReadBuffer.Address ? &Glide.ReadBuffer : &Glide.TempBuffer;
		if (targetbuffer->Lock)
		{
			// We're not interested in keeping track of unlocks since this breaks
			// Framebuffer updates when moving the cursor in Carmageddon movie mode
			// (because grLfbReadRegion() is called)
			// @todo: -> Doesn't solve the issue
			targetbuffer->Lock = false;
			return FXTRUE; 
		}
		else
		{
			return FXFALSE; 
		}
	}
}

//*************************************************
FX_ENTRY FxBool FX_CALL
grLfbReadRegion( GrBuffer_t src_buffer,
                 FxU32 src_x, FxU32 src_y,
                 FxU32 src_width, FxU32 src_height,
                 FxU32 dst_stride, void *dst_data )
{
#if defined(OGL_DONE) || defined(OGL_FRAMEBUFFER)
  GlideMsg("grLfbReadRegion( %d, %d, %d, %d, %d, %d, 0x%x )\n",
      src_buffer, src_x, src_y, src_width, src_height, dst_stride, dst_data);
#endif

	if (s_GlideApplication.GetType() == GlideApplication::Carmageddon)
	{
		RenderDrawTriangles();
		s_Framebuffer.OnLfbReadRegion();
		// To render the cursor in Movie-mode correctly, the region
		// must be filled with the framebuffer's chromakey color.
		// But Carmageddon seems to write a column filled with the
		// endian-swapped chromakey value into the framebuffer.
		// Workaround:
		// Using an endian-symetric chromakey value (outch)
		// Also one more pixel than expected must be set in order to 
		// avoid a black dot in the lower right corner of the cursor.
		// All of this could be avoided with PedanticFrameBufferEmulation.
		const unsigned jump_dst = ((dst_stride >> 1) - src_width);
		unsigned long jump = jump_dst;
		const unsigned long pixels = src_width * src_height + 1;
		FxU16* d = reinterpret_cast<FxU16*>(dst_data);
		FxU16 chromakey = s_Framebuffer.GetChromaKeyValue();
		swapshort(&chromakey);
		for(unsigned int i = 0; i < pixels; i+=1)
		{
			d[i] = chromakey;
			jump--;
			if (jump == 0)
			{
				jump = jump_dst;
				i += jump;
			}
		}
		return FXTRUE;
	}
	
	// Copied from the linux sst1 driver src
  FxBool rv = FXTRUE;
  GrLfbInfo_t info;
  
//#if (GLIDE_PLATFORM & GLIDE_HW_SST1)
//  gc->lfbSliOk = 1;
  info.size = sizeof( info );
  if ( grLfbLock( GR_LFB_READ_ONLY,
                 src_buffer, 
                 GR_LFBWRITEMODE_ANY,
                 GR_ORIGIN_UPPER_LEFT,
                 FXFALSE,
                 &info ) ) {
    FxU32 *srcData;             /* Tracking Source Pointer */
    FxU32 *dstData;             /* Tracking Destination Pointer */
    FxU32 *end;                 /* Demarks End of each Scanline */
    FxU32 srcJump;              /* bytes to next scanline */
    FxU32 dstJump;              /* bytes to next scanline */
    FxU32 length;               /* bytes to copy in scanline */
    FxU32 scanline;             /* scanline number */
    int   aligned;              /* word aligned? */
    FxU32 odd;                  /* is src_y odd? ( for sli ) */
    
    dstData = ( FxU32 * ) dst_data;
    srcData = ( FxU32 * ) ( ((char*)info.lfbPtr)+
                           (src_y*info.strideInBytes) +
                           (src_x<<1) );
    scanline = src_height;
    length   = src_width * 2;
    dstJump  = dst_stride - length;
    srcJump  = info.strideInBytes - length;
    aligned  = !((int)srcData&0x2);
    odd      = (src_y+src_height) & 0x1;
    
    if ( aligned ) {
      while( scanline-- ) {
        end = (FxU32*)((char*)srcData + length - 2);
/*
        if(gc->scanline_interleaved == FXTRUE) {
          if((scanline+odd) & 0x1)
            sst1InitSliPciOwner(gc->base_ptr, SST_SLI_MASTER_OWNPCI);
          else
            sst1InitSliPciOwner(gc->base_ptr, SST_SLI_SLAVE_OWNPCI);
        }
*/
        while( srcData < end ) 
          *dstData++ = *srcData++;
                
        if ( ((int)length) & 0x2 ) {
          (*(FxU16*)dstData) = (*(FxU16*)srcData);
          dstData = (FxU32*)(((FxU16*)dstData) + 1 );
          srcData = (FxU32*)(((FxU16*)srcData) + 1 );
        }
                
        dstData = (FxU32*)(((char*)dstData)+dstJump);
        srcData = (FxU32*)(((char*)srcData)+srcJump);
      }
    } else {
      while( scanline-- ) {
        end = (FxU32*)((char*)srcData + length - 2);
/*                
        if(gc->scanline_interleaved == FXTRUE) {
          if((scanline+odd) & 0x1)
            sst1InitSliPciOwner(gc->base_ptr, SST_SLI_MASTER_OWNPCI);
          else
            sst1InitSliPciOwner(gc->base_ptr, SST_SLI_SLAVE_OWNPCI);
        }
*/
        (*(FxU16*)dstData) = (*(FxU16*)srcData);
        dstData = (FxU32*)(((FxU16*)dstData) + 1 );
        srcData = (FxU32*)(((FxU16*)srcData) + 1 );

        while( srcData < end )
          *dstData++ = *srcData++;

        if ( !(((int)length) & 0x2) ) {
          (*(FxU16*)dstData) = (*(FxU16*)srcData);
          dstData = (FxU32*)(((FxU16*)dstData) + 1 );
          srcData = (FxU32*)(((FxU16*)srcData) + 1 );
        }
                
        dstData = (FxU32*)(((char*)dstData)+dstJump);
        srcData = (FxU32*)(((char*)srcData)+srcJump);
      }
    }
    grLfbUnlock( GR_LFB_READ_ONLY, src_buffer );
    /*
    if ( gc->scanline_interleaved ) 
      sst1InitSliPciOwner( gc->base_ptr, SST_SLI_MASTER_OWNPCI );
    */
  } else {
    rv = FXFALSE;
  }
  return rv; 
}

//*************************************************
FX_ENTRY FxBool FX_CALL
grLfbWriteRegion( GrBuffer_t dst_buffer,
                  FxU32 dst_x, FxU32 dst_y,
                  GrLfbSrcFmt_t src_format,
                  FxU32 src_width, FxU32 src_height,
                  FxI32 src_stride, void *src_data )
{
#if defined(OGL_DONE) || defined(OGL_FRAMEBUFFER)
  GlideMsg("grLfbWriteRegion( %d, %d, %d, %d, %d, %d, %d, 0x%x )\n",
      dst_buffer, dst_x, dst_y, src_format, src_width, src_height, src_stride, src_data);
#endif

	// Copied from the linux sst1 driver src
  FxBool           rv = FXTRUE;
  GrLfbInfo_t      info;
  GrLfbWriteMode_t writeMode;
  
  /*
  GR_BEGIN_NOFIFOCHECK("grLfbWriteRegion",82);
  GDBG_INFO_MORE((gc->myLevel,
                  "(0x%x,%d,%d,%d,%d,%d,%d,0x%x)\n", 
                  dst_buffer, dst_x, dst_y, 
                  src_format, src_width, src_height,
                  src_stride, src_data ));
  */
  
// #if ( GLIDE_PLATFORM & GLIDE_HW_SST1 )
  if ( src_format == GR_LFB_SRC_FMT_RLE16 ) 
    writeMode = GR_LFBWRITEMODE_565;
  else 
    writeMode = src_format;
  
  info.size = sizeof( info );
  
  if ( grLfbLock( GR_LFB_WRITE_ONLY | GR_LFB_NOIDLE, 
                 dst_buffer, 
                 writeMode,
                 GR_ORIGIN_UPPER_LEFT,
                 FXFALSE,
                 &info ) ) {
    FxU32 *srcData;             /* Tracking Source Pointer */
    FxU32 *dstData;             /* Tracking Destination Pointer */
    FxU32 *end;                 /* Demarks End of each Scanline */
    FxI32 srcJump;              /* bytes to next scanline */
    FxU32 dstJump;              /* bytes to next scanline */
    FxU32 length;               /* bytes to copy in scanline */
    FxU32 scanline;             /* scanline number */
    int   aligned;              /* word aligned? */
    
    
    srcData = ( FxU32 * ) src_data;
    dstData = ( FxU32 * ) ( ((char*)info.lfbPtr)+
                           (dst_y*info.strideInBytes) );
    scanline = src_height;
    
    switch( src_format ) {
      /* 16-bit aligned */
    case GR_LFB_SRC_FMT_565:
    case GR_LFB_SRC_FMT_555:
    case GR_LFB_SRC_FMT_1555:
    case GR_LFB_SRC_FMT_ZA16:
      dstData = (FxU32*)(((FxU16*)dstData) + dst_x);
      length  = src_width * 2;
      aligned = !((int)dstData&0x2);
      srcJump = src_stride - length;
      dstJump = info.strideInBytes - length;
      if ( aligned )
      {
        while( scanline-- )
        {
          // GR_SET_EXPECTED_SIZE(length);
          end = (FxU32*)((char*)srcData + length - 2);
          while( srcData < end )
          {
            // GR_SET( dstData[0], srcData[0] );
#ifdef OPENGLIDE_HOST_MAC
            swaplong(dstData, srcData);
#else
            *dstData = *srcData;
#endif
            dstData++;
            srcData++;
          }
                    
          if ( ((int)length) & 0x2 )
          {
          	/*
            GR_SET16( (*(FxU16*)&(dstData[0])),
                     (*(FxU16*)&(srcData[0])) );
            */
            (*(FxU16*)&(dstData[0])) = (*(FxU16*)&(srcData[0]));
            
            dstData = (FxU32*)(((FxU16*)dstData) + 1 );
            srcData = (FxU32*)(((FxU16*)srcData) + 1 );
          }
                    
          dstData = (FxU32*)(((char*)dstData)+dstJump);
          srcData = (FxU32*)(((char*)srcData)+srcJump);
          // GR_CHECK_SIZE_SLOPPY();
        }
      }
      else
      {
        while( scanline-- ) {
          // GR_SET_EXPECTED_SIZE(length);
          end = (FxU32*)((char*)srcData + length - 2);
          /*          
          GR_SET16( (*(FxU16*)&(dstData[0])),
                   (*(FxU16*)&(srcData[0])) );
          */
#ifdef OPENGLIDE_HOST_MAC
            swapshort(dstData, srcData);
#else
	          (*(FxU16*)&(dstData[0])) = (*(FxU16*)&(srcData[0]));
#endif

          dstData = (FxU32*)(((FxU16*)dstData) + 1 );
          srcData = (FxU32*)(((FxU16*)srcData) + 1 );
                    
          while( srcData < end ) {
            // GR_SET( dstData[0], srcData[0] );
#ifdef OPENGLIDE_HOST_MAC
            swaplong(dstData, srcData);
#else
            *dstData = *srcData;
#endif
            dstData++;
            srcData++;
          }
                    
          if ( !(length & 0x2) )
          {
          	/*
            GR_SET16( (*(FxU16*)&(dstData[0])),
                     (*(FxU16*)&(srcData[0])) );
            */
#ifdef OPENGLIDE_HOST_MAC
            swapshort(dstData, srcData);
#else
	          (*(FxU16*)&(dstData[0])) = (*(FxU16*)&(srcData[0]));
#endif
 
            dstData = (FxU32*)(((FxU16*)dstData) + 1 );
            srcData = (FxU32*)(((FxU16*)srcData) + 1 );
          }
                    
          dstData = (FxU32*)(((char*)dstData)+dstJump);
          srcData = (FxU32*)(((char*)srcData)+srcJump);
          // GR_CHECK_SIZE_SLOPPY();
        }
      }
      break;
      /* 32-bit aligned */
    case GR_LFB_SRC_FMT_888:
    case GR_LFB_SRC_FMT_8888:
    case GR_LFB_SRC_FMT_565_DEPTH:
    case GR_LFB_SRC_FMT_555_DEPTH:
    case GR_LFB_SRC_FMT_1555_DEPTH:
      dstData = ((FxU32*)dstData) + dst_x;
      length  = src_width * 4;
      srcJump = src_stride - length;
      dstJump = info.strideInBytes - length;
      while( scanline-- ) {
        // GR_SET_EXPECTED_SIZE(length);
        end = (FxU32*)((char*)srcData + length);
        while( srcData < end ) {
          // GR_SET( dstData[0], srcData[0] );
#ifdef OPENGLIDE_HOST_MAC
            swaplong(dstData, srcData);
#else
            *dstData = *srcData;
#endif
          dstData++;
          srcData++;
        }
        dstData = (FxU32*)(((char*)dstData)+dstJump);
        srcData = (FxU32*)(((char*)srcData)+srcJump);
        // GR_CHECK_SIZE_SLOPPY();
      }
      break;
    case GR_LFB_SRC_FMT_RLE16:
      /* needs to be implemented */
      rv = FXFALSE;
      break;
    }
    grLfbUnlock( GR_LFB_WRITE_ONLY, dst_buffer );
  } else {
    rv = FXFALSE;
  }
	return rv;
}

FX_ENTRY void FX_CALL 
grLfbConstantAlpha( GrAlpha_t alpha )
{
#ifdef /* OGL_CRITICAL*/ OGL_DONE
	GlideMsg("grLfbConstantAlpha( %lu )\n", alpha );
#endif
	s_Framebuffer.SetAlpha(alpha);
}

FX_ENTRY void FX_CALL 
grLfbConstantDepth( FxU16 depth )
{
#ifdef /* OGL_CRITICAL*/ OGL_DONE
	GlideMsg("grLfbConstantDepth( %u )\n", depth );
#endif
	if (OpenGL.DepthBufferType ==  1) //  == GR_DEPTHBUFFER_ZBUFFER)
	{
		// map depth to range (0.0, 1.0)
		s_Framebuffer.SetDepth(depth * D1OVER65535);
	}
	else
	{
		// depth is a float value
#ifdef /* OGL_CRITICAL*/ OGL_DONE
	GlideMsg("grLfbConstantDepth( %u ) wbuffer not done\n", depth );
#endif
		s_Framebuffer.SetDepth(depth * D1OVER65535);
	}
}

FX_ENTRY void FX_CALL 
grLfbWriteColorSwizzle( FxBool swizzleBytes, FxBool swapFxU16s )
{
#ifdef /* OGL_CRITICAL*/ OGL_NOTDONE
	GlideMsg("grLfbWriteColorSwizzle( %d, %d )\n",
						swizzleBytes, swapFxU16s );
#endif
}

FX_ENTRY void FX_CALL
grLfbWriteColorFormat( GrColorFormat_t colorFormat )
{
#ifdef /* OGL_CRITICAL */ OGL_NOTDONE
	GlideMsg("grLfbWriteColorFormat( %u )\n", colorFormat );
#endif
}
