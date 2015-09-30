/*!
[config]
name: printf builtin
clc_version_min: 12
!*/

kernel void test_printf() {
	printf("%s\n", "test_printf");
}
