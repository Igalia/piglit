/*!
[config]
name: 2D image writing

dimensions:  2
global_size: 2 3 0
local_size:  1 1 0

[test]
name: write float to CL_FLOAT CL_RGBA image.
kernel_name: writef

arg_out: 0 image float4      0.0 0.1 0.2 0.3  0.4 0.5 0.6 0.7 \
                             1.0 1.1 1.2 1.3  1.4 1.5 1.6 1.7 \
                             2.0 2.1 2.2 2.3  2.4 2.5 2.6 2.7 \
           image_type 2d                                      \
           image_width 2                                      \
           image_height 3                                     \
           image_channel_order RGBA                           \
           image_channel_data_type FLOAT
arg_in:  1 buffer float4[6]  0.0 0.1 0.2 0.3  0.4 0.5 0.6 0.7 \
                             1.0 1.1 1.2 1.3  1.4 1.5 1.6 1.7 \
                             2.0 2.1 2.2 2.3  2.4 2.5 2.6 2.7 \

[test]
name: write signed integer to CL_SIGNED_INT8 CL_RGBA image.
kernel_name: writei

arg_out: 0 image char4      -0  -1  -2  -3   -4  -5  -6  -7 \
                           -10 -11 -12 -13  -14 -15 -16 -17 \
                           -20 -21 -22 -23  -24 -25 -26 -27 \
           image_type 2d                                    \
           image_width 2                                    \
           image_height 3                                   \
           image_channel_order RGBA                         \
           image_channel_data_type SIGNED_INT8
arg_in:  1 buffer int4[6]   -0  -1  -2  -3   -4  -5  -6  -7 \
                           -10 -11 -12 -13  -14 -15 -16 -17 \
                           -20 -21 -22 -23  -24 -25 -26 -27

[test]
name: write unsigned integer to CL_UNSIGNED_INT8 CL_RGBA image.
kernel_name: writeui

arg_out: 0 image uchar4     0  1  2  3   4  5  6  7 \
                           10 11 12 13  14 15 16 17 \
                           20 21 22 23  24 25 26 27 \
           image_type 2d                            \
           image_width 2                            \
           image_height 3                           \
           image_channel_order RGBA                 \
           image_channel_data_type UNSIGNED_INT8
arg_in:  1 buffer uint4[6]  0  1  2  3   4  5  6  7 \
                           10 11 12 13  14 15 16 17 \
                           20 21 22 23  24 25 26 27

!*/

kernel void writef(write_only image2d_t img, global float4 *colors) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	const int k = i + j * get_image_width(img);
	int2 coord = (int2)(i, j);
	write_imagef(img, coord, colors[k]);
}

kernel void writei(write_only image2d_t img, global int4 *colors) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	const int k = i + j * get_image_width(img);
	int2 coord = (int2)(i, j);
	write_imagei(img, coord, colors[k]);
}

kernel void writeui(write_only image2d_t img, global uint4 *colors) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	const int k = i + j * get_image_width(img);
	int2 coord = (int2)(i, j);
	write_imageui(img, coord, colors[k]);
}
