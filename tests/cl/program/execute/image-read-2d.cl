/*!
[config]
name: 2D image reading

dimensions:  1
global_size: 1 0 0
local_size:  1 0 0

[test]
name: read float from CL_FLOAT CL_RGBA image.
kernel_name: readf

arg_in:  1 image float4  0.0 0.0 0.0 0.0  0.0 0.0 0.0 0.0 \
                         0.0 0.0 0.0 0.0  0.0 0.0 0.0 0.0 \
                         0.0 0.0 0.0 0.0  0.1 0.2 0.3 0.4 \
           image_type 2d                                  \
           image_width 2                                  \
           image_height 3                                 \
           image_channel_order RGBA                       \
           image_channel_data_type FLOAT
arg_in:  2 sampler normalized_coords 0                    \
                   addressing_mode NONE                   \
                   filter_mode NEAREST
arg_in:  3 int2     1   2
arg_in:  4 float2 1.5 2.5
arg_out: 0 buffer float4[2] 0.1 0.2 0.3 0.4 \
                            0.1 0.2 0.3 0.4

[test]
name: read signed integer from CL_SIGNED_INT8 CL_RGBA image.
kernel_name: readi

arg_in:  1 image char4   0 0 0 0     0   0   0   0 \
                         0 0 0 0     0   0   0   0 \
                         0 0 0 0  -128  -1   0 127 \
           image_type 2d                           \
           image_width 2                           \
           image_height 3                          \
           image_channel_order RGBA                \
           image_channel_data_type SIGNED_INT8
arg_in:  2 sampler normalized_coords 0             \
                   addressing_mode NONE            \
                   filter_mode NEAREST
arg_in:  3 int2     1   2
arg_in:  4 float2 1.5 2.5
arg_out: 0 buffer int4[2]  -128 -1 0 127 \
                           -128 -1 0 127

[test]
name: read unsigned integer from CL_UNSIGNED_INT8 CL_RGBA image.
kernel_name: readui

arg_in:  1 image uchar4  0 0 0 0     0   0   0   0 \
                         0 0 0 0     0   0   0   0 \
                         0 0 0 0     0   1 127 255 \
           image_type 2d                           \
           image_width 2                           \
           image_height 3                          \
           image_channel_order RGBA                \
           image_channel_data_type UNSIGNED_INT8
arg_in:  2 sampler normalized_coords 0             \
                   addressing_mode NONE            \
                   filter_mode NEAREST
arg_in:  3 int2     1   2
arg_in:  4 float2 1.5 2.5
arg_out: 0 buffer uint4[2] 0 1 127 255 \
                           0 1 127 255
!*/

kernel void readf(global float4 *out, read_only image2d_t img, sampler_t s,
                  int2 coords_i, float2 coords_f) {
	out[0] = read_imagef(img, s, coords_i);
	out[1] = read_imagef(img, s, coords_f);
}

kernel void readi(global int4 *out, read_only image2d_t img, sampler_t s,
                  int2 coords_i, float2 coords_f) {
	out[0] = read_imagei(img, s, coords_i);
	out[1] = read_imagei(img, s, coords_f);
}

kernel void readui(global uint4 *out, read_only image2d_t img, sampler_t s,
                  int2 coords_i, float2 coords_f) {
	out[0] = read_imageui(img, s, coords_i);
	out[1] = read_imageui(img, s, coords_f);
}
