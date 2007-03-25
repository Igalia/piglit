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

// Authors: Brian Paul, Keith Whitwell

#include "tfpexceptions.h"
#include <cassert>
#include <cmath>

#define INCLUDE_FPU_CONTROL 0
#if INCLUDE_FPU_CONTROL
#include <fpu_control.h>
#endif


namespace GLEAN {


// This might be useful at some point
void
FPExceptionsTest::enableExceptions(bool enable)
{
#if INCLUDE_FPU_CONTROL
   const fpu_control_t bits =
      _FPU_MASK_IM |
      _FPU_MASK_DM |
      _FPU_MASK_ZM |
      _FPU_MASK_OM |
      _FPU_MASK_UM;

   if (enable) {
      /* generate FP exceptions */
      fpu_control_t mask;
      _FPU_GETCW(mask);
      mask &= ~bits;
      _FPU_SETCW(mask);
   }
   else {
      fpu_control_t mask;
      _FPU_GETCW(mask);
      mask |= bits;
      _FPU_SETCW(mask);
   }
#else
   (void) enable;
#endif
}



// XXX any endian issues with this???
// Works on x86 / little endian
union fi {
	float f;
	struct {
		unsigned mantissa:23;
		unsigned exponent:8;
		unsigned sign:1;
	} bits;
	unsigned ui;
};


static void
make_float(float *dest, unsigned sign, unsigned exponent, unsigned mantissa)
{
	union fi *destfi = (union fi *) dest;
	destfi->bits.sign = sign;
	destfi->bits.exponent = exponent;
	destfi->bits.mantissa = mantissa;
}

static void
make_denorm_float(float *dest, int sign, int mantissa)
{
	make_float(dest, sign, 0, mantissa);
}

static void
make_pos_inf_float(float *dest)
{
	make_float(dest, 0, 255, 0); // or HUGE_VALF?
}

static void
make_neg_inf_float(float *dest)
{
	make_float(dest, 1, 255, 0); // or -HUGE_VALF?
}

static void
make_signaling_nan_float(float *dest)
{
	make_float(dest, 0, 255, 1);
}

static void
make_quiet_nan_float(float *dest)
{
	make_float(dest, 0, 255, 1 << 22);
}

static void
make_denorm_double(double * /*dest*/, int /*sign*/, int /*mantissa*/)
{
	// XXX to do
}

static void
make_pos_inf_double(double *dest)
{
	*dest = HUGE_VAL;
}

static void
make_neg_inf_double(double *dest)
{
	*dest = -HUGE_VAL;
}

static void
make_signaling_nan_double(double * /*dest*/)
{
	// XXX to do
}

static void
make_quiet_nan_double(double * /*dest*/)
{
	// XXX to do
}


static void
print_float(float f)
{
   union fi fi, fi2;
   int iexp, imnt, isgn;

   fi.f = f;
   printf("float %f (%e)\n\tuint 0x%x\n\tsign %d exponent %d mantissa 0x%x\n",
	  fi.f, fi.f, fi.ui, fi.bits.sign, fi.bits.exponent, fi.bits.mantissa);

   switch (fi.bits.exponent) {
   case 0:
      if (fi.bits.mantissa == 0)
	 printf("\t%szero\n", fi.bits.sign ? "-" : "+");
      else {
	 printf("\tdenormalized float\n");

	 iexp = -126 - 23;	/* -149 */
	 imnt = (int)fi.bits.mantissa;
	 isgn = fi.bits.sign ? -1 : 1;
	 fi2.f = isgn * imnt * ldexp(1.0, iexp);
      
	 printf("\trecombining: %d * 0x%x * 2.0^%d  == %f (%e)\n",
		isgn, imnt, iexp, fi2.f, fi2.f);
	 printf("\trecombined: sign %d exponent %d mantissa 0x%x\n",
		fi2.bits.sign, fi2.bits.exponent, fi2.bits.mantissa);      
      }
      break;

   case 255:
      if (fi.bits.mantissa & (1<<22)) 
	 printf("\tQNaN (Quiet NaN/indeterminate value)\n");
      else if (fi.bits.mantissa)
	 printf("\tSNaN (Signalling NaN/invalid value)\n");
      else 
	 printf("\t%sinf\n", fi.bits.sign ? "-" : "+");
      break;

   default:
      iexp = fi.bits.exponent - (127 + 23);
      imnt = (1<<23) + (int)fi.bits.mantissa;
      isgn = fi.bits.sign ? -1 : 1;
      fi2.f = isgn * imnt * ldexp(1.0, iexp);
      
      printf("\trecombining: %d * 0x%x * 2.0^%d  == %f\n",
	     isgn, imnt, iexp, fi2.f);
      
      printf("\trecombined: sign %d exponent %d mantissa 0x%x\n",
	     fi2.bits.sign, fi2.bits.exponent, fi2.bits.mantissa);
      break;
   }

   /* Let's look and see what would happen if we interpret all these
    * cases as normal floats:
    */
   iexp = fi.bits.exponent - (127 + 23);
   imnt = (1<<23) + (int)fi.bits.mantissa;
   isgn = fi.bits.sign ? -1 : 1;
   fi2.f = isgn * imnt * ldexp(1.0, iexp);

   printf("\tvalue if treated as normalized: %f (%e)\n",
	  fi2.f, fi2.f);
}


/* Examine some interesting floats
 */
#if 0
int main() 
{
   float f;
   int i;

   for (i = -3; i < 10; i++) {
      printf("%d:\n  ", i);
      print_float(ldexp(1.0, i));
   }
   
   for (f = -4 ; f < 4; f += 1)
      print_float(f);

   for (f = -.01 ; f < .01; f += .002)
      print_float(f);

   f = 1.0/0;
   print_float(f);

   f += 1.0;
   print_float(f);

   /* Explicitly make a denormal - I've no idea how to create these
    * with regular calculations:
    */
   make_float(&f, 0, 0, 0x1000);
   print_float(f);

   /* It seems you can just specify them!
    */
   f = 5.739719e-42;
   print_float(f);

   /* A little, non-denormalized float
    */
   make_float(&f, 0, 1, 0x1);
   print_float(f);

   /* A negative little, non-denormalized float
    */
   make_float(&f, 1, 1, 0x1);
   print_float(f);

   /* A big float
    */
   make_float(&f, 0, 254, ~0);
   print_float(f);

   make_float(&f, 1, 254, ~0);
   print_float(f);

   /* Littlest and biggest denormals:
    */
   make_float(&f, 0, 0, 1);
   print_float(f);
   make_float(&f, 0, 0, ~0);
   print_float(f);


   make_float(&f, 1, 0, 1);
   print_float(f);
   make_float(&f, 1, 0, ~0);
   print_float(f);

}
#endif
      

bool
FPExceptionsTest::testVertices(Mode m)
{
	float v[3][4];
	// nice coords
	for (int i = 0; i < 3; i++) {
		v[i][0] = 0.0;
		v[i][1] = 0.0;
		v[i][2] = 0.0;
		v[i][3] = 1.0;
	}

	// set problematic values
	switch (m) {
	case MODE_INFINITY:
		make_pos_inf_float(&v[1][0]);
		make_neg_inf_float(&v[2][1]);
		break;
	case MODE_NAN:
		make_signaling_nan_float(&v[1][0]);
		make_quiet_nan_float(&v[2][1]);
		break;
	case MODE_DIVZERO:
		v[0][3] = 0.0;
		v[1][3] = 0.0;
		v[2][3] = 0.0;
		break;
	case MODE_DENORM:
		make_denorm_float(&v[0][0], 0, 1);
		make_denorm_float(&v[1][1], 1, 1);
		break;
	default:
		; // nothing
	}

	// vertex positions
	glBegin(GL_POLYGON);
	glVertex4fv(v[0]);
	glVertex4fv(v[1]);
	glVertex4fv(v[2]);
	glEnd();

	// colors
	glBegin(GL_POLYGON);
	glColor4fv(v[0]);  glVertex2f(-1, -1);
	glColor4fv(v[1]);  glVertex2f( 1, -1);
	glColor4fv(v[2]);  glVertex2f( 0,  1);
	glEnd();

	// normals
	glEnable(GL_LIGHTING);
	glBegin(GL_POLYGON);
	glNormal3fv(v[0]);  glVertex2f(-1, -1);
	glNormal3fv(v[1]);  glVertex2f( 1, -1);
	glNormal3fv(v[2]);  glVertex2f( 0,  1);
	glEnd();
	glDisable(GL_LIGHTING);

	// texcoords
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	glTexCoord4fv(v[0]);  glVertex2f(-1, -1);
	glTexCoord4fv(v[1]);  glVertex2f( 1, -1);
	glTexCoord4fv(v[2]);  glVertex2f( 0,  1);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	return true;
}


bool
FPExceptionsTest::testTransformation(Mode m)
{
	float mat[16];

	// identity
	for (int i = 0; i < 15; i++)
		mat[i] = 0.0;
	mat[0] = mat[5] = mat[10] = mat[15] = 1.0;

	// set problematic values
	switch (m) {
	case MODE_INFINITY:
		make_pos_inf_float(&mat[0]);   // X scale
		make_neg_inf_float(&mat[13]);  // Y translate
		break;
	case MODE_NAN:
		make_signaling_nan_float(&mat[0]);   // X scale
		make_quiet_nan_float(&mat[13]);  // Y translate
		break;
	case MODE_DIVZERO:
		// all zero matrix
		mat[0] = mat[5] = mat[10] = mat[15] = 0.0;
		break;
	case MODE_DENORM:
		make_denorm_float(&mat[0], 0, 1);
		make_denorm_float(&mat[13], 1, 1);
		break;
	default:
		; // nothing
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(mat);

	// vertex positions
	glBegin(GL_POLYGON);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 0,  1);
	glEnd();

	glPopMatrix();

	return true;
}


bool
FPExceptionsTest::testClipping(Mode m)
{
	double plane[4];

	// start w/ nice values
	plane[0] = plane[1] = plane[2] = plane[3] = 0.0;

	// set problematic values
	switch (m) {
	case MODE_INFINITY:
		make_pos_inf_double(&plane[0]);
		make_neg_inf_double(&plane[3]);
		break;
	case MODE_NAN:
		make_signaling_nan_double(&plane[0]);
		make_quiet_nan_double(&plane[3]);
		break;
	case MODE_DIVZERO:
		// nothing		
		break;
	case MODE_DENORM:
		make_denorm_double(&plane[0], 0, 1);
		make_denorm_double(&plane[3], 1, 1);
		break;
	case MODE_OVERFLOW:
		plane[0] = 1.0e300;
		plane[3] = 1.0e-300;
		break;
	default:
		; // nothing
	}

	glClipPlane(GL_CLIP_PLANE0, plane);
	glEnable(GL_CLIP_PLANE0);

	// vertex positions
	glBegin(GL_POLYGON);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 0,  1);
	glEnd();

