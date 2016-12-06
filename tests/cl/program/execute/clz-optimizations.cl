/*!

[config]
name: clz optimizations
clc_version_min: 10

dimensions: 1
global_size: 34 0 0

## clz ##

[test]
name: v_clz_u32
kernel_name: v_clz_u32

arg_out: 0 buffer int[34] \
  32  31  30  29  28  \
  27  26  25  24  23  \
  22  21  20  19  18  \
  17  16  15  14  13  \
  12  11  10  9   8   \
  7   6   5   4   3   \
  2   1   0   0

arg_in: 1 buffer uint[34] \
  0          1          2          4          8 \
  16         32         64         128        256 \
  512        1024       2048       4096       8192 \
  16384      32768      65536      131072     262144 \
  524288     1048576    2097152    4194304    8388608 \
  16777216   33554432   67108864   134217728  268435456 \
  536870912  1073741824 2147483648 4294967295


[test]
name: s_clz_u32
kernel_name: s_clz_u32
global_size: 6 0 0

arg_out: 0 buffer uint[6] \
  42  41  40  11  10 10

arg_in: 1 uint 0
arg_in: 2 uint 1
arg_in: 3 uint 2
arg_in: 4 uint 1073741824
arg_in: 5 uint 2147483648
arg_in: 6 uint 4294967295

[test]
name: s_firstbit_u32
kernel_name: s_firstbit_u32
global_size: 6 0 0

arg_out: 0 buffer uint[6] \
  9  41  40  11  10 10

arg_in: 1 uint 0
arg_in: 2 uint 1
arg_in: 3 uint 2
arg_in: 4 uint 1073741824
arg_in: 5 uint 2147483648
arg_in: 6 uint 4294967295


[test]
name: s_firstbit_cmp_result_u32
kernel_name: s_firstbit_cmp_result_u32
global_size: 6 0 0

arg_out: 0 buffer uint[6] \
  9  41  40  11  10 10

arg_in: 1 uint 0
arg_in: 2 uint 1
arg_in: 3 uint 2
arg_in: 4 uint 1073741824
arg_in: 5 uint 2147483648
arg_in: 6 uint 4294967295


[test]
name: v_firstbit_u32
kernel_name: v_firstbit_u32

arg_out: 0 buffer int[34] \
  0xffffffff  31  30  29  28  \
  27          26  25  24  23  \
  22          21  20  19  18  \
  17          16  15  14  13  \
  12          11  10  9   8   \
  7            6   5  4   3  \
  2            1   0  0

arg_in: 1 buffer uint[34] \
  0          1          2          4          8 \
  16         32         64         128        256 \
  512        1024       2048       4096       8192 \
  16384      32768      65536      131072     262144 \
  524288     1048576    2097152    4194304    8388608 \
  16777216   33554432   67108864   134217728  268435456 \
  536870912  1073741824 2147483648 4294967295


[test]
name: v_firstbit_u32_inv
kernel_name: v_firstbit_u32_inv

arg_out: 0 buffer int[34] \
  0xffffffff  31  30  29  28  \
  27          26  25  24  23  \
  22          21  20  19  18  \
  17          16  15  14  13  \
  12          11  10  9   8   \
  7            6   5  4   3  \
  2            1   0  0

arg_in: 1 buffer uint[34] \
  0          1          2          4          8 \
  16         32         64         128        256 \
  512        1024       2048       4096       8192 \
  16384      32768      65536      131072     262144 \
  524288     1048576    2097152    4194304    8388608 \
  16777216   33554432   67108864   134217728  268435456 \
  536870912  1073741824 2147483648 4294967295

[test]
name: v_firstbit_u32_cmp_result
kernel_name: v_firstbit_u32_cmp_result

arg_out: 0 buffer int[34] \
  0xffffffff  31  30  29  28  \
  27          26  25  24  23  \
  22          21  20  19  18  \
  17          16  15  14  13  \
  12          11  10  9   8   \
  7            6   5  4   3  \
  2            1   0  0

arg_in: 1 buffer uint[34] \
  0          1          2          4          8 \
  16         32         64         128        256 \
  512        1024       2048       4096       8192 \
  16384      32768      65536      131072     262144 \
  524288     1048576    2097152    4194304    8388608 \
  16777216   33554432   67108864   134217728  268435456 \
  536870912  1073741824 2147483648 4294967295

[test]
name: v_firstbit_u32_cmp_result_inv
kernel_name: v_firstbit_u32_cmp_result_inv

arg_out: 0 buffer int[34] \
  0xffffffff  31  30  29  28  \
  27          26  25  24  23  \
  22          21  20  19  18  \
  17          16  15  14  13  \
  12          11  10  9   8   \
  7            6   5  4   3  \
  2            1   0  0

arg_in: 1 buffer uint[34] \
  0          1          2          4          8 \
  16         32         64         128        256 \
  512        1024       2048       4096       8192 \
  16384      32768      65536      131072     262144 \
  524288     1048576    2097152    4194304    8388608 \
  16777216   33554432   67108864   134217728  268435456 \
  536870912  1073741824 2147483648 4294967295


[test]
name: v_clz_u64
kernel_name: v_clz_u64
global_size: 66 0 0

arg_out: 0 buffer int[66] \
  64  63  62  61 \
  60  59  58  57 \
  56  55  54  53 \
  52  51  50  49 \
  48  47  46  45 \
  44  43  42  41 \
  40  39  38  37 \
  36  35  34  33 \
  32  31  30  29 \
  28  27  26  25 \
  24  23  22  21 \
  20  19  18  17 \
  16  15  14  13 \
  12  11  10  9  \
  8   7   6   5  \
  4   3   2   1  \
  0   0

arg_in: 1 buffer ulong[66] \
  0x0         0x1         0x2          0x4 \
  0x8         0x10        0x20         0x40 \
  0x80        0x100       0x200        0x400 \
  0x800       0x1000      0x2000       0x4000 \
  0x8000      0x10000     0x20000      0x40000 \
  0x80000     0x100000    0x200000     0x400000 \
  0x800000    0x1000000   0x2000000    0x4000000 \
  0x8000000   0x10000000  0x20000000   0x40000000  \
  0x80000000  0x100000000 0x200000000  0x400000000  \
  0x800000000  0x1000000000  0x2000000000  0x4000000000  \
  0x8000000000  0x10000000000  0x20000000000 0x40000000000  \
  0x80000000000  0x100000000000  0x200000000000 0x400000000000  \
  0x800000000000  0x1000000000000  0x2000000000000 0x4000000000000  \
  0x8000000000000  0x10000000000000  0x20000000000000 0x40000000000000  \
  0x80000000000000  0x100000000000000  0x200000000000000 0x400000000000000  \
  0x800000000000000  0x1000000000000000  0x2000000000000000 0x4000000000000000  \
  0x8000000000000000  \
  0xffffffffffffffff

[test]
name: v_firstbit_u64
kernel_name: v_firstbit_u64
global_size: 66 0 0

arg_out: 0 buffer int[66] \
  -1  63  62  61 \
  60  59  58  57 \
  56  55  54  53 \
  52  51  50  49 \
  48  47  46  45 \
  44  43  42  41 \
  40  39  38  37 \
  36  35  34  33 \
  32  31  30  29 \
  28  27  26  25 \
  24  23  22  21 \
  20  19  18  17 \
  16  15  14  13 \
  12  11  10  9  \
  8   7   6   5  \
  4   3   2   1  \
  0   0

arg_in: 1 buffer ulong[66] \
  0x0         0x1         0x2          0x4 \
  0x8         0x10        0x20         0x40 \
  0x80        0x100       0x200        0x400 \
  0x800       0x1000      0x2000       0x4000 \
  0x8000      0x10000     0x20000      0x40000 \
  0x80000     0x100000    0x200000     0x400000 \
  0x800000    0x1000000   0x2000000    0x4000000 \
  0x8000000   0x10000000  0x20000000   0x40000000  \
  0x80000000  0x100000000 0x200000000  0x400000000  \
  0x800000000  0x1000000000  0x2000000000  0x4000000000  \
  0x8000000000  0x10000000000  0x20000000000 0x40000000000  \
  0x80000000000  0x100000000000  0x200000000000 0x400000000000  \
  0x800000000000  0x1000000000000  0x2000000000000 0x4000000000000  \
  0x8000000000000  0x10000000000000  0x20000000000000 0x40000000000000  \
  0x80000000000000  0x100000000000000  0x200000000000000 0x400000000000000  \
  0x800000000000000  0x1000000000000000  0x2000000000000000 0x4000000000000000  \
  0x8000000000000000  \
  0xffffffffffffffff


[test]
name: v_clz_u16
kernel_name: v_clz_u16

arg_out: 0 buffer int[18] \
  16  15  14  13  12  \
  11  10  9   8   7   \
  6   5   4   3   2   \
  1   0   0

arg_in: 1 buffer ushort[18] \
  0          1          2          4          8  \
  16         32         64         128        256 \
  512        1024       2048       4096       8192 \
  16384      32768      0xffff

[test]
name: v_firstbit_u16
kernel_name: v_firstbit_u16

arg_out: 0 buffer int[18] \
  -1  15  14  13  12  \
  11  10  9   8   7   \
  6   5   4   3   2   \
  1   0   0

arg_in: 1 buffer ushort[18] \
  0          1          2          4          8  \
  16         32         64         128        256 \
  512        1024       2048       4096       8192 \
  16384      32768      0xffff

!*/

