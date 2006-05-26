//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                    Drawing Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "GLRender.h"
#include "GLRenderUpdateState.h"

//*************************************************
//* Draws a Triangle on the screen
//*************************************************
FX_ENTRY void FX_CALL
grDrawTriangle( const GrVertex *a, const GrVertex *b, const GrVertex *c )
{
#ifdef OGL_CRITICAL
	GlideMsg( "grDrawTriangle( ---, ---, --- )\n" );
#endif

	SetClipVerticesState(false);
	
	RenderAddTriangle( a, b, c, true );

	if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
	{
		RenderDrawTriangles( );
		glFlush( );
	}
}

//*************************************************
//* Draws a planar polygon on the screen
//*************************************************
FX_ENTRY void FX_CALL
grDrawPlanarPolygonVertexList( int nVertices, const GrVertex vlist[] )
{
#ifdef OGL_CRITICAL
	GlideMsg("grDrawPlanarPolygonVertexList( %d, --- )\n", nVertices );
#endif

	SetClipVerticesState(false);

	for ( int i = 2; i < nVertices; i++ )
	{
		RenderAddTriangle( &vlist[ 0 ], &vlist[ i - 1 ], &vlist[ i ], true );
	}
	if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
	{
		RenderDrawTriangles( );
		glFlush( );
	}
}

//*************************************************
//* Draws a Line on the screen
//*************************************************
FX_ENTRY void FX_CALL
grDrawLine( const GrVertex *a, const GrVertex *b )
{
#ifdef OGL_CRITICAL
	GlideMsg("grDrawLine( ---, --- )\n");
#endif

	SetClipVerticesState(false);
    
	RenderAddLine( a, b, true );
}

//*************************************************
//* Draws a Point on the screen
//*************************************************
FX_ENTRY void FX_CALL
grDrawPoint( const GrVertex *a )
{
#ifdef OGL_CRITICAL
	GlideMsg( "grDrawPoint( --- )\n" );
#endif

	SetClipVerticesState(false);

	RenderAddPoint( a, true );
}

//*************************************************
//* Draw a convex non-planar polygon
//*************************************************
FX_ENTRY void FX_CALL
grDrawPolygon( int nverts, const int ilist[], const GrVertex vlist[] )
{
#ifdef OGL_CRITICAL
	GlideMsg( "grDrawPolygon( %d, ---, --- )\n" );
#endif

	SetClipVerticesState(false);

	for ( int i = 2; i < nverts; i++ )
	{
		RenderAddTriangle( &vlist[ ilist[ 0 ] ], 
		                   &vlist[ ilist[ i - 1 ] ], 
		                   &vlist[ ilist[ i ] ],
		                   true );
	}

	if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
	{
		RenderDrawTriangles( );
		glFlush( );
	}
}

//*************************************************
//* Draw a convex planar polygon
//*************************************************
FX_ENTRY void FX_CALL
grDrawPlanarPolygon( int nverts, const int ilist[], const GrVertex vlist[] )
{
#ifdef OGL_CRITICAL
	GlideMsg( "grDrawPlanarPolygon( %d, ---, --- )\n", nverts );
#endif

	SetClipVerticesState(false);

	for ( int i = 2; i < nverts; i++ )
	{
		RenderAddTriangle( &vlist[ ilist[ 0 ] ], 
		                   &vlist[ ilist[ i - 1 ] ],
		                   &vlist[ ilist[ i ] ],
		                   true );
	}

	if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
	{
		RenderDrawTriangles( );
		glFlush( );
	}
}

//*************************************************
//* Draw a convex non-planar polygon
//*************************************************
FX_ENTRY void FX_CALL
grDrawPolygonVertexList( int nVertices, const GrVertex vlist[] )
{
#ifdef OGL_CRITICAL
  GlideMsg( "grDrawPolygonVertexList( %d, --- )\n", nVertices );
#endif

	SetClipVerticesState(false);

	for ( int i = 2; i < nVertices; i++ )
	{
		RenderAddTriangle( &vlist[ 0 ],
											&vlist[ i - 1 ],
											&vlist[ i ],
											true );
	}
	if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
	{
		RenderDrawTriangles( );
		glFlush( );
	}
}

