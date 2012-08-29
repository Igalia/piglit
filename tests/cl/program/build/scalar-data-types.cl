/*!
[config]
name: Scalar data types declarations
clc_version_min: 10
!*/

kernel void test() {
	bool b;
	char c;
	unsigned char uc1; uchar uc2;
	short s;
	unsigned short us1; ushort us2;
	int i;
	unsigned int ui1; uint ui2;
	long l;
	unsigned long ul1; ulong ul2;
	float f;
	size_t st;
	ptrdiff_t pt;
	intptr_t it;
	uintptr_t uit;
	//half h; // Can be only used as a pointer to a buffer

// Needs cl_khr_fp64 or OpenCL C 1.2
#if __OPENCL_C_VERSION__ >= 120
	double d;
	double* d;
#endif
}
