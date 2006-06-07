//**************************************************************
//*            MacGLide - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*               Shared Library entry points
//*
//*  MacGLideTexturesPlugin is OpenSource under LGPL license
//*          Originally made by Jens-Olaf Hemprich
//**************************************************************

#include "TexturesPlugin.h"
#include <stdio.h>

void(*glide_message)(const char *message, ...);
void(*glide_error)(const char *message, ...);

void setMsgFunction(void(*function)(const char *message, ...))
{
	glide_message = function;
}

void setErrorFunction(void(*function)(const char *message, ...))
{
	glide_error = function;
}

void GlideMsg(const char* message, ... )
{
	if (glide_message)
	{
		va_list(args);
		va_start(args, message);
		char buffer[StringBufferSize];
		vsnprintf(buffer, StringBufferSize, message, args);
		(*glide_message)(&buffer[0]);
		va_end(args);
	}
}

void GlideError(const char *message, ... )
{
	if (glide_error)
	{
		va_list(args);
		va_start(args, message);
		char buffer[StringBufferSize];
		vsnprintf(buffer, StringBufferSize, message, args);
		(*glide_error)(&buffer[0]);
		va_end(args);
	}
}
