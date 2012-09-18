/* The OpenCL kernel comes from GEGL (www.gegl.org)
 * File: gegl/opencl/gegl-cl-color-kernel.h
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2012 Victor Oliveira (victormatheus@gmail.com)
 */


/*!
[config]
name: GEGL gamma_2_2_to_linear
clc_version_min: 10
kernel_name: gamma_2_2_to_linear

[test]
name: Input = 0.3928
arg_in:  1 float           0.03928
arg_out: 0 buffer float[1] 0.00304 tolerance 0.00001

[test]
name: Input = 1.0
arg_in:  1 float           1.0
arg_out: 0 buffer float[1] 1.0

[test]
name: Input = -12.92
arg_in:  1 float           -12.92
arg_out: 0 buffer float[1] -1.0

[test]
name: Input = 0.5
arg_in:  1 float           0.5
arg_out: 0 buffer float[1] 0.214041 tolerance 0.00001
!*/

kernel void gamma_2_2_to_linear (global float *out, float value)
{
  if (value > 0.03928f)
    out[0] =  native_powr ((value + 0.055f) / 1.055f, 2.4f);
  else
    out[0] = value / 12.92f;
}
