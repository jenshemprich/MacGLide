//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*          implementation of the GLRender class
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Glide.h"
#include "GlideApplication.h"
#include "GlideFrameBuffer.h"
#include "GlideSettings.h"
#include "GLextensions.h"
#include "GLRender.h"
#include "GLRenderUpdateState.h"
#include "PGTexture.h"


// The functions for the color combine
ALPHAFACTORFUNCPROC AlphaFactorFunc;
COLORFACTORFUNCPROC ColorFactor3Func;
COLORFUNCTIONPROC   ColorFunctionFunc;

// Snapping constant
static const float vertex_snap_compare = 4096.0f;
static const float vertex_snap = float( 3L << 18 );

#define DEBUG_MIN_MAX( var, maxvar, minvar )    \
    if ( var > maxvar ) maxvar = var;           \
    if ( var < minvar ) minvar = var;

// Standard structs for the render
RenderStruct OGLRender;

// Variables for the AddXXX functions
static TColorStruct     Local, 
                        Other, 
                        CFactor;

#ifdef OGL_DEBUG_GLIDE_COORDS
	void GlideMsg(const GrVertex* v, float maxoow)
	{
		GlideMsg("vertex(x=%g,y=%g,ooz=%g,oow=%g)", v->x, v->y, v->ooz, v->oow);
		GlideMsg(",tmu0(sow=%g,tow=%g,oow=%g)\n", v->tmuvtx[0].sow * maxoow, v->tmuvtx[0].tow * maxoow, v->tmuvtx[0].oow);
	}
#endif

#ifdef OGL_DEBUG_OPENGL_COORDS
	void GlideMsg(TVertexStruct* v)
	{
		GlideMsg("	glVertex(x=%g,y=%g,z=%g)", v->ax, v->ay, v->az);
		GlideMsg("	glVertex(x=%g,y=%g,z=%g)", v->bx, v->by, v->bz);
		GlideMsg("	glVertex(x=%g,y=%g,z=%g)", v->cx, v->cy, v->cz);
	}
#endif

#ifdef OGL_DEBUG_OPENGL_COORDS
	void GlideMsg(TVertexStruct* v, TTextureStruct* t)
	{
		GlideMsg("	glVertex(x=%g,y=%g,z=%g)", v->ax, v->ay, v->az);
		GlideMsg(", glTexture(s=%g,t=%g,q=%g,w=%g)\n", t->as, t->at, t->aq, t->aoow);
		GlideMsg("	glVertex(x=%g,y=%g,z=%g)", v->bx, v->by, v->bz);
		GlideMsg(", glTexture(s=%g,t=%g,q=%g,w=%g)\n", t->bs, t->bt, t->bq, t->boow);
		GlideMsg("	glVertex(x=%g,y=%g,z=%g)", v->cx, v->cy, v->cz);
		GlideMsg(", glTexture(s=%g,t=%g,q=%g,w=%g)\n", t->cs, t->ct, t->cq, t->coow);
	}
#endif

inline GLfloat precision_fix(GLfloat oow)
{
	const GLfloat w = 1.0f / oow;
	return 8.9375f - (GLfloat( ( (*(FxU32 *)&w >> 11) & 0xFFFFF ) * D1OVER65536) );
}

void RenderInitialize(void)
{
#ifdef OGL_ALL
    GlideMsg( "RenderInitialize()\n");
#endif
	glReportErrors("RenderInitialize");
	// initialise triagle buffers
	OGLRender.NumberOfTriangles = 0;
	const int triangles = InternalConfig.EXT_compiled_vertex_array ? OGLRender.FrameBufferStartIndex + 2 * Framebuffer::MaxTiles * Framebuffer::MaxTiles : OGLRender.FrameBufferStartIndex;
	// Framebuffer utilises color, vertex and texture array only
	OGLRender.TColor = (TColorStruct*) AllocBuffer(triangles, sizeof(TColorStruct));
	if (OpenGL.ColorAlphaUnit2 == 0)
	{
		// This must be initialiased even if the secondary color extension is absent
		// (Because there are no additional checks later on)
		OGLRender.TColor2  = (TColorStruct*) AllocBuffer(OGLRender.FrameBufferStartIndex, sizeof(TColorStruct));
	}
	else
	{
		OGLRender.TColor2 = NULL;
	}
	OGLRender.TTexture = (TTextureStruct*) AllocBuffer(triangles, sizeof(TTextureStruct));
 	// Preinit static data and save a few cycles in RenderAddXXX()
	for(int i = 0; i < OGLRender.FrameBufferStartIndex; i++)
	{
		TTextureStruct* pTS = &OGLRender.TTexture[i];
		pTS->aq = pTS->bq = pTS->cq = 0.0f;
	}
	OGLRender.TVertex  = (TVertexStruct*)  AllocBuffer(triangles, sizeof(TVertexStruct));
	OGLRender.TFog     = (TFogStruct*)     AllocBuffer(triangles, sizeof(TFogStruct));
	// Initialise compiled vertex arrays
	OGLRender.BufferLocked = false;
	OGLRender.BufferStart = 0;
	OGLRender.BufferLockedStart = OGLRender.RenderBufferSize;
	// fog is initially turned off
	OGLRender.UseEnvCombineFog = false;
	if (InternalConfig.EXT_compiled_vertex_array)
	{
		// Initialise to start at the first element of the buffer
		glVertexPointer(3, GL_FLOAT, 4 * sizeof(GLfloat), &OGLRender.TVertex[0]);
		glEnableClientState(GL_VERTEX_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, &OGLRender.TColor[0]);
		glEnableClientState( GL_COLOR_ARRAY );
		glReportError();
		if (InternalConfig.EXT_secondary_color && OpenGL.ColorAlphaUnit2 == NULL)
		{
			glEnableClientState(GL_SECONDARY_COLOR_ARRAY_EXT);
			glSecondaryColorPointerEXT(3, GL_FLOAT, 4 * sizeof(GLfloat), &OGLRender.TColor2[0]);
			glReportError();
		}
#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
		if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
		{
			glFogCoordPointerEXT(1, GL_FLOAT, &OGLRender.TFog[0]);
			glEnableClientState(GL_FOG_COORDINATE_ARRAY_EXT);
			glReportError();
		}
#endif
	}

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
	VERIFY_TEXTURE_ENABLED_STATE();

#ifdef OGL_DEBUG
    OGLRender.FrameTriangles = 0;
    OGLRender.MaxTriangles = 0;
    OGLRender.MaxSequencedTriangles = 0;
    OGLRender.OverallTriangles = 0;
    OGLRender.OverallRenderTriangleCalls = 0;
    OGLRender.OverallLines = 0;
    OGLRender.OverallPoints = 0;
    OGLRender.MinX = OGLRender.MinY = OGLRender.MinZ = OGLRender.MinW = 99999999.0f;
    OGLRender.MaxX = OGLRender.MaxY = OGLRender.MaxZ = OGLRender.MaxW = -99999999.0f;
    OGLRender.MinS = OGLRender.MinT = OGLRender.MinF = 99999999.0f;
    OGLRender.MaxS = OGLRender.MaxT = OGLRender.MaxF = -99999999.0f;

    OGLRender.MinR = OGLRender.MinG = OGLRender.MinB = OGLRender.MinA = 99999999.0f;
    OGLRender.MaxR = OGLRender.MaxG = OGLRender.MaxB = OGLRender.MaxA = -99999999.0f;
#endif
}

// Shuts down the renderer and frees memory
void RenderFree(void)
{
#ifdef OGL_ALL
    GlideMsg("RenderFree()\n");
#endif

	// free triangle buffers
	FreeBuffer(OGLRender.TColor);
	if (OGLRender.TColor2) FreeBuffer(OGLRender.TColor2);
	FreeBuffer(OGLRender.TTexture);
	FreeBuffer(OGLRender.TVertex);
	FreeBuffer(OGLRender.TFog);
}

void RenderDrawTriangles_ImmediateMode(bool use_two_tex)
{
	glReportErrors("RenderDrawTriangles_ImmediateMode");
		
	if(!OpenGL.Blend && OpenGL.ChromaKey)
	{
		// Render only the color, not the alpha
		glBegin( GL_TRIANGLES );
		const int count = OGLRender.BufferStart + OGLRender.NumberOfTriangles;
		for (int i = OGLRender.BufferStart; i < count; i++ )
		{
			glColor3fv( &OGLRender.TColor[ i ].ar );
			if (InternalConfig.EXT_secondary_color)
			{
				glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].ar );
			}
			if (OpenGL.Texture)
			{
				glTexCoord4fv( &OGLRender.TTexture[ i ].as );
				if (OpenGL.ColorAlphaUnit2)
				{
					glMultiTexCoord4fvARB(OpenGL.ColorAlphaUnit2, &OGLRender.TTexture[ i ].as);
				}
			}
			if (OpenGL.Fog)
			{
				if (OGLRender.UseEnvCombineFog)
				{
					glMultiTexCoord1fARB(OpenGL.FogTextureUnit, OGLRender.TFog[ i ].af);
				}
				#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
					else if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
					{
						glFogCoordfEXT( OGLRender.TFog[ i ].af );
					}
				#endif
			}
			glVertex3fv( &OGLRender.TVertex[ i ].ax );

			glColor3fv( &OGLRender.TColor[ i ].br );
			if (InternalConfig.EXT_secondary_color)
			{
				glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].br );
			}
			if (OpenGL.Texture)
			{
				glTexCoord4fv( &OGLRender.TTexture[ i ].bs );
				if (OpenGL.ColorAlphaUnit2)
				{
					glMultiTexCoord4fvARB(OpenGL.ColorAlphaUnit2, &OGLRender.TTexture[ i ].bs);
				}
			}
			if (OpenGL.Fog)
			{
				if (OGLRender.UseEnvCombineFog)
				{
					glMultiTexCoord1fARB(OpenGL.FogTextureUnit, OGLRender.TFog[ i ].bf);
				}
				#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
					else if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
					{
						glFogCoordfEXT( OGLRender.TFog[ i ].bf );
					}
				#endif
			}
			glVertex3fv( &OGLRender.TVertex[ i ].bx );

			glColor3fv( &OGLRender.TColor[ i ].cr );
			if (InternalConfig.EXT_secondary_color)
			{
				glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].cr );
			}
			if (OpenGL.Texture)
			{
				glTexCoord4fv(&OGLRender.TTexture[ i ].cs);
				if (OpenGL.ColorAlphaUnit2)
				{
					glMultiTexCoord4fvARB(OpenGL.ColorAlphaUnit2, &OGLRender.TTexture[ i ].cs);
				}
			}
			if (OpenGL.Fog)
			{
				if (OGLRender.UseEnvCombineFog)
				{
					glMultiTexCoord1fARB(OpenGL.FogTextureUnit, OGLRender.TFog[ i ].cf);
				}
				#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
					else if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
					{
						glFogCoordfEXT( OGLRender.TFog[ i ].cf );
					}
				#endif
			}
			glVertex3fv( &OGLRender.TVertex[ i ].cx );
		}
		glEnd( );
		glReportError();
	}
	else
	{
		glBegin( GL_TRIANGLES );
		const int count = OGLRender.BufferStart + OGLRender.NumberOfTriangles;
		for (int i = OGLRender.BufferStart; i < count; i++ )
		{
			glColor4fv( &OGLRender.TColor[ i ].ar );
			if (InternalConfig.EXT_secondary_color)
			{
				glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].ar );
			}
			if (OpenGL.Texture)
			{
				glTexCoord4fv( &OGLRender.TTexture[ i ].as );
				if (OpenGL.ColorAlphaUnit2 || use_two_tex )
				{
					glMultiTexCoord4fvARB( OpenGL.ColorAlphaUnit1 + 1, &OGLRender.TTexture[ i ].as );
				}
			}
			if (OpenGL.Fog)
			{
				if (OGLRender.UseEnvCombineFog)
				{
					glMultiTexCoord1fARB( OpenGL.FogTextureUnit, OGLRender.TFog[ i ].af );
				}
				#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
					else if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
					{
						glFogCoordfEXT( OGLRender.TFog[ i ].af );
					}
				#endif
			}
			glVertex3fv( &OGLRender.TVertex[ i ].ax );

			glColor4fv( &OGLRender.TColor[ i ].br );
			if (InternalConfig.EXT_secondary_color)
			{
				glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].br );
			}
			if (OpenGL.Texture)
			{
				glTexCoord4fv( &OGLRender.TTexture[ i ].bs );
				if (OpenGL.ColorAlphaUnit2 || use_two_tex )
				{
					glMultiTexCoord4fvARB( OpenGL.ColorAlphaUnit1 + 1, &OGLRender.TTexture[ i ].bs );
				}
			}
			if (OpenGL.Fog)
			{
				if (OGLRender.UseEnvCombineFog)
				{
					glMultiTexCoord1fARB( OpenGL.FogTextureUnit, OGLRender.TFog[ i ].bf );
				}
				#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
					else if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
					{
						glFogCoordfEXT( OGLRender.TFog[ i ].bf );
					}
				#endif
			}
			glVertex3fv( &OGLRender.TVertex[ i ].bx );

			glColor4fv( &OGLRender.TColor[ i ].cr );
			if (InternalConfig.EXT_secondary_color)
			{
				glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].cr );
			}
			if (OpenGL.Texture)
			{
				glTexCoord4fv( &OGLRender.TTexture[ i ].cs );
				if (OpenGL.ColorAlphaUnit2 || use_two_tex )
				{
					glMultiTexCoord4fvARB( OpenGL.ColorAlphaUnit1 + 1, &OGLRender.TTexture[ i ].cs );
				}
			}
			if (OpenGL.Fog)
			{
				if (OGLRender.UseEnvCombineFog)
				{
					glMultiTexCoord1fARB( OpenGL.FogTextureUnit, OGLRender.TFog[ i ].cf );
				}
				#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
					else if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
					{
						glFogCoordfEXT( OGLRender.TFog[ i ].cf );
					}
				#endif
			}
			glVertex3fv( &OGLRender.TVertex[ i ].cx );
		}
		glEnd( );
		glReportError();
	}
}

