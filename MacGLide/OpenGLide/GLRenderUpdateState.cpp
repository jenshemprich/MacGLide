//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*       update the GL state before rendering vertices
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "GlideSettings.h"
#include "GLColorAlphaCombineEnvTables.h"
#include "GLRender.h"
#include "GLRenderUpdateState.h"

struct
{
	GrChromakeyMode_t ChromaKeyMode;
} OldGlideState = 
{
	GR_CHROMAKEY_DISABLE,
};

bool s_bUpdateTextureState = false;
bool s_bUpdateFogModeState = false;
bool s_bUpdateFogColorState = false;
bool s_bUpdateBlendState = false;
bool s_bUpdateChromaKeyAndAlphaState = false;
bool s_bUpdateColorCombineState = false;
bool s_bUpdateAlphaCombineState = false;
bool s_bUpdateColorInvertState = false;
bool s_bUpdateAlphaInvertState = false;
bool s_bUpdateConstantColorValueState = false;
bool s_bUpdateConstantColorValue4State = false;

bool s_bForceChromaKeyAndAlphaStateUpdate = false;

/*
inline void SetTextureState_update()
{
	glReportErrors("SetTextureState_update");

	if (OpenGL.ColorAlphaUnit2 == 0)
	{
		if (OpenGL.Texture)
		{
			glEnable(GL_TEXTURE_2D);
			if (InternalConfig.EXT_compiled_vertex_array)
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			}
		}
		else
		{
			if (InternalConfig.EXT_compiled_vertex_array)
			{
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			glDisable(GL_TEXTURE_2D);
		}
		glReportError();
	}
	else
	{
		// If no texture is used, TMU0 provides 0
		for(long unit_index = 1; unit_index >= 0; unit_index--)
		{
			glActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
			glReportError();
			if (OpenGL.Texture == false)
			{
				glBindTexture(GL_TEXTURE_2D, OpenGL.DummyTextureName);
				glReportError();
			}
			if (OpenGL.ColorAlphaUnitColorEnabledState[unit_index] || OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index])
			{
				glEnable(GL_TEXTURE_2D);
				if (InternalConfig.EXT_compiled_vertex_array)
				{
					glClientActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
					glReportError();
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		 		}
				glReportError();
			}
			else
			{
				if (InternalConfig.EXT_compiled_vertex_array)
				{
					glClientActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
					glReportError();
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}
				glDisable(GL_TEXTURE_2D);
				glReportError();
			}
		}
#ifdef OPENGL_DEBUG
		for(long unit_index = 0; unit_index < 2; unit_index++)
		{
			GlideMsg( "OpenGL.ColorAlphaUnitColorEnabledState[%d] = %d\n", unit_index, OpenGL.ColorAlphaUnitColorEnabledState[unit_index]);
			GlideMsg( "OpenGL.ColorAlphaUnitAlphaEnabledState[%d] = %d\n", unit_index, OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index]);
			GlideMsg("Texture unit GL_TEXTURE%d_ARB ", unit_index, GL_TEXTURE0_ARB + unit_index);
			if (OpenGL.ColorAlphaUnitColorEnabledState[unit_index] || OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index])
			{
				GlideMsg("enabled\n");
			}
			else
			{
				GlideMsg("disabled\n");
			}
		}
#endif
	}
}
*/

/*
// Setup color combine for extended color alpha model
inline void SetColorCombineState_update() 
{
	glReportErrors("SetColorCombine_update");
	
	GrCombineFunction_t function = Glide.State.ColorCombineFunction;
	GrCombineFactor_t factor = Glide.State.ColorCombineFactor;
	GrCombineLocal_t local = Glide.State.ColorCombineLocal;
	GrCombineOther_t other = Glide.State.ColorCombineOther;

	if (OpenGL.ColorAlphaUnit2 == 0)
	{
		glActiveTextureARB(OpenGL.ColorAlphaUnit1);
		if ( ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA ) &&
		     ( Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
		{
		    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
		}
		else
		if ( ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB ) &&
		     ( Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
		{
		    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
		}
		else
		if (InternalConfig.EXT_texture_env_add &&
		    ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_ONE ) &&
		    ( Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) &&
		    ( ( Glide.State.ColorCombineFunction == GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA ) || 
		      ( Glide.State.ColorCombineFunction == GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL ) ) )
		{
		    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );
		}
		else
		{
		    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		}
		glReportError();
	}
	else
	{
		// Reduce function for constant factors?
		if (factor == GR_COMBINE_FACTOR_ZERO)
		{
			function = ColorCombineFunctionsFactorZero[function].ReducedTerm;
		}
		else if (factor == GR_COMBINE_FACTOR_ONE)
		{
			function = ColorCombineFunctionsFactorOne[function].ReducedTerm;
		}
		const CombineArgument** combine_argument = &OpenGL.ColorCombineArguments[0];
		combine_argument[CFARG_Local] =  &ColorCombineLocals[local];
		combine_argument[CFARG_Other] =  Glide.State.TextureCombineRGBInvert ? &ColorCombineOthersInverted[other] : &ColorCombineOthers[other];
		combine_argument[CFARG_LocalAlpha] =  &AlphaCombineLocals[Glide.State.AlphaLocal];
		combine_argument[CFARG_OtherAlpha] =  Glide.State.TextureCombineAInvert ? &AlphaCombineOthersInverted[Glide.State.AlphaOther] : &AlphaCombineOthers[Glide.State.AlphaOther];
		combine_argument[CFARG_Factor] = Glide.State.TextureCombineRGBInvert ? &ColorCombineFactorsInverted[factor] : &ColorCombineFactors[factor];
		GLint source;
		GLint operand;
		CombineFunctionColorAlphaArg arg;
		for(long unit_index = 1; unit_index >= 0; unit_index--)
		{
			// unit 2 has to be the next one after unit 1
			glActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
			glReportError();
			const CombineFunctionGLTextureUnit& unit = OpenGL.ColorCombineFunctions[function].ColorAlphaUnit[unit_index];
			if (unit.Function == CF_Unused)
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
				glReportError();
				OpenGL.ColorAlphaUnitColorEnabledState[unit_index] = false;
			}
			else
			{
				OpenGL.ColorAlphaUnitColorEnabledState[unit_index] = true;
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, unit.Function);
				glReportError();
				for(unsigned long arg_index = 0; arg_index < 3; arg_index++)
				{
					arg = unit.CombineArg[arg_index];
					if (arg < CFARG_None)
					{
						if (arg == CFARG_Factor); // || arg == CFARG_FactorAlpha)
						{
							if (combine_argument[arg]->Source < CFARG_None)
							{
								// Resolve factor to local/other:
								// The factor source references to the local/other,
								// which can be retrieved via the combine_argument[] array
								source = combine_argument[combine_argument[arg]->Source]->Source;
								// but the operand of the factor specifies already the
								// correct component of the pixel (rgb or alpha) to be used.
								// Example: if factor == GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA
								// then the other alpha in AlphaCombineOthers is GL_SRC_ALPHA
								// but the factor operand is correctly GL_ONE_MINUS_SRC_ALPHA.
								operand = combine_argument[arg]->Operand;
								// operand = combine_argument[combine_argument[arg]->Source]->Operand; // No
							}
							else
							{
								source = combine_argument[arg]->Source;
								operand = combine_argument[arg]->Operand;
							}
						}
						else
						{
							// combinearg = local or other (alpha)
							source = combine_argument[arg]->Source;
							operand = combine_argument[arg]->Operand;
						}
					}
					else if (arg > CFARG_None)
					{
						source = unit.CombineArg[arg_index];
						operand = GL_SRC_COLOR;
					}
					else
					{
					// arg == CFARG_None -> argument not used
						continue;
					}
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT + arg_index, source);
					glReportError();
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT + arg_index, operand);
					glReportError();
				}
			}
		}
	}

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}
*/

