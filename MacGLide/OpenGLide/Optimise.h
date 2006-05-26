//**************************************************************
//*     OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                    Optimise switches
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************


// Check if the Glide state really changes before executing a function.
// The idea behind this is to avoid unnecessary calls to RenderDrawTriangles().
// As a result, the necessary call to RenderDrawTriangles() can render more
// triangles at once. 
//
// Most called functions (by analysing Falcon4.0):
// - grAlphaBlendFunction
// - grAlphaCombine
// - grAlphaTestFunction
// - grAlphaTestReferenceValue
// - grChromakeyMode
// - grChromakeyValue
// - grConstantColorValue
// - grConstantColorValue4
// - grClipWindow
// - grColorCombine
// - grDepthBufferMode
// - grDitherMode
// - grFogMode
// - grTexClampMode
// - grTexCombine
// - grTexFilterMode
// - grTexMipMapMode
// - guTexSource
//
// Would need support but are not yet implemented
// - grTexDetailControl
// - grHints
//
// The following function names appear in the logs on each call,
// but just call other functions which are already state-optimised.
// - guTexCombineFunction
// - guColorCombineFunction
//
// Optimises Glide calls with parameters that wouldn't change the Glide state
// (by placing the CHECK_STATE_CHANGED macro at the beginning of a function)
#define OPTIMISE_GLIDE_STATE_CHANGES

// Optimises internal function calls that wouldn't result
// in a change of the OpenGL state
#define OPTIMISE_OPENGL_STATE_CHANGES

// Retrieve TexDB records via a lookup table half the size of the texture memory. 
#define OPTIMISE_TEXTURE_LOOKUP
