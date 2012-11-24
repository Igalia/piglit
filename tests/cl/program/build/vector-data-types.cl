/*!
[config]
name: Vector data types declarations
clc_version_min: 10
!*/

kernel void test(global int* out) {
	char2    c2; char4    c4; char8    c8; char16    c16;
	uchar2  uc2; uchar4  uc4; uchar8  uc8; uchar16  uc16;
	short2   s2; short4   s4; short8   s8; short16   s16;
	ushort2 us2; ushort4 us4; ushort8 us8; ushort16 us16;
	int2     i2; int4     i4; int8     i8; int16     i16;
	uint2   ui2; uint4   ui4; uint8   ui8; uint16   ui16;
	long2    l2; long4    l4; long8    l8; long16    l16;
	ulong2  ul2; ulong4  ul4; ulong8  ul8; ulong16  ul16;
	float2   f2; float4   f4; float8   f8; float16   f16;

#if __OPENCL_C_VERSION__ >= 110
	char3    c3;
	uchar3  uc3;
	short3   s3;
	ushort3 us3;
	int3     i3;
	uint3   ui3;
	long3    l3;
	ulong3  ul3;
	float3   f3;
#endif

#if __OPENCL_C_VERSION__ >= 120
	double2 d2; double3 d3; double4 d4; double8 d8; double16 d16;
#endif
}
