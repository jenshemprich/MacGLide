//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                   OpenGLide Settings File
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "GlideSettings_FSp.h"

char* s_SettingsFolderName = "XMacGLide Settings";
const char* s_DefaultSettingsFileName = "Defaults";
const char* s_LogFileName = "MacGLide.log";

OSType s_SettingsFileCreator = 0x54545854;  // "ttxt"
OSType s_SettingsFileType = 0x54455854;  // "TEXT"
OSType s_LogFileCreator = 0x54545854;  // "ttxt"
OSType s_LogFileType = 0x54455854;  // "TEXT"

GlideSettingsFSp::GlideSettingsFSp()
: m_vRefNumPrefsFolder(0)
, m_dirIDPrefsFolder(0)
, m_SettingsFileRefnum(0)
, m_LogFileRefnum(0)
{
}

GlideSettingsFSp::~GlideSettingsFSp(void)
{
	if (m_FileBuffer) DisposePtr(m_FileBuffer);
	FSClose(m_LogFileRefnum);
}

GlideSettings::IOErr GlideSettingsFSp::makeSettingsFolderFileSpec(const char* path, const char*  filename, FSSpec* fsspec)
{
	char buffer[StringBufferSize];
	strcpy(&buffer[1], ":");
	strcat(&buffer[1], path);
	strcat(&buffer[1], ":");
	strcat(&buffer[1], filename);
	GlideSettings::IOErr err = FSMakeFSSpec(m_vRefNumPrefsFolder, m_dirIDPrefsFolder, PascalString(buffer), fsspec);
	if (err == fnfErr) err = noErr;
	return err;
}

GlideSettings::IOErr GlideSettingsFSp::init(const char* applicationname)
{
	// Make the settings folder
	GlideSettings::IOErr err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &m_vRefNumPrefsFolder, &m_dirIDPrefsFolder);
	if (err == noErr)
	{
		FSSpec fsspec;
		err = FSMakeFSSpec(m_vRefNumPrefsFolder, m_dirIDPrefsFolder, PascalString(s_SettingsFolderName), &fsspec);
		if (err == fnfErr)
		{
			long dirID;
			err = FSpDirCreate(&fsspec, smSystemScript, &dirID);
		}
	}
	// Make references to various files
	if (err == noErr)
	{
		err = makeSettingsFolderFileSpec(&s_SettingsFolderName[1], applicationname, &m_fsSettingsFile);
		if (err == noErr)
		{
			err = makeSettingsFolderFileSpec(&s_SettingsFolderName[1], s_DefaultSettingsFileName, &m_fsDefaultSettingsFile);
			if (err == noErr)
			{
				err = makeSettingsFolderFileSpec(&s_SettingsFolderName[1], s_LogFileName, &m_fsLogFile);
			}
		}
	}
	return err;
}

GlideSettings::IOErr GlideSettingsFSp::read_file(FSSpec* file, char** mem, long* size, short* refnum)
{
	GlideSettings::IOErr err = FSpOpenDF(file, fsRdPerm, refnum);
	if (err == noErr)
	{
		err = GetEOF(*refnum, size);
		if (err == noErr)
		{
			*mem = new char[*size + 1];
			(*mem)[*size] = 0x00;
			err = FSRead(*refnum, size, *mem);
			if (err == noErr)
			{
				err = FSClose(*refnum);
			}
			else
			{
				FSClose(*refnum);
			}
		}
	}
	return err;	
}

GlideSettings::IOErr GlideSettingsFSp::read_defaults()
{
#ifdef OGL_DEBUG
	GlideMsg("Reading default settings...\n");
#endif
	return read_settings_from_file(&m_fsDefaultSettingsFile, &GlideSettings::create_defaults);
}

GlideSettings::IOErr GlideSettingsFSp::read()
{
#ifdef OGL_DEBUG
	GlideMsg("Reading application specific settings...\n");
#endif
	return read_settings_from_file(&m_fsSettingsFile, &GlideSettings::create);
}

