//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*                     Macintosh display
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Displays.h"
#include "GlideApplication.h"
#include "GlideDisplay.h"

// display manager related stuff
const unsigned int VideoGammaTableSize = sizeof(CMVideoCardGamma) + 256 * 2;
struct DisplayModeSpec
{
	unsigned long HorizontalPixels_wanted;
	unsigned long VerticalLines_wanted;
	unsigned long RefreshRate_wanted;
	unsigned long DisplayMode_wanted;
	unsigned long HorizontalPixels;
	unsigned long VerticalLines;
	unsigned long RefreshRate;
	unsigned long DisplayMode;
	unsigned long ColorDepth;
	GDHandle Device;
	DisplayIDType DisplayID;
	char VideoGammaTable[VideoGammaTableSize];	// Actually the CMVideoCardGamma table
	bool Valid;
	bool GammaValid;
};

// gamma related stuff
CMError DisplayManager_RememberGamma(DisplayModeSpec& display);
CMError DisplayManager_RestoreGamma(DisplayModeSpec& display);
CMError DisplayManager_SetGamma(DisplayModeSpec& display, FxFloat gamma);
bool DisplayManager_applyGamma();
CMError DisplayManager_SetGammaBlack(DisplayModeSpec& display);
// Usally games have a Glide resolution
DisplayModeSpec DesktopDisplay;
// a resolution for displaying In-Game graphics via passthrough
DisplayModeSpec PassthroughDisplay;
// and the desktop resolution
DisplayModeSpec GlideDisplay;
// Copy of the colorsync profile of the monitor we're using
CMProfileRef DesktopColorSyncProfile;
// Whether to restore the menubar
bool RestoreMenuBar;

bool DisplayManager_ColorSyncAvailable()
{
	return 	(Ptr) CMCloseProfile != (Ptr) kUnresolvedCFragSymbolAddress &&
	       	(Ptr) CMGetSystemProfile != (Ptr) kUnresolvedCFragSymbolAddress &&
	       	(Ptr) CMProfileElementExists != (Ptr) kUnresolvedCFragSymbolAddress &&
	       	(Ptr) CMGetProfileByAVID != (Ptr) kUnresolvedCFragSymbolAddress &&
	       	(Ptr) CMSetProfileByAVID != (Ptr) kUnresolvedCFragSymbolAddress &&
	       	(Ptr) CMGetGammaByAVID != (Ptr) kUnresolvedCFragSymbolAddress &&
	        (Ptr) CMSetGammaByAVID != (Ptr) kUnresolvedCFragSymbolAddress;
}

bool DisplayManager_applyGamma()
{
	return DisplayManager_ColorSyncAvailable && DesktopDisplay.GammaValid && DesktopColorSyncProfile;
}

CMProfileRef DisplayManager_GetDesktopProfile()
{
	CMProfileRef profile = NULL;
	// F/A-18 exits here
	CMError err = CMGetProfileByAVID(DesktopDisplay.DisplayID, &profile);
	if (err == noErr)
	{
		bool gammatag_found = false;
		// Check for gamma tag
		err = CMProfileElementExists(profile, (unsigned long) cmVideoCardGammaTag, (unsigned char*) &gammatag_found);
		if (gammatag_found == false)
		{
			GlideMsg("Display Manager Warning: Using CMGetSystemProfile() to retrieve desktop gamma table\n");
			err = CMGetSystemProfile(&profile);
			if (err == noErr)
			{
				// Check for gamma tag
				err = CMProfileElementExists(profile, (unsigned long) cmVideoCardGammaTag, (unsigned char*) &gammatag_found);
				if (err == noErr && gammatag_found == false)
				{
					GlideMsg("Display Manager Warning: Gamma correction has been disabled because the System profile doesn't contain a gamma table\n");
					err = cmElementTagNotFound;
				}
			}
		}
	}
	if (err != noErr)
	{
		GlideError("Display Manager error: Retrieving the Desktop gamma correction table failed with error %d\n", err);
		if (profile)
		{
			CMCloseProfile(profile);
			profile = NULL;
		}
	}
	return profile;
}