	glDisable(GL_CLIP_PLANE0);

	return true;
}


// pass large doubles to OpenGL and see what happens when converted to float.
bool
FPExceptionsTest::testOverflow(void)
{
	GLdouble v[3][4];
	for (int i = 0; i < 3; i++) {
		v[i][0] = 0.0;
		v[i][1] = 0.0;
		v[i][2] = 0.0;
		v[i][3] = 1.0;
	}
	v[0][0] = 1.0e300;
	v[0][1] = -1.0e300;
	v[1][0] = 1.0e-300;
	v[1][1] = 1.0e-300;

	GLdouble mat[16];
	for (int i = 0; i < 15; i++)
		mat[i] = 0.0;
	mat[0] = mat[5] = mat[10] = mat[15] = 1.0e500;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(mat);

	glBegin(GL_POLYGON);
	glVertex4dv(v[0]);
	glVertex4dv(v[1]);
	glVertex4dv(v[2]);
	glEnd();

	glPopMatrix();

	return true;
}



void
FPExceptionsTest::setup(void)
{
	// Simple texture map
	static const GLfloat texImage[2][2][3] = {
		{ {1, 1, 1}, {0, 0, 0} },
		{ {0, 0, 0}, {1, 1, 1} }
	};

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0,
		     GL_RGB, GL_FLOAT, texImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// simple lighting
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}


