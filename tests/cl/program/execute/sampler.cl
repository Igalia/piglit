/*!
[config]
name: Sampler tests

dimensions:  1
global_size: 4 0 0
local_size:  1 0 0

[test]
name: read from image using linear filtering and unnormalized coords
kernel_name: readf_f_x

arg_in:  1 image float4   0.0 0.0 0.0 1.0 1.0 1.0 1.0 1.0       \
                          2.0 2.0 2.0 1.0 3.0 3.0 3.0 1.0       \
           image_type 2d                 \
           image_width 2                 \
           image_height 2                \
           image_channel_order RGBA \
           image_channel_data_type FLOAT
arg_in:  2 sampler normalized_coords 0   \
                   addressing_mode NONE  \
                   filter_mode LINEAR
arg_in:  3 buffer float2[4] 0.5 0.5  1.0 0.5  1.0 1.0  0.5 1.0
arg_out: 0 buffer float[4]      0.0      0.5      1.5      1.0

[test]
name: read from image using linear filtering and normalized coords
kernel_name: readf_f_x

arg_in:  1 image float4   0.0 0.0 0.0 1.0 1.0 1.0 1.0 1.0       \
                          2.0 2.0 2.0 1.0 3.0 3.0 3.0 1.0       \
           image_type 2d                 \
           image_width 2                 \
           image_height 2                \
           image_channel_order RGBA \
           image_channel_data_type FLOAT
arg_in:  2 sampler normalized_coords 1   \
                   addressing_mode NONE  \
                   filter_mode LINEAR
arg_in:  3 buffer float2[4] 0.25 0.25  0.5 0.25  0.5 0.5  0.25 0.5
arg_out: 0 buffer float[4]       0.0       0.5       1.5       1.0

[test]
name: read from image using clamp_to_edge addressing mode
kernel_name: readf_f_x

arg_in:  1 image float4   0.0 0.0 0.0 1.0 1.0 1.0 1.0 1.0       \
                          2.0 2.0 2.0 1.0 3.0 3.0 3.0 1.0       \
           image_type 2d                 \
           image_width 2                 \
           image_height 2                \
           image_channel_order RGBA \
           image_channel_data_type FLOAT
arg_in:  2 sampler normalized_coords 0   \
                   addressing_mode CLAMP_TO_EDGE  \
                   filter_mode NEAREST
arg_in:  3 buffer float2[4] -1.0 -1.0  3.0 -1.0  3.0 3.0  -1.0 3.0
arg_out: 0 buffer float[4]        0.0       1.0      3.0       2.0
!*/

kernel void readf_f_x(global float *out, read_only image2d_t img, sampler_t s,
                      global float2 *coords) {
	int i = get_global_id(0);
	out[i] = read_imagef(img, s, coords[i]).x;
}
