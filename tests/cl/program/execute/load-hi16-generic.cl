/*!

[config]
name: load into high 16-bits of 32-bit register with generic addressing
clc_version_min: 20
dimensions: 1

[test]
name: load hi16 generic
kernel_name: load_hi16_generic
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0xabcd9999  0x12343333  0x11114444 0xdeadbeef

arg_in: 1 buffer ushort[4] \
   0x9999   0x3333   0x4444  0xbeef

arg_in: 2 buffer ushort[4] \
   0xabcd   0x1234   0x1111  0xdead


[test]
name: zextloadi8 hi16 generic
kernel_name: zextloadi8_hi16_generic
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0x00ab0099  0x00120033  0x00110044 0x00de00be

arg_in: 1 buffer uchar[4] \
   0x99   0x33   0x44  0xbe

arg_in: 2 buffer uchar[4] \
   0xab   0x12   0x11  0xde


[test]
name: sextloadi8 hi16 generic
kernel_name: sextloadi8_hi16_generic
global_size: 4 0 0
local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0xffabff99  0x00120033  0x00110044 0xffdeffbe

arg_in: 1 buffer char[4] \
   0x99   0x33   0x44  0xbe

arg_in: 2 buffer char[4] \
   0xab   0x12   0x11  0xde

!*/

kernel void load_hi16_generic(volatile global uint* out,
                              volatile global ushort* in0,
                              volatile global ushort* in1)
{
    volatile generic ushort* generic_in0 = (volatile generic ushort*)in0;
    volatile generic ushort* generic_in1 = (volatile generic ushort*)in1;
    int gid = get_global_id(0);
    ushort lo = generic_in0[gid];
    ushort hi = generic_in1[gid];
    ushort2 vec = { lo, hi };
    out[gid] = as_uint(vec);
}

kernel void zextloadi8_hi16_generic(volatile global uint* out,
                                    volatile global uchar* in0,
                                    volatile global uchar* in1)
{
    volatile generic uchar* generic_in0 = (volatile generic uchar*)in0;
    volatile generic uchar* generic_in1 = (volatile generic uchar*)in1;

    int gid = get_global_id(0);
    ushort lo = (ushort)generic_in0[gid];
    ushort hi = (ushort)generic_in1[gid];
    ushort2 vec = { lo, hi };
    out[gid] = as_uint(vec);
}

kernel void sextloadi8_hi16_generic(volatile global uint* out,
                                    volatile global char* in0,
                                    volatile global char* in1)
{
    volatile generic char* generic_in0 = (volatile generic char*)in0;
    volatile generic char* generic_in1 = (volatile generic char*)in1;

    int gid = get_global_id(0);
    short lo = (short)generic_in0[gid];
    short hi = (short)generic_in1[gid];
    short2 vec = { lo, hi };
    out[gid] = as_uint(vec);
}