void
FPExceptionsTest::reportPassFail(MultiTestResult &r,
				 bool pass, const char *msg) const
{
	if (pass) {
		if (env->options.verbosity)
			env->log << name << " PASS: " << msg << " test\n";
		r.numPassed++;
	}
	else {
		if (env->options.verbosity)
			env->log << name << " FAILURE: " << msg << " test\n";
		r.numFailed++;
	}
}

void
FPExceptionsTest::runOne(MultiTestResult &r, Window &w)
{
	bool p;

	(void) w;

	p = testVertices(MODE_INFINITY);
	reportPassFail(r, p, "Infinite value vertex");

	p = testVertices(MODE_NAN);
	reportPassFail(r, p, "NaN value vertex");

	p = testVertices(MODE_DIVZERO);
	reportPassFail(r, p, "Divide by zero vertex");

	p = testVertices(MODE_DENORM);
	reportPassFail(r, p, "Denorm vertex");


	p = testTransformation(MODE_INFINITY);
	reportPassFail(r, p, "Infinite matrix transform");

	p = testTransformation(MODE_NAN);
	reportPassFail(r, p, "NaN matrix transform");

	p = testTransformation(MODE_DIVZERO);
	reportPassFail(r, p, "Zero matrix transform");

	p = testTransformation(MODE_DENORM);
	reportPassFail(r, p, "Denorm matrix transform");


	p = testClipping(MODE_INFINITY);
	reportPassFail(r, p, "Infinite clip plane");

	p = testClipping(MODE_NAN);
	reportPassFail(r, p, "NaN clip plane");

	p = testClipping(MODE_DIVZERO);
	reportPassFail(r, p, "Zero clip plane");

	p = testClipping(MODE_DENORM);
	reportPassFail(r, p, "Denorm clip plane");

	p = testClipping(MODE_OVERFLOW);
	reportPassFail(r, p, "Overflow clip plane");


	p = testOverflow();
	reportPassFail(r, p, "Overflow");

	r.pass = (r.numFailed == 0);
}


// The test object itself:
FPExceptionsTest FPExceptionsTest("fpexceptions", // test name
	"window, rgb",  // surface/pixel format
	"", // no extensions required
	"Test for floating point exceptions caused by +/-infinity, Nan, divide by zero, etc in a number of circumstances.\n");


} // namespace GLEAN