kernel void v_clz_u32(global int* out, global uint* in)
{
    int id = get_global_id(0);
    out[id] = clz(in[id]);
}

kernel void v_clz_u16(global int* out, global ushort* in)
{
    int id = get_global_id(0);
    out[id] = clz(in[id]);
}

kernel void s_clz_u32(global uint* out,
              uint arg0, uint arg1, uint arg2,
              uint arg3, uint arg4, uint arg5) {
    uint array[] = { arg0, arg1, arg2, arg3, arg4, arg5 };

#pragma unroll
    for (int i = 0; i < sizeof(array) / sizeof(array[0]); ++i)
        out[i] = clz(array[i]) + 10;
}

kernel void v_firstbit_u32(global uint* out, global uint* in)
{
    uint id = get_global_id(0);
    uint val = in[id];
    out[id] = (val == 0) ? -1 : clz(val);
}

kernel void v_firstbit_u16(global uint* out, global ushort* in)
{
    uint id = get_global_id(0);
    ushort val = in[id];
    out[id] = (val == 0) ? -1 : clz(val);
}

kernel void v_firstbit_u32_inv(global uint* out, global uint* in)
{
    uint id = get_global_id(0);
    uint val = in[id];
    out[id] = (val != 0) ? clz(val) : -1;
}