/*
// Setup alpha combine for extended color alpha model
inline void SetAlphaCombineState_update() 
{
	glReportErrors("SetAlphaCombine_update");

	if (OpenGL.ColorAlphaUnit2 == 0)
	{
		glActiveTextureARB(OpenGL.ColorAlphaUnit1);
	}
	else
	{
		// Handle chromakey special case
		if (OpenGL.ChromaKey && OpenGL.Texture && !OpenGL.Blend)
		{
			// need to disable alpha combining until an additional
			// texture unit can be used to mask out the chroma key color.
			// Note that the second unit must be setup because the
			// texture unit might be turned on due to the color setup.
			glActiveTextureARB(OpenGL.ColorAlphaUnit1 + 1);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
			glActiveTextureARB(OpenGL.ColorAlphaUnit1);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
			glReportError();
			OpenGL.ColorAlphaUnitAlphaEnabledState[0] = true;
			OpenGL.ColorAlphaUnitAlphaEnabledState[1] = false;
		}
		else
		{
			GrCombineFunction_t function = Glide.State.AlphaFunction;
			GrCombineFactor_t factor = Glide.State.AlphaFactor;
			GrCombineLocal_t local = Glide.State.AlphaLocal;
			GrCombineOther_t other = Glide.State.AlphaOther;
			// Reduce function for constant factors?
			if (factor == GR_COMBINE_FACTOR_ZERO)
			{
				function = AlphaCombineFunctionsFactorZero[function].ReducedTerm;
			}
			else if (factor == GR_COMBINE_FACTOR_ONE)
			{
				function = AlphaCombineFunctionsFactorOne[function].ReducedTerm;
			}
			const CombineArgument** combine_argument = &OpenGL.AlphaCombineArguments[0];
			// @todo: colors are unused, aren't they?
			combine_argument[CFARG_Local] = &ColorCombineLocals[local];
			combine_argument[CFARG_Other] = Glide.State.TextureCombineAInvert ? &ColorCombineOthersInverted[other] : &ColorCombineOthers[other];
			combine_argument[CFARG_LocalAlpha] = &AlphaCombineLocals[local];
			combine_argument[CFARG_OtherAlpha] = Glide.State.TextureCombineAInvert ? &AlphaCombineOthersInverted[other] : &AlphaCombineOthers[other];
			combine_argument[CFARG_Factor] = Glide.State.TextureCombineAInvert ? &AlphaCombineFactorsInverted[factor] : &AlphaCombineFactors[factor];
			GLint source;
			GLint operand;
			CombineFunctionColorAlphaArg arg;
			for(long unit_index = 1; unit_index >= 0; unit_index--)
			{
				// unit 2 must be the next one after unit 1
				glActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
				glReportError();
				CombineFunctionGLTextureUnit unit = OpenGL.AlphaCombineFunctions[function].ColorAlphaUnit[unit_index];
				if (unit.Function == CF_Unused)
				{
					if (OpenGL.ChromaKey && OpenGL.Texture && OpenGL.Blend)
					{
						// Modulate the alpha mask with the output of the previous unit
						// to make chromakey-colored pixels invisible (so they wouldn't pass the alpha test)
						// (chromakey-colored pixels have an alpha value of 0.0, the others 1.0)
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_TEXTURE);
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
						glReportError();
						OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = true;
					}
					else
					{
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
						glReportError();
						OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = false;
					}
				}
				else
				{
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, unit.Function);
					glReportError();
					OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = true;
					for(unsigned long arg_index = 0; arg_index < 3; arg_index++)
					{
						arg = unit.CombineArg[arg_index];
						if (arg < CFARG_None)
						{
							if (arg == CFARG_Factor); // Alpha)
							{
								if (combine_argument[arg]->Source < CFARG_None)
								{
									// Resolve factor to local/other:
									// The factor source references to local/other,
									// which can be retrieved via the combine_argument[] array.
									source = combine_argument[combine_argument[arg]->Source]->Source;
									// but the operand of the factor specifies already which
									// component of the pixel and how it should be used.
									// Example: if factor == GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA
									// then the other alpha in AlphaCombineOthers is GL_SRC_ALPHA
									// but the factor operand is correctly GL_ONE_MINUS_SRC_ALPHA.
									operand = combine_argument[arg]->Operand;
								}
								else
								{
									source = combine_argument[arg]->Source;
									operand = combine_argument[arg]->Operand;
								}
							}
							else
							{
								source = combine_argument[arg]->Source;
								operand = combine_argument[arg]->Operand;
							}
						}
						else if (arg > CFARG_None)
						{
							source = unit.CombineArg[arg_index];
							operand = GL_SRC_ALPHA;
						}
						else
						{
							// arg == CFARG_None -> argument not used
							continue;
						}
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT + arg_index, source);
						glReportError();
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT + arg_index, operand);
						glReportError();
					}
				}
			}
		}
	}

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}
*/

/*************************************************
 * Avoid setting blending, alpha and chromakeying
 * each time triangles are rendered, May cause
 * some overhead when the state is changed more
 * than once but the effect is reduced by using
 * the CHECK_STATE_CHANGED macro
 ************************************************/
