/*!

[config]
name: store high 16-bits of 32-bit value
clc_version_min: 10

dimensions: 1

[test]
name: store hi16 global
kernel_name: store_hi16_global
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer ushort[4] \
    0xabcd    0x1111   0x2222 0x3333

arg_in: 1 buffer uint[4] \
   0xabcd1234    0x11112222    0x22221111    0x33334444

[test]
name: store hi16 local
kernel_name: store_hi16_local
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer ushort[4] \
  0xabcd    0x1111   0x2222 0x3333

arg_in: 1 buffer uint[4] \
   0xabcd1234    0x11112222    0x22221111    0x33334444

[test]
name: store hi16 private
kernel_name: store_hi16_private
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer ushort[4] \
  0xabcd    0x1111   0x2222 0x3333

arg_in: 1 buffer uint[4] \
   0xabcd1234    0x11112222    0x22221111    0x33334444


[test]
name: store hi16 global trunc i8
kernel_name: truncstorei8_hi16_global
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer uchar[4] \
  0xcd 0x22 0xad 0x80

arg_in: 1 buffer uint[4] \
   0xabcd1234    0x11223344    0xdeadbeef  0x70809024


[test]
name: store hi16 local trunc i8
kernel_name: truncstorei8_hi16_local
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer uchar[4] \
  0xcd 0x22 0xad 0x80

arg_in: 1 buffer uint[4] \
   0xabcd1234    0x11223344    0xdeadbeef  0x70809024


[test]
name: store hi16 private trunc i8
kernel_name: truncstorei8_hi16_private
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer uchar[4] \
  0xcd 0x22 0xad 0x80

arg_in: 1 buffer uint[4] \
   0xabcd1234    0x11223344    0xdeadbeef  0x70809024

!*/

kernel void store_hi16_global(volatile global ushort* out, volatile global uint* in)
{
    int gid = get_global_id(0);
    uint value = in[gid];
    out[gid] = value >> 16;
}

kernel void store_hi16_local(volatile global ushort* out, volatile global uint* in)
{
    volatile local ushort lds[64];
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    uint value = in[gid];
    lds[lid] = value >> 16;
    out[gid] = lds[lid];
}

kernel void store_hi16_private(volatile global ushort* out, volatile global uint* in)
{
    int gid = get_global_id(0);
    volatile private ushort stack = in[gid] >> 16;
    out[gid] = stack;
}

kernel void truncstorei8_hi16_global(volatile global uchar* out, volatile global uint* in)
{
    int gid = get_global_id(0);
    uint value = in[gid];
    out[gid] = (uchar)(value >> 16);
}

kernel void truncstorei8_hi16_local(volatile global uchar* out, volatile global uint* in)
{
    volatile local ushort lds[64];
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    uint value = in[gid];
    lds[lid] = value >> 16;
    out[gid] = (uchar)lds[lid];
}

kernel void truncstorei8_hi16_private(volatile global uchar* out, volatile global uint* in)
{
    int gid = get_global_id(0);
    volatile private ushort stack = in[gid] >> 16;
    out[gid] = (uchar)stack;
}