void DisplayManager_Initialise()
{
	memset(&DesktopDisplay, 0, sizeof(DisplayModeSpec));
	memset(&PassthroughDisplay, 0, sizeof(DisplayModeSpec));
	memset(&GlideDisplay, 0, sizeof(DisplayModeSpec));
	// Get devices
	DesktopDisplay.Device = GetMainDevice();
	PassthroughDisplay.Device = GetMainDevice();
	GlideDisplay.Device = GetMainDevice();
	DesktopDisplay.GammaValid = false;
	RestoreMenuBar = false;
	DesktopColorSyncProfile = NULL;
 	if (DisplayManager_ColorSyncAvailable())
 	{
		// Get AVids
	 	OSErr err = DMGetDisplayIDByGDevice(DesktopDisplay.Device, &DesktopDisplay.DisplayID, true);
	 	if (err == noErr)
	 	{
		 	err = DMGetDisplayIDByGDevice(PassthroughDisplay.Device, &PassthroughDisplay.DisplayID, true);
		 	if (err == noErr)
		 	{
			 	err = DMGetDisplayIDByGDevice(GlideDisplay.Device, &GlideDisplay.DisplayID, true);
		 	}
	 	}
	 	if (err != noErr)
	 	{
			GlideMsg("Error: Unable to determine display id - gamma correction disabled\n"); 		
	 	}
	 	else
	 	{
	 		// Exits right at the start otherwise
			if (s_GlideApplication.GetType() == GlideApplication::FA18)
			{
				DesktopDisplay.GammaValid = false;
			}
			else
			{
				CMProfileRef profile = DisplayManager_GetDesktopProfile();
				if (profile)
		 		{
		 			err = CMCopyProfile(&DesktopColorSyncProfile, NULL, profile);
		 			if (err == noErr)
		 			{
						// Finally remember the gamma correction
						err =  DisplayManager_RememberGamma(DesktopDisplay);
						if (err == noErr)
						{
							DesktopDisplay.GammaValid = true;
						}
		 			}
					CMCloseProfile(profile);
		 		}
			 	if (err != noErr)
			 	{
					GlideMsg("Error: Unable to get desktop colorsync profile - gamma correction disabled\n"); 		
			 	}
			}
	 	}
	}
 	else
 	{
		GlideMsg("Warning: ColorSync not installed or available - gamma correction disabled\n");
 	}
}

void DisplayManager_Cleanup()
{
	// Release the temporary profile
	if (DesktopColorSyncProfile) CMCloseProfile(DesktopColorSyncProfile);
}

