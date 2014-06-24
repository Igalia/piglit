/*!
[config]
name: convert_long(float)
dimensions: 1
global_size: 1 1 1
local_size: 1 1 1

[test]
name: convert_long(float)
kernel_name: test_long
arg_out: 0 buffer long[8] 0 36864 -47104 41943040 -35651584 131 5000000000 -6000000000
# These values were choosen to exercise all code paths in the generic
# implementation of __fixsfdi in compiler-rt:
# https://github.com/llvm-mirror/compiler-rt/blob/master/lib/builtins/fixsfdi.c
arg_in:  1 buffer float[8] 0x1.2p-5      \ # exp < 0
                           0x1.2p+15     \ # pos exp <= 23
                           -0x1.7p+15    \ # neg exp <= 23
                           0x1.4p25      \ # pos exp > 23
                           -0x1.1p25     \ # neg exp > 23
                           131.35        \ # Random non-integer value
                           5000000000.0  \ # Positive value requiring more than 32-bits
                           -6000000000.0   # Negative value requiring more than 32-bits

[test]
name: convert_ulong(float)
kernel_name: test_ulong
arg_out: 0 buffer ulong[8] 0 36864 47104 41943040 35651584 131 5000000000 9223372036854775808
# These values were choosen to exercise all code paths in the generic
# implementation of __fixsfdi in compiler-rt:
# https://github.com/llvm-mirror/compiler-rt/blob/master/lib/builtins/fixsfdi.c
arg_in:  1 buffer float[8] 0x1.2p-5      \ # exp < 0
                           0x1.2p+15     \ # pos exp <= 23
                           0x1.7p+15     \ # pos exp <= 23
                           0x1.4p25      \ # pos exp > 23
                           0x1.1p25      \ # pos exp > 23
                           131.35        \ # Random non-integer value
                           5000000000.0  \ # Positive value requiring more than 32-bits
                           9223372036854775808.0  # Positive value requiring 64-bits

!*/

kernel void test_long(global long *out, global float *in) {
	unsigned i;
	for (i = 0; i < 8; i++) {
		out[i] = convert_long(in[i]);
	}
}

kernel void test_ulong(global ulong *out, global float *in) {
	unsigned i;
	for (i = 0; i < 8; i++) {
		out[i] = convert_ulong(in[i]);
	}
}
