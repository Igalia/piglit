/* The OpenCL kernel comes from GEGL (www.gegl.org)
 * File: gegl/operations/common/gaussian-blur.c
 *
 * This file is an image processing operation for GEGL
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
 * Copyright 2006 Dominik Ernst <dernst@gmx.de>
 *
 * Recursive Gauss IIR Filter as described by Young / van Vliet
 * in "Signal Processing 44 (1995) 139 - 151"
 *
 */


/*!
[config]
name: GEGL fir_get_mean_component_1D_CL
clc_version_min: 10
kernel_name: fir_get_mean_component_1D_CL

[test]
arg_in:  1  buffer float4[5] repeat 1.0
arg_in:  2  int              0                  #offset
arg_in:  3  int              1                  #delta_offset
arg_in:  4  buffer float[5]  0.1 0.2 0.3 0.4 0.5
arg_in:  5  int              5                 #matrix_legnth

arg_out: 0 buffer float4[1] repeat 1.5
!*/

kernel void fir_get_mean_component_1D_CL(global float4 *out,
                                    const global float4 *buf,
                                    int offset,
                                    const int delta_offset,
                                    constant float *cmatrix,
                                    const int matrix_length)
{
    float4 acc = 0.0f;
    int i;

    for(i=0; i<matrix_length; i++)
      {
        acc    += buf[offset] * cmatrix[i];
        offset += delta_offset;
      }
    out[0] = acc;
}