inline void SetChromaKeyAndAlphaState_update()
{
	glReportErrors("SetChromaKeyAndAlpha_update");

	// Setup alpha and chrma keying
	// Chromakeying really works for textures only, and chroma keying
	// without textures doesn't make sense anyway.
	if(!OpenGL.Blend && OpenGL.ChromaKey && OpenGL.Texture)
	{
#ifdef OPENGL_DEBUG
		GlideMsg("Changing Chromakeymode state to enabled\n");
#endif
		// setup chroma keying
		glAlphaFunc(GL_GEQUAL, 0.5);
		glEnable(GL_ALPHA_TEST);
		if (InternalConfig.EXT_compiled_vertex_array)
		{
			glColorPointer(3, GL_FLOAT, 4 * sizeof(GLfloat), &OGLRender.TColor[0]);
		}
		glReportError();
	}
	else
	{
#ifdef OPENGL_DEBUG
		GlideMsg("Changing Chromakeymode state to disabled\n");
#endif
		// Alpha Fix
		// (todo: find out why this is a fix)
		if (Glide.State.AlphaOther != GR_COMBINE_OTHER_TEXTURE)
		{
			glDisable(GL_ALPHA_TEST);
		}
		else 
		{
			if ( Glide.State.AlphaTestFunction != GR_CMP_ALWAYS )
			{
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(OpenGL.AlphaTestFunction, OpenGL.AlphaReferenceValue);
			}
			else
			{
				glDisable(GL_ALPHA_TEST);
			}
		}
		if (InternalConfig.EXT_compiled_vertex_array)
		{
			glColorPointer(4, GL_FLOAT, 0, &OGLRender.TColor[0]);
		}
		glReportError();
	}

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}

/*
static const GLfloat ZeroColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
inline void SetFogModeState_update()
{
	glReportErrors("SetFogMode_update");

	if (InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine)
	{
		if (InternalConfig.EXT_compiled_vertex_array)
		{
			glClientActiveTextureARB(OpenGL.FogTextureUnit);
			glReportError();
		}
		glActiveTextureARB(OpenGL.FogTextureUnit);
		glReportError();
		if (Glide.State.FogMode & (GR_FOG_WITH_ITERATED_ALPHA | GR_FOG_WITH_TABLE))
		{
			glEnable(GL_TEXTURE_2D);
			if (InternalConfig.EXT_compiled_vertex_array)
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(1, GL_FLOAT, 0, &OGLRender.TFog[0]);
				glReportError();
			}
			OGLRender.UseEnvCombineFog = true;
			// Now set the fog function
			GrFogMode_t modeAdd = Glide.State.FogMode & (GR_FOG_MULT2 | GR_FOG_ADD2);
			switch (modeAdd)
			{
			case GR_FOG_ADD2:
				// Cout = (1 - f) * Cin
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_COLOR);
				break;
			case GR_FOG_MULT2:
				// Cout = f * Cfog
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
				break;
			default:
				// The usual blending
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_INTERPOLATE_EXT);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
				break;
			}
			glReportError();
		}
		else if (Glide.State.ColorCombineInvert || Glide.State.AlphaInvert)
		{
			// If the texture unit is turned on to invert color or alpha then we need to supply
			// fog coords anyway and the combine function can still be GL_INTERPOLATE_EXT
			// because the minimal fog value is chosen. However, choosing replace might
			// save some vram memory access cycles  
			glEnable(GL_TEXTURE_2D);
			if (InternalConfig.EXT_compiled_vertex_array)
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(1, GL_FLOAT, 0, &OGLRender.TFog[0]);
				glReportError();
			}
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
			glReportError();
			OGLRender.UseEnvCombineFog = true;
		}
		else
		{
			if (InternalConfig.EXT_compiled_vertex_array)
			{
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				// On MacOS9 (Classic?) the texcoord pointer needs to be reset
				// to the default value when glLockArrays/glUnlockArrays is used
				glTexCoordPointer( 4, GL_FLOAT, 0, NULL );
				glReportError();
			}
			glDisable(GL_TEXTURE_2D);
			// If the texture unit is disabled, the combine env function doesn't matter
			OGLRender.UseEnvCombineFog = false;
		}
		if (InternalConfig.EXT_compiled_vertex_array)
		{
			glClientActiveTextureARB(OpenGL.ColorAlphaUnit1);
		}
		glActiveTextureARB(OpenGL.ColorAlphaUnit1);
		glReportError();
	}
	else if (InternalConfig.FogMode != OpenGLideFogEmulation_None)
	{
		if (Glide.State.FogMode & (GR_FOG_WITH_ITERATED_ALPHA | GR_FOG_WITH_TABLE))
		{
			glEnable(GL_FOG);
		}
		else
		{
			glDisable(GL_FOG);
		}
		glReportError();
		// Change the fog color in order to emulate the correct fog equation
		// (Imperfect emulation)
		GrFogMode_t modeAdd = Glide.State.FogMode & (GR_FOG_MULT2 | GR_FOG_ADD2);
		switch (modeAdd)
		{
		case GR_FOG_MULT2:
		case GR_FOG_ADD2:
			glFogfv( GL_FOG_COLOR, &ZeroColor[0]);
			glReportError();
			break;
		default:
			SetFogColorState();
			break;
		}
		glReportError();
	}

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}
*/

/*
inline void SetFogColorState_update()
{
	glReportErrors("SetFogColor_update");

	if (Glide.State.FogMode < GR_FOG_MULT2)
	{
		if (InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine)
		{
			glActiveTextureARB(OpenGL.FogTextureUnit);
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &OpenGL.FogColor[0]);
			glActiveTextureARB(OpenGL.ColorAlphaUnit1);
			glReportError();
		}
		else if (InternalConfig.FogMode != OpenGLideFogEmulation_None )
		{
			glFogfv( GL_FOG_COLOR, &OpenGL.FogColor[0] );
			glReportError();
		}
	}
	
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}
*/

