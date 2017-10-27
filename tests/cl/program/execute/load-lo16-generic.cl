/*!

[config]
name: load into low 16-bits of 32-bit register with generic addressing
clc_version_min: 20
dimensions: 1

[test]
  name: load lo16 generic
  kernel_name: load_lo16_generic
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0xabcd9999  0x12343333  0x11114444 0xdeadbeef

  arg_in: 1 buffer uint[4] \
  0xabcdf00f   0x1234f00f   0x1111f00f  0xdeadf00f

  arg_in: 2 buffer ushort[4] \
  0x9999   0x3333   0x4444  0xbeef

[test]
  name: zextloadi8 lo16 generic
  kernel_name: zextloadi8_lo16_generic
  global_size: 4 0 0
  local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0x00ab0099  0x00120033  0x00110044 0x00de00be

arg_in: 1 buffer uint[4]                     \
  0x00abf00f   0x0012f00f   0x0011f00f  0x00def00f

arg_in: 2 buffer uchar[4] \
  0x99   0x33   0x44  0xbe


[test]
  name: sextloadi8 lo16 generic
  kernel_name: sextloadi8_lo16_generic
  global_size: 4 0 0
  local_size: 4 0 0

arg_out: 0 buffer uint[4] \
  0x0099ffab  0x00330012  0x00440011 0x00beffde

arg_in: 1 buffer uint[4] \
  0x0099f00f   0x0033f00f   0x0044f00f  0x00bef00f

arg_in: 2 buffer char[4] \
  0xab   0x12   0x11  0xde

!*/

kernel void load_lo16_generic(volatile global uint* out,
                              volatile global uint* in0,
                              volatile global ushort* in1)
{
    volatile generic uint* generic_in0 = (volatile generic uint*)in0;
    volatile generic ushort* generic_in1 = (volatile generic ushort*)in1;
    int gid = get_global_id(0);
    ushort2 val = as_ushort2(in0[gid]);
    val.lo = generic_in1[gid];
    out[gid] = as_uint(val);
}

kernel void zextloadi8_lo16_generic(volatile global uint* out,
                                    volatile global uint* in0,
                                    volatile global uchar* in1)
{
    volatile generic uint* generic_in0 = (volatile generic uint*)in0;
    volatile generic uchar* generic_in1 = (volatile generic uchar*)in1;
    int gid = get_global_id(0);
    ushort2 val = as_ushort2(in0[gid]);
    val.lo = (ushort)generic_in1[gid];
    out[gid] = as_uint(val);
}

kernel void sextloadi8_lo16_generic(volatile global uint* out,
                                    volatile global uint* in0,
                                    volatile global char* in1)
{
    volatile generic uint* generic_in0 = (volatile generic uint*)in0;
    volatile generic char* generic_in1 = (volatile generic char*)in1;
    int gid = get_global_id(0);
    short2 val = as_short2(in0[gid]);
    val.lo = (short)generic_in1[gid];
    out[gid] = as_uint(val);
}
