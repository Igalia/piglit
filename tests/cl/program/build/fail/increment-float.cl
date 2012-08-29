/*!
[config]
name: Increment operator on float
clc_version_min: 10
expect_build_fail: true
!*/

kernel void() {
	float f;
	f++;
}
