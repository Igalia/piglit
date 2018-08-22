/*!

[config]
name: calls workitem IDs
clc_version_min: 10

[test]
name: Callee function use get_global_id(0)
kernel_name: kernel_call_pass_get_global_id_0
dimensions: 1
global_size: 64 0 0
arg_out: 0 buffer uint[64] \
  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 \
 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 \
 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 \
 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63

[test]
name: Callee function use get_global_id 0..2
kernel_name: kernel_call_pass_get_global_id_012
dimensions: 3
global_size: 8 4 2
arg_out: 0 buffer uint[64] \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7 \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7 \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7 \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7

arg_out: 1 buffer uint[64] \
  0  0  0  0  0  0  0  0  1  1  1  1  1  1  1  1 \
  2  2  2  2  2  2  2  2  3  3  3  3  3  3  3  3 \
  0  0  0  0  0  0  0  0  1  1  1  1  1  1  1  1 \
  2  2  2  2  2  2  2  2  3  3  3  3  3  3  3  3

arg_out: 2 buffer uint[64] \
  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 \
  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 \
  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1 \
  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1

[test]
name: Callee function stack passed get_local_id
kernel_name: kernel_call_too_many_argument_regs_get_local_id_012
dimensions: 3
global_size: 8 4 2
local_size: 8 4 2

arg_out: 0 buffer uint[64] \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7 \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7 \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7 \
  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7

arg_out: 1 buffer uint[64] \
  0  0  0  0  0  0  0  0  1  1  1  1  1  1  1  1 \
  2  2  2  2  2  2  2  2  3  3  3  3  3  3  3  3 \
  0  0  0  0  0  0  0  0  1  1  1  1  1  1  1  1 \
  2  2  2  2  2  2  2  2  3  3  3  3  3  3  3  3

arg_out: 2 buffer uint[64] \
  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 \
  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 \
  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1 \
  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1

[test]
name: Callee function stack passed get_local_id with byval
kernel_name: kernel_call_too_many_argument_regs_byval_get_local_id_012
dimensions: 3
global_size: 8 4 2
local_size: 8 4 2

arg_out: 0 buffer uint[64] \
  45  46  47  48  49  50  51  52  45  46  47  48  49  50  51  52  \
  45  46  47  48  49  50  51  52  45  46  47  48  49  50  51  52  \
  45  46  47  48  49  50  51  52  45  46  47  48  49  50  51  52  \
  45  46  47  48  49  50  51  52  45  46  47  48  49  50  51  52

arg_out: 1 buffer uint[64] \
  47  47  47  47  47  47  47  47  48  48  48  48  48  48  48  48 \
  49  49  49  49  49  49  49  49  50  50  50  50  50  50  50  50 \
  47  47  47  47  47  47  47  47  48  48  48  48  48  48  48  48 \
  49  49  49  49  49  49  49  49  50  50  50  50  50  50  50  50

arg_out: 2 buffer uint[64] \
  50  50  50  50  50  50  50  50  50  50  50  50  50  50  50  50 \
  50  50  50  50  50  50  50  50  50  50  50  50  50  50  50  50 \
  51  51  51  51  51  51  51  51  51  51  51  51  51  51  51  51 \
  51  51  51  51  51  51  51  51  51  51  51  51  51  51  51  51

!*/

#define NOINLINE __attribute__((noinline))

NOINLINE
void func_get_global_id_0(volatile global uint* out)
{
    uint gid = get_global_id(0);
    out[gid] = gid;
}

kernel void kernel_call_pass_get_global_id_0(global uint *out)
{
    func_get_global_id_0(out);
}

NOINLINE
void func_get_global_id_012(volatile global uint* out0,
                            volatile global uint* out1,
                            volatile global uint* out2)
{
    uint id0 = get_global_id(0);
    uint id1 = get_global_id(1);
    uint id2 = get_global_id(2);
    uint flat_id = (id2 * get_global_size(1) + id1) * get_global_size(0) + id0;

    out0[flat_id] = id0;
    out1[flat_id] = id1;
    out2[flat_id] = id2;
}