GlideSettings::IOErr GlideSettingsFSp::read_settings_from_file(FSSpec* file, GlideSettingsFSp::IOErr (GlideSettings::*default_creator) ())
{
	GlideSettings::IOErr err = read_file(file, &m_FileBuffer, &m_FileBufferSize, &m_SettingsFileRefnum);
	if (err != noErr)
	{
		// create file with default settings
		defaults();
		err = (this->*default_creator)();
		if (err == noErr)
		{
			err = save();
			if (err == noErr)
			{
				// Now we have o load the file into memory to make the settings reader happy
				err = read_file(file, &m_FileBuffer, &m_FileBufferSize, &m_SettingsFileRefnum);	
			}
		}
	}
	return err;
}

GlideSettings::IOErr GlideSettingsFSp::create_defaults()
{
#ifdef OGL_DEBUG
	GlideMsg("Creating new file with default settings...\n");
#endif
	GlideSettings::IOErr err = FSpDelete(&m_fsDefaultSettingsFile);
	err = FSpCreate(&m_fsDefaultSettingsFile, s_SettingsFileCreator, s_SettingsFileType, smSystemScript);
	if (err == noErr || err == dupFNErr)
	{
		err = FSpOpenDF(&m_fsDefaultSettingsFile, fsWrPerm, &m_SettingsFileRefnum);
	}
	if (err == noErr)
	{
#ifdef OGL_DEBUG
	GlideMsg("Looking for obsolete 0.09 settings in app dir...\n");
#endif
		ProcessSerialNumber psn;
		if (GetCurrentProcess(&psn) == noErr)
		{
			ProcessInfoRec info;
			memset(&info, 0, sizeof(info));
			info.processInfoLength = sizeof(info);
			FSSpec fsspec;
			info.processAppSpec = &fsspec;
			GlideSettings::IOErr err2 = GetProcessInformation(&psn, &info);
			if (err2 == noErr)
			{
				const unsigned int number_of_old_files = 3;
				char* old_files[number_of_old_files] =
				{
					"X:OpenGLid.INI",
					"X:OpenGLid.LOG",
					"X:OpenGLid.ERR"
				};
				for(unsigned long i = 0; i < number_of_old_files; i++)
				{
					err2 = HDelete(fsspec.vRefNum, fsspec.parID, PascalString(old_files[i]));
					if (err2 == noErr) GlideMsg("Removed 0.09 %s in app dir\n", &(old_files[i])[2]);
				}
			}
		}
	}
	return err;
}

GlideSettings::IOErr GlideSettingsFSp::create()
{
#ifdef OGL_DEBUG
	GlideMsg("Creating new file with application specific settings...\n");
#endif
	GlideSettings::IOErr err = FSpDelete(&m_fsSettingsFile);
	err = FSpCreate(&m_fsSettingsFile, s_SettingsFileCreator, s_SettingsFileType, smSystemScript);
	if (err == noErr || err == dupFNErr)
	{
		err = FSpOpenDF(&m_fsSettingsFile, fsWrPerm, &m_SettingsFileRefnum);
		if (err == noErr)
		{
			// Enable using this file
			UseApplicationSpecificSettings = true;
		}
	}
	return err;
}

GlideSettings::IOErr GlideSettingsFSp::put_raw(const char* string)
{
	long count = strlen(string);
	GlideSettings::IOErr err = FSWrite(m_SettingsFileRefnum, &count, string);
	return err;
}

GlideSettings::IOErr GlideSettingsFSp::close()
{
	GlideSettings::IOErr err = FSClose(m_SettingsFileRefnum);
	return err;
}

GlideSettings::IOErr GlideSettingsFSp::create_log()
{
	GlideSettings::IOErr err = FSpDelete(&m_fsLogFile);
	if (err == noErr || err == fnfErr)
	{
		err = FSpCreate(&m_fsLogFile, s_SettingsFileCreator, s_SettingsFileType, smSystemScript);
		if (err == noErr)
		{
			err = FSpOpenDF(&m_fsLogFile, fsWrPerm, &m_LogFileRefnum);
		}
	}
	return err;
}

GlideSettings::IOErr GlideSettingsFSp::write_log(const char* message)
{
	long count = strlen(message);
	IOErr err = FSWrite(m_LogFileRefnum, &count, message);
	return err;
}

const unsigned char* GlideSettingsFSp::PascalString(char* string)
{
	string[0] = static_cast<char>(strlen(&string[1]));
	return reinterpret_cast<const unsigned char*>(string);
}
