//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*               format conversion functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#ifndef __FORMAT_CONVERSION_H__
#define __FORMAT_CONVERSION_H__

// Introduced by MacGLide (available for Big Endian only)
void Convert565Kto5551(const FxU32* Buffer1, FxU32 key, FxU32* Buffer2, FxU32 NumberOfPixels );
void ConvertP8Kto8888(const FxU8 *Buffer1, FxU32 Key, FxU32 *Buffer2, FxU32 Pixels, FxU32 *palette );

// Litte Endian OpenGLide functions implemented in FormatConversion.cpp
// (Obsolete functions are commented out)
void Convert565to8888(const FxU16 *Buffer1, FxU32 *Buffer2, FxU32 Pixels );
void Convert565Kto8888(const FxU16 *Buffer1, FxU16 key, FxU32 *Buffer2, FxU32 Pixels );
void Convert565to5551(const FxU32 *Buffer1, FxU32 *Buffer2, FxU32 Pixels );
void Convert5551to565(const FxU32 *Buffer1, FxU32 *Buffer2, FxU32 Pixels );
void Convert4444to4444special(const FxU32 *Buffer1, FxU32 *Buffer2, int Pixels );
void Convert1555to5551(const FxU32 *Buffer1, FxU32 *Buffer2, int Pixels );
/*
void MMXConvert1555to5551(const void *Src, void *Dst, int NumberOfPixels );
void MMXConvert565to5551(const void *Src, void *Dst, int NumberOfPixels );
void MMXConvert565Kto5551(const void *Src, FxU32 key, void *Dst, int NumberOfPixels );
void MMXConvert5551to565(const void *Src, void *Dst, int NumberOfPixels );
void MMXConvert565to8888(const void *Src, void *Dst, FxU32 NumberOfPixels );
void MMXConvert4444to4444special(const void *Src, void *Dst, int NumberOfPixels );
*/
void ConvertA8toAP88(const FxU8 *Buffer1, FxU16 *Buffer2, FxU32 Pixels );
void ConvertAI44toAP88(const FxU8 *Buffer1, FxU16 *Buffer2, FxU32 Pixels );
void Convert8332to8888(const FxU16 *Buffer1, FxU32 *Buffer2, FxU32 Pixels );
void ConvertP8to8888(const FxU8 *Buffer1, FxU32 *Buffer2, FxU32 Pixels, FxU32 *palette );
void ConvertAP88to8888(const FxU16 *Buffer1, FxU32 *Buffer2, FxU32 Pixels, FxU32 *palette );
void ConvertYIQto8888(const FxU8 *in, FxU32 *out, FxU32 Pixels, GuNccTable *ncc );
void ConvertAYIQto8888(const FxU16 *in, FxU32 *out, FxU32 Pixels, GuNccTable *ncc );
void SplitAP88(const FxU16 *ap88, FxU8 *index, FxU8 *alpha, FxU32 pixels );
void Convert1555Kto8888(const FxU16* Buffer1, FxU16 Key, FxU32* Buffer2, FxU32 Pixels);

#endif
