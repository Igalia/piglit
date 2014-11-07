/* 
 * BEGIN_COPYRIGHT -*- glean -*-
 * 
 * Copyright (C) 1999  Allen Akin   All Rights Reserved.
 * Copyright (C) 2014  Intel Corporation.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 * 
 * END_COPYRIGHT

 * Authors: Brian Paul, Keith Whitwell, Laura Ekstrand
 */

/**
 * Test for floating point exceptions caused by +/-infinity, Nan, 
 * divide by zero, etc in a number of circumstances.
 */

#include "piglit-util-gl.h"

#include <math.h> /* For HUGE_VAL */

#define INCLUDE_FPU_CONTROL 0
#if INCLUDE_FPU_CONTROL
#include <fpu_control.h>
#endif

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | 
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv) 
{
	/* Nothing to init. */
	
} /* piglit_init */

/* No idea if this actually works because it's never used. */
/* This might be useful at some point */
void
enable_exceptions(bool enable)
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



/*
 * XXX any endian issues with this???
 * Works on x86 / little endian
 */
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
	make_float(dest, 0, 255, 0); /* or HUGE_VALF? */
}

static void
make_neg_inf_float(float *dest)
{
	make_float(dest, 1, 255, 0); /* or -HUGE_VALF? */
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

#if 0
static void
make_denorm_double(double *dest, int sign, int mantissa)
{
	/* XXX to do */
}
#endif

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

#if 0
static void
make_signaling_nan_double(double *dest)
{
	/* XXX to do */
}

static void
make_quiet_nan_double(double *dest)
{
	/* XXX to do */
}
#endif


/* Uncomment to test float production */
#if 0
static void
print_float(float f)
{
   union fi fi, fi2;
   int iexp, imnt, isgn;

   fi.f = f;
   printf("float %f (%e)\n\tuint 0x%x\n\tsign %d exponent %d mantissa 0x%x\n",
	  fi.f, fi.f, fi.ui, fi.bits.sign, fi.bits.exponent, 
	  fi.bits.mantissa);

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
      
enum mode {
		MODE_INFINITY,
		MODE_NAN,
		MODE_DIVZERO,
		MODE_DENORM,
		MODE_OVERFLOW
};

bool
test_vertices(enum mode m)
{
	bool pass = true;
	int i;

	/* Make three nice vertices */
	float v[3][4];
	for (i = 0; i < 3; i++) {
		v[i][0] = 0.0;
		v[i][1] = 0.0;
		v[i][2] = 0.0;
		v[i][3] = 1.0;
	}

	/* Set problematic values */
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
		; /* nothing */
	}

	/* Send the vertices to GL, using them in multiple ways */
	/* As geometry */
	glBegin(GL_POLYGON);
	glVertex4fv(v[0]);
	glVertex4fv(v[1]);
	glVertex4fv(v[2]);
	glEnd();
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* As colors */
	glBegin(GL_POLYGON);
	glColor4fv(v[0]);  glVertex2f(-1, -1);
	glColor4fv(v[1]);  glVertex2f( 1, -1);
	glColor4fv(v[2]);  glVertex2f( 0,  1);
	glEnd();
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* As lighting normals */
	glEnable(GL_LIGHTING);
	glBegin(GL_POLYGON);
	glNormal3fv(v[0]);  glVertex2f(-1, -1);
	glNormal3fv(v[1]);  glVertex2f( 1, -1);
	glNormal3fv(v[2]);  glVertex2f( 0,  1);
	glEnd();
	glDisable(GL_LIGHTING);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* As texture coordinates */
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	glTexCoord4fv(v[0]);  glVertex2f(-1, -1);
	glTexCoord4fv(v[1]);  glVertex2f( 1, -1);
	glTexCoord4fv(v[2]);  glVertex2f( 0,  1);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	return pass;
}


bool
test_transformation(enum mode m)
{
	bool pass = true;
	int i;
	float mat[16];

	/* Create an identity matrix */
	for (i = 0; i < 15; i++)
		mat[i] = 0.0;
	mat[0] = mat[5] = mat[10] = mat[15] = 1.0;

	/* Set problematic values */
	switch (m) {
	case MODE_INFINITY:
		make_pos_inf_float(&mat[0]);   /* X scale */
		make_neg_inf_float(&mat[13]);  /* Y translate */
		break;
	case MODE_NAN:
		make_signaling_nan_float(&mat[0]);   /* X scale */
		make_quiet_nan_float(&mat[13]);  /* Y translate */
		break;
	case MODE_DIVZERO:
		/* all zero matrix */
		mat[0] = mat[5] = mat[10] = mat[15] = 0.0;
		break;
	case MODE_DENORM:
		make_denorm_float(&mat[0], 0, 1);
		make_denorm_float(&mat[13], 1, 1);
		break;
	default:
		; /* nothing */
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	/* Send matrix to GL */
	glLoadMatrixf(mat);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* Vertices */
	glBegin(GL_POLYGON);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 0,  1);
	glEnd();
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glPopMatrix();

	return pass;
}


bool
test_clipping(enum mode m)
{
	bool pass = true;
	/* TODO: Implement MODE_NAN and MODE_DENORM */
	double plane[4];

	/* Start w/ nice values */
	plane[0] = plane[1] = plane[2] = plane[3] = 0.0;

	/* Set problematic values */
	switch (m) {
	case MODE_INFINITY:
		make_pos_inf_double(&plane[0]);
		make_neg_inf_double(&plane[3]);
		break;
	/*
	 * case MODE_NAN:
	 *	 make_signaling_nan_double(&plane[0]);
	 *	 make_quiet_nan_double(&plane[3]);
	 *	 break;
	 */
	case MODE_DIVZERO:
		/* nothing */
		break;
	/*
	 * case MODE_DENORM:
	 * 	 make_denorm_double(&plane[0], 0, 1);
	 * 	 make_denorm_double(&plane[3], 1, 1);
	 * 	 break;
	 */
	case MODE_OVERFLOW:
		plane[0] = 1.0e300;
		plane[3] = 1.0e-300;
		break;
	default:
		; /* nothing */
	}

	/* Send plane to GL to use for clipping */
	glClipPlane(GL_CLIP_PLANE0, plane);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	glEnable(GL_CLIP_PLANE0);

	/* Some vertex positions */
	glBegin(GL_POLYGON);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 0,  1);
	glEnd();
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glDisable(GL_CLIP_PLANE0);

	return pass;
}


