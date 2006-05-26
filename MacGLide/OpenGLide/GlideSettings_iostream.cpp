//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                   OpenGLide Settings File
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "GlideSettings_iostream.h"

GlideSettingsIOStream::GlideSettingsIOStream()
{
}

GlideSettingsIOStream::~GlideSettingsIOStream(void)
{
	if (m_FileBuffer) delete(m_FileBuffer);
}

GlideSettings::IOErr GlideSettingsIOStream::init(const char* applicationname)
{
	return -1;	
}

GlideSettings::IOErr GlideSettingsIOStream::read_defaults()
{
#ifdef OGL_DEBUG
	GlideMsg("Reading default settings...\n");
#endif
	return -1;	
}

GlideSettings::IOErr GlideSettingsIOStream::read()
{
#ifdef OGL_DEBUG
	GlideMsg("Reading application specific settings...\n");
#endif
	return -1;	
}

GlideSettings::IOErr GlideSettingsIOStream::create_defaults()
{
#ifdef OGL_DEBUG
	GlideMsg("Creating new file with default settings...\n");
#endif
	return -1;	
}

GlideSettings::IOErr GlideSettingsIOStream::create()
{
#ifdef OGL_DEBUG
	GlideMsg("Creating new file with application specific settings...\n");
#endif
	return -1;	
}

GlideSettings::IOErr GlideSettingsIOStream::put_raw(const char* string)
{
	return -1;	
}

GlideSettings::IOErr GlideSettingsIOStream::close()
{
	return -1;	
}

GlideSettings::IOErr GlideSettingsIOStream::create_log()
{
	return -1;	
}

GlideSettings::IOErr GlideSettingsIOStream::write_log(const char* message)
{
	return -1;	
}