extern "C"
{
	pascal void DisplayModeForResolution(void* userData, DMListIndexType itemIndex, DMDisplayModeListEntryPtr displaymodeInfo)
	{
		DisplayModeSpec* d = static_cast<DisplayModeSpec*>(userData);
		VDResolutionInfoRec* res = displaymodeInfo->displayModeResolutionInfo;
		// Get the exact resolution, and try to pick the highest available (or exact) refresh rate available below the wanted one
		if (res->csHorizontalPixels == d->HorizontalPixels_wanted
		 && res->csVerticalLines == d->VerticalLines_wanted
		 && (d->RefreshRate_wanted == 0
			|| (d->RefreshRate_wanted >= (res->csRefreshRate / 65536) && d->RefreshRate <= (res->csRefreshRate / 65536)))
		 )
		{
			#ifdef OGL_DEBUG
				GlideMsg("Display Manager: Considering matching display mode %d with resolution %dx%d@%dHz\n",
					displaymodeInfo->displayModeResolutionInfo->csDisplayModeID,
					res->csHorizontalPixels,
					res->csVerticalLines,
					res->csRefreshRate / 65536);
			#endif
			// Copy values
			d->DisplayMode = displaymodeInfo->displayModeResolutionInfo->csDisplayModeID;
			d->HorizontalPixels = res->csHorizontalPixels;
			d->VerticalLines = res->csVerticalLines;
			d->RefreshRate = res->csRefreshRate / 65536;
			d->Valid = true;
		}
		// Get the next larger resolution, and try to pick the highest available (or exact) refresh rate available below the wanted one
		else if (res->csHorizontalPixels >= d->HorizontalPixels_wanted
		      && res->csVerticalLines >= d->VerticalLines_wanted
		      && d->HorizontalPixels >= res->csHorizontalPixels
		      && d->VerticalLines >= res->csVerticalLines
		      && (d->RefreshRate_wanted == 0
			|| (d->RefreshRate_wanted >= (res->csRefreshRate / 65536) && d->RefreshRate <= (res->csRefreshRate / 65536)))
		 )
		{
			#ifdef OGL_DEBUG
				GlideMsg("Display Manager: Considering next larger display mode %d with resolution %dx%d@%dHz\n",
					displaymodeInfo->displayModeResolutionInfo->csDisplayModeID,
					res->csHorizontalPixels,
					res->csVerticalLines,
					res->csRefreshRate / 65536);
			#endif
			// Copy values
			d->DisplayMode = displaymodeInfo->displayModeResolutionInfo->csDisplayModeID;
			d->HorizontalPixels = res->csHorizontalPixels;
			d->VerticalLines = res->csVerticalLines;
			d->RefreshRate = res->csRefreshRate / 65536;
			d->Valid = true;
		}
		else
		{
		#ifdef OGL_DEBUG
			GlideMsg("Display Manager: Found display mode %d with resolution %dx%d@%dHz\n",
				displaymodeInfo->displayModeResolutionInfo->csDisplayModeID,
				res->csHorizontalPixels,
				res->csVerticalLines,
				res->csRefreshRate / 65536);
		#endif
		}
	}

	pascal void ResolutionForDisplayMode(void* userData, DMListIndexType itemIndex, DMDisplayModeListEntryPtr displaymodeInfo)
	{
		DisplayModeSpec* d = static_cast<DisplayModeSpec*>(userData);
		VDResolutionInfoRec* res = displaymodeInfo->displayModeResolutionInfo;
		#ifdef OGL_DEBUG
			GlideMsg("Display Manager: Found display mode %d with resolution %dx%d@%dHz\n",
				displaymodeInfo->displayModeResolutionInfo->csDisplayModeID,
				res->csHorizontalPixels,
				res->csVerticalLines,
				res->csRefreshRate / 65536);
		#endif
		if (displaymodeInfo->displayModeResolutionInfo->csDisplayModeID == d->DisplayMode_wanted
				// && spec.device == res->device			
			)
		{
			d->HorizontalPixels = res->csHorizontalPixels;
			d->VerticalLines = res->csVerticalLines;
			d->RefreshRate = res->csRefreshRate / 65536;
			d->DisplayMode = displaymodeInfo->displayModeResolutionInfo->csDisplayModeID;
			d->Valid = true;
		}
	}
}

OSErr DisplayManager_Query(DMDisplayModeListIteratorProcPtr iteratorproc, DisplayModeSpec& spec)
{
	OSErr err = noErr;
	DMDisplayModeListIteratorUPP MyDMDisplayModeListIteratorUPP;
	MyDMDisplayModeListIteratorUPP = NewDMDisplayModeListIteratorUPP(iteratorproc);
	// use display manager to change the resolution
	spec.Device = DMGetFirstScreenDevice(dmOnlyActiveDisplays);
	while (spec.Device != nil)
	{
		DisplayIDType displayID;
		err = DMGetDisplayIDByGDevice(spec.Device, &displayID, true);
		if (err == noErr)
		{
			DMListIndexType thePanelCount;
			DMListType thePanelList;
			err = DMNewDisplayModeList(displayID, NULL, NULL, &thePanelCount, &thePanelList);
			if (err == noErr)
			{
				for (int i = 0; i < thePanelCount; i++)
				{
					err = DMGetIndexedDisplayModeFromList(thePanelList, i, NULL, MyDMDisplayModeListIteratorUPP, &spec);
					if (err == noErr)
					{
						// search until the exact resolution with the exact display mode was found
						if (spec.Valid && spec.RefreshRate == spec.RefreshRate_wanted) break;
					}
				}
				DMDisposeList(thePanelList);
			}
		}
		// At this point, we might have found a valid spec with the exact resolution,
		// but with a refresh rate smaller than requested one, which is ok
		// (For instance on LCD-iMacs, the refresh rate may be 0)
		// -> Don't get the next screen device
		if (spec.Valid) break;
	  // Get the next device in the list
	  spec.Device = DMGetNextScreenDevice(spec.Device, dmOnlyActiveDisplays);
	}
	DisposeDMDisplayModeListIteratorUPP(MyDMDisplayModeListIteratorUPP);
	return err;
}