inline GLfloat dist(GLfloat ax, GLfloat ay, GLfloat bx, GLfloat by)
{
	const GLfloat ad = ax - bx;
	const GLfloat bd = ay - by;
	return sqrt(ad * ad + bd * bd);
}

inline void GapFix(const TVertexStruct* u, TVertexStruct* v, GLfloat p)
{
	// Enlarge the triangle by enlarging the incircle of the triangle by p.
	// The enlarged triangle will close background pixel-gaps between landscape tiles
	// as seen in TR1 & TR2.
	// For p = 1.0, horizontal and vertical one-pixel wide gaps can be closed 
	// by drawing just lines, whereas for p > 1 the whole triangle must be repainted.
	// (preperably with disabling zbuffer writes and drawing behind the original
	// triangle with z=OpenGL.ZMin)
	// Background: http://de.wikipedia.org/wiki/Inkreis
	// Many thanks to Yves Edel for providing the mathematical background
	//
	// The incircle:
	// let r=sqrt({(s-a)(s-b)(s-c)}/s)
	// with
	// s = (a+b+c)/2
	// Centre of the incircle:
	// M = (aA+bB+cC) / (a+b+c)
	// (with barycentric coordinates)
	// New vertices of the triangle
	// A' = A+(A-M)* p / r.
	const GLfloat a = dist(u->bx, u->by, u->cx, u->cy);
	const GLfloat b = dist(u->ax, u->ay, u->cx, u->cy);
	const GLfloat c = dist(u->ax, u->ay, u->bx, u->by);
	const GLfloat abc = a + b + c;
	const GLfloat s = abc / 2;
	const GLfloat r = sqrt( (s-a)*(s-b)*(s-c) / s );
	// Enlarge the radius by a factor derived from the avarage vertex depth
	// As a result, triangles in the foreground must have a larger incircle than
	// the ones in the background in order to apply the gapfix.
	GLfloat zr;
	if (InternalConfig.GapFix & OpenGLideGapFixFlag_DepthFactor)
	{
		GLfloat z;
		if (InternalConfig.PrecisionFix == false &&  OpenGL.DepthBufferType == 0)
		{
			z = (precision_fix(u->az) +
			     precision_fix(u->bz) +
				 precision_fix(u->cz)) / 3.0f;
		}
		else
		{
			z= (u->az + u->bz + u->cz) / 3.0f;
		}
		// Now z is [0...1] and the precision fix has been applied in all cases,
		// and the average z in TR1 is about 0.34
		// (and we can use the same depth factor regardless of the precision fix state)
		zr = InternalConfig.GapFixDepthFactor * z;
	}
	else
	{
		zr = 1.0f;
	}
	// Filter artecfacts when rendering geometry in front of "holes", which means 
	// all fragments produced by glClear() and not painted over by other geometry
	const GLfloat r1 = InternalConfig.GapFixParam1;
	const GLfloat gs = InternalConfig.GapFixParam2;
	const GLfloat r2 = InternalConfig.GapFixParam3;
	// Apply the gapfix:
	// smaller z values allow smaller triangles to pass the gapfix test
	const OpenGLideGapFixFlags g = InternalConfig.GapFix;
	if (
	    ((g & OpenGLideGapFixFlag_IncircleOr) &&
	     (r > r1 * zr || abc < gs * r))
	 || ((g & OpenGLideGapFixFlag_IncircleAnd) &&
	     (r > r1 * zr && abc < gs * r))
	     // r1 > r2 or the formula will degenerate to r > r2
	 || ((g & OpenGLideGapFixFlag_IncircleSecondRadius) &&
	     ((r > r1 * zr || abc < gs * r) && r > r2 * zr))
	 || ((g & OpenGLideGapFixFlag_VertexLengthSecondRadius) &&
	     ((r > r1 * zr || max(a, max(b, c)) > gs) && r > r2 * zr))
	   )
	{
		const GLfloat Mx = (a * u->ax + b * u->bx + c * u->cx) / abc;
		const GLfloat My = (a * u->ay + b * u->by + c * u->cy) / abc;
		// Transform coordinates to fill the gaps
		v->ax = u->ax + (u->ax - Mx) * p / r;
		v->ay = u->ay + (u->ay - My) * p / r; 
		v->bx = u->bx + (u->bx - Mx) * p / r;
		v->by = u->by + (u->by - My) * p / r;
		v->cx = u->cx + (u->cx - Mx) * p / r;
		v->cy = u->cy + (u->cy - My) * p / r;
	}
	else if (u != v)
	{
		*v = *u;
	}
}

void RenderDrawTriangles_impl( void )
{
	glReportErrors("RenderDrawTriangles");
	#ifdef OGL_OPTIMISE_DEBUG
		GlideMsg("RenderDrawTriangles(): %d\n", OGLRender.NumberOfTriangles); 
	#else
		#ifdef OGL_ALL
		    GlideMsg( "RenderDrawTriangles()\n");
		#endif
	#endif

	// Finish rendering of the last buffer
	RenderUnlockArrays();
	s_Framebuffer.OnRenderDrawTriangles();
	// Update the state after processing the frame buffer because
	// the framebuffer class might request state changes as well
	// in order to restore the previous state
	RenderUpdateState();
	bool use_two_tex = false;
	if (OpenGL.Texture)
	{
		use_two_tex = Textures->MakeReady(&OGLRender.TTexture[OGLRender.BufferStart], OGLRender.NumberOfTriangles);
		if (use_two_tex)
		{
			glActiveTextureARB(OpenGL.ColorAlphaUnit1 + 1);
			glEnable(GL_TEXTURE_2D);
			if (InternalConfig.EXT_compiled_vertex_array)
			{
				glClientActiveTextureARB(OpenGL.ColorAlphaUnit1 + 1);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(4, GL_FLOAT, 0, &OGLRender.TTexture[0]);
			}
			glActiveTextureARB(OpenGL.ColorAlphaUnit1);
			glReportError();
		}
	}
	// Provide dummy coordinates when fog is turned off but
	// the texture unit is on because of color/alpha inversion
	if (OGLRender.UseEnvCombineFog && OpenGL.Fog == false)
	{
		int buffer_end = OGLRender.BufferStart + OGLRender.NumberOfTriangles;
		TFogStruct* pF = OGLRender.TFog;
		for (int index = OGLRender.BufferStart; index < buffer_end; index++)
		{
			pF[index].af =
			pF[index].bf =
			pF[index].cf = 0.0f;
		}
	}
	// Render the triangles
	const bool use_compiled_vertex_arrays = InternalConfig.EXT_compiled_vertex_array
		// && OGLRender.NumberOfTriangles > 1 // Might be more optimal
		;
	if (use_compiled_vertex_arrays)
	{
		// Remember the position of the currently locked buffer
		OGLRender.BufferLockedStart = OGLRender.BufferStart;
		// Continue rendering in next buffer
		OGLRender.BufferStart += OGLRender.NumberOfTriangles;
		// choose the largest buffer
		if (OGLRender.BufferStart > OGLRender.RenderBufferSize / 2
		 || OGLRender.BufferStart >= OGLRender.RenderBufferSize -1)
		{
			OGLRender.BufferStart = 0;
		}
		glLockArraysEXT(OGLRender.BufferLockedStart * 3, OGLRender.NumberOfTriangles * 3);
		OGLRender.BufferLocked = true;
		glDrawArrays(GL_TRIANGLES, OGLRender.BufferLockedStart * 3, OGLRender.NumberOfTriangles * 3);
	}
	else
	{
		RenderDrawTriangles_ImmediateMode(use_two_tex);
	}
	// Fill gaps?
	if ((InternalConfig.GapFix & OpenGLideGapFixFlag_Enabled) && !OpenGL.Blend)
	{
		// Only in 3D scenery
		if (OpenGL.DepthBufferWritting)
		{
			if (use_compiled_vertex_arrays)
			{
				RenderUnlockArrays();
			}
			// Enlarge triangles to fill the gaps between tiles in TR1 & TR2
			const int buffer_end = OGLRender.BufferLockedStart + OGLRender.NumberOfTriangles;
			for (int index = OGLRender.BufferLockedStart; index < buffer_end; index++)
			{
				TVertexStruct* t = &OGLRender.TVertex[index];
				const GLfloat p = 1.0;
				GapFix(t, t, p);
			}
			// Turn off z-buffer writes
			// disable depth buffer because although the stencil buffer is used,
			// gapfix striangles may be rendered first and end up in front of
			// regular triangles
			glDepthMask(false);
			glDepthFunc(GL_EQUAL);
			glDisable(GL_DEPTH_TEST);
			glStencilFunc(GL_NOTEQUAL, 0x02, 0x03);
			glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
			glReportError();
			if (use_compiled_vertex_arrays)
			{
				glLockArraysEXT(OGLRender.BufferLockedStart * 3, OGLRender.NumberOfTriangles * 3);
				OGLRender.BufferLocked = true;
				glDrawArrays(GL_TRIANGLES, OGLRender.BufferLockedStart * 3, OGLRender.NumberOfTriangles * 3);
				glReportError();
			}
			else
			{
				RenderDrawTriangles_ImmediateMode(use_two_tex);			
			}
			// restore previous state
			glDepthMask(OpenGL.DepthBufferWritting);
			glDepthFunc(OpenGL.DepthFunction);
			if (Glide.State.DepthBufferMode != GR_DEPTHBUFFER_DISABLE)
			{
				glEnable(GL_DEPTH_TEST);
			}
			glStencilFunc(GL_ALWAYS, 0x02, 0x03);
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
			glReportError();
		}
	}
	if ( use_two_tex )
	{
		glActiveTextureARB(OpenGL.ColorAlphaUnit1 + 1);
		if (InternalConfig.EXT_compiled_vertex_array)
		{
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer( 4, GL_FLOAT, 0, NULL);
			glClientActiveTextureARB(OpenGL.ColorAlphaUnit1);
		}
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(OpenGL.ColorAlphaUnit1);
		glReportError();
	}

#ifdef OGL_DEBUG
	if ( OGLRender.NumberOfTriangles > OGLRender.MaxSequencedTriangles )
	{
		OGLRender.MaxSequencedTriangles = OGLRender.NumberOfTriangles;
	}
	OGLRender.OverallTriangles += OGLRender.NumberOfTriangles;
	OGLRender.OverallRenderTriangleCalls++;
#endif

	OGLRender.NumberOfTriangles = 0;
	// Render the front buffer after each triangle sequence
	// (However, rendering each triangle seperately would be more correct)
	if (Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER)
	{
		glFlush();
	}
	s_Framebuffer.SetRenderBufferChanged();
	// Clipping is turned off here per default to save the state-change call
	// for unclipped triangles. Usally this never triggers, so it's called
	// once per triangle sequence.
	// This allows for one call per triangle sequence + one call per clipped
	//  triangle instead of one call for any (even unclipped) triangle
	SetClipVerticesState(false);

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}

