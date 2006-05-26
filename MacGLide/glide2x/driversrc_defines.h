#pragma once

#ifndef _DRIVERSRC_DEFINES_H_
#define _DRIVERSRC_DEFINES_H_

#include "sdk2_glide.h"

#define GLIDE_PLUG
// #define GLIDE_PLATFORM
// #define GLIDE_HW_SST1
// #define GLIDE_HW_SST96
#define GR_BEGIN_NOFIFOCHECK(x,y)
#define GDBG_INFO_MORE(x)
// #define GR_END()
#define GR_DIENTRY(f,r,p) r f p

// These defines are obsolete in the emulation
// (they can be commented out in the src)
// #define GR_SET_EXPECTED_SIZE(n,p)
// #define GR_CHECK_SIZE_SLOPPY()

#endif //_DRIVERSRC_DEFINES_H_
