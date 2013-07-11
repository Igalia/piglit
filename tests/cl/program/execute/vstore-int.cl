/*!
[config]
name: Vector store int2,3,4,8,16
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

[test]
name: vector store int2
kernel_name: vstore2_test
arg_out: 0 buffer int[2] 56 65
arg_in: 1 buffer int[2] 56 65

[test]
name: vector store int2 with offset
kernel_name: vstore2_offset
arg_out: 0 buffer int[4] 0 0 56 65
arg_in: 1 buffer int[2] 56 65

[test]
name: vector store int3
kernel_name: vstore3_test
arg_out: 0 buffer int[3] 56 65 12
arg_in: 1 buffer int[3] 56 65 12

[test]
name: vector store int3 with offset
kernel_name: vstore3_offset
arg_out: 0 buffer int[6] 0 0 0 56 65 12
arg_in: 1 buffer int[3] 56 65 12

[test]
name: vector store int4
kernel_name: vstore4_test
arg_out: 0 buffer int[4] 56 65 18 81
arg_in: 1 buffer int[4] 56 65 18 81

[test]
name: vector store int4 with offset
kernel_name: vstore4_offset
arg_out: 0 buffer int[8] 0 0 0 0 56 65 18 81
arg_in: 1 buffer int[4] 56 65 18 81

[test]
name: vector store int8
kernel_name: vstore8_test
arg_out: 0 buffer int[8] 56 65 18 81 12 21 34 43
arg_in: 1 buffer int[8] 56 65 18 81 12 21 34 43

[test]
name: vector store int8 with offset
kernel_name: vstore8_offset
arg_out: 0 buffer int[16] 0 0 0 0 0 0 0 0 56 65 18 81 12 21 34 43
arg_in: 1 buffer int[8] 56 65 18 81 12 21 34 43

[test]
name: vector store int16
kernel_name: vstore16_test
arg_out: 0 buffer int[16] 56 65 18 81 12 21 34 43 23 32 67 76 78 87 89 98
arg_in: 1 buffer int[16] 56 65 18 81 12 21 34 43 23 32 67 76 78 87 89 98

[test]
name: vector store int16 with offset
kernel_name: vstore16_offset
arg_out: 0 buffer int[32] 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
                         56 65 18 81 12 21 34 43 23 32 67 76 78 87 89 98
arg_in: 1 buffer int[16] 56 65 18 81 12 21 34 43 23 32 67 76 78 87 89 98
!*/

kernel void vstore2_test(global int* out, global int* in) {
  int2 val = {in[0], in[1]};
  vstore2(val, 0, out);
}

kernel void vstore2_offset(global int* out, global int* in) {
  int2 val = {in[0], in[1]};
  vstore2((int2)0, 0, out);
  vstore2(val, 1, out);
}

kernel void vstore3_test(global int* out, global int* in) {
  int3 val = {in[0], in[1], in[2]};
  vstore3(val, 0, out);
}

kernel void vstore3_offset(global int* out, global int* in) {
  int3 val = {in[0], in[1], in[2]};
  vstore3((int3)0, 0, out);
  vstore3(val, 1, out);
}

kernel void vstore4_test(global int* out, global int* in) {
  int4 val = {in[0], in[1], in[2], in[3]};
  vstore4(val, 0, out);
}

kernel void vstore4_offset(global int* out, global int* in) {
  int4 val = {in[0], in[1], in[2], in[3]};
  vstore4((int4)0, 0, out);
  vstore4(val, 1, out);
}

kernel void vstore8_test(global int* out, global int* in) {
  int8 val = {in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7]};
  vstore8(val, 0, out);
}

kernel void vstore8_offset(global int* out, global int* in) {
  int8 val = {in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7]};
  vstore8((int8)0, 0, out);
  vstore8(val, 1, out);
}

kernel void vstore16_test(global int* out, global int* in) {
  int16 val = {in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7],
               in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15]};
  vstore16(val, 0, out);
}

kernel void vstore16_offset(global int* out, global int* in) {
  int16 val = {in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7],
               in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15]};
  vstore16((int16)0, 0, out);
  vstore16(val, 1, out);
}
