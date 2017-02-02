/*!
[config]
name: sign extend in register
clc_version_min: 10

dimensions: 1

## Addition ##

[test]
name: SALU i8 in i64 0
kernel_name: s_sext_in_reg_i8_in_i64

arg_out: 0 buffer long[14] \
  0x0000                   \
  0x000f                   \
  0xfffffffffffffff0       \
  0xffffffffffffffff       \
  0x0                      \
  0x0                      \
  0x79                     \
  0xffffffffffffff80       \
  0xffffffffffffff81       \
  0xffffffffffffff82       \
  0xfffffffffffffffe       \
  0x7f                     \
  0xffffffffffffffaa       \
  0x55

arg_in: 1 long 0x0000
arg_in: 2 long 0x000f
arg_in: 3 long 0x00f0
arg_in: 4 long 0x00ff
arg_in: 5 long 0xff00
arg_in: 6 long 0xf000
arg_in: 7 long 0x0079
arg_in: 8 long 0x0080
arg_in: 9 long 0x0081
arg_in: 10 long 0x0082
arg_in: 11 long 0x00fe
arg_in: 12 long 0x007f
arg_in: 13 long 0x00aa
arg_in: 14 long 0x0055
arg_in: 15 int 0


[test]
name: SALU i16 in i64 0
kernel_name: s_sext_in_reg_i16_in_i64

arg_out: 0 buffer long[14] \
  0x0000                   \
  0x000f                   \
  0x00f0                   \
  0x0f00                   \
  0xfffffffffffff000       \
  0xffffffffffffffff       \
  0xfffffffffffffffe       \
  0xffffffffffff8000       \
  0xffffffffffffaaaa       \
  0xffffffffffffbbbb       \
  0xff                     \
  0xfff                    \
  0xffffffffffff8001       \
  0xffffffffffff8080

arg_in: 1 long 0x0000
arg_in: 2 long 0x000f
arg_in: 3 long 0x00f0
arg_in: 4 long 0x0f00
arg_in: 5 long 0xf000
arg_in: 6 long 0xffff
arg_in: 7 long 0xfffe
arg_in: 8 long 0x8000
arg_in: 9 long 0xaaaa
arg_in: 10 long 0xbbbb
arg_in: 11 long 0x00ff
arg_in: 12 long 0x0fff
arg_in: 13 long 0x8001
arg_in: 14 long 0x8080
arg_in: 15 int 0


[test]
name: SALU i32 in i64 0
kernel_name: s_sext_in_reg_i32_in_i64
global_size: 1 0 0

arg_out: 0 buffer long[16] \
  0                        \
  0xf                      \
  0xffffffffffff0000       \
  0xffff                   \
  0xffffffffffffffff       \
  0xfffffffffffffffe       \
  0x7fffffff               \
  0xffffffff80000000       \
  0xffffffff80000001       \
  0x0                      \
  0xffffffffaaaaaaaa       \
  0x55555555               \
  0x40000000               \
  0xff                     \
  0x0                      \
  0x0

arg_in: 1  long 0x0
arg_in: 2  long 0xf
arg_in: 3  long 0xffff0000
arg_in: 4  long 0x0000ffff
arg_in: 5  long 0xffffffffffffffff
arg_in: 6  long 0xfffffffffffffffe
arg_in: 7  long 0x7fffffff
arg_in: 8  long 0x80000000
arg_in: 9  long 0x80000001
arg_in: 10 long 0x100000000
arg_in: 11 long 0xaaaaaaaa
arg_in: 12 long 0x55555555
arg_in: 13 long 0x40000000
arg_in: 14 long 0xff
arg_in: 15 long 0x0000000100000000
arg_in: 16 long 0x100000000000000

arg_in: 17 int 0


[test]
name: VALU i8 in i64 0
kernel_name: v_sext_in_reg_i8_in_i64
global_size: 14 0 0

arg_out: 0 buffer long[14] \
  0x0000                   \
  0x000f                   \
  0xfffffffffffffff0       \
  0xffffffffffffffff       \
  0x0                      \
  0x0                      \
  0x79                     \
  0xffffffffffffff80       \
  0xffffffffffffff81       \
  0xffffffffffffff82       \
  0xfffffffffffffffe       \
  0x7f                     \
  0xffffffffffffffaa       \
  0x55

arg_in: 1 buffer long[14] \
  0x0000 \
  0x000f \
  0x00f0 \
  0x00ff \
  0xff00 \
  0xf000 \
  0x0079 \
  0x0080 \
  0x0081 \
  0x0082 \
  0x00fe \
  0x007f \
  0x00aa \
  0x0055

arg_in: 2 int 0

[test]
name: VALU i16 in i64 0
kernel_name: v_sext_in_reg_i16_in_i64
global_size: 14 0 0

arg_out: 0 buffer long[14] \
  0x0000                   \
  0x000f                   \
  0x00f0                   \
  0x0f00                   \
  0xfffffffffffff000       \
  0xffffffffffffffff       \
  0xfffffffffffffffe       \
  0xffffffffffff8000       \
  0xffffffffffffaaaa       \
  0xffffffffffffbbbb       \
  0xff                     \
  0xfff                    \
  0xffffffffffff8001       \
  0xffffffffffff8080

arg_in: 1 buffer long[14] \
  0x0000 \
  0x000f \
  0x00f0 \
  0x0f00 \
  0xf000 \
  0xffff \
  0xfffe \
  0x8000 \
  0xaaaa \
  0xbbbb \
  0x00ff \
  0x0fff \
  0x8001 \
  0x8080

arg_in: 2 int 0

[test]
name: VALU i32 in i64 0
kernel_name: v_sext_in_reg_i32_in_i64
global_size: 16 0 0

arg_out: 0 buffer long[16] \
  0                        \
  0xf                      \
  0xffffffffffff0000       \
  0xffff                   \
  0xffffffffffffffff       \
  0xfffffffffffffffe       \
  0x7fffffff               \
  0xffffffff80000000       \
  0xffffffff80000001       \
  0x0                      \
  0xffffffffaaaaaaaa       \
  0x55555555               \
  0x40000000               \
  0xff                     \
  0x0                      \
  0x0

arg_in: 1 buffer long[16] \
 0x0                 \
 0xf                 \
 0xffff0000          \
 0x0000ffff          \
 0xffffffffffffffff  \
 0xfffffffffffffffe  \
 0x7fffffff          \
 0x80000000          \
 0x80000001          \
 0x100000000         \
 0xaaaaaaaa          \
 0x55555555          \
 0x40000000          \
 0xff                \
 0x0000000100000000  \
 0x1000000000000000

arg_in: 2 int 0



!*/