void RenderAddLine( const GrVertex *a, const GrVertex *b, bool unsnap )
{
#ifdef OGL_DEBUG
	OGLRender.OverallLines++;
#endif
#ifdef OGL_ALL
    GlideMsg( "RenderAddLine()\n");
#endif

	glReportErrors("RenderAddLine");

	// Peek to avoid updating the render state twice
	if (OGLRender.NumberOfTriangles)
	{
		RenderDrawTriangles_impl();
	}
	else
	{
		// Finish rendering of last render buffer
		RenderUnlockArrays();
		s_Framebuffer.OnRenderDrawTriangles();
		// Update the state after processing the frame buffer because
		// the framebuffer class might request state changes as well
		// in order to restore the previous state
		RenderUpdateState();
	}
	
	TColorStruct* pC = &OGLRender.TColor[OGLRender.RenderBufferSize];
	TColorStruct* pC2;
	const GlideState glidestate = Glide.State;
	if (OpenGL.ColorAlphaUnit2)
	{
		{
			// Color
			pC->ar = a->r * D1OVER255;
			pC->ag = a->g * D1OVER255;
			pC->ab = a->b * D1OVER255;
			pC->br = b->r * D1OVER255;
			pC->bg = b->g * D1OVER255;
			pC->bb = b->b * D1OVER255;
		}
		// Alpha
		if (glidestate.AlphaLocal == GR_COMBINE_LOCAL_DEPTH)
		{
			// @todo: find out whether z has to be divided by 255 or by 65535
			pC->aa = a->z * D1OVER255;
			pC->ba = b->z * D1OVER255;
		}
		else
		{
			pC->aa = a->a * D1OVER255;
			pC->ba = b->a * D1OVER255;
		}
	}
	else
	{
		pC2 = &OGLRender.TColor2[OGLRender.RenderBufferSize];
		memset( pC2, 0, sizeof( TColorStruct ) );
		// Color Stuff, need to optimize it
		if ( Glide.ALocal )
		{
		    switch (glidestate.AlphaLocal)
		    {
		    case GR_COMBINE_LOCAL_ITERATED:
		        Local.aa = a->a * D1OVER255;
		        Local.ba = b->a * D1OVER255;
		        break;
		
		    case GR_COMBINE_LOCAL_CONSTANT:
		        Local.aa = Local.ba = OpenGL.ConstantColor[3];
		        break;
		
		    case GR_COMBINE_LOCAL_DEPTH:
		        Local.aa = a->z;
		        Local.ba = b->z;
		        break;
		    }
		}

		if ( Glide.AOther )
		{
		    switch (glidestate.AlphaOther)
		    {
		    case GR_COMBINE_OTHER_ITERATED:
		        Other.aa = a->a * D1OVER255;
		        Other.ba = b->a * D1OVER255;
		        break;
		
		    case GR_COMBINE_OTHER_CONSTANT:
		        Other.aa = Other.ba = OpenGL.ConstantColor[3];
		        break;
		
		    case GR_COMBINE_OTHER_TEXTURE:
		        Other.aa = Other.ba = 1.0f;
		        break;
		    }
		}

		if ( Glide.CLocal )
		{
		    switch (glidestate.ColorCombineLocal)
		    {
		    case GR_COMBINE_LOCAL_ITERATED:
		        Local.ar = a->r * D1OVER255;
		        Local.ag = a->g * D1OVER255;
		        Local.ab = a->b * D1OVER255;
		        Local.br = b->r * D1OVER255;
		        Local.bg = b->g * D1OVER255;
		        Local.bb = b->b * D1OVER255;
		        break;
		
		    case GR_COMBINE_LOCAL_CONSTANT:
					{
						GLfloat* color;
						if (glidestate.Delta0Mode)
						{
							color = &OpenGL.Delta0Color[0];
						}
						else
						{
							color = &OpenGL.ConstantColor[0];
						}
						Local.ar = Local.br = color[0];
						Local.ag = Local.bg = color[1];
						Local.ab = Local.bb = color[2];
					}
					break;
		    }
		}

		if ( Glide.COther )
		{
		    switch (glidestate.ColorCombineOther)
		    {
		    case GR_COMBINE_OTHER_ITERATED:
		        Other.ar = a->r * D1OVER255;
		        Other.ag = a->g * D1OVER255;
		        Other.ab = a->b * D1OVER255;
		        Other.br = b->r * D1OVER255;
		        Other.bg = b->g * D1OVER255;
		        Other.bb = b->b * D1OVER255;
		        break;
		
		    case GR_COMBINE_OTHER_CONSTANT:
					{
						GLfloat* color;
						if (glidestate.Delta0Mode)
						{
							color = &OpenGL.Delta0Color[0];
						}
						else
						{
							color = &OpenGL.ConstantColor[0];
						}
						Other.ar = Other.br = color[0];
						Other.ag = Other.bg = color[1];
						Other.ab = Other.bb = color[2];
					}
					break;
		
		    case GR_COMBINE_OTHER_TEXTURE:
		        Other.ar = Other.ag = Other.ab = 1.0f;
		        Other.br = Other.bg = Other.bb = 1.0f;
		        break;
		    }
		}

		switch (glidestate.ColorCombineFunction)
		{
		case GR_COMBINE_FUNCTION_ZERO:
		    pC->ar = pC->ag = pC->ab = 0.0f; 
		    pC->br = pC->bg = pC->bb = 0.0f; 
		    break;
		
		case GR_COMBINE_FUNCTION_LOCAL:
		    pC->ar = Local.ar;
		    pC->ag = Local.ag;
		    pC->ab = Local.ab;
		    pC->br = Local.br;
		    pC->bg = Local.bg;
		    pC->bb = Local.bb;
		    break;
		
		case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
		    pC->ar = pC->ag = pC->ab = Local.aa;
		    pC->br = pC->bg = pC->bb = Local.ba;
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_OTHER:
		    ColorFactor3Func( &CFactor, &Local, &Other );
		    pC->ar = CFactor.ar * Other.ar;
		    pC->ag = CFactor.ag * Other.ag;
		    pC->ab = CFactor.ab * Other.ab;
		    pC->br = CFactor.br * Other.br;
		    pC->bg = CFactor.bg * Other.bg;
		    pC->bb = CFactor.bb * Other.bb;
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
		    ColorFactor3Func( &CFactor, &Local, &Other );
		    pC->ar = CFactor.ar * Other.ar;
		    pC->ag = CFactor.ag * Other.ag;
		    pC->ab = CFactor.ab * Other.ab;
		    pC->br = CFactor.br * Other.br;
		    pC->bg = CFactor.bg * Other.bg;
		    pC->bb = CFactor.bb * Other.bb;
		    pC2->ar = Local.ar;
		    pC2->ag = Local.ag;
		    pC2->ab = Local.ab;
		    pC2->br = Local.br;
		    pC2->bg = Local.bg;
		    pC2->bb = Local.bb;
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
		    ColorFactor3Func( &CFactor, &Local, &Other );
		    pC->ar = CFactor.ar * Other.ar;
		    pC->ag = CFactor.ag * Other.ag;
		    pC->ab = CFactor.ab * Other.ab;
		    pC->br = CFactor.br * Other.br;
		    pC->bg = CFactor.bg * Other.bg;
		    pC->bb = CFactor.bb * Other.bb;
		    pC2->ar = pC2->ag = pC2->ab = Local.aa;
		    pC2->br = pC2->bg = pC2->bb = Local.ba;
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
		    ColorFactor3Func( &CFactor, &Local, &Other );
		    pC->ar = CFactor.ar * (Other.ar - Local.ar);
		    pC->ag = CFactor.ag * (Other.ag - Local.ag);
		    pC->ab = CFactor.ab * (Other.ab - Local.ab);
		    pC->br = CFactor.br * (Other.br - Local.br);
		    pC->bg = CFactor.bg * (Other.bg - Local.bg);
		    pC->bb = CFactor.bb * (Other.bb - Local.bb);
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
		    if (((glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA) ||
		         (glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB)) &&
		         (glidestate.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE)) 
		    {
		        pC->ar = Local.ar;
		        pC->ag = Local.ag;
		        pC->ab = Local.ab;
		        pC->br = Local.br;
		        pC->bg = Local.bg;
		        pC->bb = Local.bb;
		    }
		    else
		    {
		        ColorFactor3Func( &CFactor, &Local, &Other );
		        pC->ar = CFactor.ar * (Other.ar - Local.ar);
		        pC->ag = CFactor.ag * (Other.ag - Local.ag);
		        pC->ab = CFactor.ab * (Other.ab - Local.ab);
		        pC->br = CFactor.br * (Other.br - Local.br);
		        pC->bg = CFactor.bg * (Other.bg - Local.bg);
		        pC->bb = CFactor.bb * (Other.bb - Local.bb);
		        pC2->ar = Local.ar;
		        pC2->ag = Local.ag;
		        pC2->ab = Local.ab;
		        pC2->br = Local.br;
		        pC2->bg = Local.bg;
		        pC2->bb = Local.bb;
		    }
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
		    if (((glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA) ||
		         (glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB)) &&
		         (glidestate.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE))
		    {
		        pC->ar = pC->ag = pC->ab = Local.aa;
		        pC->br = pC->bg = pC->bb = Local.ba;
		    }
		    else
		    {
		        ColorFactor3Func( &CFactor, &Local, &Other );
		        pC->ar = CFactor.ar * (Other.ar - Local.ar);
		        pC->ag = CFactor.ag * (Other.ag - Local.ag);
		        pC->ab = CFactor.ab * (Other.ab - Local.ab);
		        pC->br = CFactor.br * (Other.br - Local.br);
		        pC->bg = CFactor.bg * (Other.bg - Local.bg);
		        pC->bb = CFactor.bb * (Other.bb - Local.bb);
		        pC2->ar = pC2->ag = pC2->ab = Local.aa;
		        pC2->br = pC2->bg = pC2->bb = Local.ba;
		    }
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
		    ColorFactor3Func( &CFactor, &Local, &Other );
		    pC->ar = ( 1.0f - CFactor.ar ) * Local.ar;
		    pC->ag = ( 1.0f - CFactor.ag ) * Local.ag;
		    pC->ab = ( 1.0f - CFactor.ab ) * Local.ab;
		    pC->br = ( 1.0f - CFactor.br ) * Local.br;
		    pC->bg = ( 1.0f - CFactor.bg ) * Local.bg;
		    pC->bb = ( 1.0f - CFactor.bb ) * Local.bb;
		    pC2->ar = Local.ar;
		    pC2->ag = Local.ag;
		    pC2->ab = Local.ab;
		    pC2->br = Local.br;
		    pC2->bg = Local.bg;
		    pC2->bb = Local.bb;
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
		    ColorFactor3Func( &CFactor, &Local, &Other );
		    pC->ar = CFactor.ar * -Local.ar;
		    pC->ag = CFactor.ag * -Local.ag;
		    pC->ab = CFactor.ab * -Local.ab;
		    pC->br = CFactor.br * -Local.br;
		    pC->bg = CFactor.bg * -Local.bg;
		    pC->bb = CFactor.bb * -Local.bb;
		    pC2->ar = pC2->ag = pC2->ab = Local.aa;
		    pC2->br = pC2->bg = pC2->bb = Local.ba;
		    break;
		}

		switch (glidestate.AlphaFunction)
		{
		case GR_COMBINE_FUNCTION_ZERO:
		    pC->aa = pC->ba = 0.0f;
		    break;
		
		case GR_COMBINE_FUNCTION_LOCAL:
		case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
		    pC->aa = Local.aa;
		    pC->ba = Local.ba;
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
		case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
		    pC->aa = ((1.0f - AlphaFactorFunc( Local.aa, Other.aa )) * Local.aa);
		    pC->ba = ((1.0f - AlphaFactorFunc( Local.ba, Other.ba )) * Local.ba);
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_OTHER:
		    pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa);
		    pC->ba = (AlphaFactorFunc( Local.ba, Other.ba ) * Other.ba);
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
		case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
		    pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa + Local.aa);
		    pC->ba = (AlphaFactorFunc( Local.ba, Other.ba ) * Other.ba + Local.ba);
		//      pC2->aa =  Local.aa;
		//      pC2->ba =  Local.ba;
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
		    pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa ));
		    pC->ba = (AlphaFactorFunc( Local.ba, Other.ba ) * ( Other.ba - Local.ba ));
		    break;
		
		case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
		case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
		    pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa ) + Local.aa);
		    pC->ba = (AlphaFactorFunc( Local.ba, Other.ba ) * ( Other.ba - Local.ba ) + Local.ba);
		//      pC2->aa =  Local.aa;
		//      pC2->ba =  Local.ba;
		    break;
		}

		if (OpenGL.FogTextureUnit == 0)
		{
			if (glidestate.ColorCombineInvert)
			{
					pC->ar = 1.0f - pC->ar - pC2->ar;
					pC->ag = 1.0f - pC->ag - pC2->ag;
					pC->ab = 1.0f - pC->ab - pC2->ab;
					pC->br = 1.0f - pC->br - pC2->br;
					pC->bg = 1.0f - pC->bg - pC2->bg;
					pC->bb = 1.0f - pC->bb - pC2->bb;
					pC2->ar = pC2->ag = pC2->ab = 0.0f;
					pC2->br = pC2->bg = pC2->bb = 0.0f;
			}

			if (glidestate.AlphaInvert)
			{
					pC->aa = 1.0f - pC->aa - pC2->aa;
					pC->ba = 1.0f - pC->ba - pC2->ba;
					pC2->aa = pC2->ba = 0.0f;
			}
		}
	}
	
	TVertexStruct* pV = &OGLRender.TVertex[OGLRender.RenderBufferSize];
	// Z-Buffering
	if ((glidestate.DepthBufferMode == GR_DEPTHBUFFER_DISABLE) || 
	    (glidestate.DepthBufferMode == GR_CMP_ALWAYS))
	{
		pV->az = 0.0f;
		pV->bz = 0.0f;
	}
	else 
	if ( OpenGL.DepthBufferType )
	{
		pV->az = a->ooz * D1OVER65536;
		pV->bz = b->ooz * D1OVER65536;
	}
	else
	{
		/*
		* For silly values of oow, depth buffering doesn't
		* seem to work, so map them to a sensible z.  When
		* games use these silly values, they probably don't
		* use z buffering anyway.
		*/
		if ( a->oow > 1.0 )
		{
			pV->az = pV->bz = 0.9f;
		}
		else 
		if ( InternalConfig.PrecisionFix )
		{
			pV->az = precision_fix(a->oow);
			pV->bz = precision_fix(b->oow);
		}
		else
		{
			pV->az = a->oow;
			pV->bz = b->oow;
		}
	}

 	if ( ( unsnap ) &&
	     ( a->x > vertex_snap_compare ) )
	{
		pV->ax = a->x - vertex_snap;
		pV->ay = a->y - vertex_snap;
		pV->bx = b->x - vertex_snap;
		pV->by = b->y - vertex_snap;
	}
	else
	{
		pV->ax = a->x;
		pV->ay = a->y;
		pV->bx = b->x;
		pV->by = b->y;
	}

	TTextureStruct* pTS = &OGLRender.TTexture[OGLRender.RenderBufferSize];
	if (OpenGL.Texture)
	{
		const float hAspect = Textures->GetHAspect();
		const float wAspect = Textures->GetWAspect();
		pTS->as = a->tmuvtx[0].sow * wAspect;
		pTS->at = a->tmuvtx[0].tow * hAspect;
		pTS->bs = b->tmuvtx[0].sow * wAspect;
		pTS->bt = b->tmuvtx[0].tow * hAspect;
		float atmuoow;
		float btmuoow;
		if ((glidestate.STWHint & GR_STWHINT_W_DIFF_TMU0) == 0)
		{
			atmuoow = a->oow;
			btmuoow = b->oow;
		}
		else
		{
			atmuoow = a->tmuvtx[ 0 ].oow;
			btmuoow = b->tmuvtx[ 0 ].oow;
		}
		pTS->aoow = atmuoow;
		pTS->boow = btmuoow;
		
#ifdef OGL_DEBUG_GLIDE_COORDS
		GlideMsg(a, atmuoow);
		GlideMsg(b, btmuoow);
#endif
	}
