/*!

[config]
name: bswap
clc_version_min: 10
dimensions: 1

[test]
name: v_bswap_u32
kernel_name: v_bswap_u32
global_size: 24 0 0

arg_out: 0 buffer uint[24] \
  0x00000000 0xffffffff 0xddccbbaa 0xaabbccdd \
  0x01000000 0x00010000 0x00000100 0x00000001 \
  0x10000000 0x00100000 0x00001000 0x00000010 \
  0x000000ff 0x0000ff00 0x00ff0000 0xff000000 \
  0x01020304 0x04030201 0x78563412 0x21436587 \
  0x0000ffff 0xffff0000 0x00ffff00 0x0000ffff

arg_in: 1 buffer uint[24] \
  0x00000000 0xffffffff 0xaabbccdd 0xddccbbaa \
  0x00000001 0x00000100 0x00010000 0x01000000 \
  0x00000010 0x00001000 0x00100000 0x10000000 \
  0xff000000 0x00ff0000 0x0000ff00 0x000000ff \
  0x04030201 0x01020304 0x12345678 0x87654321 \
  0xffff0000 0x0000ffff 0x00ffff00 0xffff0000


[test]
name: s_bswap_u32
kernel_name: s_bswap_u32
global_size: 1 0 0

arg_out: 0 buffer uint[24] \
  0x00000000 0xffffffff 0xddccbbaa 0xaabbccdd \
  0x01000000 0x00010000 0x00000100 0x00000001 \
  0x10000000 0x00100000 0x00001000 0x00000010 \
  0x000000ff 0x0000ff00 0x00ff0000 0xff000000 \
  0x01020304 0x04030201 0x78563412 0x21436587 \
  0x0000ffff 0xffff0000 0x00ffff00 0x0000ffff

arg_in: 1 buffer uint[24] \
  0x00000000 0xffffffff 0xaabbccdd 0xddccbbaa \
  0x00000001 0x00000100 0x00010000 0x01000000 \
  0x00000010 0x00001000 0x00100000 0x10000000 \
  0xff000000 0x00ff0000 0x0000ff00 0x000000ff \
  0x04030201 0x01020304 0x12345678 0x87654321 \
  0xffff0000 0x0000ffff 0x00ffff00 0xffff0000

[test]
name: v_bswap_u64
kernel_name: v_bswap_u64
global_size: 4 0 0

arg_out: 0 buffer ulong[4] \
  0x0000000000000000  0xffffffffffffffff  0x8877665544332211  \
  0x2131415161718191

arg_in: 1 buffer ulong[4] \
  0x0000000000000000  0xffffffffffffffff  0x1122334455667788  \
  0x9181716151413121


[test]
name: s_bswap_u64
kernel_name: s_bswap_u64
global_size: 1 0 0

arg_out: 0 buffer ulong[4] \
  0x0000000000000000  0xffffffffffffffff  0x8877665544332211  \
  0x2131415161718191

arg_in: 1 buffer ulong[4] \
  0x0000000000000000  0xffffffffffffffff  0x1122334455667788  \
  0x9181716151413121

!*/

uint bswap32(uint src) {
    uchar4 vec = as_uchar4(src);
    return as_uint(vec.wzyx);
}

ulong bswap64(ulong src) {
    uchar8 vec = as_uchar8(src);
    return as_ulong(vec.s76543210);
}

kernel void v_bswap_u32(global uint* out, global uint* in)
{
    int id = get_global_id(0);
    out[id] = bswap32(in[id]);
}

kernel void s_bswap_u32(global uint* out, constant uint* in) {
  #pragma unroll
  for (int i = 0; i < 24; ++i)
    out[i] = bswap32(in[i]);
}

kernel void v_bswap_u64(global ulong* out, global ulong* in)
{
    int id = get_global_id(0);
    out[id] = bswap64(in[id]);
}

kernel void s_bswap_u64(global ulong* out, constant ulong* in) {
  #pragma unroll
  for (int i = 0; i < 4; ++i)
    out[i] = bswap64(in[i]);
}
