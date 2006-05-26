//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*               Glide application management
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "GlideApplication.h"

GlideApplication s_GlideApplication;

GlideApplication::GlideApplication()
{
	Init();
}

void GlideApplication::Init()
{
	ProcessSerialNumber psn;
	if (GetCurrentProcess(&psn) == noErr)
	{
		ProcessInfoRec info;
		memset(&info, 0, sizeof(info));
		info.processInfoLength = sizeof(info);
		memset(&m_Name[0], 0, sizeof(m_Name));
		info.processName = reinterpret_cast<unsigned char*>(&m_Name[0]);
		GetProcessInformation(&psn, &info);
		// Convert pascal to c-string
		m_Name[m_Name[0] + 1] = 0x0;
		// set application type
		     if (strstr((char*) &m_Name[1], "Tomb Raider III") ) m_Type = TombRaiderIII;
		else if (strstr((char*) &m_Name[1], "Tomb Raider II Gold") ) m_Type = TombRaiderII;
		else if (strstr((char*) &m_Name[1], "Tomb Raider II") ) m_Type = TombRaiderII;
		else if (strstr((char*) &m_Name[1], "Tomb Raider I") ) m_Type = TombRaiderI;
		else if (strstr((char*) &m_Name[1], "TR Unfinished Business") ) m_Type = TombRaiderI;
		else if (strstr((char*) &m_Name[1], "Carmageddon 2") ) m_Type = Carmageddon2;
		else if (strstr((char*) &m_Name[1], "Carmageddon 3dfx") ) m_Type = Carmageddon;
		else if (strstr((char*) &m_Name[1], "Falcon 4.0") ) m_Type = Falcon40;
		else if (strstr((char*) &m_Name[1], "F/A-18") ) m_Type = FA18;
		else if (strstr((char*) &m_Name[1], "Future Cop") ) m_Type = FutureCop;
		else if (strstr((char*) &m_Name[1], "Myth The Fallen Lords") ) m_Type = MythTFL;
		else if (strstr((char*) &m_Name[1], "Quake 3Dfx") ) m_Type = Quake;
		else m_Type = Generic;
	}
}