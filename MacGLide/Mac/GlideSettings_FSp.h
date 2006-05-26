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

class GlideSettingsFSp : public GlideSettings
{
public:
	GlideSettingsFSp();
	virtual ~GlideSettingsFSp(void);
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
	const unsigned char* PascalString(char* string);
	short m_vRefNumPrefsFolder;
	long  m_dirIDPrefsFolder;
	FSSpec m_fsSettingsFile;
	FSSpec m_fsDefaultSettingsFile;
	short m_SettingsFileRefnum;
	FSSpec m_fsLogFile;
	short m_LogFileRefnum;
	IOErr makeSettingsFolderFileSpec(const char* path, const char*  filename, FSSpec* fsspec);
	IOErr read_file(FSSpec* file, char** mem, long* size, short* refnum);
	IOErr read_settings_from_file(FSSpec* file, IOErr (GlideSettings::*default_creator)());
};