inline void SetBlendState_update()
{
	glReportErrors("SetBlendState_update");

#ifdef OPENGLIDE_SYSTEM_HAS_BLENDFUNC_SEPERATE
	if (!InternalConfig.EXT_blend_func_separate)
	{
		glBlendFuncSeparateEXT(OpenGL.SrcBlend, OpenGL.DstBlend, 
		                       OpenGL.SrcAlphaBlend, OpenGL.DstAlphaBlend);
	}
	else
	{
#endif
		glBlendFunc(OpenGL.SrcBlend, OpenGL.DstBlend);
#ifdef OPENGLIDE_SYSTEM_HAS_BLENDFUNC_SEPERATE
	}
#endif

	if ( OpenGL.Blend )
	{
#ifdef OPENGL_DEBUG
		GlideMsg("Changing Blend state to enabled\n");
#endif
		glEnable(GL_BLEND);
	}
	else
	{
#ifdef OPENGL_DEBUG
		GlideMsg("Changing Blend state to disabled\n");
#endif
		glDisable(GL_BLEND);
	}
	glReportError();

	// I'd like to see this in the Set*() function,
	// but it's more optimal placed here
	#ifdef OPTIMISE_OPENGL_STATE_CHANGES
	if (OpenGL.ChromaKey && OpenGL.Texture)
	{
	#endif
		SetChromaKeyAndAlphaState();
	#ifdef OPTIMISE_OPENGL_STATE_CHANGES
	}
	else
	{
		SetAlphaCombineState();
	}
	#endif

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}

/*
void SetColorInvertState_update()
{
	glReportErrors("SetColorInvert_update");

	// Setup color inversion on fog texture unit
	if (OpenGL.FogTextureUnit)
	{
		FxBool invert = Glide.State.ColorCombineInvert;
		glActiveTextureARB(OpenGL.FogTextureUnit);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, invert ? GL_ONE_MINUS_SRC_COLOR : GL_SRC_COLOR);
		glActiveTextureARB(OpenGL.ColorAlphaUnit1);
		glReportError();  
	}

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}
*/

/*
void SetAlphaInvertState_update()
{
	glReportErrors("SetAlphaInvert_update");
	
	// Setup alpha inversion on fog texture unit
	if (OpenGL.FogTextureUnit)
	{
		FxBool invert = Glide.State.AlphaInvert;
		glActiveTextureARB(OpenGL.FogTextureUnit);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, invert ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA);
		glActiveTextureARB(OpenGL.ColorAlphaUnit1);
		glReportError();  
	}

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}
*/

/*
void SetConstantColorValueState_update()
{
	glReportErrors("SetConstantColorState_update");

	GLfloat* color;
	if (Glide.State.Delta0Mode)
	{
		color = &OpenGL.Delta0Color[0];
		s_bUpdateConstantColorValue4State = false;
	}
	else
	{
		color = &OpenGL.ConstantColor[0];
		s_bUpdateConstantColorValueState = false;
	}
	
#ifdef OPENGL_DEBUG
	GlideMsg("OpenGL.ConstantColor=(%g, %g, %g, %g)\n",
	         color[0],
	         color[1],
	         color[2],
	         color[3]);
#endif

	if (OpenGL.ColorAlphaUnit2)
	{
		for(long unit_index = 1; unit_index >= 0; unit_index--)
		{
			glActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &color[0]);
		}
	}
	
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}
*/

/*
void RenderUpdateState_old()
{
	glReportErrors("RenderUpdateState");
	
	// This filters out a few state changes in Carmageddon
	// (changing chromakey mode is expensive)
	if (OldGlideState.ChromaKeyMode != Glide.State.ChromaKeyMode)
	{
		OldGlideState.ChromaKeyMode = Glide.State.ChromaKeyMode;
#ifdef OPTIMISE_OPENGL_STATE_CHANGES
		if (!OpenGL.Blend)
#endif
		SetChromaKeyAndAlphaState();
	}
	else
	{
#ifdef OPENGL_DEBUG
		GlideMsg( "Calls to grChromakeyMode() didn't change ChromaKeyAndAlphaState\n");
#endif
	}

	// Triggered by the frambuffer
	if (s_bForceChromaKeyAndAlphaStateUpdate)
	{
		s_bForceChromaKeyAndAlphaStateUpdate = false;
		SetChromaKeyAndAlphaState();
	}
	
	if (s_bUpdateFogModeState)
	{
		s_bUpdateFogModeState = false;
		SetFogModeState_update();
	}

	if (s_bUpdateFogColorState)
	{
		s_bUpdateFogColorState = false;
		SetFogColorState_update();
	}

	if (s_bUpdateBlendState)
	{
		s_bUpdateBlendState = false;
		SetBlendState_update();
	}

	if (s_bUpdateChromaKeyAndAlphaState)
	{
		s_bUpdateChromaKeyAndAlphaState = false;
		SetChromaKeyAndAlphaState_update();
	}

	if (s_bUpdateAlphaCombineState)
	{
		s_bUpdateAlphaCombineState = false;
		SetAlphaCombineState_update();
	}

	if (s_bUpdateColorCombineState)
	{
		s_bUpdateColorCombineState = false;
		SetColorCombineState_update();
	}
	
	if (s_bUpdateColorInvertState)
	{
		s_bUpdateColorInvertState = false;
		SetColorInvertState_update();
	}

	if (s_bUpdateAlphaInvertState)
	{
		s_bUpdateAlphaInvertState = false;
		SetAlphaInvertState_update();
	}

	if (s_bUpdateTextureState)
	{
		s_bUpdateTextureState = false;
		SetTextureState_update();
	}

	if (s_bUpdateConstantColorValueState || s_bUpdateConstantColorValue4State)
	{
		SetConstantColorValueState_update();
	}

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
	VERIFY_TEXTURE_ENABLED_STATE();
}
*/

