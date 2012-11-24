/*!
# This test is only valid if the device supports images
# i.e. CL_DEVICE_IMAGE_SUPPORT is CL_TRUE

[config]
name: Other data types declarations (image, sampler, event)
clc_version_min: 10
!*/

kernel void test(global int* out) {
	image2d_t i2;
	image3d_t i3;
	sampler_t s;
	event_t   e;

#if __OPENCL_C_VERSION__ >= 120
	image1d_t        i1;
	image1d_array_t  i1a;
	image1d_buffer_t i1b;
	image2d_array_t  i2a;
#endif
}
