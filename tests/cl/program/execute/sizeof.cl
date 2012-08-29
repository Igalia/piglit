/*!

[config]
name: Sizeof operator
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## sizeof ##

[test]
name: sizeof
kernel_name: so
arg_out: 0 buffer int[61] 1  1  2  2  4  4   8   8  4 \  # 1
                          2  2  4  4  8  8  16  16  8 \  # 2
                          4  4  8  8 16 16  32  32 16 \  # 3
                          4  4  8  8 16 16  32  32 16 \  # 4
                          8  8 16 16 32 32  64  64 32 \  # 8
                         16 16 32 32 64 64 128 128 64 \  # 16
                          2 \  # half
                          8 16 32 32 64 128  # double

!*/

kernel void so(global int* out) {
	int i = 0;

	out[i++] = sizeof(char);
	out[i++] = sizeof(uchar);
	out[i++] = sizeof(short);
	out[i++] = sizeof(ushort);
	out[i++] = sizeof(int);
	out[i++] = sizeof(uint);
	out[i++] = sizeof(long);
	out[i++] = sizeof(ulong);
	out[i++] = sizeof(float);

	out[i++] = sizeof(char2);
	out[i++] = sizeof(uchar2);
	out[i++] = sizeof(short2);
	out[i++] = sizeof(ushort2);
	out[i++] = sizeof(int2);
	out[i++] = sizeof(uint2);
	out[i++] = sizeof(long2);
	out[i++] = sizeof(ulong2);
	out[i++] = sizeof(float2);

#if __OPENCL_C_VERSION__ >= 110
	out[i++] = sizeof(char3);
	out[i++] = sizeof(uchar3);
	out[i++] = sizeof(short3);
	out[i++] = sizeof(ushort3);
	out[i++] = sizeof(int3);
	out[i++] = sizeof(uint3);
	out[i++] = sizeof(long3);
	out[i++] = sizeof(ulong3);
	out[i++] = sizeof(float3);
#else
	out[i++] = 4;
	out[i++] = 4;
	out[i++] = 8;
	out[i++] = 8;
	out[i++] = 16;
	out[i++] = 16;
	out[i++] = 32;
	out[i++] = 32;
	out[i++] = 16;
#endif

	out[i++] = sizeof(char4);
	out[i++] = sizeof(uchar4);
	out[i++] = sizeof(short4);
	out[i++] = sizeof(ushort4);
	out[i++] = sizeof(int4);
	out[i++] = sizeof(uint4);
	out[i++] = sizeof(long4);
	out[i++] = sizeof(ulong4);
	out[i++] = sizeof(float4);

	out[i++] = sizeof(char8);
	out[i++] = sizeof(uchar8);
	out[i++] = sizeof(short8);
	out[i++] = sizeof(ushort8);
	out[i++] = sizeof(int8);
	out[i++] = sizeof(uint8);
	out[i++] = sizeof(long8);
	out[i++] = sizeof(ulong8);
	out[i++] = sizeof(float8);

	out[i++] = sizeof(char16);
	out[i++] = sizeof(uchar16);
	out[i++] = sizeof(short16);
	out[i++] = sizeof(ushort16);
	out[i++] = sizeof(int16);
	out[i++] = sizeof(uint16);
	out[i++] = sizeof(long16);
	out[i++] = sizeof(ulong16);
	out[i++] = sizeof(float16);

	out[i++] = sizeof(half);

#if __OPENCL_C_VERSION__ >= 120
	out[i++] = sizeof(double);
	out[i++] = sizeof(double2);
	out[i++] = sizeof(double3);
	out[i++] = sizeof(double4);
	out[i++] = sizeof(double8);
	out[i++] = sizeof(double16);
#else
	out[i++] = 8;
	out[i++] = 16;
	out[i++] = 32;
	out[i++] = 32;
	out[i++] = 64;
	out[i++] = 128;
#endif
}