/** 
 * Pass large doubles to OpenGL and see what
 * happens when converted to float. 
 */
bool
test_float_overflow(void)
{
	bool pass = true;
	int i;
	GLdouble v[3][4];
	GLdouble mat[16];

	/* Make some nice vertices */
	for (i = 0; i < 3; i++) {
		v[i][0] = 0.0;
		v[i][1] = 0.0;
		v[i][2] = 0.0;
		v[i][3] = 1.0;
	}

	/* Set problematic values */
	v[0][0] = 1.0e300;
	v[0][1] = -1.0e300;
	v[1][0] = 1.0e-300;
	v[1][1] = 1.0e-300;

	/* Create a problematic matrix */
	/* Identity * a scalar value of 1e100 */
	for (i = 0; i < 15; i++)
		mat[i] = 0.0;
	mat[0] = mat[5] = mat[10] = mat[15] = 1.0e100;

	/*
	 * Why are these functions the double version? 
	 *
	 * Answer: Because the GL driver may not support double precision 
	 * and may automatically convert the doubles to floats. 
	 * (See glLoadMatrix in OpenGL 2.1 Reference Pages)
	 */

	/* Send matrix to GL */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(mat);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* Send vertices to GL */
	glBegin(GL_POLYGON);
	glVertex4dv(v[0]);
	glVertex4dv(v[1]);
	glVertex4dv(v[2]);
	glEnd();
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glPopMatrix();

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	/* 
	 * These tests are supposed to succeed.  GL is not supposed to
	 * throw errors for these things. 
	 * Original Glean test passes in every case.
	 */
	pass &= test_vertices(MODE_INFINITY);
	pass &= test_vertices(MODE_NAN);
	pass &= test_vertices(MODE_DIVZERO);
	pass &= test_vertices(MODE_DENORM);

	pass &= test_transformation(MODE_INFINITY);
	pass &= test_transformation(MODE_NAN);
	pass &= test_transformation(MODE_DIVZERO);
	pass &= test_transformation(MODE_DENORM);

	pass &= test_clipping(MODE_INFINITY);
	pass &= test_clipping(MODE_NAN);
	pass &= test_clipping(MODE_DIVZERO);
	pass &= test_clipping(MODE_DENORM);
	pass &= test_clipping(MODE_OVERFLOW);

	pass &= test_float_overflow();

	if (!piglit_automatic)
		piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
} /* piglit_display */