OSErr DisplayManager_SetGlideDisplay(unsigned int width, unsigned int height, unsigned int freq)
{
	GlideDisplay.HorizontalPixels_wanted = width;
	GlideDisplay.VerticalLines_wanted = height;
	GlideDisplay.HorizontalPixels = 65535;
	GlideDisplay.VerticalLines = 65535;
	GlideDisplay.RefreshRate_wanted = freq;
	GlideDisplay.ColorDepth = 32;
	GlideDisplay.DisplayMode = 0;
	GlideDisplay.RefreshRate = 0;
	GlideDisplay.Valid = false;
	GlideDisplay.GammaValid = true; // we're not remembering this one
	Handle displayState = NULL;
 	OSErr err = DMBeginConfigureDisplays(&displayState);
	if (err == noErr)
	{
#ifdef OGL_DEBUG
		GlideMsg("Display Manager: Searching for display resolution %dx%dx%d@%dHz\n",
		         GlideDisplay.HorizontalPixels_wanted, GlideDisplay.VerticalLines_wanted,
		         GlideDisplay.ColorDepth, GlideDisplay.RefreshRate_wanted);
#endif
		err =  DisplayManager_Query(&DisplayModeForResolution, GlideDisplay);
		if (!GlideDisplay.Valid && err == noErr)
		{
			GlideMsg("Display Manager: Unable to find a valid display mode for Glide display\n");
			err = kDMNotFoundErr;
		}
		else if (err == noErr)
		{
			err = DMSetDisplayMode(GlideDisplay.Device, GlideDisplay.DisplayMode, &GlideDisplay.ColorDepth, NULL, displayState);
		}
		DMEndConfigureDisplays(displayState);
 	}
	if (err == noErr)
	{
#ifdef OGL_DEBUG
		GlideMsg("Display Manager: Using display mode %d with resolution %dx%dx%d@%dHz\n",
		         GlideDisplay.DisplayMode,
		         GlideDisplay.HorizontalPixels, GlideDisplay.VerticalLines,
		         GlideDisplay.ColorDepth, GlideDisplay.RefreshRate);
#endif
		if (IsMenuBarVisible()) HideMenuBar();

	}
	else
	{
		GlideMsg("Display Manager: Error while setting Glide display to %dx%dx%d@%dHz: %d\n",
		         GlideDisplay.HorizontalPixels_wanted, GlideDisplay.VerticalLines_wanted,
		         GlideDisplay.ColorDepth, GlideDisplay.RefreshRate_wanted, err);
	}
	return err;
}

OSErr DisplayManager_RememberPassthroughDisplay()
{
#ifdef OGL_DEBUG
	GlideMsg(OGL_LOG_SEPARATE);
#endif
	// Invalidate any previous search
	PassthroughDisplay.Valid = false;
	PassthroughDisplay.GammaValid = false;
	Handle displayState = NULL;
 	OSErr err = DMBeginConfigureDisplays(&displayState);
	if (err == noErr)
	{
		// save the old desktop resolution
		VDSwitchInfoRec switchinfo;
		err = DMGetDisplayMode(PassthroughDisplay.Device, &switchinfo);
		if (err == noErr)
		{
			PassthroughDisplay.DisplayMode_wanted = switchinfo.csData;
			PassthroughDisplay.ColorDepth = switchinfo.csMode;
#ifdef OGL_DEBUG
			GlideMsg("Display Manager: Searching for passthrough display mode %d\n", PassthroughDisplay.DisplayMode_wanted);
#endif
			// save previous display resolution
			err = DisplayManager_Query(&ResolutionForDisplayMode, PassthroughDisplay);
			if (!PassthroughDisplay.Valid && err == noErr)
			{
				GlideMsg("Display Manager: Unable to find the display resolution for the passthrough display\n");
				err = kDMNotFoundErr;
			}
		}
		DMEndConfigureDisplays(displayState);
	}
	if (err == noErr)
	{
#ifdef OGL_DEBUG
		GlideMsg("Display Manager: Remembering passthrough display mode %d with resolution %dx%dx%d@%dHz\n",
		         PassthroughDisplay.DisplayMode,
		         PassthroughDisplay.HorizontalPixels, PassthroughDisplay.VerticalLines,
		         PassthroughDisplay.ColorDepth, PassthroughDisplay.RefreshRate);
#endif
		err = DisplayManager_RememberGamma(PassthroughDisplay); 
		if (err == noErr)
		{
			PassthroughDisplay.GammaValid = true;
		}
		// Remember menubar status
		RestoreMenuBar = IsMenuBarVisible();
	}
	if (err != noErr)
	{
		GlideMsg("Display Manager: Error while remembering passthrough display mode: %d\n", err);
	}
	return err;
}

