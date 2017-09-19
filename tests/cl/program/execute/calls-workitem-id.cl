/*!

[config]
name: calls workitem IDs
clc_version_min: 10

[test]
name: Callee function use get_global_id(0)
kernel_name: kernel_call_pass_get_global_id_0
dimensions: 1
global_size: 64 0 0
arg_out: 0 buffer uint[64] \
  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 \
 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 \
 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 \
 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63

[test]
name: Callee function use get_global_id 0..2
kernel_name: kernel_call_pass_get_global_id_012
dimensions: 3
global_size: 8 4 2
arg_out: 0 buffer uint[64] \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7 \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7 \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7 \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7

arg_out: 1 buffer uint[64] \
  0  0  0  0  0  0  0  0  1  1  1  1  1  1  1  1 \
  2  2  2  2  2  2  2  2  3  3  3  3  3  3  3  3 \
  0  0  0  0  0  0  0  0  1  1  1  1  1  1  1  1 \
  2  2  2  2  2  2  2  2  3  3  3  3  3  3  3  3

arg_out: 2 buffer uint[64] \
  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 \
  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 \
  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1 \
  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1

!*/

#define NOINLINE __attribute__((noinline))

NOINLINE
void func_get_global_id_0(volatile global uint* out)
{
    uint gid = get_global_id(0);
    out[gid] = gid;
}

kernel void kernel_call_pass_get_global_id_0(global uint *out)
{
    func_get_global_id_0(out);
}

NOINLINE
void func_get_global_id_012(volatile global uint* out0,
                            volatile global uint* out1,
                            volatile global uint* out2)
{
    uint id0 = get_global_id(0);
    uint id1 = get_global_id(1);
    uint id2 = get_global_id(2);
    uint flat_id = (id2 * get_global_size(1) + id1) * get_global_size(0) + id0;

    out0[flat_id] = id0;
    out1[flat_id] = id1;
    out2[flat_id] = id2;
}

kernel void kernel_call_pass_get_global_id_012(global uint *out0,
                                               global uint *out1,
                                               global uint *out2)
{
    func_get_global_id_012(out0, out1, out2);
}
