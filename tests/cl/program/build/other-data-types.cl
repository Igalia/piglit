/*!
# This test checks for other data types that are defined in the OpenCL
# specification.
#
# event_t is always expected to work.
# image2d_t, image3d_t, and sampler_t require image support
# image1d* and image2d_array_t require image support and CL 1.2+

[config]
name: Other data types declarations (image, sampler, event)
clc_version_min: 10
!*/

kernel void test(global int* out) {
    event_t e;
}

#ifdef __IMAGE_SUPPORT__
kernel void test_images_100(global int *out, image2d_t i2,
        image3d_t i3, sampler_t s) {
}

#if __OPENCL_C_VERSION__ >= 120
kernel void test_images_120(
        global int* out,
        image1d_t i1,
        image1d_array_t i1a,
        image1d_buffer_t i1b,
        image2d_array_t i2a) {
}
#endif
#endif