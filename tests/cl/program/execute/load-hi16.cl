/*!

[config]
  name: load into high 16-bits of 32-bit register
  clc_version_min: 10
  dimensions: 1

[test]
  name: load hi16 global
  kernel_name: load_hi16_global
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0xabcd9999  0x12343333  0x11114444 0xdeadbeef

  arg_in: 1 buffer ushort[4] \
  0x9999   0x3333   0x4444  0xbeef

  arg_in: 2 buffer ushort[4] \
  0xabcd   0x1234   0x1111  0xdead


[test]
  name: load hi16 local
  kernel_name: load_hi16_local
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0xabcd9999  0x12343333  0x11114444 0xdeadbeef

  arg_in: 1 buffer ushort[4] \
  0x9999   0x3333   0x4444  0xbeef

  arg_in: 2 buffer ushort[4] \
  0xabcd   0x1234   0x1111  0xdead


[test]
  name: load hi16 private
  kernel_name: load_hi16_private
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0xabcd9999  0x12343333  0x11114444 0xdeadbeef

  arg_in: 1 buffer ushort[4] \
  0x9999   0x3333   0x4444  0xbeef

  arg_in: 2 buffer ushort[4] \
  0xabcd   0x1234   0x1111  0xdead


[test]
  name: zextloadi8 hi16 global
  kernel_name: zextloadi8_hi16_global
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0x00ab0099  0x00120033  0x00110044 0x00de00be

  arg_in: 1 buffer uchar[4] \
  0x99   0x33   0x44  0xbe

  arg_in: 2 buffer uchar[4] \
  0xab   0x12   0x11  0xde


[test]
  name: sextloadi8 hi16 global
  kernel_name: sextloadi8_hi16_global
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0xffabff99  0x00120033  0x00110044 0xffdeffbe

  arg_in: 1 buffer char[4] \
  0x99   0x33   0x44  0xbe

  arg_in: 2 buffer char[4] \
  0xab   0x12   0x11  0xde


[test]
  name: zextloadi8 hi16 local
  kernel_name: zextloadi8_hi16_local
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0x00ab0099  0x00120033  0x00110044 0x00de00be

  arg_in: 1 buffer uchar[4] \
  0x99   0x33   0x44  0xbe

  arg_in: 2 buffer uchar[4] \
  0xab   0x12   0x11  0xde


[test]
  name: sextloadi8 hi16 local
  kernel_name: sextloadi8_hi16_local
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0xffabff99  0x00120033  0x00110044 0xffdeffbe

  arg_in: 1 buffer char[4] \
  0x99   0x33   0x44  0xbe

  arg_in: 2 buffer char[4] \
  0xab   0x12   0x11  0xde


[test]
  name: zextloadi8 hi16 private
  kernel_name: zextloadi8_hi16_private
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0x00ab0099  0x00120033  0x00110044 0x00de00be

  arg_in: 1 buffer uchar[4] \
  0x99   0x33   0x44  0xbe

  arg_in: 2 buffer uchar[4] \
  0xab   0x12   0x11  0xde


[test]
  name: sextloadi8 hi16 private
  kernel_name: sextloadi8_hi16_private
  global_size: 4 0 0
  local_size: 4 0 0

  arg_out: 0 buffer uint[4] \
  0xffabff99  0x00120033  0x00110044 0xffdeffbe

  arg_in: 1 buffer char[4] \
  0x99   0x33   0x44  0xbe

  arg_in: 2 buffer char[4] \
  0xab   0x12   0x11  0xde

!*/

kernel void load_hi16_global(volatile global uint* out,
                             volatile global ushort* in0,
                             volatile global ushort* in1)
{
    int gid = get_global_id(0);
    ushort lo = in0[gid];
    ushort hi = in1[gid];
    ushort2 vec = { lo, hi };
    out[gid] = as_uint(vec);
}

kernel void load_hi16_local(volatile global uint* out,
                            volatile global ushort* in0,
                            volatile global ushort* in1)
{
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    volatile local ushort lds0[64];
    volatile local ushort lds1[64];

    lds0[lid] = in0[gid];
    lds1[lid] = in1[gid];

    ushort lo = lds0[lid];
    ushort hi = lds1[lid];
    ushort2 vec = { lo, hi };
    out[gid] = as_uint(vec);
}

kernel void load_hi16_private(volatile global uint* out,
                              volatile global ushort* in0,
                              volatile global ushort* in1)
{
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    volatile private ushort stack0 = in0[gid];
    volatile private ushort stack1 = in1[gid];

    ushort2 vec = { stack0, stack1 };
    out[gid] = as_uint(vec);
}

kernel void zextloadi8_hi16_global(volatile global uint* out,
                                   volatile global uchar* in0,
                                   volatile global uchar* in1)
{
    int gid = get_global_id(0);
    ushort lo = (ushort)in0[gid];
    ushort hi = (ushort)in1[gid];
    ushort2 vec = { lo, hi };
    out[gid] = as_uint(vec);
}

kernel void sextloadi8_hi16_global(volatile global uint* out,
                                   volatile global char* in0,
                                   volatile global char* in1)
{
    int gid = get_global_id(0);
    short lo = (short)in0[gid];
    short hi = (short)in1[gid];
    short2 vec = { lo, hi };
    out[gid] = as_uint(vec);
}

kernel void zextloadi8_hi16_local(volatile global uint* out,
                                  volatile global uchar* in0,
                                  volatile global uchar* in1)
{
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    volatile local uchar lds0[64];
    volatile local uchar lds1[64];

    lds0[lid] = in0[gid];
    lds1[lid] = in1[gid];

    ushort lo = lds0[lid];
    ushort hi = lds1[lid];
    ushort2 vec = { lo, hi };
    out[gid] = as_uint(vec);
}

kernel void sextloadi8_hi16_local(volatile global uint* out,
                                  volatile global char* in0,
                                  volatile global char* in1)
{
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    volatile local char lds0[64];
    volatile local char lds1[64];

    lds0[lid] = in0[gid];
    lds1[lid] = in1[gid];

    short lo = lds0[lid];
    short hi = lds1[lid];
    ushort2 vec = { lo, hi };
    out[gid] = as_uint(vec);
}

kernel void zextloadi8_hi16_private(volatile global uint* out,
                                    volatile global uchar* in0,
                                    volatile global uchar* in1)
{
    int gid = get_global_id(0);

    volatile private uchar stack0 = in0[gid];
    volatile private uchar stack1 = in1[gid];

    ushort2 vec = { (ushort)stack0, (ushort)stack1 };
    out[gid] = as_uint(vec);
}

kernel void sextloadi8_hi16_private(volatile global uint* out,
                                    volatile global char* in0,
                                    volatile global char* in1)
{
    int gid = get_global_id(0);
    volatile private char stack0 = in0[gid];
    volatile private char stack1 = in1[gid];

    ushort2 vec = { (short)stack0, (short)stack1 };
    out[gid] = as_uint(vec);
}
