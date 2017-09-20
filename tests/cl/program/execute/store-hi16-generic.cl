/*!

[config]
name: store high 16-bits of 32-bit value with generic addressing
clc_version_min: 20
dimensions: 1

[test]
name: store hi16 generic
kernel_name: store_hi16_generic
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer ushort[4] \
    0xabcd    0x1111   0x2222 0x3333

arg_in: 1 buffer uint[4] \
   0xabcd1234    0x11112222    0x22221111    0x33334444


[test]
name: store hi16 generic trunc i8
kernel_name: truncstorei8_hi16_generic
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer uchar[4] \
  0xcd 0x22 0xad 0x80

arg_in: 1 buffer uint[4] \
   0xabcd1234    0x11223344    0xdeadbeef  0x70809024

!*/

kernel void store_hi16_generic(volatile global ushort* out, volatile global uint* in)
{
    int gid = get_global_id(0);
    uint value = in[gid];

    volatile generic ushort* generic_out = (volatile generic ushort*)out;
    generic_out[gid] = value >> 16;
}

kernel void truncstorei8_hi16_generic(volatile global uchar* out, volatile global uint* in)
{
    int gid = get_global_id(0);
    uint value = in[gid];

    volatile generic uchar* generic_out = (volatile generic ushort*)out;
    generic_out[gid] = (uchar)(value >> 16);
}
