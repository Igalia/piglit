/*
 * Copyright (c) VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
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
 */


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "piglit-matrix.h"


#define DEG_TO_RAD(D) ((D) * M_PI / 180.0)


/**
 * Create a scaling matrix.
 */
void
piglit_identity_matrix(float mat[16])
{
   mat[0] = 1.0f;
   mat[1] = 0.0f;
   mat[2] = 0.0f;
   mat[3] = 0.0f;

   mat[4] = 0.0f;
   mat[5] = 1.0f;
   mat[6] = 0.0f;
   mat[7] = 0.0f;

   mat[8] = 0.0f;
   mat[9] = 0.0f;
   mat[10] = 1.0f;
   mat[11] = 0.0f;

   mat[12] = 0.0f;
   mat[13] = 0.0f;
   mat[14] = 0.0f;
   mat[15] = 1.0f;
}


/**
 * Create a scaling matrix.
 */
void
piglit_scale_matrix(float mat[16], float sx, float sy, float sz)
{
   mat[0] = sx;
   mat[1] = 0.0f;
   mat[2] = 0.0f;
   mat[3] = 0.0f;

   mat[4] = 0.0f;
   mat[5] = sy;
   mat[6] = 0.0f;
   mat[7] = 0.0f;

   mat[8] = 0.0f;
   mat[9] = 0.0f;
   mat[10] = sz;
   mat[11] = 0.0f;

   mat[12] = 0.0f;
   mat[13] = 0.0f;
   mat[14] = 0.0f;
   mat[15] = 1.0f;
}


/**
 * Create a translation matrix.
 */
void
piglit_translation_matrix(float mat[16], float tx, float ty, float tz)
{
   mat[0] = 0.0f;
   mat[1] = 0.0f;
   mat[2] = 0.0f;
   mat[3] = 0.0f;

   mat[4] = 0.0f;
   mat[5] = 0.0f;
   mat[6] = 0.0f;
   mat[7] = 0.0f;

   mat[8] = 0.0f;
   mat[9] = 0.0f;
   mat[10] = 0.0f;
   mat[11] = 0.0f;

   mat[12] = tx;
   mat[13] = ty;
   mat[14] = tz;
   mat[15] = 1.0f;
}


void
piglit_rotation_matrix(float mat[16], float angle, float x, float y, float z)
{
   /* Implementation borrowed from Mesa */
   float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c, s, c;
   float m[16];
   bool optimized = false;

   s = (float) sin(DEG_TO_RAD(angle));
   c = (float) cos(DEG_TO_RAD(angle));

   piglit_identity_matrix(m);

#define M(row,col)  m[col*4+row]

   if (x == 0.0F) {
      if (y == 0.0F) {
         if (z != 0.0F) {
            optimized = true;
            /* rotate only around z-axis */
            M(0,0) = c;
            M(1,1) = c;
            if (z < 0.0F) {
               M(0,1) = s;
               M(1,0) = -s;
            }
            else {
               M(0,1) = -s;
               M(1,0) = s;
            }
         }
      }
      else if (z == 0.0F) {
         optimized = true;
         /* rotate only around y-axis */
         M(0,0) = c;
         M(2,2) = c;
         if (y < 0.0F) {
            M(0,2) = -s;
            M(2,0) = s;
         }
         else {
            M(0,2) = s;
            M(2,0) = -s;
         }
      }
   }
   else if (y == 0.0F) {
      if (z == 0.0F) {
         optimized = true;
         /* rotate only around x-axis */
         M(1,1) = c;
         M(2,2) = c;
         if (x < 0.0F) {
            M(1,2) = s;
            M(2,1) = -s;
         }
         else {
            M(1,2) = -s;
            M(2,1) = s;
         }
      }
   }

   if (!optimized) {
      const float mag = sqrtf(x * x + y * y + z * z);

      if (mag <= 1.0e-4) {
         /* no rotation, leave mat as-is */
         return;
      }

      x /= mag;
      y /= mag;
      z /= mag;


      /*
       *     Arbitrary axis rotation matrix.
       *
       *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
       *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
       *  (which is about the X-axis), and the two composite transforms
       *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
       *  from the arbitrary axis to the X-axis then back.  They are
       *  all elementary rotations.
       *
       *  Rz' is a rotation about the Z-axis, to bring the axis vector
       *  into the x-z plane.  Then Ry' is applied, rotating about the
       *  Y-axis to bring the axis vector parallel with the X-axis.  The
       *  rotation about the X-axis is then performed.  Ry and Rz are
       *  simply the respective inverse transforms to bring the arbitrary
       *  axis back to its original orientation.  The first transforms
       *  Rz' and Ry' are considered inverses, since the data from the
       *  arbitrary axis gives you info on how to get to it, not how
       *  to get away from it, and an inverse must be applied.
       *
       *  The basic calculation used is to recognize that the arbitrary
       *  axis vector (x, y, z), since it is of unit length, actually
       *  represents the sines and cosines of the angles to rotate the
       *  X-axis to the same orientation, with theta being the angle about
       *  Z and phi the angle about Y (in the order described above)
       *  as follows:
       *
       *  cos ( theta ) = x / sqrt ( 1 - z^2 )
       *  sin ( theta ) = y / sqrt ( 1 - z^2 )
       *
       *  cos ( phi ) = sqrt ( 1 - z^2 )
       *  sin ( phi ) = z
       *
       *  Note that cos ( phi ) can further be inserted to the above
       *  formulas:
       *
       *  cos ( theta ) = x / cos ( phi )
       *  sin ( theta ) = y / sin ( phi )
       *
       *  ...etc.  Because of those relations and the standard trigonometric
       *  relations, it is pssible to reduce the transforms down to what
       *  is used below.  It may be that any primary axis chosen will give the
       *  same results (modulo a sign convention) using thie method.
       *
       *  Particularly nice is to notice that all divisions that might
       *  have caused trouble when parallel to certain planes or
       *  axis go away with care paid to reducing the expressions.
       *  After checking, it does perform correctly under all cases, since
       *  in all the cases of division where the denominator would have
       *  been zero, the numerator would have been zero as well, giving
       *  the expected result.
       */

      xx = x * x;
      yy = y * y;
      zz = z * z;
      xy = x * y;
      yz = y * z;
      zx = z * x;
      xs = x * s;
      ys = y * s;
      zs = z * s;
      one_c = 1.0F - c;

      /* We already hold the identity-matrix so we can skip some statements */
      M(0,0) = (one_c * xx) + c;
      M(0,1) = (one_c * xy) - zs;
      M(0,2) = (one_c * zx) + ys;
/*    M(0,3) = 0.0F; */

      M(1,0) = (one_c * xy) + zs;
      M(1,1) = (one_c * yy) + c;
      M(1,2) = (one_c * yz) - xs;
/*    M(1,3) = 0.0F; */

      M(2,0) = (one_c * zx) - ys;
      M(2,1) = (one_c * yz) + xs;
      M(2,2) = (one_c * zz) + c;
/*    M(2,3) = 0.0F; */

/*
      M(3,0) = 0.0F;
      M(3,1) = 0.0F;
      M(3,2) = 0.0F;
      M(3,3) = 1.0F;
*/
   }
#undef M
}


