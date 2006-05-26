//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                  OpenGLide Settings File
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#pragma once

#include "GlideSettings.h"

class GlideSettingsIOStream : public GlideSettings
{
public:
	GlideSettingsIOStream();
	virtual ~GlideSettingsIOStream(void);
public:
	IOErr init(const char* application);
	IOErr create_log();
	IOErr write_log(const char* message);
protected:
	IOErr create_defaults();
	IOErr create();
	IOErr read_defaults();
	IOErr read();
	IOErr put_raw(const char* string);
	IOErr close();
};
