/*!
[config]
name: 2D image attributes

dimensions:  1
global_size: 1 0 0
local_size:  1 0 0

[test]
name: write unsigned integer to CL_UNSIGNED_INT8 CL_RGBA image.
kernel_name: attr

arg_out: 0 buffer int[6]  2 3 2 3 1 1
arg_in:  1 image uchar4  0 0 0 0  0 0 0 0        \
                         0 0 0 0  0 0 0 0        \
                         0 0 0 0  0 0 0 0        \
           image_type 2d                         \
           image_width 2                         \
           image_height 3                        \
           image_channel_order RGBA              \
           image_channel_data_type UNSIGNED_INT8
!*/

kernel void attr(global int *out, image2d_t img) {
	out[0] = get_image_width(img);
	out[1] = get_image_height(img);
	out[2] = get_image_dim(img).x;
	out[3] = get_image_dim(img).y;
	out[4] = get_image_channel_order(img) == CLK_RGBA;
	out[5] = get_image_channel_data_type(img) == CLK_UNSIGNED_INT8;
}