OSErr DisplayManager_RememberDesktopDisplay()
{
#ifdef OGL_DEBUG
	GlideMsg(OGL_LOG_SEPARATE);
#endif
	// Invalidate any previous search
	DesktopDisplay.Valid = false;
	// save the old desktop resolution
	Handle displayState = NULL;
 	OSErr err = DMBeginConfigureDisplays(&displayState);
	if (err == noErr)
	{
		VDSwitchInfoRec switchinfo;
		err = DMGetDisplayMode(DesktopDisplay.Device, &switchinfo);
		if (err == noErr)
		{
			DesktopDisplay.DisplayMode_wanted = switchinfo.csData;
			DesktopDisplay.ColorDepth = switchinfo.csMode;
#ifdef OGL_DEBUG
			GlideMsg("Display Manager: Searching for Desktop display mode %d\n", DesktopDisplay.DisplayMode_wanted);
#endif
			// save previous display resolution
			err = DisplayManager_Query(&ResolutionForDisplayMode, DesktopDisplay);
			if (!DesktopDisplay.Valid && err == noErr)
			{
				GlideMsg("Display Manager: Unable to find the display resolution for the Desktop display\n");
				err = kDMNotFoundErr;
			}
		}
		DMEndConfigureDisplays(displayState);
		// Desktop gamma already remembered in DisplayManager_Initialise()
	}
	if (err == noErr)
	{
#ifdef OGL_DEBUG
		GlideMsg("Display Manager: Remembering desktop display mode %d with resolution %dx%dx%d@%dHz\n",
			DesktopDisplay.DisplayMode,
			DesktopDisplay.HorizontalPixels, DesktopDisplay.VerticalLines,
			DesktopDisplay.ColorDepth, DesktopDisplay.RefreshRate);
#endif
	}
	else
	{
		GlideMsg("Display Manager: Error while remembering desktop display mode: %d\n", err);
	}
	return err;
}

OSErr DisplayManager_RestoreGlideDisplay()
{
#ifdef OGL_DEBUG
	GlideMsg(OGL_LOG_SEPARATE);
#endif
	OSErr err = noErr;
	if (GlideDisplay.Valid)
	{
		Handle displayState = NULL;
		err = DMBeginConfigureDisplays(&displayState);
		if (err == noErr)
		{
#ifdef OGL_DEBUG
			GlideMsg("Display Manager: Restoring Glide display mode %d with resolution %dx%dx%d@%dHz\n",
			         GlideDisplay.DisplayMode,
			         GlideDisplay.HorizontalPixels, GlideDisplay.VerticalLines,
			         GlideDisplay.ColorDepth, GlideDisplay.RefreshRate);
#endif
			if (err == noErr)
			{
				err = DMSetDisplayMode(GlideDisplay.Device, GlideDisplay.DisplayMode, &GlideDisplay.ColorDepth, NULL, displayState);
			}
			DMEndConfigureDisplays(displayState);
		}
	}
	else
	{
		GlideMsg("Display Manager: Glide display not restored\n");
	}
	if (err == noErr)
	{
		// Remember menubar status
		RestoreMenuBar = IsMenuBarVisible();
		if (RestoreMenuBar) HideMenuBar();
	}
	else
	{
		GlideMsg("Display Manager: Error while restoring Glide display mode: %d\n", err);
	}
	return noErr;
}

