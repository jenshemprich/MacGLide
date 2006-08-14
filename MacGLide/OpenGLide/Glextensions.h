//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                 OpenGL Extensions Header
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#ifndef __GLEXTENSIONS__
#define __GLEXTENSIONS__

void ValidateUserConfig();
void GLExtensions( );
void GLExtensionsCleanup();
bool OGLIsExtensionSupported(const char* extensions, const char* extension);

#endif