#ifdef OGL_DEBUG_GLIDE_COORDS
	else
	{
		GlideMsg(a, 1.0f);
		GlideMsg(b, 1.0f);
	}
#endif

	TFogStruct* pF = &OGLRender.TFog[OGLRender.RenderBufferSize];
	if (glidestate.FogMode)
	{
		if (glidestate.FogMode & GR_FOG_WITH_TABLE)
		{
			pF->af = (float)OpenGL.FogTable[ (FxU16)(1.0f / a->oow) ] * D1OVER255;
			pF->bf = (float)OpenGL.FogTable[ (FxU16)(1.0f / b->oow) ] * D1OVER255;
		}
		else
		{
			pF->af = a->a * D1OVER255;
			pF->bf = b->a * D1OVER255;
		}
		/*
		if ( glidestate.FogMode & GR_FOG_ADD2 )
		{
			pF->af = 1.0f - pF->af;
			pF->bf = 1.0f - pF->bf;
		}
		*/

#ifdef OGL_DEBUG
		DEBUG_MIN_MAX( pF->af, OGLRender.MaxF, OGLRender.MinF );
		DEBUG_MIN_MAX( pF->bf, OGLRender.MaxF, OGLRender.MinF );
#endif
	}
	else /* if (OGLRender.UseEnvCombineFog) */ // env combine fog is the default
	{
		// Must provide dummy coords if fog is turned off but
		// the texture unit is active because of inveting color/alpha
		pF->af = pF->bf = 0.0;
	}

#ifdef OGL_DEBUG
	DEBUG_MIN_MAX( pC->ar, OGLRender.MaxR, OGLRender.MinR );
	DEBUG_MIN_MAX( pC->br, OGLRender.MaxR, OGLRender.MinR );

	DEBUG_MIN_MAX( pC->ag, OGLRender.MaxG, OGLRender.MinG );
	DEBUG_MIN_MAX( pC->bg, OGLRender.MaxG, OGLRender.MinG );

	DEBUG_MIN_MAX( pC->ab, OGLRender.MaxB, OGLRender.MinB );
	DEBUG_MIN_MAX( pC->bb, OGLRender.MaxB, OGLRender.MinB );

	DEBUG_MIN_MAX( pC->aa, OGLRender.MaxA, OGLRender.MinA );
	DEBUG_MIN_MAX( pC->ba, OGLRender.MaxA, OGLRender.MinA );

	DEBUG_MIN_MAX( pV->az, OGLRender.MaxZ, OGLRender.MinZ );
	DEBUG_MIN_MAX( pV->bz, OGLRender.MaxZ, OGLRender.MinZ );

	DEBUG_MIN_MAX( pV->ax, OGLRender.MaxX, OGLRender.MinX );
	DEBUG_MIN_MAX( pV->bx, OGLRender.MaxX, OGLRender.MinX );

	DEBUG_MIN_MAX( pV->ay, OGLRender.MaxY, OGLRender.MinY );
	DEBUG_MIN_MAX( pV->by, OGLRender.MaxY, OGLRender.MinY );

	if (OpenGL.Texture)
	{
		DEBUG_MIN_MAX( pTS->as, OGLRender.MaxS, OGLRender.MinS );
		DEBUG_MIN_MAX( pTS->bs, OGLRender.MaxS, OGLRender.MinS );

		DEBUG_MIN_MAX( pTS->at, OGLRender.MaxT, OGLRender.MinT );
		DEBUG_MIN_MAX( pTS->bt, OGLRender.MaxT, OGLRender.MinT );
	}
#endif

#ifdef OGL_DEBUG_OPENGL_COORDS
	GlideMsg(pV, pTS);
#endif

	RenderUpdateState();
	if (OpenGL.Texture)
	{
		Textures->MakeReady();
	}

	glBegin( GL_LINES );
		OpenGL.ChromaKey ? glColor3fv( &pC->ar ) : glColor4fv( &pC->ar );
		if (InternalConfig.EXT_secondary_color)
		{
			glSecondaryColor3fvEXT( &pC2->ar );
		}
		if (OpenGL.Texture)
		{
			glTexCoord4fv( &pTS->as );
			if (OpenGL.ColorAlphaUnit2)
			{
				glMultiTexCoord4fvARB( OpenGL.ColorAlphaUnit2, &pTS->as );
			}
		}
		if (glidestate.FogMode)
		{
			if (OGLRender.UseEnvCombineFog)
			{
				glMultiTexCoord1fARB( OpenGL.FogTextureUnit, pF->af );
			}
	#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
			else if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
			{
				glFogCoordfEXT( pF->af );
			}
	#endif
		}
		glVertex3fv( &pV->ax );

		OpenGL.ChromaKey ? glColor3fv( &pC->br ) : glColor4fv( &pC->br );
		if (InternalConfig.EXT_secondary_color)
		{
			glSecondaryColor3fvEXT( &pC2->br );
		}
		if (OpenGL.Texture)
		{
			glTexCoord4fv( &pTS->bs );
			if (OpenGL.ColorAlphaUnit2)
			{
				glMultiTexCoord4fvARB( OpenGL.ColorAlphaUnit2, &pTS->bs );
			}
		}
		if (glidestate.FogMode)
		{
			if (OGLRender.UseEnvCombineFog)
			{
				glMultiTexCoord1fARB( OpenGL.FogTextureUnit, pF->bf );
			}
	#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
			if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
			{
				glFogCoordfEXT( pF->bf );
			}
	#endif
		}
		glVertex3fv( &pV->bx );
	glEnd();
	glReportError();

	s_Framebuffer.SetRenderBufferChanged();

	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}