OSErr DisplayManager_RestorePassthroughDisplay()
{
#ifdef OGL_DEBUG
	GlideMsg(OGL_LOG_SEPARATE);
#endif
	OSErr err = noErr;
	if (PassthroughDisplay.Valid)
	{
		Handle displayState = NULL;
		err = DMBeginConfigureDisplays(&displayState);
		if (err == noErr)
		{
#ifdef OGL_DEBUG
			GlideMsg("Display Manager: Restoring passthrough display mode %d with resolution %dx%dx%d@%dHz\n",
			         PassthroughDisplay.DisplayMode,
			         PassthroughDisplay.HorizontalPixels, PassthroughDisplay.VerticalLines,
			         PassthroughDisplay.ColorDepth, PassthroughDisplay.RefreshRate);
#endif
			err = DMSetDisplayMode(PassthroughDisplay.Device, PassthroughDisplay.DisplayMode, &PassthroughDisplay.ColorDepth, NULL, displayState);
			DMEndConfigureDisplays(displayState);
		}
	}
	else
	{
		GlideMsg("Display Manager: Passthrough display not restored\n");
	}
	// Restore menubar and gamma
	if (err == noErr)
	{
		if (RestoreMenuBar) ShowMenuBar();
		if (PassthroughDisplay.GammaValid == true)
		{
			err = DisplayManager_RestoreGamma(PassthroughDisplay);
		}
	}
	if (err != noErr)
	{
		GlideMsg("Display Manager: Error while restoring passthrough display mode: %d\n", err);
	}
	return err;
}

OSErr DisplayManager_RestoreDesktopDisplay()
{
#ifdef OGL_DEBUG
	GlideMsg(OGL_LOG_SEPARATE);
#endif
	OSErr err = noErr;
	if (DesktopDisplay.Valid)
	{
		Handle displayState = NULL;
 		err = DMBeginConfigureDisplays(&displayState);
		if (err == noErr)
		{
#ifdef OGL_DEBUG
			GlideMsg("Display Manager: Restoring desktop display mode %d with resolution %dx%dx%d@%dHz\n",
			         DesktopDisplay.DisplayMode,
			         DesktopDisplay.HorizontalPixels, DesktopDisplay.VerticalLines,
			         DesktopDisplay.ColorDepth, DesktopDisplay.RefreshRate);
#endif
			err = DMSetDisplayMode(DesktopDisplay.Device, DesktopDisplay.DisplayMode, &DesktopDisplay.ColorDepth, NULL, displayState);
			DMEndConfigureDisplays(displayState);
		}
	}
	else
	{
#ifdef OGL_DEBUG
		GlideMsg("Display Manager: Desktop display not restored\n");
#endif
	}
	if (err == noErr)
	{
		if (DisplayManager_applyGamma())
		{
			err = CMSetProfileByAVID(DesktopDisplay.DisplayID, DesktopColorSyncProfile);
		}
	}
	if (err != noErr)
	{
		GlideMsg("Display Manager: Error while restoring desktop display mode: %d\n", err);
	}
	return err;
}

bool DisplayManager_GetDesktopDisplayResolution(unsigned long& width, unsigned long& height)
{
	width = DesktopDisplay.HorizontalPixels;
	height = DesktopDisplay.VerticalLines;
	return DesktopDisplay.Valid;
}

bool DisplayManager_GetGlideDisplayResolution(unsigned long& width, unsigned long& height)
{
	width = GlideDisplay.HorizontalPixels;
	height = GlideDisplay.VerticalLines;
	return GlideDisplay.Valid;
}

