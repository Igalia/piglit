/*!

[config]
name: Integer Definitions
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Integer definitions from spec ##

[test]
name: Char Definitions
kernel_name: test_char
arg_out: 0 buffer int[6] 8 127 -128 127 -128 255

[test]
name: Short Definitions
kernel_name: test_short
arg_out: 0 buffer int[3] 32767 -32768 65535

[test]
name: Int Definitions
kernel_name: test_int
arg_out: 0 buffer int[3] 2147483647 -2147483648 0xffffffff

[test]
name: Long Definitions
kernel_name: test_long
arg_out: 0 buffer long[3] 9223372036854775807 \
                          -9223372036854775808 \
                          18446744073709551615
!*/

kernel void test_char(global int* out) {
  int i = 0;
  out[i++] = CHAR_BIT;
  out[i++] = CHAR_MAX;
  out[i++] = CHAR_MIN;
  out[i++] = SCHAR_MAX;
  out[i++] = SCHAR_MIN;
  out[i++] = UCHAR_MAX;
}

kernel void test_short(global int* out) {
  int i = 0;
  out[i++] = SHRT_MAX;
  out[i++] = SHRT_MIN;
  out[i++] = USHRT_MAX;
}

kernel void test_int(global int* out) {
  int i = 0;
  out[i++] = (INT_MAX - (int2)(0)).s0;
  out[i++] = (INT_MIN - (int2)(0)).s0;
  out[i++] = (UINT_MAX - (uint2)(0)).s0;
}

kernel void test_long(global long* out) {
  int i = 0;
  out[i++] = (LONG_MAX - (long2)(0)).s0;
  out[i++] = (LONG_MIN - (long2)(0)).s0;
  out[i++] = (ULONG_MAX - (ulong2)(0)).s0;
}