void RenderAddTriangle( const GrVertex *a, const GrVertex *b, const GrVertex *c, bool unsnap )
{
	glReportErrors("RenderAddTriangle");
#ifdef OGL_DEBUG
	OGLRender.OverallTriangles++;
#endif

	const int TriangleIndex = OGLRender.BufferStart + OGLRender.NumberOfTriangles;

#ifdef OGL_ALL
    GlideMsg( "RenderAddTriangle(%d)\n", TriangleIndex);
#endif

	TColorStruct* pC = &OGLRender.TColor[TriangleIndex];
	TColorStruct* pC2;
	const GlideState glidestate = Glide.State;
	if (OpenGL.ColorAlphaUnit2)
	{
		{
			// Color
			pC->ar = a->r * D1OVER255;
			pC->ag = a->g * D1OVER255;
			pC->ab = a->b * D1OVER255;
			pC->br = b->r * D1OVER255;
			pC->bg = b->g * D1OVER255;
			pC->bb = b->b * D1OVER255;
			pC->cr = c->r * D1OVER255;
			pC->cg = c->g * D1OVER255;
			pC->cb = c->b * D1OVER255;
		}
		// Alpha
		if (glidestate.AlphaLocal == GR_COMBINE_LOCAL_DEPTH)
		{
			// @todo: find out whether z has to be divided by 255 or by 65535
			pC->aa = a->z * D1OVER255;
			pC->ba = b->z * D1OVER255;
			pC->ca = c->z * D1OVER255;
		}
		else
		{
			pC->aa = a->a * D1OVER255;
			pC->ba = b->a * D1OVER255;
			pC->ca = c->a * D1OVER255;
		}
	}
	else
	{
		pC2 = &OGLRender.TColor2[ TriangleIndex ];
		memset( pC2, 0, sizeof( TColorStruct ) );
		if ( Glide.ALocal )
		{
				switch (glidestate.AlphaLocal)
				{
				case GR_COMBINE_LOCAL_ITERATED:
						Local.aa = a->a * D1OVER255;
						Local.ba = b->a * D1OVER255;
						Local.ca = c->a * D1OVER255;
						break;

				case GR_COMBINE_LOCAL_CONSTANT:
						Local.aa = Local.ba = Local.ca = OpenGL.ConstantColor[ 3 ];
						break;

				case GR_COMBINE_LOCAL_DEPTH:
						Local.aa = a->z * D1OVER255;
						Local.ba = b->z * D1OVER255;
						Local.ca = c->z * D1OVER255;
						break;
				}
		}

		if ( Glide.AOther )
		{
				switch (glidestate.AlphaOther)
				{
				case GR_COMBINE_OTHER_ITERATED:
						Other.aa = a->a * D1OVER255;
						Other.ba = b->a * D1OVER255;
						Other.ca = c->a * D1OVER255;
						break;

				case GR_COMBINE_OTHER_CONSTANT:
						Other.aa = Other.ba = Other.ca = OpenGL.ConstantColor[ 3 ];
						break;

				case GR_COMBINE_OTHER_TEXTURE:
						if ( OpenGL.Texture )
						{
								Other.aa = Other.ba = Other.ca = 1.0f;
						}
						else
						{
								Other.aa = Other.ba = Other.ca = 0.0f;
						}
						break;
				}
		}

		if ( Glide.CLocal )
		{
				switch (glidestate.ColorCombineLocal)
				{
				case GR_COMBINE_LOCAL_ITERATED:
						Local.ar = a->r * D1OVER255;
						Local.ag = a->g * D1OVER255;
						Local.ab = a->b * D1OVER255;
						Local.br = b->r * D1OVER255;
						Local.bg = b->g * D1OVER255;
						Local.bb = b->b * D1OVER255;
						Local.cr = c->r * D1OVER255;
						Local.cg = c->g * D1OVER255;
						Local.cb = c->b * D1OVER255;
						break;

				case GR_COMBINE_LOCAL_CONSTANT:
					{
						GLfloat* color;
						if (glidestate.Delta0Mode)
						{
							color = &OpenGL.Delta0Color[0];
						}
						else
						{
							color = &OpenGL.ConstantColor[0];
						}
						Local.ar = Local.br = Local.cr = color[0];
						Local.ag = Local.bg = Local.cg = color[1];
						Local.ab = Local.bb = Local.cb = color[2];
					}
					break;
				}
		}

		if ( Glide.COther )
		{
				switch ( glidestate.ColorCombineOther )
				{
				case GR_COMBINE_OTHER_ITERATED:
						Other.ar = a->r * D1OVER255;
						Other.ag = a->g * D1OVER255;
						Other.ab = a->b * D1OVER255;
						Other.br = b->r * D1OVER255;
						Other.bg = b->g * D1OVER255;
						Other.bb = b->b * D1OVER255;
						Other.cr = c->r * D1OVER255;
						Other.cg = c->g * D1OVER255;
						Other.cb = c->b * D1OVER255;
						break;

				case GR_COMBINE_OTHER_CONSTANT:
					{
						GLfloat* color;
						if (glidestate.Delta0Mode)
						{
							color = &OpenGL.Delta0Color[0];
						}
						else
						{
							color = &OpenGL.ConstantColor[0];
						}
						Other.ar = Other.br = Other.cr = color[0];
						Other.ag = Other.bg = Other.cg = color[1];
						Other.ab = Other.bb = Other.cb = color[2];
					}
					break;

				case GR_COMBINE_OTHER_TEXTURE:
						if ( OpenGL.Texture )
						{
								Other.ar = Other.ag = Other.ab = 1.0f;
								Other.br = Other.bg = Other.bb = 1.0f;
								Other.cr = Other.cg = Other.cb = 1.0f;
						}
						else
						{
								Other.ar = Other.ag = Other.ab = 0.0f;
								Other.br = Other.bg = Other.bb = 0.0f;
								Other.cr = Other.cg = Other.cb = 0.0f;
						}
						break;
				}
		}

		ColorFunctionFunc( pC, pC2, &Local, &Other );

		switch (glidestate.AlphaFunction)
		{
		case GR_COMBINE_FUNCTION_ZERO:
				pC->aa = pC->ba = pC->ca = 0.0f;
				break;

		case GR_COMBINE_FUNCTION_LOCAL:
		case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
				pC->aa = Local.aa;
				pC->ba = Local.ba;
				pC->ca = Local.ca;
				break;

		case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
		case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
				pC->aa = ( 1.0f - AlphaFactorFunc( Local.aa, Other.aa ) ) * Local.aa;
				pC->ba = ( 1.0f - AlphaFactorFunc( Local.ba, Other.ba ) ) * Local.ba;
				pC->ca = ( 1.0f - AlphaFactorFunc( Local.ca, Other.ca ) ) * Local.ca;
				break;

		case GR_COMBINE_FUNCTION_SCALE_OTHER:
				pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa;
				pC->ba = AlphaFactorFunc( Local.ba, Other.ba ) * Other.ba;
				pC->ca = AlphaFactorFunc( Local.ca, Other.ca ) * Other.ca;
				break;

		case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
		case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
				pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa + Local.aa;
				pC->ba = AlphaFactorFunc( Local.ba, Other.ba ) * Other.ba + Local.ba;
				pC->ca = AlphaFactorFunc( Local.ca, Other.ca ) * Other.ca + Local.ca;
	//      pC2->aa =  Local.aa;
	//      pC2->ba =  Local.ba;
	//      pC2->ca =  Local.ca;
				break;

		case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
				pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa );
				pC->ba = AlphaFactorFunc( Local.ba, Other.ba ) * ( Other.ba - Local.ba );
				pC->ca = AlphaFactorFunc( Local.ca, Other.ca ) * ( Other.ca - Local.ca );
				break;

		case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
		case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
				pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa ) + Local.aa;
				pC->ba = AlphaFactorFunc( Local.ba, Other.ba ) * ( Other.ba - Local.ba ) + Local.ba;
				pC->ca = AlphaFactorFunc( Local.ca, Other.ca ) * ( Other.ca - Local.ca ) + Local.ca;
	//      pC2->aa =  Local.aa;
	//      pC2->ba =  Local.ba;
	//      pC2->ca =  Local.ca;
				break;
		}

		if (OpenGL.FogTextureUnit == 0)
		{
			if (glidestate.ColorCombineInvert)
			{
					pC->ar = 1.0f - pC->ar - pC2->ar;
					pC->ag = 1.0f - pC->ag - pC2->ag;
					pC->ab = 1.0f - pC->ab - pC2->ab;
					pC->br = 1.0f - pC->br - pC2->br;
					pC->bg = 1.0f - pC->bg - pC2->bg;
					pC->bb = 1.0f - pC->bb - pC2->bb;
					pC->cr = 1.0f - pC->cr - pC2->cr;
					pC->cg = 1.0f - pC->cg - pC2->cg;
					pC->cb = 1.0f - pC->cb - pC2->cb;
					pC2->ar = pC2->ag = pC2->ab = 0.0f;
					pC2->br = pC2->bg = pC2->bb = 0.0f;
					pC2->cr = pC2->cg = pC2->cb = 0.0f;
			}

			if (glidestate.AlphaInvert)
			{
					pC->aa = 1.0f - pC->aa - pC2->aa;
					pC->ba = 1.0f - pC->ba - pC2->ba;
					pC->ca = 1.0f - pC->ca - pC2->ca;
					pC2->aa = pC2->ba = pC2->ca = 0.0f;
			}
		}
	}

	TVertexStruct* pV = &OGLRender.TVertex[ TriangleIndex ];
	// Z-Buffering
	// @todo: check why this has been added , since it look s unnecessary
	if ((glidestate.DepthBufferMode == GR_DEPTHBUFFER_DISABLE) || 
	    (glidestate.DepthFunction == GR_CMP_ALWAYS))
	{
		pV->az = 0.0f;
		pV->bz = 0.0f;
		pV->cz = 0.0f;
	}
	else
	if ( OpenGL.DepthBufferType )
	{
		pV->az = a->ooz * D1OVER65536;
		pV->bz = b->ooz * D1OVER65536;
		pV->cz = c->ooz * D1OVER65536;
	}
	else
	{
		//
		// For silly values of oow, depth buffering doesn't
		// seem to work, so map them to a sensible z.  When
		// games use these silly values, they probably don't
		// use z buffering anyway.
		//
		if ( a->oow > 1.0 )
		{
			pV->az = pV->bz = pV->cz = 0.9f;
		}
		else 
		if ( InternalConfig.PrecisionFix )
		{
			pV->az = precision_fix(a->oow);
			pV->bz = precision_fix(b->oow);
			pV->cz = precision_fix(c->oow);
		}
		else
		{
			pV->az = a->oow;
			pV->bz = b->oow;
			pV->cz = c->oow;
		}
	}

	if ( ( unsnap ) &&
	     ( a->x > vertex_snap_compare ) )
	{
		pV->ax = a->x - vertex_snap;
		pV->ay = a->y - vertex_snap;
		pV->bx = b->x - vertex_snap;
		pV->by = b->y - vertex_snap;
		pV->cx = c->x - vertex_snap;
		pV->cy = c->y - vertex_snap;
	}
	else
	{
		pV->ax = a->x;
		pV->ay = a->y;
		pV->bx = b->x;
		pV->by = b->y;
		pV->cx = c->x;
		pV->cy = c->y;
	}

