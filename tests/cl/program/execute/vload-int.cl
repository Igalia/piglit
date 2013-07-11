/*!
[config]
name: Vector load int2,3,4,8,16
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

[test]
name: vector load int2
kernel_name: vload2_test
arg_out: 0 buffer int[2] 56 65
arg_in: 1 buffer int[2] 56 65

[test]
name: vector load int2 with offset
kernel_name: vload2_offset
arg_out: 0 buffer int[2] 56 65
arg_in: 1 buffer int[4] 0 0 56 65

[test]
name: vector load int3
kernel_name: vload3_test
arg_out: 0 buffer int[3] 56 65 12
arg_in: 1 buffer int[3] 56 65 12

[test]
name: vector load int3 with offset
kernel_name: vload3_offset
arg_out: 0 buffer int[3] 56 65 12
arg_in: 1 buffer int[6] 0 0 0 56 65 12

[test]
name: vector load int4
kernel_name: vload4_test
arg_out: 0 buffer int[4] 56 65 18 81
arg_in: 1 buffer int[4] 56 65 18 81

[test]
name: vector load int4 with offset
kernel_name: vload4_offset
arg_out: 0 buffer int[4] 56 65 18 81
arg_in: 1 buffer int[8] 0 0 0 0 56 65 18 81

[test]
name: vector load int8
kernel_name: vload8_test
arg_out: 0 buffer int[8] 56 65 18 81 12 21 34 43
arg_in: 1 buffer int[8] 56 65 18 81 12 21 34 43

[test]
name: vector load int8 with offset
kernel_name: vload8_offset
arg_out: 0 buffer int[8] 56 65 18 81 12 21 34 43
arg_in: 1 buffer int[16] 0 0 0 0 0 0 0 0 56 65 18 81 12 21 34 43

[test]
name: vector load int16
kernel_name: vload16_test
arg_out: 0 buffer int[16] 56 65 18 81 12 21 34 43 23 32 67 76 78 87 89 98
arg_in: 1 buffer int[16] 56 65 18 81 12 21 34 43 23 32 67 76 78 87 89 98

[test]
name: vector load int16 with offset
kernel_name: vload16_offset
arg_out: 0 buffer int[16] 56 65 18 81 12 21 34 43 23 32 67 76 78 87 89 98
arg_in: 1 buffer int[32] 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
                          56 65 18 81 12 21 34 43 23 32 67 76 78 87 89 98
!*/

kernel void vload2_test(global int* out, global int* in) {
  int2 val = vload2(0, in);
  out[0] = val.s0;
  out[1] = val.s1;
}

kernel void vload2_offset(global int* out, global int* in) {
  int2 val = vload2(1, in);
  out[0] = val.s0;
  out[1] = val.s1;
}

kernel void vload3_test(global int* out, global int* in) {
  int3 val = vload3(0, in);
  out[0] = val.s0;
  out[1] = val.s1;
  out[2] = val.s2;
}

kernel void vload3_offset(global int* out, global int* in) {
  int3 val = vload3(1, in);
  out[0] = val.s0;
  out[1] = val.s1;
  out[2] = val.s2;
}

kernel void vload4_test(global int* out, global int* in) {
  int4 val = vload4(0, in);
  out[0] = val.s0;
  out[1] = val.s1;
  out[2] = val.s2;
  out[3] = val.s3;
}

kernel void vload4_offset(global int* out, global int* in) {
  int4 val = vload4(1, in);
  out[0] = val.s0;
  out[1] = val.s1;
  out[2] = val.s2;
  out[3] = val.s3;
}

kernel void vload8_test(global int* out, global int* in) {
  int8 val = vload8(0, in);
  out[0] = val.s0;
  out[1] = val.s1;
  out[2] = val.s2;
  out[3] = val.s3;
  out[4] = val.s4;
  out[5] = val.s5;
  out[6] = val.s6;
  out[7] = val.s7;
}

kernel void vload8_offset(global int* out, global int* in) {
  int8 val = vload8(1, in);
  out[0] = val.s0;
  out[1] = val.s1;
  out[2] = val.s2;
  out[3] = val.s3;
  out[4] = val.s4;
  out[5] = val.s5;
  out[6] = val.s6;
  out[7] = val.s7;
}

kernel void vload16_test(global int* out, global int* in) {
  int16 val = vload16(0, in);
  out[0] = val.s0;
  out[1] = val.s1;
  out[2] = val.s2;
  out[3] = val.s3;
  out[4] = val.s4;
  out[5] = val.s5;
  out[6] = val.s6;
  out[7] = val.s7;
  out[8] = val.s8;
  out[9] = val.s9;
  out[10] = val.sa;
  out[11] = val.sb;
  out[12] = val.sc;
  out[13] = val.sd;
  out[14] = val.se;
  out[15] = val.sf;
}

kernel void vload16_offset(global int* out, global int* in) {
  int16 val = vload16(1, in);
  out[0] = val.s0;
  out[1] = val.s1;
  out[2] = val.s2;
  out[3] = val.s3;
  out[4] = val.s4;
  out[5] = val.s5;
  out[6] = val.s6;
  out[7] = val.s7;
  out[8] = val.s8;
  out[9] = val.s9;
  out[10] = val.sa;
  out[11] = val.sb;
  out[12] = val.sc;
  out[13] = val.sd;
  out[14] = val.se;
  out[15] = val.sf;
}
