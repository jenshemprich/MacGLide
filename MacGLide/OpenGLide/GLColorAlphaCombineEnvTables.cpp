//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*            Color and Alpha CombineEnv tables
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "GLColorAlphaCombineEnvTables.h"

// Additional internal combine functions for reduced terms
// Some of them are really senceless because they're clamped
// to {0.0, 1.0} anyway but they have been defined for complecity.
#define GR_COMBINE_FUNCTION_OTHER 0x11
#define GR_COMBINE_FUNCTION_MINUS_LOCAL 0x12
#define GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL 0x13
#define GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL_ALPHA 0x14
#define GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL 0x15
#define GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA 0x16
#define GR_COMBINE_FUNCTION_MINUS_LOCAL_ADD_LOCAL_ALPHA 0x17

// GL equivalents for Glide color combine functions
const CombineFunction ColorCombineFunctionsEnvCombineARB[25] =
{
	{{{CF_Replace  ,{CFARG_Constant  ,CFARG_None      ,CFARG_None  }},{CF_Substract  ,{CFARG_Previous  ,CFARG_Constant  ,CFARG_None}}}}, // 0x00 GR_COMBINE_FUNCTION_ZERO
	{{{CF_Replace  ,{CFARG_Local     ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x01 GR_COMBINE_FUNCTION_LOCAL
	{{{CF_Replace  ,{CFARG_LocalAlpha,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x02 GR_COMBINE_FUNCTION_LOCAL_ALPHA
	{{{CF_Modulate ,{CFARG_Factor    ,CFARG_Other     ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x03 GR_COMBINE_FUNCTION_SCALE_OTHER

	{{{CF_Modulate ,{CFARG_Factor    ,CFARG_Other     ,CFARG_None  }},{CF_Add        ,{CFARG_Previous  ,CFARG_Local     ,CFARG_None}}}}, // 0x04 GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL
	{{{CF_Modulate ,{CFARG_Factor    ,CFARG_Other     ,CFARG_None  }},{CF_Add        ,{CFARG_Previous  ,CFARG_LocalAlpha,CFARG_None}}}}, // 0x05 GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA
	{{{CF_Substract,{CFARG_Other     ,CFARG_Local     ,CFARG_None  }},{CF_Modulate   ,{CFARG_Previous  ,CFARG_Factor    ,CFARG_None}}}}, // 0x06 GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL
	{{{CF_Blend    ,{CFARG_Other     ,CFARG_Local     ,CFARG_Factor}},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x07 GR_COMBINE_FUNCTION_BLEND

	{{{CF_Substract,{CFARG_Other     ,CFARG_Local     ,CFARG_None  }},{CF_Modulate   ,{CFARG_Factor    ,CFARG_Previous  ,CFARG_None}}}}, // 0x08 GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA // not correct, needs CF_ModulateAdd
	{{{CF_Modulate ,{CFARG_Factor    ,CFARG_Local     ,CFARG_None  }},{CF_Substract  ,{CFARG_Local     ,CFARG_Previous  ,CFARG_None}}}}, // 0x09 GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL
	
	{{{CF_Replace  ,{CFARG_Constant  ,CFARG_None      ,CFARG_None  }},{CF_Substract  ,{CFARG_Previous  ,CFARG_Constant  ,CFARG_None}}}}, // 0x0a dummy
	{{{CF_Replace  ,{CFARG_Constant  ,CFARG_None      ,CFARG_None  }},{CF_Substract  ,{CFARG_Previous  ,CFARG_Constant  ,CFARG_None}}}}, // 0x0b dummy
	{{{CF_Replace  ,{CFARG_Constant  ,CFARG_None      ,CFARG_None  }},{CF_Substract  ,{CFARG_Previous  ,CFARG_Constant  ,CFARG_None}}}}, // 0x0c dummy
	{{{CF_Replace  ,{CFARG_Constant  ,CFARG_None      ,CFARG_None  }},{CF_Substract  ,{CFARG_Previous  ,CFARG_Constant  ,CFARG_None}}}}, // 0x0d dummy
	{{{CF_Replace  ,{CFARG_Constant  ,CFARG_None      ,CFARG_None  }},{CF_Substract  ,{CFARG_Previous  ,CFARG_Constant  ,CFARG_None}}}}, // 0x0e dummy
	{{{CF_Replace  ,{CFARG_Constant  ,CFARG_None      ,CFARG_None  }},{CF_Substract  ,{CFARG_Previous  ,CFARG_Constant  ,CFARG_None}}}}, // 0x0f dummy
	
	{{{CF_Modulate ,{CFARG_Factor    ,CFARG_Local     ,CFARG_None  }},{CF_Substract  ,{CFARG_LocalAlpha,CFARG_Previous  ,CFARG_None}}}}, // 0x10 GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA

	// Additional functions for reducing the above terms if factor == GR_COMBINE_FACTOR_ZERO
	
	// Additional functions for reducing the above terms if factor == GR_COMBINE_FACTOR_ONE
	{{{CF_Replace  ,{CFARG_Other     ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x011 GR_COMBINE_FUNCTION_OTHER
	{{{CF_Substract,{CFARG_Constant  ,CFARG_Constant  ,CFARG_None  }},{CF_Substract  ,{CFARG_Previous  ,CFARG_Local     ,CFARG_None}}}}, // 0x12 GR_COMBINE_FUNCTION_MINUS_LOCAL // clamped to 0.0
	{{{CF_Add      ,{CFARG_Other     ,CFARG_Local     ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x13 GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL
	{{{CF_Add      ,{CFARG_Other     ,CFARG_LocalAlpha,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x14 GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL_ALPHA
	{{{CF_Substract,{CFARG_Other     ,CFARG_Local     ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x15 GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL
	{{{CF_Substract,{CFARG_Other     ,CFARG_Local     ,CFARG_None  }},{CF_Add        ,{CFARG_Previous  ,CFARG_LocalAlpha,CFARG_None}}}}, // 0x16 GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA
	{{{CF_Substract,{CFARG_LocalAlpha,CFARG_Local     ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}  // 0x17 GR_COMBINE_FUNCTION_MINUS_LOCAL_ADD_LOCAL_ALPHA
};

const CombineFunction ColorCombineFunctionsEnvCombine3ATI[25] =
{
	{{{CF_Replace    ,{CFARG_Zero      ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x00 GR_COMBINE_FUNCTION_ZERO
	{{{CF_Replace    ,{CFARG_Local     ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x01 GR_COMBINE_FUNCTION_LOCAL
	{{{CF_Replace    ,{CFARG_LocalAlpha,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x02 GR_COMBINE_FUNCTION_LOCAL_ALPHA
	{{{CF_Modulate   ,{CFARG_Factor    ,CFARG_Other     ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x03 GR_COMBINE_FUNCTION_SCALE_OTHER

	{{{CF_ModulateAdd,{CFARG_Factor    ,CFARG_Local     ,CFARG_Other }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x04 GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL
	{{{CF_ModulateAdd,{CFARG_Factor    ,CFARG_LocalAlpha,CFARG_Other }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x05 GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA
	{{{CF_Substract  ,{CFARG_Other     ,CFARG_Local     ,CFARG_None  }},{CF_Modulate   ,{CFARG_Previous  ,CFARG_Factor    ,CFARG_None}}}}, // 0x06 GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL
//	{{{CF_Blend      ,{CFARG_Other     ,CFARG_Local     ,CFARG_Factor}},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x07 GR_COMBINE_FUNCTION_BLEND // psychedelic colors on G5 with ATI9600XT
	{{{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None  }},{CF_Blend      ,{CFARG_Other     ,CFARG_Local     ,CFARG_Factor}}}}, // 0x07 GR_COMBINE_FUNCTION_BLEND // works (don't ask me why)

	{{{CF_Substract  ,{CFARG_Other     ,CFARG_Local     ,CFARG_None  }},{CF_ModulateAdd,{CFARG_Factor    ,CFARG_LocalAlpha,CFARG_Previous}}}}, // 0x08 GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA
	{{{CF_Modulate   ,{CFARG_Factor    ,CFARG_Local     ,CFARG_None  }},{CF_Substract  ,{CFARG_Local     ,CFARG_Previous  ,CFARG_None}}}}, // 0x09 GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL
	
	{{{CF_Replace    ,{CFARG_Zero      ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x0a dummy
	{{{CF_Replace    ,{CFARG_Zero      ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x0b dummy
	{{{CF_Replace    ,{CFARG_Zero      ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x0c dummy
	{{{CF_Replace    ,{CFARG_Zero      ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x0d dummy
	{{{CF_Replace    ,{CFARG_Zero      ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x0e dummy
	{{{CF_Replace    ,{CFARG_Zero      ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x0f dummy
	
	{{{CF_Modulate   ,{CFARG_Factor    ,CFARG_Local     ,CFARG_None  }},{CF_Substract  ,{CFARG_LocalAlpha,CFARG_Previous  ,CFARG_None}}}}, // 0x10 GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA

	// Additional functions for reducing the above terms if factor == GR_COMBINE_FACTOR_ZERO
	
	// Additional functions for reducing the above terms if factor == GR_COMBINE_FACTOR_ONE
	{{{CF_Replace    ,{CFARG_Other     ,CFARG_None      ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x011 GR_COMBINE_FUNCTION_OTHER
	{{{CF_Substract  ,{CFARG_Zero      ,CFARG_Local     ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x12 GR_COMBINE_FUNCTION_MINUS_LOCAL // clamped to 0.0
	{{{CF_Add        ,{CFARG_Other     ,CFARG_Local     ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x13 GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL
	{{{CF_Add        ,{CFARG_Other     ,CFARG_LocalAlpha,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x14 GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL_ALPHA
	{{{CF_Substract  ,{CFARG_Other     ,CFARG_Local     ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}, // 0x15 GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL
	{{{CF_Substract  ,{CFARG_Other     ,CFARG_Local     ,CFARG_None  }},{CF_Add        ,{CFARG_Previous  ,CFARG_LocalAlpha,CFARG_None}}}}, // 0x16 GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA
	{{{CF_Substract  ,{CFARG_LocalAlpha,CFARG_Local     ,CFARG_None  }},{CF_Unused     ,{CFARG_None      ,CFARG_None      ,CFARG_None}}}}  // 0x17 GR_COMBINE_FUNCTION_MINUS_LOCAL_ADD_LOCAL_ALPHA
};

// Mapping for reducing combine functions if factor == GR_COMBINE_FACTOR_ZERO
const CombineReduceTerm	ColorCombineFunctionsFactorZero[0x11] =
{
	/* GR_COMBINE_FUNCTION_ZERO */                                    {GR_COMBINE_FUNCTION_ZERO},
	/* GR_COMBINE_FUNCTION_LOCAL */                                   {GR_COMBINE_FUNCTION_LOCAL},
	/* GR_COMBINE_FUNCTION_LOCAL_ALPHA */                             {GR_COMBINE_FUNCTION_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER */                             {GR_COMBINE_FUNCTION_ZERO},

	/* GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL */                   {GR_COMBINE_FUNCTION_LOCAL},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA */             {GR_COMBINE_FUNCTION_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL */                 {GR_COMBINE_FUNCTION_ZERO},
	/* GR_COMBINE_FUNCTION_BLEND */                                   {GR_COMBINE_FUNCTION_LOCAL},

	/* GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA */ {GR_COMBINE_FUNCTION_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL */             {GR_COMBINE_FUNCTION_LOCAL},

	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},

	/* GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA */       {GR_COMBINE_FUNCTION_LOCAL_ALPHA}
};

// Mapping for reducing combine functions if factor == GR_COMBINE_FACTOR_ONE
const CombineReduceTerm ColorCombineFunctionsFactorOne[0x11] =
{
	/* GR_COMBINE_FUNCTION_ZERO */                                    {GR_COMBINE_FUNCTION_ZERO},
	/* GR_COMBINE_FUNCTION_LOCAL */                                   {GR_COMBINE_FUNCTION_LOCAL},
	/* GR_COMBINE_FUNCTION_LOCAL_ALPHA */                             {GR_COMBINE_FUNCTION_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER */                             {GR_COMBINE_FUNCTION_OTHER},

	/* GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL */                   {GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA */             {GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL */                 {GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL},
	/* GR_COMBINE_FUNCTION_BLEND */                                   {GR_COMBINE_FUNCTION_OTHER},

	/* GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA */ {GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL */             {GR_COMBINE_FUNCTION_ZERO},

	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},

	/* GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA */       {GR_COMBINE_FUNCTION_MINUS_LOCAL_ADD_LOCAL_ALPHA}
};

const CombineArgument ColorCombineFactors[14] =
{
	{CFARG_Constant,GL_SRC_COLOR},          // 0x00 GR_COMBINE_FACTOR_ZERO
	{CFARG_Local,0},                        // 0x01 GR_COMBINE_FACTOR_LOCAL
	{CFARG_OtherAlpha,0},                   // 0x02 GR_COMBINE_FACTOR_OTHER_ALPHA
	{CFARG_LocalAlpha,0},                   // 0x03 GR_COMBINE_FACTOR_LOCAL_ALPHA
	{CFARG_Texture,GL_SRC_ALPHA},           // 0x04 GR_COMBINE_FACTOR_TEXTURE_ALPHA GR_COMBINE_FACTOR_DETAIL_FACTOR
	{CFARG_Texture,GL_SRC_COLOR},           // 0x05 GR_COMBINE_FACTOR_TEXTURE_RGB GR_COMBINE_FACTOR_LOD_FRACTION (unused)
	{CFARG_Local,0},                        // 0x06 placeholder
	{CFARG_Local,0},                        // 0x07 placeholder
	{CFARG_Constant,GL_SRC_COLOR},          // 0x08 GR_COMBINE_FACTOR_ONE
	{CFARG_Local,0},                        // 0x09 GR_COMBINE_FACTOR_ONE_MINUS_LOCAL
	{CFARG_OtherAlpha,0},                   // 0x0a GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA
	{CFARG_LocalAlpha,0},                   // 0x0b GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA
	{CFARG_Texture,GL_ONE_MINUS_SRC_ALPHA}, // 0x0c GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR
	{CFARG_Texture,GL_ONE_MINUS_SRC_ALPHA}  // 0x0d GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION (unused)
};

const CombineArgument ColorCombineFactorsInverted[14] =
{
	{CFARG_Constant,GL_SRC_COLOR},          // 0x00 GR_COMBINE_FACTOR_ZERO
	{CFARG_Local,0},                        // 0x01 GR_COMBINE_FACTOR_LOCAL
	{CFARG_OtherAlpha,0},                   // 0x02 GR_COMBINE_FACTOR_OTHER_ALPHA
	{CFARG_LocalAlpha,0},                   // 0x03 GR_COMBINE_FACTOR_LOCAL_ALPHA
	{CFARG_Texture,GL_ONE_MINUS_SRC_ALPHA}, // 0x04 GR_COMBINE_FACTOR_TEXTURE_ALPHA GR_COMBINE_FACTOR_DETAIL_FACTOR
	{CFARG_Texture,GL_ONE_MINUS_SRC_COLOR}, // 0x05 GR_COMBINE_FACTOR_TEXTURE_RGB GR_COMBINE_FACTOR_LOD_FRACTION (unused)
	{CFARG_Local,0},                        // 0x06 placeholder
	{CFARG_Local,0},                        // 0x07 placeholder
	{CFARG_Constant,GL_SRC_COLOR},          // 0x08 GR_COMBINE_FACTOR_ONE
	{CFARG_Local,0},                        // 0x09 GR_COMBINE_FACTOR_ONE_MINUS_LOCAL
	{CFARG_OtherAlpha,0},                   // 0x0a GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA
	{CFARG_LocalAlpha,0},                   // 0x0b GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA
	{CFARG_Texture,GL_SRC_ALPHA},           // 0x0c GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR
	{CFARG_Texture,GL_SRC_ALPHA}            // 0x0d GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION (unused)
};

const CombineArgument ColorCombineLocals[4] =
{
	{GL_PRIMARY_COLOR_EXT,GL_SRC_COLOR}, // 0x00 GR_COMBINE_LOCAL_ITERATED
	{GL_CONSTANT_EXT,GL_SRC_COLOR},      // 0x01 GR_COMBINE_LOCAL_CONSTANT GR_COMBINE_LOCAL_NONE
	{GL_CONSTANT_EXT,GL_SRC_COLOR},      // 0x02 unused
	{GL_TEXTURE,GL_SRC_COLOR}            // 0x03 GR_COMBINE_LOCAL_PIXELPIPELINE - needed to support the framebuffer
};

const CombineArgument ColorCombineOthers[4] =
{
	{GL_PRIMARY_COLOR_EXT,GL_SRC_COLOR}, // 0x00 GR_COMBINE_OTHER_ITERATED
	{GL_TEXTURE,GL_SRC_COLOR},           // 0x01 GR_COMBINE_OTHER_TEXTURE
	{GL_CONSTANT_EXT,GL_SRC_COLOR},      // 0x02 GR_COMBINE_OTHER_CONSTANT GR_COMBINE_OTHER_NONE
	{GL_TEXTURE,GL_SRC_COLOR}            // 0x03 GR_COMBINE_OTHER_PIXELPIPELINE - needed to support the framebuffer
};

const CombineArgument ColorCombineOthersInverted[4] =
{
	{GL_PRIMARY_COLOR_EXT,GL_SRC_COLOR}, // 0x00 GR_COMBINE_OTHER_ITERATED
	{GL_TEXTURE,GL_ONE_MINUS_SRC_COLOR}, // 0x01 GR_COMBINE_OTHER_TEXTURE
	{GL_CONSTANT_EXT,GL_SRC_COLOR},      // 0x02 GR_COMBINE_OTHER_CONSTANT GR_COMBINE_OTHER_NONE
	{GL_TEXTURE,GL_SRC_COLOR}            // 0x03 GR_COMBINE_OTHER_PIXELPIPELINE - needed to support the framebuffer
};

// GL equivalents for Glide alpha combine functions
const CombineFunction AlphaCombineFunctionsEnvCombineARB[25] =
{
	{{{CF_Replace  ,{CFARG_Constant   ,CFARG_None      ,CFARG_None       }},{CF_Substract  ,{CFARG_Previous   ,CFARG_Constant   ,CFARG_None}}}}, // 0x00 GR_COMBINE_FUNCTION_ZERO
	{{{CF_Replace  ,{CFARG_LocalAlpha ,CFARG_None      ,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x01 GR_COMBINE_FUNCTION_LOCAL
	{{{CF_Replace  ,{CFARG_LocalAlpha ,CFARG_None      ,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x02 GR_COMBINE_FUNCTION_LOCAL_ALPHA
	{{{CF_Modulate ,{CFARG_Factor     ,CFARG_OtherAlpha,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x03 GR_COMBINE_FUNCTION_SCALE_OTHER

	{{{CF_Modulate ,{CFARG_Factor     ,CFARG_OtherAlpha,CFARG_None       }},{CF_Add        ,{CFARG_Previous   ,CFARG_LocalAlpha ,CFARG_None}}}}, // 0x04 GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL
	{{{CF_Modulate ,{CFARG_Factor     ,CFARG_OtherAlpha,CFARG_None       }},{CF_Add        ,{CFARG_Previous   ,CFARG_LocalAlpha ,CFARG_None}}}}, // 0x05 GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA
	{{{CF_Substract,{CFARG_OtherAlpha ,CFARG_LocalAlpha,CFARG_None       }},{CF_Modulate   ,{CFARG_Previous   ,CFARG_Factor     ,CFARG_None}}}}, // 0x06 GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL
	{{{CF_Blend    ,{CFARG_OtherAlpha ,CFARG_LocalAlpha,CFARG_Factor     }},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x07 GR_COMBINE_FUNCTION_BLEND

	{{{CF_Substract,{CFARG_OtherAlpha ,CFARG_LocalAlpha,CFARG_None       }},{CF_Modulate   ,{CFARG_Factor     ,CFARG_Previous   ,CFARG_None}}}}, // 0x08 GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA // not correct, needs CF_ModulateAdd
	{{{CF_Modulate ,{CFARG_Factor     ,CFARG_LocalAlpha,CFARG_None       }},{CF_Substract  ,{CFARG_LocalAlpha ,CFARG_Previous   ,CFARG_None}}}}, // 0x09 GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL

	{{{CF_Replace  ,{CFARG_Constant   ,CFARG_None      ,CFARG_None       }},{CF_Substract  ,{CFARG_Previous   ,CFARG_Constant   ,CFARG_None}}}}, // 0x0a dummy
	{{{CF_Replace  ,{CFARG_Constant   ,CFARG_None      ,CFARG_None       }},{CF_Substract  ,{CFARG_Previous   ,CFARG_Constant   ,CFARG_None}}}}, // 0x0b dummy
	{{{CF_Replace  ,{CFARG_Constant   ,CFARG_None      ,CFARG_None       }},{CF_Substract  ,{CFARG_Previous   ,CFARG_Constant   ,CFARG_None}}}}, // 0x0c dummy
	{{{CF_Replace  ,{CFARG_Constant   ,CFARG_None      ,CFARG_None       }},{CF_Substract  ,{CFARG_Previous   ,CFARG_Constant   ,CFARG_None}}}}, // 0x0d dummy
	{{{CF_Replace  ,{CFARG_Constant   ,CFARG_None      ,CFARG_None       }},{CF_Substract  ,{CFARG_Previous   ,CFARG_Constant   ,CFARG_None}}}}, // 0x0e dummy
	{{{CF_Replace  ,{CFARG_Constant   ,CFARG_None      ,CFARG_None       }},{CF_Substract  ,{CFARG_Previous   ,CFARG_Constant   ,CFARG_None}}}}, // 0x0f dummy

	{{{CF_Modulate ,{CFARG_Factor     ,CFARG_LocalAlpha,CFARG_None       }},{CF_Substract  ,{CFARG_LocalAlpha ,CFARG_Previous   ,CFARG_None}}}},  // 0x10 GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA

	// Additional functions for reducing the above terms if factor == GR_COMBINE_FACTOR_ZERO

	// Additional functions for reducing the above terms if factor == GR_COMBINE_FACTOR_ONE
	{{{CF_Replace  ,{CFARG_OtherAlpha ,CFARG_None      ,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}, // 0x011 GR_COMBINE_FUNCTION_OTHER
	{{{CF_Substract,{CFARG_Constant   ,CFARG_Constant  ,CFARG_None       }},{CF_Substract  ,{CFARG_Previous   ,CFARG_LocalAlpha,CFARG_None}}}}, // 0x12 GR_COMBINE_FUNCTION_MINUS_LOCAL
	{{{CF_Add      ,{CFARG_OtherAlpha ,CFARG_LocalAlpha,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}, // 0x13 GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL
	{{{CF_Add      ,{CFARG_OtherAlpha ,CFARG_OtherAlpha,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}, // 0x14 GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL_ALPHA
	{{{CF_Substract,{CFARG_OtherAlpha ,CFARG_LocalAlpha,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}, // 0x15 GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL
	{{{CF_Replace  ,{CFARG_OtherAlpha ,CFARG_None      ,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}, // 0x16 GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA
	{{{CF_Substract,{CFARG_LocalAlpha ,CFARG_LocalAlpha,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}  // 0x17 GR_COMBINE_FUNCTION_MINUS_LOCAL_ADD_LOCAL_ALPHA
};

// GL equivalents for Glide alpha combine functions
const CombineFunction AlphaCombineFunctionsEnvCombine3ATI[25] =
{
	{{{CF_Replace    ,{CFARG_Zero      ,CFARG_None      ,CFARG_None      }},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x00 GR_COMBINE_FUNCTION_ZERO
	{{{CF_Replace    ,{CFARG_LocalAlpha,CFARG_None      ,CFARG_None      }},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x01 GR_COMBINE_FUNCTION_LOCAL
	{{{CF_Replace    ,{CFARG_LocalAlpha,CFARG_None      ,CFARG_None      }},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x02 GR_COMBINE_FUNCTION_LOCAL_ALPHA
	{{{CF_Modulate   ,{CFARG_Factor    ,CFARG_OtherAlpha,CFARG_None      }},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x03 GR_COMBINE_FUNCTION_SCALE_OTHER

	{{{CF_ModulateAdd,{CFARG_Factor    ,CFARG_LocalAlpha,CFARG_OtherAlpha}},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x04 GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL
	{{{CF_ModulateAdd,{CFARG_Factor    ,CFARG_LocalAlpha,CFARG_OtherAlpha}},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x05 GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA
	{{{CF_Substract  ,{CFARG_OtherAlpha,CFARG_LocalAlpha,CFARG_None      }},{CF_Modulate   ,{CFARG_Previous   ,CFARG_Factor     ,CFARG_None}}}}, // 0x06 GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL
//	{{{CF_Blend      ,{CFARG_OtherAlpha,CFARG_LocalAlpha,CFARG_Factor    }},{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x07 GR_COMBINE_FUNCTION_BLEND // psychedelic colors on G5 with ATI9600XT
	{{{CF_Unused     ,{CFARG_None       ,CFARG_None       ,CFARG_None    }},{CF_Blend      ,{CFARG_OtherAlpha ,CFARG_LocalAlpha ,CFARG_Factor    }}}}, // 0x07 GR_COMBINE_FUNCTION_BLEND //  works (don't ask me why)

	{{{CF_Substract  ,{CFARG_OtherAlpha ,CFARG_LocalAlpha,CFARG_None       }},{CF_ModulateAdd,{CFARG_Factor     ,CFARG_LocalAlpha ,CFARG_Previous}}}}, // 0x08 GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA
	{{{CF_Modulate   ,{CFARG_Factor     ,CFARG_LocalAlpha,CFARG_None       }},{CF_Substract,{CFARG_LocalAlpha ,CFARG_Previous   ,CFARG_None}}}}, // 0x09 GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL

	{{{CF_Replace    ,{CFARG_Zero       ,CFARG_None      ,CFARG_None       }},{CF_Unused   ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x0a dummy
	{{{CF_Replace    ,{CFARG_Zero       ,CFARG_None      ,CFARG_None       }},{CF_Unused   ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x0b dummy
	{{{CF_Replace    ,{CFARG_Zero       ,CFARG_None      ,CFARG_None       }},{CF_Unused   ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x0c dummy
	{{{CF_Replace    ,{CFARG_Zero       ,CFARG_None      ,CFARG_None       }},{CF_Unused   ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x0d dummy
	{{{CF_Replace    ,{CFARG_Zero       ,CFARG_None      ,CFARG_None       }},{CF_Unused   ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x0e dummy
	{{{CF_Replace    ,{CFARG_Zero       ,CFARG_None      ,CFARG_None       }},{CF_Unused   ,{CFARG_None       ,CFARG_None       ,CFARG_None}}}}, // 0x0f dummy

	{{{CF_Modulate ,{CFARG_Factor     ,CFARG_LocalAlpha,CFARG_None       }},{CF_Substract  ,{CFARG_LocalAlpha ,CFARG_Previous   ,CFARG_None}}}},  // 0x10 GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA

	// Additional functions for reducing the above terms if factor == GR_COMBINE_FACTOR_ZERO

	// Additional functions for reducing the above terms if factor == GR_COMBINE_FACTOR_ONE
	{{{CF_Replace  ,{CFARG_OtherAlpha ,CFARG_None      ,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}, // 0x011 GR_COMBINE_FUNCTION_OTHER
	{{{CF_Substract,{CFARG_Zero       ,CFARG_LocalAlpha,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}, // 0x12 GR_COMBINE_FUNCTION_MINUS_LOCAL
	{{{CF_Add      ,{CFARG_OtherAlpha ,CFARG_LocalAlpha,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}, // 0x13 GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL
	{{{CF_Add      ,{CFARG_OtherAlpha ,CFARG_OtherAlpha,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}, // 0x14 GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL_ALPHA
	{{{CF_Substract,{CFARG_OtherAlpha ,CFARG_LocalAlpha,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}, // 0x15 GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL
	{{{CF_Substract,{CFARG_OtherAlpha ,CFARG_LocalAlpha,CFARG_None       }},{CF_Add        ,{CFARG_Previous   ,CFARG_LocalAlpha,CFARG_None}}}}, // 0x16 GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA
	{{{CF_Replace  ,{CFARG_Zero       ,CFARG_None      ,CFARG_None       }},{CF_Unused     ,{CFARG_None       ,CFARG_None      ,CFARG_None}}}}  // 0x17 GR_COMBINE_FUNCTION_MINUS_LOCAL_ADD_LOCAL_ALPHA
};

// Mapping for reducing combine functions if factor == GR_COMBINE_FACTOR_ZERO
const CombineReduceTerm	AlphaCombineFunctionsFactorZero[0x11] =
{
	/* GR_COMBINE_FUNCTION_ZERO */                                    {GR_COMBINE_FUNCTION_ZERO},
	/* GR_COMBINE_FUNCTION_LOCAL */                                   {GR_COMBINE_FUNCTION_LOCAL},
	/* GR_COMBINE_FUNCTION_LOCAL_ALPHA */                             {GR_COMBINE_FUNCTION_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER */                             {GR_COMBINE_FUNCTION_ZERO},

	/* GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL */                   {GR_COMBINE_FUNCTION_LOCAL},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA */             {GR_COMBINE_FUNCTION_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL */                 {GR_COMBINE_FUNCTION_ZERO},
	/* GR_COMBINE_FUNCTION_BLEND */                                   {GR_COMBINE_FUNCTION_LOCAL},

	/* GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA */ {GR_COMBINE_FUNCTION_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL */             {GR_COMBINE_FUNCTION_LOCAL},

	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},

	/* GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA */       {GR_COMBINE_FUNCTION_LOCAL_ALPHA}
};

// Mapping for reducing combine functions if factor == GR_COMBINE_FACTOR_ONE
const CombineReduceTerm AlphaCombineFunctionsFactorOne[0x11] =
{
	/* GR_COMBINE_FUNCTION_ZERO */                                    {GR_COMBINE_FUNCTION_ZERO},
	/* GR_COMBINE_FUNCTION_LOCAL */                                   {GR_COMBINE_FUNCTION_LOCAL},
	/* GR_COMBINE_FUNCTION_LOCAL_ALPHA */                             {GR_COMBINE_FUNCTION_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER */                             {GR_COMBINE_FUNCTION_OTHER},

	/* GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL */                   {GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA */             {GR_COMBINE_FUNCTION_OTHER_ADD_LOCAL_ALPHA},
	/* GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL */                 {GR_COMBINE_FUNCTION_OTHER_MINUS_LOCAL},
	/* GR_COMBINE_FUNCTION_BLEND */                                   {GR_COMBINE_FUNCTION_OTHER},

	/* GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA */ {GR_COMBINE_FUNCTION_OTHER},
	/* GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL */             {GR_COMBINE_FUNCTION_ZERO},

	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},
	/* dummy */ {GR_COMBINE_FUNCTION_ZERO},

	/* GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA */       {GR_COMBINE_FUNCTION_ZERO}
};

const CombineArgument AlphaCombineFactors[14] =
{
	{CFARG_Constant,GL_SRC_ALPHA},          // 0x00 GR_COMBINE_FACTOR_ZERO
	{CFARG_LocalAlpha,0},                   // 0x01 GR_COMBINE_FACTOR_LOCAL
	{CFARG_OtherAlpha,0},                   // 0x02 GR_COMBINE_FACTOR_OTHER_ALPHA
	{CFARG_LocalAlpha,0},                   // 0x03 GR_COMBINE_FACTOR_LOCAL_ALPHA
	{CFARG_Texture,GL_SRC_ALPHA},           // 0x04 GR_COMBINE_FACTOR_TEXTURE_ALPHA GR_COMBINE_FACTOR_DETAIL_FACTOR
	{CFARG_Texture,GL_SRC_COLOR},           // 0x05 GR_COMBINE_FACTOR_TEXTURE_RGB GR_COMBINE_FACTOR_LOD_FRACTION (unused)
	{CFARG_LocalAlpha,0},                   // 0x06 placeholder
	{CFARG_LocalAlpha,0},                   // 0x07 placeholder
	{CFARG_Constant,GL_SRC_ALPHA},          // 0x08 GR_COMBINE_FACTOR_ONE
	{CFARG_LocalAlpha,0},                   // 0x09 GR_COMBINE_FACTOR_ONE_MINUS_LOCAL
	{CFARG_OtherAlpha,0},                   // 0x0a GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA
	{CFARG_LocalAlpha,0},                   // 0x0b GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA
	{CFARG_Texture,GL_ONE_MINUS_SRC_ALPHA}, // 0x0c GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR
	{CFARG_Texture,GL_ONE_MINUS_SRC_ALPHA}  // 0x0d GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION (unused)
};

const CombineArgument AlphaCombineFactorsInverted[14] =
{
	{CFARG_Constant,GL_SRC_ALPHA},          // 0x00 GR_COMBINE_FACTOR_ZERO
	{CFARG_LocalAlpha,0},                   // 0x01 GR_COMBINE_FACTOR_LOCAL
	{CFARG_OtherAlpha,0},                   // 0x02 GR_COMBINE_FACTOR_OTHER_ALPHA
	{CFARG_LocalAlpha,0},                   // 0x03 GR_COMBINE_FACTOR_LOCAL_ALPHA
	{CFARG_Texture,GL_ONE_MINUS_SRC_ALPHA}, // 0x04 GR_COMBINE_FACTOR_TEXTURE_ALPHA GR_COMBINE_FACTOR_DETAIL_FACTOR
	{CFARG_Texture,GL_ONE_MINUS_SRC_COLOR}, // 0x05 GR_COMBINE_FACTOR_TEXTURE_RGB GR_COMBINE_FACTOR_LOD_FRACTION (unused)
	{CFARG_LocalAlpha,0},                   // 0x06 placeholder
	{CFARG_LocalAlpha,0},                   // 0x07 placeholder
	{CFARG_Constant,GL_SRC_ALPHA},          // 0x08 GR_COMBINE_FACTOR_ONE
	{CFARG_LocalAlpha,0},                   // 0x09 GR_COMBINE_FACTOR_ONE_MINUS_LOCAL
	{CFARG_OtherAlpha,0},                   // 0x0a GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA
	{CFARG_LocalAlpha,0},                   // 0x0b GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA
	{CFARG_Texture,GL_SRC_ALPHA},           // 0x0c GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR
	{CFARG_Texture,GL_SRC_ALPHA}            // 0x0d GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION (unused)
};

const CombineArgument AlphaCombineLocals[4] =
{
	{GL_PRIMARY_COLOR_EXT,GL_SRC_ALPHA}, // 0x00 GR_COMBINE_LOCAL_ITERATED
	{GL_CONSTANT_EXT,GL_SRC_ALPHA},      // 0x01 GR_COMBINE_LOCAL_CONSTANT GR_COMBINE_LOCAL_NONE
	{GL_PRIMARY_COLOR_EXT,GL_SRC_ALPHA}, // 0x02 GR_COMBINE_LOCAL_DEPTH
	{GL_TEXTURE,GL_SRC_ALPHA}            // 0x03 GR_COMBINE_LOCAL_PIXELPIPELINE - needed to support the framebuffer
};

const CombineArgument AlphaCombineOthers[4] =
{
	{GL_PRIMARY_COLOR_EXT,GL_SRC_ALPHA}, // 0x00 GR_COMBINE_OTHER_ITERATED
	{GL_TEXTURE,GL_SRC_ALPHA},           // 0x01 GR_COMBINE_OTHER_TEXTURE
	{GL_CONSTANT_EXT,GL_SRC_ALPHA},      // 0x02 GR_COMBINE_OTHER_CONSTANT GR_COMBINE_OTHER_NONE
	{GL_TEXTURE,GL_SRC_ALPHA}            // 0x03 GR_COMBINE_OTHER_PIXELPIPELINE - needed to support the framebuffer
};

const CombineArgument AlphaCombineOthersInverted[4] =
{
	{GL_PRIMARY_COLOR_EXT,GL_SRC_ALPHA}, // 0x00 GR_COMBINE_OTHER_ITERATED
	{GL_TEXTURE,GL_ONE_MINUS_SRC_ALPHA}, // 0x01 GR_COMBINE_OTHER_TEXTURE
	{GL_CONSTANT_EXT,GL_SRC_ALPHA},      // 0x02 GR_COMBINE_OTHER_CONSTANT GR_COMBINE_OTHER_NONE
	{GL_TEXTURE,GL_SRC_ALPHA}            // 0x03 GR_COMBINE_OTHER_PIXELPIPELINE - needed to support the framebuffer
};