void
piglit_ortho_matrix(float mat[16],
		    float left, float right,
		    float bottom, float top,
		    float nearval, float farval)
{
#define M(row,col)  mat[col*4+row]
   M(0,0) = 2.0F / (right-left);
   M(0,1) = 0.0F;
   M(0,2) = 0.0F;
   M(0,3) = -(right+left) / (right-left);

   M(1,0) = 0.0F;
   M(1,1) = 2.0F / (top-bottom);
   M(1,2) = 0.0F;
   M(1,3) = -(top+bottom) / (top-bottom);

   M(2,0) = 0.0F;
   M(2,1) = 0.0F;
   M(2,2) = -2.0F / (farval-nearval);
   M(2,3) = -(farval+nearval) / (farval-nearval);

   M(3,0) = 0.0F;
   M(3,1) = 0.0F;
   M(3,2) = 0.0F;
   M(3,3) = 1.0F;
#undef M
}


void
piglit_frustum_matrix(float mat[16],
		      float left, float right,
		      float bottom, float top,
		      float nearval, float farval)
{
   float x, y, a, b, c, d;

   x = (2.0F*nearval) / (right-left);
   y = (2.0F*nearval) / (top-bottom);
   a = (right+left) / (right-left);
   b = (top+bottom) / (top-bottom);
   c = -(farval+nearval) / ( farval-nearval);
   d = -(2.0F*farval*nearval) / (farval-nearval);  /* error? */

#define M(row,col)  mat[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M
}


void
piglit_matrix_mul_matrix(float product[16],
                         const float a[16], const float b[16])
{
#define ELEM(MAT, ROW, COL)  MAT[(COL) * 4 + (ROW)]
   float tmp[16];
   int i, j;

   for (i = 0; i < 4; i++) {
      for (j = 0; j < 4; j++) {
         ELEM(tmp, i, j) = (ELEM(a, i, 0) * ELEM(b, 0, j) +
                            ELEM(a, i, 1) * ELEM(b, 1, j) +
                            ELEM(a, i, 2) * ELEM(b, 2, j) +
                            ELEM(a, i, 3) * ELEM(b, 3, j));
      }
   }

   for (i = 0; i < 16; i++) {
      product[i] = tmp[i];
   }
#undef ELEM
}


/**
 * Compute "out = mat * in" where in and out are column vectors
 * Typically used to transform homogeneous coordinates by a matrix.
 */
