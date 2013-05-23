/*!
[config]
dimensions: 1
global_size: 1 0 0
kernel_name: test_bitselect

[test]
# x = 01010101
# y = 01100110
# z = 00111100
#out= 01100101
name: bitselect int1
arg_out: 0 buffer int[1] 0x65
arg_in:  1 int 0x55
arg_in:  2 int 0x66
arg_in:  3 int 0x3c

!*/

kernel void test_bitselect(global int *out, int x, int y, int z)
{
	*out = bitselect(x,y,z);
}
