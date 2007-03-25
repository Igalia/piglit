// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT

// tscissor.cpp:  Basic test of OpenGL scissor.
//
// This test verifies that the four corner pixels, and the four pixels
// diagonally inside the corners, of a scissored region are filled
// correctly.  It then tests up to two pixels in both the horizontal and
// vertical directions of the scissor region to verify that they are
// unfilled.
//
// To test for pass/fail, we examine the color buffer for white or black,
// respectively.
//
// Author: Gareth Hughes <gareth@valinux.com>  December 2000

#include "tscissor.h"

namespace GLEAN {

/* Verification helper macros:
 */
#define BAD_PIXEL( X, Y, R, G, B )	( image[X][Y][0] != (R) ||	\
					  image[X][Y][1] != (G) ||	\
					  image[X][Y][2] != (B) )

#define TEST_PIXEL( X, Y, R, G, B, E )					\
do {									\
	if ( BAD_PIXEL( X, Y, R, G, B ) ) {				\
		FailMessage( r, E, X, Y,				\
			     i, i, 10-2*i, 10-2*i );			\
		passed = false;						\
	}								\
} while (0)

#define TEST_CORNER( X, Y, SX, SY )					\
do {									\
	TEST_PIXEL( X,       Y,       1.0, 1.0, 1.0,  1 );		\
	TEST_PIXEL( X SX 1,  Y SY 1,  1.0, 1.0, 1.0,  2 );		\
	for ( j = 1 ; j <= i ; j++ ) {					\
		TEST_PIXEL( X - SX j,  Y,         0.0, 0.0, 0.0,  j );	\
		TEST_PIXEL( X,         Y - SY j,  0.0, 0.0, 0.0,  j );	\
	}								\
} while (0)


void
ScissorTest::FailMessage( BasicResult &r, int error, int x, int y,
			  int sx, int sy, int sw, int sh ) const
{
	env->log << name << ": FAIL "
		 << r.config->conciseDescription() << "\n";
	env->log << "\tOff by " << error << " error at"
		 << " row " << x << " column " << y;
	env->log << "\n\tglScissor( "
		 << sx << ", " << sy << ", "
		 << sw << ", " << sh << " )\n\n";
}

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
ScissorTest::runOne( BasicResult& r, Window& w ) {
	bool passed = true;
	int i, j, k;

	// Draw 10x10 quads, as they fit nicely into a terminal window
	// when dumped as RGB triplets...
	glViewport( 0, 0, 10, 10 );

	glDisable( GL_DITHER );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0.0, 1.0, 0.0, 1.0, -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glColor3f( 1.0, 1.0, 1.0 );

	if ( env->options.verbosity ) env->log << "\n";

	for ( i = 0 ; i < 3 ; i++ ) {
		float image[10][10][3];

		glDisable( GL_SCISSOR_TEST );
		glClear( GL_COLOR_BUFFER_BIT );
		glEnable( GL_SCISSOR_TEST );

		glScissor( i, i, 10-2*i, 10-2*i );

		glBegin( GL_QUADS );
		glVertex3f( 0.0, 0.0, 0.0 );
		glVertex3f( 1.0, 0.0, 0.0 );
		glVertex3f( 1.0, 1.0, 0.0 );
		glVertex3f( 0.0, 1.0, 0.0 );
		glEnd();

		w.swap();

		glReadPixels( 0, 0, 10, 10, GL_RGB, GL_FLOAT, image );

		// Dump the entire 10x10 image so the exact results can
		// be inspected.  Should make any failures pretty clear.
		if ( env->options.verbosity ) {
			env->log << "glScissor( "
				 << i << ", " << i << ", "
				 << 10-2*i << ", " << 10-2*i << " ):\n\n";
			for ( j = 0 ; j < 10 ; j++ ) {
				for ( k = 0 ; k < 10 ; k++ ) {
					env->log << "  " << image[j][k][0]
						 << " " << image[j][k][1]
						 << " " << image[j][k][2];
				}
				env->log << "\n";
			}
			env->log << "\n";
		}

		// Test the four corners.  Macro magic, I know...
		TEST_CORNER( i,     i,     +, + );
		TEST_CORNER( 9 - i, i,     -, + );
		TEST_CORNER( 9 - i, 9 - i, -, - );
		TEST_CORNER( i,     9 - i, +, - );
	}

	r.pass = passed;
} // ScissorTest::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
ScissorTest::logOne( BasicResult& r ) {
	if ( r.pass ) {
		logPassFail( r );
		logConcise( r );
	}
} // ScissorTest::logOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
ScissorTest::compareOne( BasicResult&, BasicResult& ) {
	// FIXME: Implement this...
} // ScissorTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
ScissorTest scissorTest( "scissor", "window, rgb",

	"This test performs a basic test of the OpenGL scissor.  It\n"
	"checks for off-by-one errors around all four corners of the\n"
	"scissored region, perhaps the most common cause of scissor\n"
	"test failures.\n"

	);

} // namespace GLEAN
