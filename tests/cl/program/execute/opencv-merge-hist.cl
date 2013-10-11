/*!
[config]
name: OpenCV merge-hist
kernel_name: merge_hist
dimensions: 1
global_size: 65536 1 1
local_size: 256 1 1

# This test checks that the loop:
# for(int stride = HISTOGRAM256_WORK_GROUP_SIZE /2; stride > 0; stride >>= 1)
# in this kernel is unrolled correctly or not unrolled at all.  The Clang
# OpenCL frontend was unrolling this in way that created illegal uses of the
# barrier() builtin which resulted in GPU hangs on r600g.
[test]
arg_in:  0 buffer int[65536] repeat 0
arg_out: 1 buffer int[256]   repeat 0
arg_in:  2 int 256

!*/

// The kernel in this test is taken from the opencv library (opencv.org)
// File: modules/ocl/src/opencl/imgproc_histogram.cl
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2010-2012, Institute Of Software Chinese Academy Of Science, all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Copyright (C) 2010-2012, Multicoreware, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// @Authors
//    Niko Li, newlife20080214@gmail.com
//    Jia Haipeng, jiahaipeng95@gmail.com
//    Xu Pang, pangxu010@163.com
//    Wenju He, wenju@multicorewareinc.com
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other GpuMaterials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors as is and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//


#define PARTIAL_HISTOGRAM256_COUNT     (256)
#define HISTOGRAM256_BIN_COUNT         (256)

#define HISTOGRAM256_WORK_GROUP_SIZE     (256)
#define HISTOGRAM256_LOCAL_MEM_SIZE      (HISTOGRAM256_BIN_COUNT)

#define NBANKS (16)
#define NBANKS_BIT (4)


__kernel __attribute__((reqd_work_group_size(256,1,1)))void merge_hist(__global int* buf,
                __global int* hist,
                int src_step)
{
    int lx = get_local_id(0);
    int gx = get_group_id(0);

    int sum = 0;

    for(int i = lx; i < PARTIAL_HISTOGRAM256_COUNT; i += HISTOGRAM256_WORK_GROUP_SIZE)
        sum += buf[ mad24(i, src_step, gx)];

    __local int data[HISTOGRAM256_WORK_GROUP_SIZE];
    data[lx] = sum;

    for(int stride = HISTOGRAM256_WORK_GROUP_SIZE /2; stride > 0; stride >>= 1)
    {
        barrier(CLK_LOCAL_MEM_FENCE);
        if(lx < stride)
            data[lx] += data[lx + stride];
    }

    if(lx == 0)
        hist[gx] = data[0];
}