CMError DisplayManager_SetGamma(DisplayModeSpec& display, FxFloat gamma)
{
	CMError err = noErr;
	CMVideoCardGamma* gammatable = reinterpret_cast<CMVideoCardGamma*>(display.VideoGammaTable);
	if (gammatable && DisplayManager_applyGamma())
	{
		gammatable->tagType = cmVideoCardGammaTableType;
		gammatable->u.table.channels = 1;
		const unsigned int entryCount = 256;
		gammatable->u.table.entryCount = entryCount;
		gammatable->u.table.entrySize = 2;
		FxU16* gray = reinterpret_cast<FxU16*>(&gammatable->u.table.data);
		FxFloat biased_gamma = gamma + InternalConfig.GammaBias;
		// Valid values go from 0.0 to 20.0 according to Glide24pgm
		biased_gamma = max(0.0f, biased_gamma);
		biased_gamma = min(biased_gamma, 20.0f);
		for (unsigned int i = 0; i < entryCount; i++)
		{
	    gray[i] = pow(i / 255.0, 1.0 / biased_gamma) * 65535;
		}
		err = CMSetGammaByAVID(display.DisplayID, gammatable);
		if (err != noErr)
		{
			GlideMsg("Error: DisplayManager_SetGamma() failed with error code %d\n", err);
		}
	}
	return err;
}

void DisplayManager_SetGlideDisplayGamma(FxFloat gamma)
{
	CMError err = DisplayManager_SetGamma(GlideDisplay, gamma);
}

CMError DisplayManager_RememberGamma(DisplayModeSpec& display)
{
	CMError err = noErr;
	CMVideoCardGamma* gammatable = reinterpret_cast<CMVideoCardGamma*>(display.VideoGammaTable);
	if (gammatable && DisplayManager_applyGamma())
	{
		UInt32 size = VideoGammaTableSize;
		err = CMGetGammaByAVID(display.DisplayID, gammatable, &size);
		if (err != noErr)
		{
			GlideMsg("Error: DisplayManager_RememberGamma() failed with error code %d\n", err);
		}
	}
	return err;
}

CMError DisplayManager_RestoreGamma(DisplayModeSpec& display)
{
	CMError err = noErr;
	CMVideoCardGamma* gammatable = reinterpret_cast<CMVideoCardGamma*>(display.VideoGammaTable);
	if (display.GammaValid && gammatable && DisplayManager_applyGamma())
	{
		err = CMSetGammaByAVID(display.DisplayID, gammatable);
		if (err != noErr)
		{
			GlideMsg("Error: DisplayManager_RestoreGamma() failed with error code %d\n", err);
		}
	}
	return err;
}

void DisplayManager_RestorePassthroughDisplayGamma()
{
	DisplayManager_RestoreGamma(PassthroughDisplay);
}

CMError DisplayManager_SetGammaBlack(DisplayModeSpec& display)
{
	CMError err = noErr;
	// Only change if the gamma can be restored
	if (display.GammaValid && DisplayManager_applyGamma())
	{
		// We can savely use the Glide Display table, because it can be restored via SetGlideDisplayGamma()
		CMVideoCardGamma* gammatable = reinterpret_cast<CMVideoCardGamma*>(GlideDisplay.VideoGammaTable);
		gammatable->tagType = cmVideoCardGammaTableType;
		gammatable->u.table.channels = 1;
		const unsigned int entryCount = 256;
		gammatable->u.table.entryCount = entryCount;
		gammatable->u.table.entrySize = 2;
		FxU16* gray = reinterpret_cast<FxU16*>(&gammatable->u.table.data);
		memset(gray, 0, entryCount * gammatable->u.table.entrySize);
		err = CMSetGammaByAVID(display.DisplayID, gammatable);
		if (err != noErr)
		{
			GlideMsg("Error: DisplayManager_SetGammaBlack() failed with error code %d\n", err);
		}
	}
	return err;
}

void DisplayManager_SetGlideDisplayGammaBlack()
{
	DisplayManager_SetGammaBlack(GlideDisplay);	
}

void DisplayManager_SetPassthroughDisplayGammaBlack()
{
	DisplayManager_SetGammaBlack(PassthroughDisplay);	
}

void DisplayManager_SetDesktopDisplayGammaBlack()
{
	DisplayManager_SetGammaBlack(DesktopDisplay);	
}