kernel void kernel_call_pass_get_global_id_012(global uint *out0,
                                               global uint *out1,
                                               global uint *out2)
{
    func_get_global_id_012(out0, out1, out2);
}

// On amdgcn, this will require the workitem IDs be passed as values
// on the stack after the arguments.
NOINLINE
uint3 too_many_argument_regs_get_local_id_012(
	int arg0, int arg1, int arg2, int arg3,
	int arg4, int arg5, int arg6, int arg7,
	int arg8, int arg9, int arg10, int arg11,
	int arg12, int arg13, int arg14, int arg15,
	int arg16, int arg17, int arg18, int arg19,
	int arg20, int arg21, int arg22, int arg23,
	int arg24, int arg25, int arg26, int arg27,
	int arg28, int arg29, int arg30, int arg31)
{
	uint3 id;
	id.x = get_local_id(0);
	id.y = get_local_id(1);
	id.z = get_local_id(2);
	return id;
}

kernel void kernel_call_too_many_argument_regs_get_local_id_012(global uint* out0, global uint* out1, global uint* out2)
{
	uint id0 = get_global_id(0);
	uint id1 = get_global_id(1);
	uint id2 = get_global_id(2);
	uint flat_id = (id2 * get_global_size(1) + id1) * get_global_size(0) + id0;

	uint3 result = too_many_argument_regs_get_local_id_012(
		1234, 999, 42, 5555, 8888, 9009, 777, 4242,
		202020, 6359, 8344, 1443, 552323, 33424, 666, 98765,
		2222, 232556, 57777, 934121, 94991, 1337, 0xdead, 0xbeef,
		0x5555, 0x3333, 0x666, 0x4141, 0x1234, 0x8888, 0xaaaa, 0xbbbb);

	out0[flat_id] = result.x;
	out1[flat_id] = result.y;
	out2[flat_id] = result.z;
}


typedef struct ByValStruct {
	long array[9];
} ByValStruct;

// Same as previous, with an additional byval passed argument.
NOINLINE
uint3 too_many_argument_regs_byval_get_local_id_012(
	ByValStruct byval_arg,
	int arg0, int arg1, int arg2, int arg3,
	int arg4, int arg5, int arg6, int arg7,
	int arg8, int arg9, int arg10, int arg11,
	int arg12, int arg13, int arg14, int arg15,
	int arg16, int arg17, int arg18, int arg19,
	int arg20, int arg21, int arg22, int arg23,
	int arg24, int arg25, int arg26, int arg27,
	int arg28, int arg29, int arg30, int arg31)
{
	uint3 id;
	id.x = get_local_id(0) + byval_arg.array[3]; // + 42 + 3
	id.y = get_local_id(1) + byval_arg.array[5]; // + 42 + 5
	id.z = get_local_id(2) + byval_arg.array[8]; // + 42 + 8
	return id;
}

kernel void kernel_call_too_many_argument_regs_byval_get_local_id_012(global uint* out0, global uint* out1, global uint* out2)
{
	uint id0 = get_global_id(0);
	uint id1 = get_global_id(1);
	uint id2 = get_global_id(2);
	uint flat_id = (id2 * get_global_size(1) + id1) * get_global_size(0) + id0;

	ByValStruct byval;
	for (int i = 0; i < 9; ++i)
		byval.array[i] = 42 + i;

	uint3 result = too_many_argument_regs_byval_get_local_id_012(
		byval,
		1234, 999, 42, 5555, 8888, 9009, 777, 4242,
		202020, 6359, 8344, 1443, 552323, 33424, 666, 98765,
		2222, 232556, 57777, 934121, 94991, 1337, 0xdead, 0xbeef,
		0x5555, 0x3333, 0x666, 0x4141, 0x1234, 0x8888, 0xaaaa, 0xbbbb);

	out0[flat_id] = result.x;
	out1[flat_id] = result.y;
	out2[flat_id] = result.z;
}