#ifdef OGL_DEBUG
	DEBUG_MIN_MAX( pC->ar, OGLRender.MaxR, OGLRender.MinR );
	DEBUG_MIN_MAX( pC->br, OGLRender.MaxR, OGLRender.MinR );
	DEBUG_MIN_MAX( pC->cr, OGLRender.MaxR, OGLRender.MinR );
	  
	DEBUG_MIN_MAX( pC->ag, OGLRender.MaxG, OGLRender.MinG );
	DEBUG_MIN_MAX( pC->bg, OGLRender.MaxG, OGLRender.MinG );
	DEBUG_MIN_MAX( pC->cg, OGLRender.MaxG, OGLRender.MinG );
	
	DEBUG_MIN_MAX( pC->ab, OGLRender.MaxB, OGLRender.MinB );
	DEBUG_MIN_MAX( pC->bb, OGLRender.MaxB, OGLRender.MinB );
	DEBUG_MIN_MAX( pC->cb, OGLRender.MaxB, OGLRender.MinB );
	
	DEBUG_MIN_MAX( pC->aa, OGLRender.MaxA, OGLRender.MinA );
	DEBUG_MIN_MAX( pC->ba, OGLRender.MaxA, OGLRender.MinA );
	DEBUG_MIN_MAX( pC->ca, OGLRender.MaxA, OGLRender.MinA );
	
	DEBUG_MIN_MAX( pV->az, OGLRender.MaxZ, OGLRender.MinZ );
	DEBUG_MIN_MAX( pV->bz, OGLRender.MaxZ, OGLRender.MinZ );
	DEBUG_MIN_MAX( pV->cz, OGLRender.MaxZ, OGLRender.MinZ );
	
	DEBUG_MIN_MAX( pV->ax, OGLRender.MaxX, OGLRender.MinX );
	DEBUG_MIN_MAX( pV->bx, OGLRender.MaxX, OGLRender.MinX );
	DEBUG_MIN_MAX( pV->cx, OGLRender.MaxX, OGLRender.MinX );
	
	DEBUG_MIN_MAX( pV->ay, OGLRender.MaxY, OGLRender.MinY );
	DEBUG_MIN_MAX( pV->by, OGLRender.MaxY, OGLRender.MinY );
	DEBUG_MIN_MAX( pV->cy, OGLRender.MaxY, OGLRender.MinY );

 	OGLRender.FrameTriangles++;
#endif

	bool generate_subtextures = OpenGL.Texture;
	if (OpenGL.Texture)
	{
		float atmuoow;
		float btmuoow;
		float ctmuoow;
		if ((glidestate.STWHint & GR_STWHINT_W_DIFF_TMU0) == 0)
		{
			atmuoow = a->oow;
			btmuoow = b->oow;
			ctmuoow = c->oow;
		}
		else
		{
			atmuoow = a->tmuvtx[ 0 ].oow;
			btmuoow = b->tmuvtx[ 0 ].oow;
			ctmuoow = c->tmuvtx[ 0 ].oow;
		}
		const float maxoow = 1.0f / max( atmuoow, max( btmuoow, ctmuoow ) );
		generate_subtextures = InternalConfig.GenerateSubTextures &&
		                       OpenGL.SClampMode != GL_REPEAT &&
		                       OpenGL.TClampMode != GL_REPEAT;
		TTextureStruct* pTS = &OGLRender.TTexture[TriangleIndex];
		if (generate_subtextures)
		{
			pTS->as = a->tmuvtx[ 0 ].sow / atmuoow;
			pTS->at = a->tmuvtx[ 0 ].tow / atmuoow;
			pTS->bs = b->tmuvtx[ 0 ].sow / btmuoow;
			pTS->bt = b->tmuvtx[ 0 ].tow / btmuoow;
			pTS->cs = c->tmuvtx[ 0 ].sow / ctmuoow;
			pTS->ct = c->tmuvtx[ 0 ].tow / ctmuoow;
		}
		else
		{
			const GLfloat hAspect = Textures->GetHAspect();
			const GLfloat wAspect = Textures->GetWAspect();
			pTS->as = a->tmuvtx[ 0 ].sow * wAspect * maxoow;
			pTS->at = a->tmuvtx[ 0 ].tow * hAspect * maxoow;
			pTS->bs = b->tmuvtx[ 0 ].sow * wAspect * maxoow;
			pTS->bt = b->tmuvtx[ 0 ].tow * hAspect * maxoow;
			pTS->cs = c->tmuvtx[ 0 ].sow * wAspect * maxoow;
			pTS->ct = c->tmuvtx[ 0 ].tow * hAspect * maxoow;
		}
		pTS->aoow = atmuoow * maxoow;
		pTS->boow = btmuoow * maxoow;
		pTS->coow = ctmuoow * maxoow;

#ifdef OGL_DEBUG
		DEBUG_MIN_MAX( pTS->as, OGLRender.MaxS, OGLRender.MinS );
		DEBUG_MIN_MAX( pTS->bs, OGLRender.MaxS, OGLRender.MinS );
		DEBUG_MIN_MAX( pTS->cs, OGLRender.MaxS, OGLRender.MinS );
		
		DEBUG_MIN_MAX( pTS->at, OGLRender.MaxT, OGLRender.MinT );
		DEBUG_MIN_MAX( pTS->bt, OGLRender.MaxT, OGLRender.MinT );
		DEBUG_MIN_MAX( pTS->ct, OGLRender.MaxT, OGLRender.MinT );
#endif
		
#ifdef OGL_DEBUG_GLIDE_COORDS
		GlideMsg(a, maxoow);
		GlideMsg(b, maxoow);
		GlideMsg(c, maxoow);
#endif
#ifdef OGL_DEBUG_OPENGL_COORDS
		GlideMsg(pV, pTS);
#endif
	}
#if defined(OGL_DEBUG_GLIDE_COORDS) || defined(OGL_DEBUG_OPENGL_COORDS)
	else
	{
#ifdef OGL_DEBUG_GLIDE_COORDS
		GlideMsg(a, 1.0f);
		GlideMsg(b, 1.0f);
		GlideMsg(c, 1.0f);
#endif
#ifdef OGL_DEBUG_OPENGL_COORDS
	GlideMsg(pV);
#endif
#if defined(OGL_DEBUG_GLIDE_COORDS) || defined(OGL_DEBUG_OPENGL_COORDS)
	}
#endif
#endif
	
	TFogStruct* pF = &OGLRender.TFog[TriangleIndex];
	if(glidestate.FogMode)
	{
		if (glidestate.FogMode & GR_FOG_WITH_TABLE)
		{
			pF->af = (float) OpenGL.FogTable[(FxU16)(1.0f / a->oow) ] * D1OVER255;
			pF->bf = (float) OpenGL.FogTable[(FxU16)(1.0f / b->oow) ] * D1OVER255;
			pF->cf = (float) OpenGL.FogTable[(FxU16)(1.0f / c->oow) ] * D1OVER255;
		}
		else
		{
			pF->af = a->a * D1OVER255;
			pF->bf = b->a * D1OVER255;
			pF->cf = c->a * D1OVER255;
		}
		/*
		if ( glidestate.FogMode & GR_FOG_ADD2 )
		{
			pF->af = 1.0f - pF->af;
			pF->bf = 1.0f - pF->bf;
			pF->cf = 1.0f - pF->cf;
		}
		*/
        
#ifdef OGL_DEBUG
		DEBUG_MIN_MAX( pF->af, OGLRender.MaxF, OGLRender.MinF );
		DEBUG_MIN_MAX( pF->bf, OGLRender.MaxF, OGLRender.MinF );
		DEBUG_MIN_MAX( pF->bf, OGLRender.MaxF, OGLRender.MinF );
#endif
	}

	OGLRender.NumberOfTriangles++;
	if (generate_subtextures)
	{
		RenderDrawTriangles();
	}
	else
	{
		// Check whether the current buffer is before the currently rendered buffer
		if (OGLRender.BufferLocked
		 && OGLRender.BufferStart == 0
		 && OGLRender.NumberOfTriangles >= OGLRender.BufferLockedStart)
		{
			// Finish drawing of current render buffer
			RenderUnlockArrays();
			OGLRender.BufferLocked = false;
			OGLRender.BufferLockedStart = OGLRender.RenderBufferSize;
			// Continue filling the buffer
		}
		else if (OGLRender.BufferStart + OGLRender.NumberOfTriangles >= OGLRender.RenderBufferSize - 1)
		{
			RenderDrawTriangles();
		}
	}
}