kernel void v_firstbit_u32_cmp_result(global uint* out,
                      global uint* in)
{
    uint id = get_global_id(0);
    uint res = clz(in[id]);
    out[id] = (res == 32) ? -1 : res;
}

kernel void v_firstbit_u32_cmp_result_inv(global uint* out,
                      global uint* in)
{
    uint id = get_global_id(0);
    uint res = clz(in[id]);
    out[id] = (res != 32) ? res : -1;
}

kernel void s_firstbit_u32(global uint* out,
               uint arg0, uint arg1, uint arg2,
               uint arg3, uint arg4, uint arg5) {
    uint array[] = { arg0, arg1, arg2, arg3, arg4, arg5 };

#pragma unroll
    for (int i = 0; i < sizeof(array) / sizeof(array[0]); ++i) {
        int val = array[i];
        out[i] = ((val == 0) ? -1 : clz(val)) + 10;
    }
}

kernel void s_firstbit_cmp_result_u32(global uint* out,
                      uint arg0, uint arg1, uint arg2,
                      uint arg3, uint arg4, uint arg5) {
    uint array[] = { arg0, arg1, arg2, arg3, arg4, arg5 };

#pragma unroll
    for (int i = 0; i < sizeof(array) / sizeof(array[0]); ++i) {
        int val = clz(array[i]);
        out[i] = ((val == 32) ? -1 : val) + 10;
    }
}

kernel void v_clz_u64(global int* out, global ulong* in)
{
    int id = get_global_id(0);
    out[id] = clz(in[id]);
}

kernel void v_firstbit_u64(global uint* out, global ulong* in)
{
    uint id = get_global_id(0);
    ulong val = in[id];
    out[id] = (val == 0) ? -1 : clz(val);
}
