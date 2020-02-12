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

[test]
name: v_bswap_u16
kernel_name: v_bswap_u16
global_size: 8 0 0

arg_out: 0 buffer ushort[8] \
  0x0000 0xffff 0xbbaa 0xaabb \
  0xbaab 0x0ab0 0x3412 0xf00f

arg_in: 1 buffer ushort[8] \
  0x0000 0xffff 0xaabb 0xbbaa \
  0xabba 0xb00a 0x1234 0x0ff0

[test]
name: s_bswap_u16
kernel_name: s_bswap_u16
global_size: 8 0 0

arg_out: 0 buffer ushort[8] \
  0x0000 0xffff 0xbbaa 0xaabb \
  0xbaab 0x0ab0 0x3412 0xf00f

arg_in: 1 buffer ushort[8] \
  0x0000 0xffff 0xaabb 0xbbaa \
  0xabba 0xb00a 0x1234 0x0ff0

[test]
name: v_bswap_u16_zext32
kernel_name: v_bswap_u16_zext32
global_size: 8 0 0

arg_out: 0 buffer uint[8] \
  0x00000000 0x0000ffff 0x0000bbaa 0x0000aabb \
  0x0000baab 0x00000ab0 0x00003412 0x0000f00f

arg_in: 1 buffer ushort[8] \
  0x0000 0xffff 0xaabb 0xbbaa \
  0xabba 0xb00a 0x1234 0x0ff0


[test]
name: v_bswap_v2u16
kernel_name: v_bswap_v2u16
global_size: 24 0 0

arg_out: 0 buffer ushort2[8] \
  0x0000 0x0000 0xffff 0xffff 0xbbaa 0xddcc 0xccdd 0xaabb \
  0x00ff 0xff00 0x0100 0x3311 0xad3f 0x13d0 0x9154 0x9213

arg_in: 1 buffer ushort2[8] \
  0x0000 0x0000 0xffff 0xffff 0xaabb 0xccdd 0xddcc 0xbbaa \
  0xff00 0x00ff 0x0001 0x1133 0x3fad 0xd013 0x5491 0x1392

[test]
name: s_bswap_v2u16
kernel_name: s_bswap_v2u16
global_size: 24 0 0

arg_out: 0 buffer ushort2[8] \
  0x0000 0x0000 0xffff 0xffff 0xbbaa 0xddcc 0xccdd 0xaabb \
  0x00ff 0xff00 0x0100 0x3311 0xad3f 0x13d0 0x9154 0x9213

arg_in: 1 buffer ushort2[8] \
  0x0000 0x0000 0xffff 0xffff 0xaabb 0xccdd 0xddcc 0xbbaa \
  0xff00 0x00ff 0x0001 0x1133 0x3fad 0xd013 0x5491 0x1392


!*/

// The vector shuffle pattern seems to not match right now on i16, but
// the bitshifts work.
ushort bswap16(ushort src) {
    return (src & 0x00FF) << 8 | (src & 0xFF00) >> 8;
}

// Idiom recogniztion seems to not handle vectors directly, but this
// can vectorize.
ushort2 bswap_v2u16(ushort2 src) {
    return (ushort2)(bswap16(src.x), bswap16(src.y));
}

uint bswap32(uint src) {
    uchar4 vec = as_uchar4(src);
    return as_uint(vec.wzyx);
}

ulong bswap64(ulong src) {
    uchar8 vec = as_uchar8(src);
    return as_ulong(vec.s76543210);
}

kernel void v_bswap_u32(global uint* out, global uint* in) {
    int id = get_global_id(0);
    out[id] = bswap32(in[id]);
}

kernel void s_bswap_u32(global uint* out, constant uint* in) {
  #pragma unroll
  for (int i = 0; i < 24; ++i)
    out[i] = bswap32(in[i]);
}

kernel void v_bswap_u64(global ulong* out, global ulong* in) {
    int id = get_global_id(0);
    out[id] = bswap64(in[id]);
}

kernel void s_bswap_u64(global ulong* out, constant ulong* in) {
  #pragma unroll
  for (int i = 0; i < 4; ++i)
    out[i] = bswap64(in[i]);
}

kernel void v_bswap_u16(global ushort* out, global ushort* in) {
    int id = get_global_id(0);
    out[id] = bswap16(in[id]);
}

kernel void s_bswap_u16(global ushort* out, constant ushort* in) {
  #pragma unroll
  for (int i = 0; i < 8; ++i)
    out[i] = bswap16(in[i]);
}

kernel void v_bswap_u16_zext32(global uint* out, global ushort* in) {
    int id = get_global_id(0);
    out[id] = (uint)bswap16(in[id]);
}

kernel void v_bswap_v2u16(global ushort2* out, global ushort2* in) {
    int id = get_global_id(0);
    out[id] = bswap_v2u16(in[id]);
}

kernel void s_bswap_v2u16(global ushort2* out, constant ushort2* in) {
  #pragma unroll
  for (int i = 0; i < 8; ++i)
    out[i] = bswap_v2u16(in[i]);
}
