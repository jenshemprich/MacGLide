//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                     Macintosh display
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

void DisplayManager_Initialise();
void DisplayManager_Cleanup();

OSErr DisplayManager_SetGlideDisplay(unsigned int width, unsigned int height, unsigned int freq);
OSErr DisplayManager_RememberPassthroughDisplay();
OSErr DisplayManager_RememberDesktopDisplay();
OSErr DisplayManager_RestoreGlideDisplay();
OSErr DisplayManager_RestorePassthroughDisplay();
OSErr DisplayManager_RestoreDesktopDisplay();
bool DisplayManager_GetDesktopDisplayResolution(unsigned long& width, unsigned long& height);
bool DisplayManager_GetGlideDisplayResolution(unsigned long& width, unsigned long& height);

// gamma related functions
void DisplayManager_SetGlideDisplayGamma(FxFloat gamma);
void DisplayManager_SetGlideDisplayGammaBlack();
void DisplayManager_SetPassthroughDisplayGammaBlack();
void DisplayManager_RestorePassthroughDisplayGamma();
void DisplayManager_SetDesktopDisplayGammaBlack();