void
piglit_matrix_mul_vector(float out[4],
                         const float mat[16],
                         const float in[4])
{
   const float in0 = in[0], in1 = in[1], in2 = in[2], in3 = in[3];
#define M(row,col)  mat[row + col*4]
   out[0] = M(0,0) * in0 + M(0,1) * in1 + M(0,2) * in2 + M(0,3) * in3;
   out[1] = M(1,0) * in0 + M(1,1) * in1 + M(1,2) * in2 + M(1,3) * in3;
   out[2] = M(2,0) * in0 + M(2,1) * in1 + M(2,2) * in2 + M(2,3) * in3;
   out[3] = M(3,0) * in0 + M(3,1) * in1 + M(3,2) * in2 + M(3,3) * in3;
#undef M
}


/**
 * Transfrom NDC coordinate to window coordinate using a viewport.
 */
void
piglit_ndc_to_window(float win[3],
                     const float ndc[4],
                     int vp_left, int vp_bottom, int vp_width, int vp_height)
{
   float x = ndc[0] * 0.5 + 0.5;
   float y = ndc[1] * 0.5 + 0.5;
   float z = ndc[2] * 0.5 + 0.5;
   win[0] = vp_left + x * vp_width;
   win[1] = vp_bottom + y * vp_height;
   win[2] = z;
}


/**
 * Transform an object coordinate to a window coordinate using a
 * modelview matrix, projection matrix and viewport.
 * \return true for success, false if coordinate is clipped away
 */
bool
piglit_project_to_window(float win[3],
                         const float obj[4],
                         const float modelview[16],
                         const float projection[16],
                         int vp_left, int vp_bottom,
                         int vp_width, int vp_height)
{
   float eye[4], clip[4], ndc[4];

   /* eye coord = modelview * object */
   piglit_matrix_mul_vector(eye, modelview, obj);

   /* clip coord = projection * eye */
   piglit_matrix_mul_vector(clip, projection, eye);

   /* view volume clipping */
   if ( clip[0] > clip[3] ||
       -clip[0] > clip[3] ||
        clip[1] > clip[3] ||
       -clip[1] > clip[3] ||
        clip[2] > clip[3] ||
       -clip[2] > clip[3]) {
      /* clipped */
      return false;
   }

   /* ndc = clip / clip.w (divide by w) */
   ndc[0] = clip[0] / clip[3];
   ndc[1] = clip[1] / clip[3];
   ndc[2] = clip[2] / clip[3];
   ndc[3] = clip[3];

   /* window = viewport_map(ndc) */
   piglit_ndc_to_window(win, ndc, vp_left, vp_bottom, vp_width, vp_height);

   return true;
}


void
piglit_print_matrix(const float mat[16])
{
   printf("%f %f %f %f\n", mat[0], mat[4], mat[8], mat[12]);
   printf("%f %f %f %f\n", mat[1], mat[5], mat[9], mat[13]);
   printf("%f %f %f %f\n", mat[2], mat[6], mat[10], mat[14]);
   printf("%f %f %f %f\n", mat[3], mat[7], mat[11], mat[15]);
}


/**
 * Invert matrix using cramer's rule.
 * This assumes that the matrix is non-singular (or non-near-singular).
 */
void
piglit_matrix_inverse(float inv[16], const float m[16])
{
	inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] -
		 m[9] * m[6] * m[15] + m[9] * m[7] * m[14] +
		 m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] +
		 m[8] * m[6] * m[15] - m[8] * m[7] * m[14] -
		 m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] -
		 m[8] * m[5] * m[15] + m[8] * m[7] * m[13] +
		 m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] +
		  m[8] * m[5] * m[14] - m[8] * m[6] * m[13] -
		  m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] +
		 m[9] * m[2] * m[15] - m[9] * m[3] * m[14] -
		 m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] -
		 m[8] * m[2] * m[15] + m[8] * m[3] * m[14] +
		 m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] +
		 m[8] * m[1] * m[15] - m[8] * m[3] * m[13] -
		 m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] -
		  m[8] * m[1] * m[14] + m[8] * m[2] * m[13] +
		  m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] -
		 m[5] * m[2] * m[15] + m[5] * m[3] * m[14] +
		 m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] +
		 m[4] * m[2] * m[15] - m[4] * m[3] * m[14] -
		 m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] -
		  m[4] * m[1] * m[15] + m[4] * m[3] * m[13] +
		  m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] +
		  m[4] * m[1] * m[14] - m[4] * m[2] * m[13] -
		  m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] +
		 m[5] * m[2] * m[11] - m[5] * m[3] * m[10] -
		 m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] -
		 m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
		 m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] +
		  m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
		  m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] -
		  m[4] * m[1] * m[10] + m[4] * m[2] * m[9] +
		  m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

	float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	assert(fabsf(det) > 1e-10f);

	for (int i = 0; i < 16; i++)
		inv[i] = inv[i] / det;
}


void
piglit_matrix_transpose(float out[16], const float m[16])
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			out[i+4*j] = m[4*i+j];
}


/*
 * XXX to do items:
 * We could add a simple set of matrix stack functions.
 * Could add scale/translate/rotate functions that accumulate onto
 *   the incoming matrix, similar to glScale, glTranslate, etc.
 */
