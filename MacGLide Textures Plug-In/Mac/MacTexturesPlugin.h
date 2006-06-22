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

// library entry points
#pragma export on
extern "C" OSErr __macglide_textures_plugin_initialize(struct CFragInitBlock* initBlkPtr);
extern "C" void __macglide_textures_plugin_terminate();
extern "C" OSErr __initialize(struct CFragInitBlock* initBlkPtr);
extern "C" void __terminate();
#pragma export off
