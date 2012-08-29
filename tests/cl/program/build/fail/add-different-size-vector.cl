/*!
[config]
name: Add different size vector
clc_version_min: 10
expect_build_fail: true
!*/

kernel void() {
	float4 f1;
	float3 f2;

	f1+f2;
}
