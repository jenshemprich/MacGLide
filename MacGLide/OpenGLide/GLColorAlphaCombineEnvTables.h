//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*            Color and Alpha CombineEnv tables
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#pragma once

enum CombineFunctionValue
{
	CF_Unused = 0,
	CF_Replace = GL_REPLACE,
	CF_Modulate = GL_MODULATE,
	CF_Add = GL_ADD,
	CF_Substract = GL_SUBTRACT_ARB,
	CF_Blend = GL_INTERPOLATE_EXT,
	CF_ModulateAdd = GL_MODULATE_ADD_ATI
};

struct CombineArgument
{
	GLint Source;
	GLint Operand;
};

enum CombineFunctionColorAlphaArg
{
	CFARG_Local = 0,
	CFARG_Other = 1,
	CFARG_LocalAlpha = 2,
	CFARG_OtherAlpha = 3,
	CFARG_Factor = 4,
	CFARG_None = 5,
	CFARG_Constant = GL_CONSTANT_EXT,
	CFARG_Previous = GL_PREVIOUS_EXT,
	CFARG_Texture = GL_TEXTURE,
	CFARG_PrimaryColor = GL_PRIMARY_COLOR_EXT,
	CFARG_Zero = GL_ZERO,
	CFARG_One = GL_ONE
};

struct CombineFunctionGLTextureUnit
{
	CombineFunctionValue Function;
	CombineFunctionColorAlphaArg CombineArg[3];
};
	
struct CombineFunction
{
	CombineFunctionGLTextureUnit ColorAlphaUnit[2];
};

struct CombineReduceTerm
{
	GrCombineFunction_t ReducedTerm;
};

extern const CombineFunction ColorCombineFunctionsEnvCombineARB[25];
extern const CombineFunction ColorCombineFunctionsEnvCombine3ATI[25];
extern const CombineReduceTerm ColorCombineFunctionsFactorZero[0x11];
extern const CombineReduceTerm ColorCombineFunctionsFactorOne[0x11];
extern const CombineFunction AlphaCombineFunctionsEnvCombineARB[25];
extern const CombineFunction AlphaCombineFunctionsEnvCombine3ATI[25];
extern const CombineReduceTerm AlphaCombineFunctionsFactorZero[0x11];
extern const CombineReduceTerm AlphaCombineFunctionsFactorOne[0x11];
extern const CombineArgument ColorCombineFactors[14];
extern const CombineArgument AlphaCombineFactors[14];
extern const CombineArgument ColorCombineFactorsInverted[14];
extern const CombineArgument AlphaCombineFactorsInverted[14];
extern const CombineArgument ColorCombineLocals[4];
extern const CombineArgument ColorCombineOthers[4];
extern const CombineArgument ColorCombineLocalsInverted[4];
extern const CombineArgument ColorCombineOthersInverted[4];
extern const CombineArgument AlphaCombineLocals[4];
extern const CombineArgument AlphaCombineOthers[4];
extern const CombineArgument AlphaCombineLocalsInverted[4];
extern const CombineArgument AlphaCombineOthersInverted[4];

#define GR_COMBINE_LOCAL_PIXELPIPELINE 0x03 
#define GR_COMBINE_OTHER_PIXELPIPELINE 0x03
