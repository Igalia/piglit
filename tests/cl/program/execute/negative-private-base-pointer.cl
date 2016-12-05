/*!
[config]
name: negative private buffer base index
clc_version_min: 10
dimensions: 1

[test]
kernel_name: read_write_private_base_plus_offset
name: negative base private index
global_size: 1 0 0

arg_out: 0 buffer int[16]  \
  0xab       \
  0xbc       \
  0xabcd     \
  0xdead     \
             \
  0xcafe     \
  0xf00d     \
  0xababfeed \
  0xca00fe   \
             \
  0xb00feed  \
  0xca00fe   \
  0xfeedbeef \
  0xfe       \
             \
  0xbe00fe   \
  0xabcdef   \
  0xbeef     \
  0xde


arg_in: 1 buffer int[16] \
    -1 \
    -1 \
    -4 \
    -4 \
       \
    -3 \
    -4 \
    -2 \
  -115 \
       \
  -109 \
 -1015 \
 -1011 \
 -1020 \
       \
 -1014 \
  -137 \
  -151 \
   -40

!*/

kernel void read_write_private_base_plus_offset(global int* out, global int* in)
{
    volatile int alloca[16];

    alloca[0] = 0xab;
    alloca[1] = 0xbc;
    alloca[2] = 0xde;
    alloca[3] = 0xfe;

    alloca[4] = 0xabcd;
    alloca[5] = 0xdead;
    alloca[6] = 0xbeef;
    alloca[7] = 0xcafe;

    alloca[8] = 0xca00fe;
    alloca[9] = 0xbe00fe;
    alloca[10] = 0xabdead;
    alloca[11] = 0xf00d;

    alloca[12] = 0xfeedbeef;
    alloca[13] = 0xababfeed;
    alloca[14] = 0xb00feed;
    alloca[15] = 0xabcdef;


    out[0] = alloca[in[0] + 1];
    out[1] = alloca[in[1] + 2];
    out[2] = alloca[in[2] + 8];
    out[3] = alloca[in[3] + 9];

    out[4] = alloca[in[4] + 10];
    out[5] = alloca[in[5] + 15];
    out[6] = alloca[in[6] + 15];
    out[7] = alloca[in[7] + 123];

    out[8] = alloca[in[8] + 123];
    out[9] = alloca[in[9] + 1023];
    out[10] = alloca[in[10] + 1023];
    out[11] = alloca[in[11] + 1023];

    out[12] = alloca[in[12] + 1023];
    out[13] = alloca[in[13] + 152];
    out[14] = alloca[in[14] + 157];
    out[15] = alloca[in[15] + 42];
}
