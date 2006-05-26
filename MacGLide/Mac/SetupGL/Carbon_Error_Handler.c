/*
	File:		Carbon Error Handler.c

	Contains:	SetupGL error handling

	Written by:	Geoff Stahl (ggs)

	Copyright:	Copyright � 1999-2001 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):

         <1>     1/19/01    ggs     Initial re-add
         <4>     1/24/00    ggs     double check for latest
         <4>     1/24/00    ggs     double check for latest
         <3>    12/18/99    ggs     Fix headers
         <2>    11/28/99    ggs     Added verbose error flag
         <1>    11/28/99    ggs     Initial add.  Split of just error handling functions.  Need to
                                    add more reporting examples
         <1>    11/11/99    ggs     Initial Add

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define kVerboseErrors
//#define kQuake3

// system includes ----------------------------------------------------------

#ifdef kQuake3
typedef int sysEventType_t;	// FIXME...
#endif

#ifdef __APPLE_CC__
    #include <Carbon/Carbon.h>
#else
    #include <MacTypes.h>
#endif

#include <stdio.h>


// project includes ---------------------------------------------------------

#include "Carbon_SetupDSp.h"
#include "Carbon_Error_Handler.h"

#ifdef kQuake3
#include "../renderer/tr_local.h"
#include "mac_local.h"
#endif


// globals (internal/private) -----------------------------------------------


// prototypes (internal/private) --------------------------------------------

static void CStrToPStr (StringPtr outString, const char *inString);


// functions (internal/private) ---------------------------------------------

// Copy C string to Pascal string

static void CStrToPStr (StringPtr outString, const char *inString)
{	
	unsigned char x = 0;
	do
	    *(((char*)outString) + x + 1) = *(inString + x++);
	while ((*(inString + x) != 0)  && (x < 256));
	*((char*)outString) = (char) x;									
}

#pragma mark -
// --------------------------------------------------------------------------

// central error reporting

void ReportErrorNum (char * strError, long numError)
{
	GlideMsg(strError, "%s %ld (0x%lx)\n", strError, numError, numError); 

/*
	char errMsgCStr [256];
	Str255 strErr = "\p";

	// out as debug string
#ifdef kVerboseErrors
	#ifdef kQuake3
		ri.Printf( PRINT_ALL, errMsgCStr);
	#else
		// ensure we are faded in
		if (gDSpStarted)
			DSpContext_CustomFadeGammaIn (NULL, NULL, 0);
		CStrToPStr (strErr, errMsgCStr);
		DebugStr (strErr);
	#endif // kQuake3
#endif // kVerboseErrors
*/
}

// --------------------------------------------------------------------------

void ReportError (char * strError)
{
	GlideError(strError);

/*
	char errMsgCStr [256];
	Str255 strErr = "\p";

	sprintf (errMsgCStr, "%s\n", strError); 
	// out as debug string
#ifdef kVerboseErrors
	#ifdef kQuake3
		ri.Printf( PRINT_ALL, errMsgCStr);
	#else
		// ensure we are faded in
		if (gDSpStarted)
			DSpContext_CustomFadeGammaIn (NULL, NULL, 0);
		CStrToPStr (strErr, errMsgCStr);
		DebugStr (strErr);
	#endif //  kQuake3
#endif // kVerboseErrors
*/
}

//-----------------------------------------------------------------------------------------------------------------------

OSStatus DSpReportError (OSStatus error)
{
	switch (error)
	{
		case noErr:
			break;
		case kDSpNotInitializedErr:
			ReportError ("DSp Error: Not initialized");
			break;
		case kDSpSystemSWTooOldErr:
			ReportError ("DSp Error: system Software too old");
			break;
		case kDSpInvalidContextErr:
			ReportError ("DSp Error: Invalid context");
			break;
		case kDSpInvalidAttributesErr:
			ReportError ("DSp Error: Invalid attributes");
			break;
		case kDSpContextAlreadyReservedErr:
			ReportError ("DSp Error: Context already reserved");
			break;
		case kDSpContextNotReservedErr:
			ReportError ("DSp Error: Context not reserved");
			break;
		case kDSpContextNotFoundErr:
			ReportError ("DSp Error: Context not found");
			break;
		case kDSpFrameRateNotReadyErr:
			ReportError ("DSp Error: Frame rate not ready");
			break;
		case kDSpConfirmSwitchWarning:
//			ReportError ("DSp Warning: Must confirm switch"); // removed since it is just a warning, add back for debugging
			return 0; // don't want to fail on this warning
			break;
		case kDSpInternalErr:
			ReportError ("DSp Error: Internal error");
			break;
		case kDSpStereoContextErr:
			ReportError ("DSp Error: Stereo context");
			break;
	}
	return error;
}

//-----------------------------------------------------------------------------------------------------------------------

// if error dump agl errors to debugger string, return error

OSStatus aglReportError (void)
{
	GLenum err = aglGetError();
	if (AGL_NO_ERROR != err)
	{
		GlideError("AGL-Error: %s\n", reinterpret_cast<const char *>(aglErrorString(err)));
	}
	// ensure we are returning an OSStatus noErr if no error condition
	if (err == AGL_NO_ERROR)
		return noErr;
	else
		return (OSStatus) err;
}

//-----------------------------------------------------------------------------------------------------------------------

// if error dump gl errors to debugger string, return error
/*
OSStatus glReportError (void)
{
	GLenum err = glGetError();
	switch (err)
	{
		case GL_NO_ERROR:
			break;
		case GL_INVALID_ENUM:
			ReportError ("GL Error: Invalid enumeration");
			break;
		case GL_INVALID_VALUE:
			ReportError ("GL Error: Invalid value");
			break;
		case GL_INVALID_OPERATION:
			ReportError ("GL Error: Invalid operation");
			break;
		case GL_STACK_OVERFLOW:
			ReportError ("GL Error: Stack overflow");
			break;
		case GL_STACK_UNDERFLOW:
			ReportError ("GL Error: Stack underflow");
			break;
		case GL_OUT_OF_MEMORY:
			ReportError ("GL Error: Out of memory");
			break;
	}
	// ensure we are returning an OSStatus noErr if no error condition
	if (err == GL_NO_ERROR)
		return noErr;
	else
		return (OSStatus) err;
}
*/