FX_ENTRY void FX_CALL
guAADrawTriangleWithClip( const GrVertex *a, const GrVertex *b, 
                          const GrVertex *c )
{
#ifdef OGL_CRITICAL
	GlideMsg("guAADrawTriangleWithClip( ---, ---, --- )\n");
#endif

	SetClipVerticesState(true);

	RenderAddTriangle( a, b, c, false );

	if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
	{
		RenderDrawTriangles( );
		glFlush( );
	}
}

FX_ENTRY void FX_CALL
guDrawTriangleWithClip( const GrVertex *a,
                        const GrVertex *b,
                        const GrVertex *c )
{
#ifdef OGL_CRITICAL
	GlideMsg("guDrawTriangleWithClip( ---, ---, --- )\n");
#endif

	SetClipVerticesState(true);

	RenderAddTriangle( a, b, c, false );

	if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
	{
		RenderDrawTriangles( );
		glFlush( );
	}
}

FX_ENTRY void FX_CALL
guDrawPolygonVertexListWithClip( int nVertices, const GrVertex vlist[] )
{
#ifdef OGL_CRITICAL
	GlideMsg( "guDrawPolygonVertexListWithClip( %d, --- )\n", nVertices );
#endif

	SetClipVerticesState(true);

	for ( int i = 2; i < nVertices; i++ )
	{
		RenderAddTriangle( &vlist[ 0 ], 
											&vlist[ i - 1 ],
											&vlist[ i ],
											false );
	}
	if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
	{
		RenderDrawTriangles( );
		glFlush( );
	}
}

FX_ENTRY void FX_CALL
grAADrawLine( const GrVertex *a, const GrVertex *b )
{
#ifdef OGL_CRITICAL
	GlideMsg( "grAADrawLine( ---, --- )\n" );
#endif

	SetClipVerticesState(false);

  RenderAddLine( a, b, true );
}

FX_ENTRY void FX_CALL
grAADrawPoint(const GrVertex *a )
{
#ifdef OGL_CRITICAL
  GlideMsg("grAADrawPoint( --- )\n");
#endif

	SetClipVerticesState(false);

  RenderAddPoint( a, true );
}

FX_ENTRY void FX_CALL
grAADrawPolygon( const int nverts, const int ilist[], const GrVertex vlist[] )
{
#ifdef OGL_CRITICAL
  GlideMsg( "grAADrawPolygon( %d, ---, --- )\n", nverts );
#endif

	SetClipVerticesState(false);

  for ( int i = 2; i < nverts; i++ )
  {
    RenderAddTriangle( &vlist[ ilist[ 0 ] ],
                       &vlist[ ilist[ i - 1 ] ],
                       &vlist[ ilist[ i ] ],
                       true );
  }

  if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
  {
    RenderDrawTriangles( );
    glFlush( );
  }
}

FX_ENTRY void FX_CALL
grAADrawPolygonVertexList( const int nverts, const GrVertex vlist[] )
{
#ifdef OGL_CRITICAL
  GlideMsg( "grAADrawPolygonVertexList( %d, --- )\n", nverts );
#endif

	SetClipVerticesState(false);

  for ( int i = 2; i < nverts; i++ )
  {
    RenderAddTriangle( &vlist[ 0 ],
                       &vlist[ i - 1 ],
                       &vlist[ i ],
                       true );
  }

  if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
  {
    RenderDrawTriangles( );
    glFlush( );
  }
}

FX_ENTRY void FX_CALL
grAADrawTriangle( const GrVertex *a, const GrVertex *b, const GrVertex *c,
                  FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias )
{
#ifdef OGL_CRITICAL
	GlideMsg("grAADrawTriangle( ---, ---, ---, %d, %d, %d )\n",
	    ab_antialias, bc_antialias, ca_antialias );
#endif

	SetClipVerticesState(false);

  RenderAddTriangle( a, b, c, true );

	if ( Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER )
	{
	    RenderDrawTriangles( );
	    glFlush( );
	}
}
