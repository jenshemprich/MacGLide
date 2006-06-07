//**************************************************************
//*            MacGLide - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*               Shared Library entry points
//*
//*  MacGLideTexturesPlugin is OpenSource under LGPL license
//*          Originally made by Jens-Olaf Hemprich
//**************************************************************

#pragma once

// size of temporary buffers for string processing
const int StringBufferSize = 256;

// library entry points
#pragma export on
extern "C" void setMsgFunction(void(*function)(const char *message, ...));
extern "C" void setErrorFunction(void(*function)(const char *message, ...));
#pragma export off

// Let's have the same interface for logging
void GlideMsg(const char *message, ...);
void GlideError(const char *message, ...);