// This test is mostly hacks to make sure we get 64-bit s_loads.
kernel void s_sext_in_reg_i8_in_i64(global long* restrict out,
                                    long a0,
                                    long a1,
                                    long a2,
                                    long a3,
                                    long a4,
                                    long a5,
                                    long a6,
                                    long a7,
                                    long a8,
                                    long a9,
                                    long a10,
                                    long a11,
                                    long a12,
                                    long a13,
                                    int shift0)
{
    long args[] =
        {
            a0, a1, a2, a3,
            a4, a5, a6, a7,
            a8, a9, a10, a11,
            a12, a13
        };

    // Force unrolling to make sure we don't dynamically index the array.
    #pragma unroll
    for (int i = 0 ; i < sizeof(args) / sizeof(args[0]); ++i)
    {
        // Shift by zero to make sure we load the whole 64-bit value.
        long x = args[i] << shift0;
        out[i] = (x << 56) >> 56;
    }
}

kernel void s_sext_in_reg_i16_in_i64(global long* restrict out,
                                     long a0,
                                     long a1,
                                     long a2,
                                     long a3,
                                     long a4,
                                     long a5,
                                     long a6,
                                     long a7,
                                     long a8,
                                     long a9,
                                     long a10,
                                     long a11,
                                     long a12,
                                     long a13,
                                     int shift0)
{
    long args[] =
        {
            a0, a1, a2, a3,
            a4, a5, a6, a7,
            a8, a9, a10, a11,
            a12, a13
        };

    // Force unrolling to make sure we don't dynamically index the array.
    #pragma unroll
    for (int i = 0 ; i < sizeof(args) / sizeof(args[0]); ++i)
    {
        // Shift by zero to make sure we load the whole 64-bit value.
        long x = args[i] << shift0;
        out[i] = (x << 48) >> 48;
    }
}

// This test is mostly hacks to make sure we get 64-bit s_loads.
kernel void s_sext_in_reg_i32_in_i64(global long* restrict out,
                                     long a0,
                                     long a1,
                                     long a2,
                                     long a3,
                                     long a4,
                                     long a5,
                                     long a6,
                                     long a7,
                                     long a8,
                                     long a9,
                                     long a10,
                                     long a11,
                                     long a12,
                                     long a13,
                                     long a14,
                                     long a15,
                                     int shift0)
{
    long args[] =
        {
            a0, a1, a2, a3,
            a4, a5, a6, a7,
            a8, a9, a10, a11,
            a12, a13, a14, a15
        };

    // Force unrolling to make sure we don't dynamically index the array.
    #pragma unroll
    for (int i = 0 ; i < sizeof(args) / sizeof(args[0]); ++i)
    {
        // Shift by zero to make sure we load the whole 64-bit value.
        long x = args[i] << shift0;
        out[i] = (x << 32) >> 32;
    }
}

kernel void v_sext_in_reg_i8_in_i64(global long* restrict out,
                                    global long* restrict in,
                                    int shift0)
{
    int id = get_global_id(0);
    long x = in[id] << shift0;
    out[id] = (x << 56) >> 56;
}

kernel void v_sext_in_reg_i16_in_i64(global long* restrict out,
                                     global long* restrict in,
                                     int shift0)
{
    int id = get_global_id(0);
    long x = in[id] << shift0;
    out[id] = (x << 48) >> 48;
}

kernel void v_sext_in_reg_i32_in_i64(global long* restrict out,
                                     global long* restrict in,
                                     int shift0)
{
    int id = get_global_id(0);
    long x = in[id] << shift0;
    out[id] = (x << 32) >> 32;
}