void RenderUpdateState()
{
	glReportErrors("RenderUpdateState");
	
	// This filters out a few state changes in Carmageddon
	// (changing chromakey mode is expensive)
	if (OldGlideState.ChromaKeyMode != Glide.State.ChromaKeyMode)
	{
		OldGlideState.ChromaKeyMode = Glide.State.ChromaKeyMode;
#ifdef OPTIMISE_OPENGL_STATE_CHANGES
		if (!OpenGL.Blend)
#endif
		SetChromaKeyAndAlphaState();
	}
	else
	{
#ifdef OPENGL_DEBUG
		GlideMsg( "Calls to grChromakeyMode() didn't change ChromaKeyAndAlphaState\n");
#endif
	}

	// Triggered by the framebuffer
	if (s_bForceChromaKeyAndAlphaStateUpdate)
	{
		s_bForceChromaKeyAndAlphaStateUpdate = false;
		SetChromaKeyAndAlphaState();
	}

	if (s_bUpdateBlendState)
	{
		s_bUpdateBlendState = false;
		SetBlendState_update();
	}

	if (s_bUpdateChromaKeyAndAlphaState)
	{
		s_bUpdateChromaKeyAndAlphaState = false;
		SetChromaKeyAndAlphaState_update();
	}

	bool active_texture_unit_not_coloralpha1 = false;
	bool active_texture_unit_client_state_not_coloralpha1 = false;
	if (s_bUpdateFogModeState ||
		s_bUpdateFogColorState ||
	    s_bUpdateColorInvertState ||
	    s_bUpdateAlphaInvertState)
	{
		if (InternalConfig.FogMode == OpenGLideFogEmulation_EnvCombine)
		{
			if (s_bUpdateFogModeState || s_bUpdateFogColorState)
			{
				glActiveTextureARB(OpenGL.FogTextureUnit);
				if (InternalConfig.EXT_compiled_vertex_array)
				{
					glClientActiveTextureARB(OpenGL.FogTextureUnit);
					active_texture_unit_client_state_not_coloralpha1 = true;
				}
				active_texture_unit_not_coloralpha1 = true;
				glReportError();
			}
			
			if (s_bUpdateFogModeState)
			{
				VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.FogTextureUnit);

				s_bUpdateFogModeState = false;
				if (Glide.State.FogMode & (GR_FOG_WITH_ITERATED_ALPHA | GR_FOG_WITH_TABLE))
				{
					glEnable(GL_TEXTURE_2D);
					if (InternalConfig.EXT_compiled_vertex_array)
					{
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						glTexCoordPointer(1, GL_FLOAT, 0, &OGLRender.TFog[0]);
						glReportError();
					}
					OGLRender.UseEnvCombineFog = true;
					// Now set the fog function
					GrFogMode_t modeAdd = Glide.State.FogMode & (GR_FOG_MULT2 | GR_FOG_ADD2);
					switch (modeAdd)
					{
					case GR_FOG_ADD2:
						// Cout = (1 - f) * Cin
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_COLOR);
						break;
					case GR_FOG_MULT2:
						// Cout = f * Cfog
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
						break;
					default:
						// The usual blending
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_INTERPOLATE_EXT);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
						break;
					}
					glReportError();
				}
				else if (Glide.State.ColorCombineInvert || Glide.State.AlphaInvert)
				{
					// If the texture unit is turned on to invert color or alpha then we need to supply
					// fog coords anyway and the combine function can still be GL_INTERPOLATE_EXT
					// because the minimal fog value is chosen. However, choosing replace might
					// save some vram memory access cycles  
					glEnable(GL_TEXTURE_2D);
					if (InternalConfig.EXT_compiled_vertex_array)
					{
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						glTexCoordPointer(1, GL_FLOAT, 0, &OGLRender.TFog[0]);
						glReportError();
					}
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
					glReportError();
					OGLRender.UseEnvCombineFog = true;
				}
				else
				{
					if (InternalConfig.EXT_compiled_vertex_array)
					{
						glDisableClientState(GL_TEXTURE_COORD_ARRAY);
						// On MacOS9 (Classic?) the texcoord pointer needs to be reset
						// to the default value when glLockArrays/glUnlockArrays is used
						glTexCoordPointer( 4, GL_FLOAT, 0, NULL );
						glReportError();
					}
					glDisable(GL_TEXTURE_2D);
					// If the texture unit is disabled, the combine env function doesn't matter
					OGLRender.UseEnvCombineFog = false;
				}
			}
			
			// Fog Color
			if (s_bUpdateFogColorState)
			{
				s_bUpdateFogColorState = false;
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &OpenGL.FogColor[0]);
				glReportError();
			}
		}
		else if (InternalConfig.FogMode != OpenGLideFogEmulation_None)
		{
			if (s_bUpdateFogModeState)
			{
				s_bUpdateFogModeState = false;
				if (Glide.State.FogMode & (GR_FOG_WITH_ITERATED_ALPHA | GR_FOG_WITH_TABLE))
				{
					glEnable(GL_FOG);
				}
				else
				{
					glDisable(GL_FOG);
				}
				glReportError();
				// Change the fog color in order to emulate the correct fog equation
				// (Imperfect emulation)
				static const GLfloat ZeroColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
				GrFogMode_t modeAdd = Glide.State.FogMode & (GR_FOG_MULT2 | GR_FOG_ADD2);
				switch (modeAdd)
				{
				case GR_FOG_MULT2:
				case GR_FOG_ADD2:
					glFogfv( GL_FOG_COLOR, &ZeroColor[0]);
					glReportError();
					break;
				default:
					SetFogColorState();
					break;
				}
				glReportError();
			}
			
			if (s_bUpdateFogColorState)
			{
				s_bUpdateFogColorState = false;
				glFogfv(GL_FOG_COLOR, &OpenGL.FogColor[0]);
				glReportError();
			}
		}

		if (active_texture_unit_not_coloralpha1 == false &&
		    (s_bUpdateColorInvertState || s_bUpdateAlphaInvertState))
		{
			glActiveTextureARB(OpenGL.FogTextureUnit);
			if (InternalConfig.EXT_compiled_vertex_array)
			{
				glClientActiveTextureARB(OpenGL.FogTextureUnit);
				active_texture_unit_client_state_not_coloralpha1 = true;
			}
			glReportError();
			active_texture_unit_not_coloralpha1 = true;
		}

		if (s_bUpdateColorInvertState)
		{
			VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.FogTextureUnit);
			
			s_bUpdateColorInvertState = false;
			FxBool invert = Glide.State.ColorCombineInvert;
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, invert ? GL_ONE_MINUS_SRC_COLOR : GL_SRC_COLOR);
		}

		if (s_bUpdateAlphaInvertState)
		{
			VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.FogTextureUnit);
			
			s_bUpdateAlphaInvertState = false;
			FxBool invert = Glide.State.AlphaInvert;
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, invert ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA);
		}
	}
	
	// Coloralpha state changes
	if (s_bUpdateTextureState ||
	    s_bUpdateAlphaCombineState ||
	    s_bUpdateColorCombineState ||
	    s_bUpdateConstantColorValueState ||
	    s_bUpdateConstantColorValue4State)
	{
		// Find out which texture units need to be setup
		if (OpenGL.ColorAlphaUnit2)
		{
			// Reduce alpha combine function for constant factors
			GrCombineFunction_t a_function = Glide.State.AlphaFunction;
			const GrCombineFactor_t a_factor = Glide.State.AlphaFactor;
			if (a_factor == GR_COMBINE_FACTOR_ZERO)
			{
				a_function = AlphaCombineFunctionsFactorZero[a_function].ReducedTerm;
			}
			else if (a_factor == GR_COMBINE_FACTOR_ONE)
			{
				a_function = AlphaCombineFunctionsFactorOne[a_function].ReducedTerm;
			}
			// Color
			GrCombineFunction_t c_function = Glide.State.ColorCombineFunction;
			const GrCombineFactor_t c_factor = Glide.State.ColorCombineFactor;
			// Reduce color combine function for constant factors
			if (c_factor == GR_COMBINE_FACTOR_ZERO)
			{
				c_function = ColorCombineFunctionsFactorZero[c_function].ReducedTerm;
			}
			else if (c_factor == GR_COMBINE_FACTOR_ONE)
			{
				c_function = ColorCombineFunctionsFactorOne[c_function].ReducedTerm;
			}
			// Determine which texture units must be enabled
			bool enable_texture_unit[2];
			bool previously_enabled_texture_unit[2];
			// ColorCombine
			for(long unit_index = 1; unit_index >= 0; unit_index--)
			{
				// Remember last state because if a unit gets enabled, we have to update it's whole state
				previously_enabled_texture_unit[unit_index] = OpenGL.ColorAlphaUnitColorEnabledState[unit_index] ||
			                                                OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index];
				if (OpenGL.ColorCombineFunctions[c_function].ColorAlphaUnit[unit_index].Function == CF_Unused)
				{
					OpenGL.ColorAlphaUnitColorEnabledState[unit_index] = false;
				}
				else
				{
					OpenGL.ColorAlphaUnitColorEnabledState[unit_index] = true;
				}
				// AlphaCombine
				if (OpenGL.ChromaKey && OpenGL.Texture && !OpenGL.Blend)
				{
					// OpenGL.ColorAlphaUnitAlphaEnabledState[0] = true;
					// OpenGL.ColorAlphaUnitAlphaEnabledState[1] = false;
					OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = (unit_index == 0) ? true : false;
				}
				else
				{
					if (OpenGL.AlphaCombineFunctions[a_function].ColorAlphaUnit[unit_index].Function == CF_Unused)
					{
						if (OpenGL.ChromaKey && OpenGL.Texture && OpenGL.Blend)
						{
							// Modulate the alpha mask with the output of the previous unit
							// to make chromakey-colored pixels invisible (so they wouldn't pass the alpha test)
							// (chromakey-colored pixels have an alpha value of 0.0, the others 1.0)
							OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = true;
						}
						else
						{
							OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = false;
						}
					}
					else
					{
						OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = true;
					}
				}
				enable_texture_unit[unit_index] = OpenGL.ColorAlphaUnitColorEnabledState[unit_index] ||
			                                    OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index];
			}