void RenderAddPoint( const GrVertex *a, bool unsnap )
{
#ifdef OGL_DEBUG
	OGLRender.OverallPoints++;
#endif
#ifdef OGL_ALL
    GlideMsg( "RenderAddPoint()\n");
#endif

	glReportErrors("RenderAddPoint");

	// Peek to avoid updating the render state twice
	if (OGLRender.NumberOfTriangles)
	{
		RenderDrawTriangles_impl();
	}
	else
	{
		// Finish rendering of last render buffer
		RenderUnlockArrays();
		s_Framebuffer.OnRenderDrawTriangles();
		// Update the state after processing the frame buffer because
		// the framebuffer class might request state changes as well
		// in order to restore the previous state
		RenderUpdateState();
	}

	TColorStruct* pC  = &OGLRender.TColor[OGLRender.RenderBufferSize];
	TColorStruct* pC2;
	const GlideState glidestate = Glide.State;
	if (OpenGL.ColorAlphaUnit2)
	{
		{
			// Color
			pC->ar = a->r * D1OVER255;
			pC->ag = a->g * D1OVER255;
			pC->ab = a->b * D1OVER255;
		}
		// Alpha
		if (glidestate.AlphaLocal == GR_COMBINE_LOCAL_DEPTH)
		{
			// @todo: find out whether z has to be divided by 255 or by 65535
			pC->aa = a->z * D1OVER255;
		}
		else
		{
			pC->aa = a->a * D1OVER255;
		}
	}
	else
	{
    pC2 = &OGLRender.TColor2[OGLRender.RenderBufferSize];
    memset( pC2, 0, sizeof( TColorStruct ) );
    // Color Stuff, need to optimize it
    if ( Glide.ALocal )
    {
        switch (glidestate.AlphaLocal)
        {
        case GR_COMBINE_LOCAL_ITERATED:
            Local.aa = a->a * D1OVER255;
            break;

        case GR_COMBINE_LOCAL_CONSTANT:
            Local.aa = OpenGL.ConstantColor[3];
            break;

        case GR_COMBINE_LOCAL_DEPTH:
            Local.aa = a->z;
            break;
        }
    }

    if ( Glide.AOther )
    {
        switch (glidestate.AlphaOther)
        {
        case GR_COMBINE_OTHER_ITERATED:
            Other.aa = a->a * D1OVER255;
            break;

        case GR_COMBINE_OTHER_CONSTANT:
            Other.aa = OpenGL.ConstantColor[3];
            break;

        case GR_COMBINE_OTHER_TEXTURE:
            Other.aa = 1.0f;
            break;
        }
    }

    if ( Glide.CLocal )
    {
        switch (glidestate.ColorCombineLocal)
        {
        case GR_COMBINE_LOCAL_ITERATED:
            Local.ar = a->r * D1OVER255;
            Local.ag = a->g * D1OVER255;
            Local.ab = a->b * D1OVER255;
            break;

        case GR_COMBINE_LOCAL_CONSTANT:
					{
						GLfloat* color;
						if (glidestate.Delta0Mode)
						{
							color = &OpenGL.Delta0Color[0];
						}
						else
						{
							color = &OpenGL.ConstantColor[0];
						}
						Local.ar = color[0];
						Local.ag = color[1];
						Local.ab = color[2];
					}
					break;
        }
    }

    if ( Glide.COther )
    {
        switch (glidestate.ColorCombineOther)
        {
        case GR_COMBINE_OTHER_ITERATED:
            Other.ar = a->r * D1OVER255;
            Other.ag = a->g * D1OVER255;
            Other.ab = a->b * D1OVER255;
            break;

        case GR_COMBINE_OTHER_CONSTANT:
					{
						GLfloat* color;
						if (glidestate.Delta0Mode)
						{
							color = &OpenGL.Delta0Color[0];
						}
						else
						{
							color = &OpenGL.ConstantColor[0];
						}
						Other.ar = color[0];
						Other.ag = color[1];
						Other.ab = color[2];
					}
					break;

        case GR_COMBINE_OTHER_TEXTURE:
            Other.ar = Other.ag = Other.ab = 1.0f;
            break;
        }
    }

    switch (glidestate.ColorCombineFunction)
    {
    case GR_COMBINE_FUNCTION_ZERO:
        pC->ar = pC->ag = pC->ab = 0.0f; 
        break;

    case GR_COMBINE_FUNCTION_LOCAL:
        pC->ar = Local.ar;
        pC->ag = Local.ag;
        pC->ab = Local.ab;
        break;

    case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
        pC->ar = pC->ag = pC->ab = Local.aa;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * Other.ar;
        pC->ag = CFactor.ag * Other.ag;
        pC->ab = CFactor.ab * Other.ab;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * Other.ar;
        pC->ag = CFactor.ag * Other.ag;
        pC->ab = CFactor.ab * Other.ab;
        pC2->ar = Local.ar;
        pC2->ag = Local.ag;
        pC2->ab = Local.ab;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * Other.ar;
        pC->ag = CFactor.ag * Other.ag;
        pC->ab = CFactor.ab * Other.ab;
        pC2->ar = pC2->ag = pC2->ab = Local.aa;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * (Other.ar - Local.ar);
        pC->ag = CFactor.ag * (Other.ag - Local.ag);
        pC->ab = CFactor.ab * (Other.ab - Local.ab);
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
        if ((( glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA ) ||
            ( glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB )) &&
            (  glidestate.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
        {
            pC->ar = Local.ar;
            pC->ag = Local.ag;
            pC->ab = Local.ab;
        }
        else
        {
            ColorFactor3Func( &CFactor, &Local, &Other );
            pC->ar = CFactor.ar * (Other.ar - Local.ar);
            pC->ag = CFactor.ag * (Other.ag - Local.ag);
            pC->ab = CFactor.ab * (Other.ab - Local.ab);
            pC2->ar = Local.ar;
            pC2->ag = Local.ag;
            pC2->ab = Local.ab;
        }
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        if (((glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA) ||
             (glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB)) &&
             (glidestate.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE)) 
        {
            pC->ar = pC->ag = pC->ab = Local.aa;
        }
        else
        {
            ColorFactor3Func( &CFactor, &Local, &Other );
            pC->ar = CFactor.ar * (Other.ar - Local.ar);
            pC->ag = CFactor.ag * (Other.ag - Local.ag);
            pC->ab = CFactor.ab * (Other.ab - Local.ab);
            pC2->ar = pC2->ag = pC2->ab = Local.aa;
        }
        break;

    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = ( 1.0f - CFactor.ar ) * Local.ar;
        pC->ag = ( 1.0f - CFactor.ag ) * Local.ag;
        pC->ab = ( 1.0f - CFactor.ab ) * Local.ab;
        pC2->ar = Local.ar;
        pC2->ag = Local.ag;
        pC2->ab = Local.ab;
        break;

    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * -Local.ar;
        pC->ag = CFactor.ag * -Local.ag;
        pC->ab = CFactor.ab * -Local.ab;
        pC2->ar = pC2->ag = pC2->ab = Local.aa;
        break;
    }

    switch (glidestate.AlphaFunction)
    {
    case GR_COMBINE_FUNCTION_ZERO:
        pC->aa = 0.0f;
        break;

    case GR_COMBINE_FUNCTION_LOCAL:
    case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
        pC->aa = Local.aa;
        break;

    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        pC->aa = ((1.0f - AlphaFactorFunc( Local.aa, Other.aa )) * Local.aa);
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER:
        pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa);
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
        pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa + Local.aa;
//      pC2->aa =  Local.aa;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
        pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa ));
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa ) + Local.aa;
//      pC2->aa =  Local.aa;
        break;
    }

		if (OpenGL.FogTextureUnit == 0)
		{
			if (glidestate.ColorCombineInvert)
			{
					pC->ar = 1.0f - pC->ar - pC2->ar;
					pC->ag = 1.0f - pC->ag - pC2->ag;
					pC->ab = 1.0f - pC->ab - pC2->ab;
					pC2->ar = pC2->ag = pC2->ab = 0.0f;
			}

			if (glidestate.AlphaInvert)
			{
					pC->aa = 1.0f - pC->aa - pC2->aa;
					pC2->aa = 0.0f;
			}
		}
	}

	TVertexStruct* pV = &OGLRender.TVertex[OGLRender.RenderBufferSize];
	// Z-Buffering
	if ((glidestate.DepthBufferMode == GR_DEPTHBUFFER_DISABLE) || 
	    (glidestate.DepthBufferMode == GR_CMP_ALWAYS))
	{
		pV->az = 0.0f;
	}
	else 
	if ( OpenGL.DepthBufferType )
	{
		pV->az = a->ooz * D1OVER65536;
	}
	else
	{
		//
		// For silly values of oow, depth buffering doesn't
		// seem to work, so map them to a sensible z.  When
		// games use these silly values, they probably don't
		// use z buffering anyway.
		//
		if ( a->oow > 1.0 )
		{
			pV->az = 0.9f;
		}
		else 
		if ( InternalConfig.PrecisionFix )
		{
			pV->az = precision_fix(a->oow);
		}
		else
		{
			pV->az = a->oow;
		}
	}

	if ( ( unsnap ) &&
	     ( a->x > vertex_snap_compare ) )
	{
		pV->ax = a->x - vertex_snap;
		pV->ay = a->y - vertex_snap;
	}
	else
	{
		pV->ax = a->x;
		pV->ay = a->y;
	}

	TTextureStruct* pTS = &OGLRender.TTexture[OGLRender.RenderBufferSize];
	if ( OpenGL.Texture )
	{
		pTS->as = a->tmuvtx[0].sow * Textures->GetWAspect();
		pTS->at = a->tmuvtx[0].tow * Textures->GetHAspect();
		pTS->aoow = a->oow;
#ifdef OGL_DEBUG_GLIDE_COORDS
		GlideMsg(a, a->oow);
#endif
  }
#ifdef OGL_DEBUG_GLIDE_COORDS
	else
	{
		GlideMsg(a, 1.0f);
	}
#endif
		
	TFogStruct* pF = &OGLRender.TFog[OGLRender.RenderBufferSize];
	if(glidestate.FogMode)
	{
		if (glidestate.FogMode & GR_FOG_WITH_TABLE)
		{
			pF->af = (float) OpenGL.FogTable[(FxU16)(1.0f / a->oow)] * D1OVER255;
		}
		else
		{
			pF->af = a->a * D1OVER255;
		}
		/*
		if ( glidestate.FogMode & GR_FOG_ADD2 )
		{
			pF->af = 1.0f - pF->af;
		}
		*/

	#ifdef OGL_DEBUG
		DEBUG_MIN_MAX( pF->af, OGLRender.MaxF, OGLRender.MinF );
	#endif
	}
	else /* if (OGLRender.UseEnvCombineFog) */ // env combine fog is the default
	{
		// Must provide dummy coords if fog is turned off but
		// the texture unit is active because of inveting color/alpha
		pF->af = 0.0;
	}

#ifdef OGL_DEBUG
	DEBUG_MIN_MAX( pC->ar, OGLRender.MaxR, OGLRender.MinR );

	DEBUG_MIN_MAX( pC->ag, OGLRender.MaxG, OGLRender.MinG );

	DEBUG_MIN_MAX( pC->ab, OGLRender.MaxB, OGLRender.MinB );

	DEBUG_MIN_MAX( pC->aa, OGLRender.MaxA, OGLRender.MinA );

	DEBUG_MIN_MAX( pV->az, OGLRender.MaxZ, OGLRender.MinZ );

	DEBUG_MIN_MAX( pV->ax, OGLRender.MaxX, OGLRender.MinX );

	DEBUG_MIN_MAX( pV->ay, OGLRender.MaxY, OGLRender.MinY );

	if (OpenGL.Texture)
	{
		DEBUG_MIN_MAX( pTS->as, OGLRender.MaxS, OGLRender.MinS );

		DEBUG_MIN_MAX( pTS->at, OGLRender.MaxT, OGLRender.MinT );
	}
#endif

#ifdef OGL_DEBUG_OPENGL_COORDS
	GlideMsg(pV, pTS);
#endif

	RenderUpdateState();
	if (OpenGL.Texture)
	{
		Textures->MakeReady();
	}

	glBegin( GL_POINTS );
		OpenGL.ChromaKey ? glColor3fv( &pC->ar ) : glColor4fv( &pC->ar );
		if (InternalConfig.EXT_secondary_color)
		{
			glSecondaryColor3fvEXT( &pC2->ar );
		}
		if (OpenGL.Texture)
		{
			glTexCoord4fv( &pTS->as );
			if (OpenGL.ColorAlphaUnit2)
			{
				glMultiTexCoord4fvARB( OpenGL.ColorAlphaUnit2, &pTS->as );
			}
		}
		if (glidestate.FogMode)
		{
			if (OGLRender.UseEnvCombineFog)
			{
				glMultiTexCoord1fARB( OpenGL.FogTextureUnit, pF->af );
			}
	#ifdef OPENGLIDE_SYSTEM_HAS_FOGCOORD
			else if (InternalConfig.FogMode == OpenGLideFogEmulation_FogCoord)
			{
				glFogCoordfEXT( pF->af );
			}
	#endif
		}
		glVertex3fv( &pV->ax );
	glEnd();
	glReportError();

	s_Framebuffer.SetRenderBufferChanged();
	
	VERIFY_ACTIVE_TEXTURE_UNIT(OpenGL.ColorAlphaUnit1);
}

// Color Factor functions

void __fastcall ColorFactor3Zero( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha )
{
	Result->ar = Result->ag = Result->ab = 0.0f;
	Result->br = Result->bg = Result->bb = 0.0f;
	Result->cr = Result->cg = Result->cb = 0.0f;
}

void __fastcall ColorFactor3Local( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha )
{
	Result->ar = ColorComponent->ar;
	Result->ag = ColorComponent->ag;
	Result->ab = ColorComponent->ab;
	Result->br = ColorComponent->br;
	Result->bg = ColorComponent->bg;
	Result->bb = ColorComponent->bb;
	Result->cr = ColorComponent->cr;
	Result->cg = ColorComponent->cg;
	Result->cb = ColorComponent->cb;
}

void __fastcall ColorFactor3OtherAlpha( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha )
{
	Result->ar = Result->ag = Result->ab = OtherAlpha->aa;
	Result->br = Result->bg = Result->bb = OtherAlpha->ba;
	Result->cr = Result->cg = Result->cb = OtherAlpha->ca;
}

