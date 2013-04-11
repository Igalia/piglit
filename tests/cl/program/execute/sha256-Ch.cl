
/*!
[config]
dimensions: 1
global_size: 1 0 0
kernel_name: Ch

[test]
name: 000
arg_out: 0 buffer uint[1] 0
arg_in:  1 uint 0
arg_in:  2 uint 0
arg_in:  3 uint 0

[test]
name: 001
arg_out: 0 buffer uint[1] 1
arg_in:  1 uint 0
arg_in:  2 uint 0
arg_in:  3 uint 1

[test]
name: 010
arg_out: 0 buffer uint[1] 0
arg_in:  1 uint 0
arg_in:  2 uint 1
arg_in:  3 uint 0

[test]
name: 011
arg_out: 0 buffer uint[1] 1
arg_in:  1 uint 0
arg_in:  2 uint 1
arg_in:  3 uint 1

[test]
name: 100
arg_out: 0 buffer uint[1] 0
arg_in:  1 uint 1
arg_in:  2 uint 0
arg_in:  3 uint 0

[test]
name: 101
arg_out: 0 buffer uint[1] 0
arg_in:  1 uint 1
arg_in:  2 uint 0
arg_in:  3 uint 1

[test]
name: 110
arg_out: 0 buffer uint[1] 1
arg_in:  1 uint 1
arg_in:  2 uint 1
arg_in:  3 uint 0

[test]
name: 111
arg_out: 0 buffer uint[1] 1
arg_in:  1 uint 1
arg_in:  2 uint 1
arg_in:  3 uint 1

!*/

kernel void Ch(global uint *out, uint x, uint y, uint z)
{
	// There are several ways to implement this, but this is a commonly used
	// optimized version of it.
	out[0] = (z ^ (x & (y ^ z)));
}
