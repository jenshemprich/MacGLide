//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*               Glide application management
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#pragma once

class GlideApplication
{
public:
	GlideApplication();
	enum Type
	{
		Generic,
		Carmageddon,
		Carmageddon2,
		FA18,
		Falcon40,
		FutureCop,
		MythTFL,
		Quake,
		TombRaiderI,
		TombRaiderII,
		TombRaiderIII
	};
	const char* GetName() const {return &m_Name[1];};
	inline const Type GetType() const {return m_Type;}; 
protected:
	void Init();
	char m_Name[33];
	Type m_Type;
};

extern GlideApplication s_GlideApplication;
