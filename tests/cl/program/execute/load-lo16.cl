/*!

[config]
  name: load into low 16-bits of 32-bit register
  clc_version_min: 10
  dimensions: 1

[test]
  name: load lo16 global
  kernel_name: load_lo16_global
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0xabcd9999  0x12343333  0x11114444 0xdeadbeef

  arg_in: 1 buffer uint[4] \
  0xabcdf00f   0x1234f00f   0x1111f00f  0xdeadf00f

  arg_in: 2 buffer ushort[4] \
  0x9999   0x3333   0x4444  0xbeef


[test]
  name: load lo16 local
  kernel_name: load_lo16_local
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0xabcd9999  0x12343333  0x11114444 0xdeadbeef

  arg_in: 1 buffer uint[4] \
  0xabcdf00f   0x1234f00f   0x1111f00f  0xdeadf00f

  arg_in: 2 buffer ushort[4] \
  0x9999   0x3333   0x4444  0xbeef

[test]
  name: load lo16 private
  kernel_name: load_lo16_private
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0xabcd9999  0x12343333  0x11114444 0xdeadbeef

  arg_in: 1 buffer uint[4] \
  0xabcdf00f   0x1234f00f   0x1111f00f  0xdeadf00f

  arg_in: 2 buffer ushort[4] \
  0x9999   0x3333   0x4444  0xbeef


[test]
  name: zextloadi8 lo16 global
  kernel_name: zextloadi8_lo16_global
  global_size: 4 0 0
  local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0x00ab0099  0x00120033  0x00110044 0x00de00be

arg_in: 1 buffer uint[4]                     \
  0x00abf00f   0x0012f00f   0x0011f00f  0x00def00f

arg_in: 2 buffer uchar[4] \
  0x99   0x33   0x44  0xbe


[test]
  name: sextloadi8 lo16 global
  kernel_name: sextloadi8_lo16_global
  global_size: 4 0 0
  local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0x0099ffab  0x00330012  0x00440011 0x00beffde

arg_in: 1 buffer uint[4] \
  0x0099f00f   0x0033f00f   0x0044f00f  0x00bef00f

arg_in: 2 buffer char[4] \
  0xab   0x12   0x11  0xde


[test]
  name: zextloadi8 lo16 local
  kernel_name: zextloadi8_lo16_local
  global_size: 4 0 0
  local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0x00ab0099  0x00120033  0x00110044 0x00de00be

arg_in: 1 buffer uint[4]                     \
  0x00abf00f   0x0012f00f   0x0011f00f  0x00def00f

arg_in: 2 buffer uchar[4] \
  0x99   0x33   0x44  0xbe


[test]
  name: sextloadi8 lo16 local
  kernel_name: sextloadi8_lo16_local
  global_size: 4 0 0
  local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0x0099ffab  0x00330012  0x00440011 0x00beffde

arg_in: 1 buffer uint[4] \
  0x0099f00f   0x0033f00f   0x0044f00f  0x00bef00f

arg_in: 2 buffer char[4] \
  0xab   0x12   0x11  0xde


[test]
  name: zextloadi8 lo16 private
  kernel_name: zextloadi8_lo16_private
  global_size: 4 0 0
  local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0x00ab0099  0x00120033  0x00110044 0x00de00be

arg_in: 1 buffer uint[4] \
  0x00abf00f   0x0012f00f   0x0011f00f  0x00def00f

arg_in: 2 buffer uchar[4] \
  0x99   0x33   0x44  0xbe


[test]
  name: sextloadi8 lo16 private
  kernel_name: sextloadi8_lo16_private
  global_size: 4 0 0
  local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0x0099ffab  0x00330012  0x00440011 0x00beffde

arg_in: 1 buffer uint[4] \
  0x0099f00f   0x0033f00f   0x0044f00f  0x00bef00f

arg_in: 2 buffer char[4] \
  0xab   0x12   0x11  0xde

!*/

kernel void load_lo16_global(volatile global uint* out,
                             volatile global uint* in0,
                             volatile global ushort* in1)
{
    int gid = get_global_id(0);
    ushort2 val = as_ushort2(in0[gid]);
    val.lo = in1[gid];
    out[gid] = as_uint(val);
}

kernel void load_lo16_local(volatile global uint* out,
                            volatile global uint* in0,
                            volatile global ushort* in1)
{
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    volatile local uint lds0[64];
    volatile local ushort lds1[64];

    lds0[lid] = in0[gid];
    lds1[lid] = in1[gid];

    ushort2 val = as_ushort2(lds0[gid]);
    val.lo = lds1[gid];
    out[gid] = as_uint(val);
}

kernel void load_lo16_private(volatile global uint* out,
                              volatile global uint* in0,
                              volatile global ushort* in1)
{
    int gid = get_global_id(0);

    volatile private uint stack0 = in0[gid];
    volatile private ushort stack1 = in1[gid];

    ushort2 val = as_ushort2(stack0);
    val.lo = stack1;
    out[gid] = as_uint(val);
}

kernel void zextloadi8_lo16_global(volatile global uint* out,
                                   volatile global uint* in0,
                                   volatile global uchar* in1)
{
    int gid = get_global_id(0);
    ushort2 val = as_ushort2(in0[gid]);
    val.lo = (ushort)in1[gid];
    out[gid] = as_uint(val);
}

kernel void sextloadi8_lo16_global(volatile global uint* out,
                                   volatile global uint* in0,
                                   volatile global char* in1)
{
    int gid = get_global_id(0);
    short2 val = as_short2(in0[gid]);
    val.lo = (short)in1[gid];
    out[gid] = as_uint(val);
}

kernel void zextloadi8_lo16_local(volatile global uint* out,
                                  volatile global uint* in0,
                                  volatile global uchar* in1)
{
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    volatile local uint lds0[64];
    volatile local uchar lds1[64];

    lds0[lid] = in0[gid];
    lds1[lid] = in1[gid];

    ushort2 val = as_ushort2(lds0[gid]);
    val.lo = (ushort)lds1[gid];
    out[gid] = as_uint(val);
}

kernel void sextloadi8_lo16_local(volatile global uint* out,
                                  volatile global uint* in0,
                                  volatile global char* in1)
{
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    volatile local uint lds0[64];
    volatile local char lds1[64];

    lds0[lid] = in0[gid];
    lds1[lid] = in1[gid];

    short2 val = as_short2(lds0[gid]);
    val.lo = (short)lds1[gid];
    out[gid] = as_uint(val);
}

kernel void zextloadi8_lo16_private(volatile global uint* out,
                                    volatile global uint* in0,
                                    volatile global uchar* in1)
{
    int gid = get_global_id(0);
    volatile uint stack0 = in0[gid];
    volatile uchar stack1 = in1[gid];

    ushort2 val = as_ushort2(stack0);
    val.lo = (ushort)stack1;
    out[gid] = as_uint(val);
}

kernel void sextloadi8_lo16_private(volatile global uint* out,
                                    volatile global uint* in0,
                                    volatile global char* in1)
{
    int gid = get_global_id(0);
    volatile uint stack0 = in0[gid];
    volatile char stack1 = in1[gid];

    short2 val = as_short2(stack0);
    val.lo = (short)stack1;
    out[gid] = as_uint(val);
}