#ifdef OPENGL_DEBUG
			for(long unit_index = 0; unit_index < 2; unit_index++)
			{
				GlideMsg( "OpenGL.ColorAlphaUnitColorEnabledState[%d] = %d\n", unit_index, OpenGL.ColorAlphaUnitColorEnabledState[unit_index]);
				GlideMsg( "OpenGL.ColorAlphaUnitAlphaEnabledState[%d] = %d\n", unit_index, OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index]);
				const char* enabled = OpenGL.ColorAlphaUnitColorEnabledState[unit_index] || OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index]
				                      ? "enabled" : "disabled";
				GlideMsg("Texture unit GL_TEXTURE%d_ARB = %s\n", unit_index, enabled);
			}
#endif
			// Loop through the texture units and set some state
			// but reset the state change flags on the last iteration only
			// (nothing is setup if both units are disabled, but then the
			// state has to be set later on anyway) 
			bool reset_state = false;
			for(long unit_index = 1; unit_index >= 0; unit_index--)
			{
				if (!enable_texture_unit[unit_index])
				{
					if (s_bUpdateTextureState)
					{
						if (reset_state) s_bUpdateTextureState = false;
						// Update texture unit that has just been enabled?
						if (!enable_texture_unit[unit_index] && previously_enabled_texture_unit[unit_index])
						{
#ifdef OPENGL_DEBUG
							GlideMsg("Disabling unused texture unit GL_TEXTURE%d_ARB\n", unit_index);
#endif							
							// Save state changes if fog state hasn't been changed
							// and only coloralpha texture unit 0 is used
							const bool set_active_texture_unit = active_texture_unit_not_coloralpha1 || unit_index != 0;
							if (set_active_texture_unit)
							{
								glActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
								if (InternalConfig.EXT_compiled_vertex_array)
								{
									if (set_active_texture_unit)
									{
										glClientActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
									}
									glDisableClientState(GL_TEXTURE_COORD_ARRAY);
									active_texture_unit_client_state_not_coloralpha1 = (unit_index != 0);
								}
								active_texture_unit_not_coloralpha1 = (unit_index != 0);
								glReportError();
							}
							VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1 + unit_index);
							
							glDisable(GL_TEXTURE_2D);
							glReportError();
						}
						else
						{
#ifdef OPENGL_DEBUG
							GlideMsg("Unused texture unit GL_TEXTURE%d_ARB already disabled\n", unit_index);
#endif							
						}
					}
				}
				else
				{
					// Save state changes if fog state hasn't been changed
					// and only coloralpha texture unit 0 is used
					const bool set_active_texture_unit = active_texture_unit_not_coloralpha1 || unit_index != 0;
					if (set_active_texture_unit)
					{
						glActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
						glReportError();
						active_texture_unit_not_coloralpha1 = (unit_index != 0);
					}
					VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1 + unit_index);

					// Texture state
					if (s_bUpdateTextureState)
					{
						if (reset_state) s_bUpdateTextureState = false;
						// Update texture unit that has just been enabled?
						if (enable_texture_unit[unit_index] && !previously_enabled_texture_unit[unit_index])
						{
#ifdef OPENGL_DEBUG
							GlideMsg("Enabling texture unit GL_TEXTURE%d_ARB\n", unit_index);
#endif							
							// Enable the texture unit
							glEnable(GL_TEXTURE_2D);
							if (InternalConfig.EXT_compiled_vertex_array && set_active_texture_unit)
							{
								glClientActiveTextureARB(OpenGL.ColorAlphaUnit1 + unit_index);
								glEnableClientState(GL_TEXTURE_COORD_ARRAY);
								active_texture_unit_client_state_not_coloralpha1 = (unit_index != 0);
					 		}
							glReportError();
							// Update the texture unit
							SetColorCombineState();
							SetAlphaCombineState();
							if (Glide.State.Delta0Mode) SetConstantColorValue4State();
							else SetConstantColorValueState();
						}
						else
						{
#ifdef OPENGL_DEBUG
							GlideMsg("Texture unit GL_TEXTURE%d_ARB already enabled\n", unit_index);
#endif							
						}
					}
					
					VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1 + unit_index);

					// ColorCombineState
					if (s_bUpdateColorCombineState)
					{
						if (reset_state) s_bUpdateColorCombineState = false;
						const CombineFunctionGLTextureUnit& unit = OpenGL.ColorCombineFunctions[c_function].ColorAlphaUnit[unit_index];
						if (unit.Function == CF_Unused)
						{
#ifdef OPENGL_DEBUG
								GlideMsg("OpenGL ColorCombine unit %d = unused\n", unit_index);
#endif
							glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
							glReportError();
						}
						else
						{
#ifdef OPENGL_DEBUG
								GlideMsg("OpenGL ColorCombine Function for unit %d = 0x%x\n", unit_index, unit.Function);
#endif
							glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, unit.Function);
							glReportError();
							CombineFunctionColorAlphaArg arg;
							GLint source;
							GLint operand;
							const GrCombineLocal_t c_local = Glide.State.ColorCombineLocal;
							const GrCombineOther_t c_other = Glide.State.ColorCombineOther;
							const GrCombineLocal_t a_local = Glide.State.AlphaLocal;
							const GrCombineOther_t a_other = Glide.State.AlphaOther;
							const CombineArgument** combine_argument = &OpenGL.ColorCombineArguments[0];
							combine_argument[CFARG_Local] =  &ColorCombineLocals[c_local];
							combine_argument[CFARG_Other] =  Glide.State.TextureCombineRGBInvert ? &ColorCombineOthersInverted[c_other] : &ColorCombineOthers[c_other];
							combine_argument[CFARG_LocalAlpha] =  &AlphaCombineLocals[a_local];
							combine_argument[CFARG_OtherAlpha] =  Glide.State.TextureCombineAInvert ? &AlphaCombineOthersInverted[a_other] : &AlphaCombineOthers[a_other];
							combine_argument[CFARG_Factor] = Glide.State.TextureCombineRGBInvert ? &ColorCombineFactorsInverted[c_factor] : &ColorCombineFactors[c_factor];
							for(unsigned long arg_index = 0; arg_index < 3; arg_index++)
							{
								arg = unit.CombineArg[arg_index];
								if (arg < CFARG_None)
								{
									if (arg == CFARG_Factor /* ||arg == CFARG_FactorAlpha*/)
									{
										if (combine_argument[arg]->Source < CFARG_None)
										{
											// Resolve factor to local/other:
											// The factor source references to the local/other,
											// which can be retrieved via the combine_argument[] array
											source = combine_argument[combine_argument[arg]->Source]->Source;
											// but the operand of the factor specifies already the
											// correct component of the pixel (rgb or alpha) to be used.
											// Example: if factor == GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA
											// then the other alpha in AlphaCombineOthers is GL_SRC_ALPHA
											// but the factor operand is correctly GL_ONE_MINUS_SRC_ALPHA.
											// operand = combine_argument[arg]->Operand;
											// Not after color inversion has been added
											operand = combine_argument[combine_argument[arg]->Source]->Operand; // No
										}
										else
										{
											source = combine_argument[arg]->Source;
											operand = combine_argument[arg]->Operand;
										}
									}
									else
									{
										// combinearg = local or other (alpha)
										source = combine_argument[arg]->Source;
										operand = combine_argument[arg]->Operand;
									}
								}
								else if (arg > CFARG_None)
								{
									source = unit.CombineArg[arg_index];
									operand = GL_SRC_COLOR;
								}
								else
								{
									// arg == CFARG_None -> argument not used
									continue;
								}
#ifdef OPENGL_DEBUG
								GlideMsg("Arg %d = Source:0x%x, Operand:0x%x\n", arg_index, source, operand);
#endif
								glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT + arg_index, source);
								glReportError();
								glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT + arg_index, operand);
								glReportError();
							}
						}
						glReportError();
					}

					// Alpha Combine State
					if (s_bUpdateAlphaCombineState)
					{
						if (reset_state) s_bUpdateAlphaCombineState = false;
						// Handle chromakey special case
						if (OpenGL.ChromaKey && OpenGL.Texture && !OpenGL.Blend)
						{
							// need to disable alpha combining until an additional
							// texture unit can be used to mask out the chroma key color.
							// Note that the second unit must be setup because the
							// texture unit might be turned on due to the color setup.
							if (unit_index == 1)
							{
#ifdef OPENGL_DEBUG
								GlideMsg("OpenGL AlphaCombine unit %d = unused\n", unit_index);
#endif
								// OpenGL.ColorAlphaUnitAlphaEnabledState[1] = false;
								glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
								glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
								glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
							}
							else
							{
#ifdef OPENGL_DEBUG
								GlideMsg("OpenGL AlphaCombine unit %d = texture_alpha\n", unit_index);
#endif
								// OpenGL.ColorAlphaUnitAlphaEnabledState[0] = true;
								glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
								glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
								glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
							}
							glReportError();
						}
						else
						{
							CombineFunctionGLTextureUnit unit = OpenGL.AlphaCombineFunctions[a_function].ColorAlphaUnit[unit_index];
							if (unit.Function == CF_Unused)
							{
								if (OpenGL.ChromaKey && OpenGL.Texture && OpenGL.Blend)
								{
									// Modulate the alpha mask with the output of the previous unit
									// to make chromakey-colored pixels invisible (so they wouldn't pass the alpha test)
									// (chromakey-colored pixels have an alpha value of 0.0, the others 1.0)
#ifdef OPENGL_DEBUG
								GlideMsg("OpenGL AlphaCombine unit %d = chromakey mask\n", unit_index);
#endif
									glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
									glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
									glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
									glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_TEXTURE);
									glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
									glReportError();
									// OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = true;
								}
								else
								{
#ifdef OPENGL_DEBUG
								GlideMsg("OpenGL AlphaCombine unit %d = unused\n", unit_index);
#endif
									glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
									glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
									glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
									glReportError();
									// OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = false;
								}
							}
							else
							{
#ifdef OPENGL_DEBUG
								GlideMsg("OpenGL AlphaCombine Function for unit %d = 0x%x\n", unit_index, unit.Function);
#endif
								glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, unit.Function);
								glReportError();
								CombineFunctionColorAlphaArg arg;
								GLint source;
								GLint operand;
								// Alpha @todo: colors are unused, aren't they?
								const GrCombineLocal_t a_local = Glide.State.AlphaLocal;
								const GrCombineOther_t a_other = Glide.State.AlphaOther;
								const CombineArgument** combine_argument = &OpenGL.AlphaCombineArguments[0];
								// combine_argument[CFARG_Local] = 0; // This is set to 0 onn initialising OpenGL
								// combine_argument[CFARG_Other] = 0;
								combine_argument[CFARG_LocalAlpha] = &AlphaCombineLocals[a_local];
								combine_argument[CFARG_OtherAlpha] = Glide.State.TextureCombineAInvert ? &AlphaCombineOthersInverted[a_other] : &AlphaCombineOthers[a_other];
								combine_argument[CFARG_Factor] = Glide.State.TextureCombineAInvert ? &AlphaCombineFactorsInverted[a_factor] : &AlphaCombineFactors[a_factor];
								// Color
								// OpenGL.ColorAlphaUnitAlphaEnabledState[unit_index] = true;
								for(unsigned long arg_index = 0; arg_index < 3; arg_index++)
								{
									arg = unit.CombineArg[arg_index];
									if (arg < CFARG_None)
									{
										if (arg == CFARG_Factor /*Alpha*/)
										{
											if (combine_argument[arg]->Source < CFARG_None)
											{
												// Resolve factor to local/other:
												// The factor source references to local/other,
												// which can be retrieved via the combine_argument[] array.
												source = combine_argument[combine_argument[arg]->Source]->Source;
												// but the operand of the factor specifies already which
												// component of the pixel and how it should be used.
												// Example: if factor == GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA
												// then the other alpha in AlphaCombineOthers is GL_SRC_ALPHA
												// but the factor operand is correctly GL_ONE_MINUS_SRC_ALPHA.
												// operand = combine_argument[arg]->Operand;
												// After color inversion has been added, the operand must be taken
												// from the inversion table
												operand = combine_argument[combine_argument[arg]->Source]->Operand;
											}
											else
											{
												source = combine_argument[arg]->Source;
												operand = combine_argument[arg]->Operand;
											}
										}
										else
										{
											source = combine_argument[arg]->Source;
											operand = combine_argument[arg]->Operand;
										}
									}
									else if (arg > CFARG_None)
									{
										source = unit.CombineArg[arg_index];
										operand = GL_SRC_ALPHA;
									}
									else
									{
										// arg == CFARG_None -> argument not used
										continue;
									}
#ifdef OPENGL_DEBUG
									GlideMsg("Arg %d = Source:0x%x, Operand:0x%x\n", arg_index, source, operand);
#endif
									glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT + arg_index, source);
									glReportError();
									glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT + arg_index, operand);
									glReportError();
								}
							}
						}
						glReportError();
					}

					// Constant Color State
					if (s_bUpdateConstantColorValueState || s_bUpdateConstantColorValue4State)
					{
						GLfloat* color;
						if (Glide.State.Delta0Mode)
						{
							if (reset_state) s_bUpdateConstantColorValue4State = false;
							color = &OpenGL.Delta0Color[0];
						}
						else
						{
							if (reset_state) s_bUpdateConstantColorValueState = false;
							color = &OpenGL.ConstantColor[0];
						}
#ifdef OPENGL_DEBUG
						if (reset_state) GlideMsg("OpenGL.ConstantColor=(%g, %g, %g, %g)\n",
						                          color[0],
						                          color[1],
						                          color[2],
						                          color[3]);
#endif
						glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &color[0]);
						glReportError();
					}
				}
				// Ensure the state change flags are reset during the last iteration
				reset_state = true;
			}
			glReportError();
		}
		else // OpenGL.ColorAlphaTectureUnit2 == 0
		{
			if (active_texture_unit_not_coloralpha1)
			{
				glActiveTextureARB(OpenGL.ColorAlphaUnit1);
				active_texture_unit_not_coloralpha1 = false;
			}
			if (s_bUpdateTextureState)
			{
				s_bUpdateTextureState = false;
				if (active_texture_unit_client_state_not_coloralpha1)
				{
					glClientActiveTextureARB(OpenGL.ColorAlphaUnit1);
					active_texture_unit_client_state_not_coloralpha1 = false;
				}
				if (OpenGL.Texture)
				{
					glEnable(GL_TEXTURE_2D);
					if (InternalConfig.EXT_compiled_vertex_array)
					{
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					}
				}
				else
				{
					if (InternalConfig.EXT_compiled_vertex_array)
					{
						glDisableClientState(GL_TEXTURE_COORD_ARRAY);
					}
					glDisable(GL_TEXTURE_2D);
				}
				glReportError();
			}
			
			if (s_bUpdateColorCombineState)
			{
				s_bUpdateColorCombineState = false;
				if ( ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA ) &&
				     ( Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
				{
				    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
				}
				else
				if ( ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB ) &&
				     ( Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
				{
				    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
				}
				else
				if (InternalConfig.EXT_texture_env_add &&
				    ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_ONE ) &&
				    ( Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) &&
				    ( ( Glide.State.ColorCombineFunction == GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA ) || 
				      ( Glide.State.ColorCombineFunction == GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL ) ) )
				{
				    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );
				}
				else
				{
				    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
				}
				glReportError();
			}
		}
	}

	if (active_texture_unit_not_coloralpha1)
	{
		if (active_texture_unit_client_state_not_coloralpha1)
		{
			glClientActiveTextureARB(OpenGL.ColorAlphaUnit1);
		}
		glActiveTextureARB(OpenGL.ColorAlphaUnit1);
		glReportError();
	}
		
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
	VERIFY_TEXTURE_ENABLED_STATE();
}

void SetClipVerticesState_Update(bool clip_vertices)
{
	OpenGL.ClipVerticesEnabledState = clip_vertices;
	if (InternalConfig.EXT_clip_volume_hint)
	{
		RenderDrawTriangles();
		glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, clip_vertices ? GL_DONT_CARE : GL_FASTEST);
#ifdef OPENGL_DEBUG
		GlideMsg("OpenGL.ClipVerticesEnabledState = %s\n", clip_vertices ? "GL_DONT_CARE" : "GL_FASTEST");
#endif
	}
}