void __fastcall ColorFactor3LocalAlpha( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha )
{
	Result->ar = Result->ag = Result->ab = ColorComponent->aa;
	Result->br = Result->bg = Result->bb = ColorComponent->ba;
	Result->cr = Result->cg = Result->cb = ColorComponent->ca;
}

void __fastcall ColorFactor3OneMinusLocal( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha )
{
	Result->ar = 1.0f - ColorComponent->ar;
	Result->ag = 1.0f - ColorComponent->ag;
	Result->ab = 1.0f - ColorComponent->ab;
	Result->br = 1.0f - ColorComponent->br;
	Result->bg = 1.0f - ColorComponent->bg;
	Result->bb = 1.0f - ColorComponent->bb;
	Result->cr = 1.0f - ColorComponent->cr;
	Result->cg = 1.0f - ColorComponent->cg;
	Result->cb = 1.0f - ColorComponent->cb;
}

void __fastcall ColorFactor3OneMinusOtherAlpha( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha )
{
	Result->ar = Result->ag = Result->ab = 1.0f - OtherAlpha->aa;
	Result->br = Result->bg = Result->bb = 1.0f - OtherAlpha->ba;
	Result->cr = Result->cg = Result->cb = 1.0f - OtherAlpha->ca;
}

void __fastcall ColorFactor3OneMinusLocalAlpha( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha )
{
	Result->ar = Result->ag = Result->ab = 1.0f - ColorComponent->aa;
	Result->br = Result->bg = Result->bb = 1.0f - ColorComponent->ba;
	Result->cr = Result->cg = Result->cb = 1.0f - ColorComponent->ca;
}

void __fastcall ColorFactor3One( TColorStruct *Result, const TColorStruct *ColorComponent, const TColorStruct *OtherAlpha )
{
	Result->ar = Result->ag = Result->ab = 1.0f;
	Result->br = Result->bg = Result->bb = 1.0f;
	Result->cr = Result->cg = Result->cb = 1.0f;
}

// Alpha Factor functions

float AlphaFactorZero( float LocalAlpha, float OtherAlpha )
{
	return 0.0f;
}

float AlphaFactorLocal( float LocalAlpha, float OtherAlpha )
{
	return LocalAlpha;
}

float AlphaFactorOther( float LocalAlpha, float OtherAlpha )
{
	return OtherAlpha;
}

float AlphaFactorOneMinusLocal( float LocalAlpha, float OtherAlpha )
{
	return 1.0f - LocalAlpha;
}

float AlphaFactorOneMinusOther( float LocalAlpha, float OtherAlpha )
{
	return 1.0f - OtherAlpha;
}

float AlphaFactorOne( float LocalAlpha, float OtherAlpha )
{
	return 1.0f;
}

// Color functions

void ColorFunctionZero( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	pC->ar = pC->ag = pC->ab = 0.0f; 
	pC->br = pC->bg = pC->bb = 0.0f; 
	pC->cr = pC->cg = pC->cb = 0.0f; 
}

void ColorFunctionLocal( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	pC->ar = Local->ar;
	pC->ag = Local->ag;
	pC->ab = Local->ab;
	pC->br = Local->br;
	pC->bg = Local->bg;
	pC->bb = Local->bb;
	pC->cr = Local->cr;
	pC->cg = Local->cg;
	pC->cb = Local->cb;
}

void ColorFunctionLocalAlpha( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	pC->ar = pC->ag = pC->ab = Local->aa;
	pC->br = pC->bg = pC->bb = Local->ba;
	pC->cr = pC->cg = pC->cb = Local->ca;
}

void ColorFunctionScaleOther( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	ColorFactor3Func( &CFactor, Local, Other );
	pC->ar = CFactor.ar * Other->ar;
	pC->ag = CFactor.ag * Other->ag;
	pC->ab = CFactor.ab * Other->ab;
	pC->br = CFactor.br * Other->br;
	pC->bg = CFactor.bg * Other->bg;
	pC->bb = CFactor.bb * Other->bb;
	pC->cr = CFactor.cr * Other->cr;
	pC->cg = CFactor.cg * Other->cg;
	pC->cb = CFactor.cb * Other->cb;
}

void ColorFunctionScaleOtherAddLocal( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	ColorFactor3Func( &CFactor, Local, Other );
	pC->ar = CFactor.ar * Other->ar;
	pC->ag = CFactor.ag * Other->ag;
	pC->ab = CFactor.ab * Other->ab;
	pC->br = CFactor.br * Other->br;
	pC->bg = CFactor.bg * Other->bg;
	pC->bb = CFactor.bb * Other->bb;
	pC->cr = CFactor.cr * Other->cr;
	pC->cg = CFactor.cg * Other->cg;
	pC->cb = CFactor.cb * Other->cb;
	pC2->ar = Local->ar;
	pC2->ag = Local->ag;
	pC2->ab = Local->ab;
	pC2->br = Local->br;
	pC2->bg = Local->bg;
	pC2->bb = Local->bb;
	pC2->cr = Local->cr;
	pC2->cg = Local->cg;
	pC2->cb = Local->cb;
}

void ColorFunctionScaleOtherAddLocalAlpha( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	ColorFactor3Func( &CFactor, Local, Other );
	pC->ar = CFactor.ar * Other->ar;
	pC->ag = CFactor.ag * Other->ag;
	pC->ab = CFactor.ab * Other->ab;
	pC->br = CFactor.br * Other->br;
	pC->bg = CFactor.bg * Other->bg;
	pC->bb = CFactor.bb * Other->bb;
	pC->cr = CFactor.cr * Other->cr;
	pC->cg = CFactor.cg * Other->cg;
	pC->cb = CFactor.cb * Other->cb;
	pC2->ar = pC2->ag = pC2->ab = Local->aa;
	pC2->br = pC2->bg = pC2->bb = Local->ba;
	pC2->cr = pC2->cg = pC2->cb = Local->ca;
}

void ColorFunctionScaleOtherMinusLocal( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	ColorFactor3Func( &CFactor, Local, Other );
	pC->ar = CFactor.ar * (Other->ar - Local->ar);
	pC->ag = CFactor.ag * (Other->ag - Local->ag);
	pC->ab = CFactor.ab * (Other->ab - Local->ab);
	pC->br = CFactor.br * (Other->br - Local->br);
	pC->bg = CFactor.bg * (Other->bg - Local->bg);
	pC->bb = CFactor.bb * (Other->bb - Local->bb);
	pC->cr = CFactor.cr * (Other->cr - Local->cr);
	pC->cg = CFactor.cg * (Other->cg - Local->cg);
	pC->cb = CFactor.cb * (Other->cb - Local->cb);
}

void ColorFunctionScaleOtherMinusLocalAddLocal( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	const GlideState glidestate = Glide.State;
	if (((glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA) ||
	     (glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB)) &&
	     (glidestate.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE))
	{
		pC->ar = Local->ar;
		pC->ag = Local->ag;
		pC->ab = Local->ab;
		pC->br = Local->br;
		pC->bg = Local->bg;
		pC->bb = Local->bb;
		pC->cr = Local->cr;
		pC->cg = Local->cg;
		pC->cb = Local->cb;
	}
	else
	{
		ColorFactor3Func( &CFactor, Local, Other );
		pC->ar = CFactor.ar * (Other->ar - Local->ar);
		pC->ag = CFactor.ag * (Other->ag - Local->ag);
		pC->ab = CFactor.ab * (Other->ab - Local->ab);
		pC->br = CFactor.br * (Other->br - Local->br);
		pC->bg = CFactor.bg * (Other->bg - Local->bg);
		pC->bb = CFactor.bb * (Other->bb - Local->bb);
		pC->cr = CFactor.cr * (Other->cr - Local->cr);
		pC->cg = CFactor.cg * (Other->cg - Local->cg);
		pC->cb = CFactor.cb * (Other->cb - Local->cb);
		pC2->ar = Local->ar;
		pC2->ag = Local->ag;
		pC2->ab = Local->ab;
		pC2->br = Local->br;
		pC2->bg = Local->bg;
		pC2->bb = Local->bb;
		pC2->cr = Local->cr;
		pC2->cg = Local->cg;
		pC2->cb = Local->cb;
	}
}

void ColorFunctionScaleOtherMinusLocalAddLocalAlpha( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	const GlideState glidestate = Glide.State;
	if (((glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA) ||
	     (glidestate.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB)) &&
	     (glidestate.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE))
	{
		pC->ar = pC->ag = pC->ab = Local->aa;
		pC->br = pC->bg = pC->bb = Local->ba;
		pC->cr = pC->cg = pC->cb = Local->ca;
	}
	else
	{
		ColorFactor3Func( &CFactor, Local, Other );
		pC->ar = CFactor.ar * (Other->ar - Local->ar);
		pC->ag = CFactor.ag * (Other->ag - Local->ag);
		pC->ab = CFactor.ab * (Other->ab - Local->ab);
		pC->br = CFactor.br * (Other->br - Local->br);
		pC->bg = CFactor.bg * (Other->bg - Local->bg);
		pC->bb = CFactor.bb * (Other->bb - Local->bb);
		pC->cr = CFactor.cr * (Other->cr - Local->cr);
		pC->cg = CFactor.cg * (Other->cg - Local->cg);
		pC->cb = CFactor.cb * (Other->cb - Local->cb);
		pC2->ar = pC2->ag = pC2->ab = Local->aa;
		pC2->br = pC2->bg = pC2->bb = Local->ba;
		pC2->cr = pC2->cg = pC2->cb = Local->ca;
	}
}

void ColorFunctionMinusLocalAddLocal( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	ColorFactor3Func( &CFactor, Local, Other );
	pC->ar = ( 1.0f - CFactor.ar ) * Local->ar;
	pC->ag = ( 1.0f - CFactor.ag ) * Local->ag;
	pC->ab = ( 1.0f - CFactor.ab ) * Local->ab;
	pC->br = ( 1.0f - CFactor.br ) * Local->br;
	pC->bg = ( 1.0f - CFactor.bg ) * Local->bg;
	pC->bb = ( 1.0f - CFactor.bb ) * Local->bb;
	pC->cr = ( 1.0f - CFactor.cr ) * Local->cr;
	pC->cg = ( 1.0f - CFactor.cg ) * Local->cg;
	pC->cb = ( 1.0f - CFactor.cb ) * Local->cb;
	pC2->ar = Local->ar;
	pC2->ag = Local->ag;
	pC2->ab = Local->ab;
	pC2->br = Local->br;
	pC2->bg = Local->bg;
	pC2->bb = Local->bb;
	pC2->cr = Local->cr;
	pC2->cg = Local->cg;
	pC2->cb = Local->cb;
}

void ColorFunctionMinusLocalAddLocalAlpha( TColorStruct * pC, TColorStruct * pC2, const TColorStruct * Local, const TColorStruct * Other )
{
	ColorFactor3Func( &CFactor, Local, Other );
	pC->ar = CFactor.ar * -Local->ar;
	pC->ag = CFactor.ag * -Local->ag;
	pC->ab = CFactor.ab * -Local->ab;
	pC->br = CFactor.br * -Local->br;
	pC->bg = CFactor.bg * -Local->bg;
	pC->bb = CFactor.bb * -Local->bb;
	pC->cr = CFactor.cr * -Local->cr;
	pC->cg = CFactor.cg * -Local->cg;
	pC->cb = CFactor.cb * -Local->cb;
	pC2->ar = pC2->ag = pC2->ab = Local->aa;
	pC2->br = pC2->bg = pC2->bb = Local->ba;
	pC2->cr = pC2->cg = pC2->cb = Local->ca;  
}
