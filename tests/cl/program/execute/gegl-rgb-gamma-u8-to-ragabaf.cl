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
name: GEGL rgb_gamma_u8_to_ragabaf
clc_version_min: 11
kernel_name: rgb_gamma_u8_to_ragabaf
global_size: 208320 1 1
local_size: 1 1 1

[test]
name: A
arg_in:  0  buffer uchar[624960] repeat  0   0   0   \
                                         1   1   1   \
                                         200 200 200 \
                                         255 255 255

arg_out: 1  buffer float4[208320] repeat 0.0      0.0      0.0      1.0 \
                                         0.000303 0.000303 0.000303 1.0 \
                                         0.577580 0.577580 0.577580 1.0 \
                                         1.0      1.0      1.0      1.0 \
                                         tolerance 0.000001
!*/

float gamma_2_2_to_linear (float value)
{
  if (value > 0.03928f)
    return native_powr ((value + 0.055f) / 1.055f, 2.4f);
  return value / 12.92f;
}

__kernel void rgb_gamma_u8_to_ragabaf (__global const uchar  * in,
                                       __global       float4 * out)
{
  int gid = get_global_id(0);
  uchar3 in_vc;
  in_vc.x = in[(gid * 3)];
  in_vc.y = in[(gid * 3) + 1];
  in_vc.z = in[(gid * 3) + 2];
  float3 in_v  = convert_float3(in_vc) / 255.0f;
  float4 tmp_v;
  tmp_v = (float4)(gamma_2_2_to_linear(in_v.x),
                   gamma_2_2_to_linear(in_v.y),
                   gamma_2_2_to_linear(in_v.z),
                   1.0f);
  float4 out_v;
  out_v   = tmp_v * tmp_v.w;
  out_v.w = tmp_v.w;
  out[gid] = out_v;
}
