//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                      Fog Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "GlideSettings.h"
#include "GLRender.h"
#include "GLUtil.h"
#include "GLRenderUpdateState.h"
#include "OGLTables.h"

//*************************************************
//* download a fog table
//* Fog is applied after color combining and before alpha blending.
//*************************************************
FX_ENTRY void FX_CALL
grFogTable( const GrFog_t *ft )
{
#ifdef OGL_DONE
    GlideMsg( "grFogTable( --- )\n" );
#endif

    if ( InternalConfig.FogMode > OpenGLideFogEmulation_None )
    {
        memcpy( Glide.FogTable, ft, GR_FOG_TABLE_SIZE * sizeof( FxU8 ) );
        Glide.FogTable[ GR_FOG_TABLE_SIZE ] = 255;

        for ( FxU32 i = 0; i < GR_FOG_TABLE_SIZE; i++ )
        {
            for ( FxU32 j = intStartEnd[ i ]; j < intStartEnd[ i + 1 ]; j++ )
            {
                OpenGL.FogTable[ j ] = (FxU8)( Glide.FogTable[ i ] + 
                    ( Glide.FogTable[ i + 1 ] - Glide.FogTable[ i ] ) * ( j - intStartEnd[ i ] ) / 
                    intEndMinusStart[ i ] );
            }
        }
    }
}

//*************************************************
FX_ENTRY void FX_CALL
grFogColorValue( GrColor_t fogcolor )
{
	glReportErrors("grFogColorValue");
#ifdef OGL_PARTDONE
    GlideMsg( "grFogColorValue( %x )\n", fogcolor );
#endif

    RenderDrawTriangles();
    Glide.State.FogColorValue = fogcolor;
		ConvertColorF(fogcolor, 
		              OpenGL.FogColor[0], 
		              OpenGL.FogColor[1], 
		              OpenGL.FogColor[2], 
		              OpenGL.FogColor[3]);
		SetFogColorState();
}

//*************************************************
FX_ENTRY void FX_CALL
grFogMode( GrFogMode_t mode )
{
	glReportErrors("grFogMode");

	CHECK_STATE_CHANGED(mode == Glide.State.FogMode);

#ifdef OGL_PARTDONE
    GlideMsg( "grFogMode( %d )\n", mode );
#endif

	RenderDrawTriangles();
	
	Glide.State.FogMode = mode;
	OpenGL.Fog = mode & (GR_FOG_WITH_ITERATED_ALPHA | GR_FOG_WITH_TABLE);
	SetFogModeState();
}

//*************************************************
FX_ENTRY void FX_CALL
guFogGenerateExp( GrFog_t *fogtable, float density )
{
#ifdef OGL_PARTDONE
    GlideMsg( "guFogGenerateExp( ---, %-4.2f )\n", density );
#endif
		glReportErrors("guFogGenerateExp");

    float f;
    float scale;
    float dp;
    
    dp = density * guFogTableIndexToW( GR_FOG_TABLE_SIZE - 1 );
    scale = 255.0F / ( 1.0F - (float) exp( -dp ) );
    
    for ( int i = 0; i < GR_FOG_TABLE_SIZE; i++ )
    {
        dp = density * guFogTableIndexToW( i );
        f = ( 1.0F - (float) exp( -dp ) ) * scale;
        
        if ( f > 255.0F )
        {
            f = 255.0F;
        }
        else if ( f < 0.0F )
        {
            f = 0.0F;
        }
        
        fogtable[ i ] = (GrFog_t) f;
    }

    if (InternalConfig.FogMode == OpenGLideFogEmulation_Simple)
    {
      glFogf( GL_FOG_MODE, GL_EXP );
	    glReportError();
      glFogf( GL_FOG_DENSITY, density );
	    glReportError();
		}
}

//*************************************************
FX_ENTRY void FX_CALL
guFogGenerateExp2( GrFog_t *fogtable, float density )
{
#ifdef OGL_PARTDONE
    GlideMsg( "guFogGenerateExp2( ---, %-4.2f )\n", density );
#endif

		glReportErrors("guFogGenerateExp2");

    float Temp;

    for ( int i = 0; i < GR_FOG_TABLE_SIZE; i++ )
    {
        Temp = ( 1.0f - (float) exp( ( -density)  * guFogTableIndexToW( i ) ) * 
               (float)exp( (-density)  * guFogTableIndexToW( i ) ) )  * 255.0f;
        fogtable[ i ] = (FxU8) Temp;
    }

    if (InternalConfig.FogMode == OpenGLideFogEmulation_Simple)
    {
      glFogf( GL_FOG_MODE, GL_EXP2 );
	    glReportError();
      glFogf( GL_FOG_DENSITY, density );
	    glReportError();
		}
}

//*************************************************
FX_ENTRY void FX_CALL
guFogGenerateLinear( GrFog_t *fogtable,
                     float nearZ, float farZ )
{
#ifdef OGL_PARTDONE
    GlideMsg( "guFogGenerateLinear( ---, %-4.2f, %-4.2f )\n", nearZ, farZ );
#endif

	glReportErrors("guFogGenerateLinear");

    int Start, 
        End, 
        i;

    for( Start = 0; Start < GR_FOG_TABLE_SIZE; Start++ )
    {
        if ( guFogTableIndexToW( Start ) >= nearZ )
        {
            break;
        }
    }
    for( End = 0; End < GR_FOG_TABLE_SIZE; End++ )
    {
        if ( guFogTableIndexToW( End ) >= farZ )
        {
            break;
        }
    }

    memset( fogtable, 0, GR_FOG_TABLE_SIZE );
    for( i = Start; i <= End; i++ )
    {
        fogtable[ i ] = (FxU8)((float)( End - Start ) / 255.0f * (float)( i - Start ));
    }

    for( i = End; i < GR_FOG_TABLE_SIZE; i++ )
    {
        fogtable[ i ] = 255;
    }

    if (InternalConfig.FogMode == OpenGLideFogEmulation_Simple)
    {
		glFogf( GL_FOG_MODE, GL_LINEAR );
		glReportError();
		glFogf( GL_FOG_START, nearZ );
		glReportError();
		glFogf( GL_FOG_END, farZ );
		glReportError();
		if (nearZ <0 || farZ > 1.0)
		{
			GlideMsg("Warning: guFogGenerateLinear( ---, %-4.2f, %-4.2f ) values out of expected range\n", nearZ, farZ);
		}
	}
}

//*************************************************
//* convert a fog table index to a floating point eye-space w value
//*************************************************
FX_ENTRY float FX_CALL
guFogTableIndexToW( int i )
{
#ifdef OGL_DONE
    GlideMsg( "guFogTableIndexToW( %d )\n", i );
#endif
#ifdef OGL_DEBUG
    if ( ( i < 0 ) ||
         ( i >= GR_FOG_TABLE_SIZE ) )
    {
        GlideError( "Error on guFogTableIndexToW( %d )\n", i );
    }
#endif

    return tableIndexToW[ i ];
}
