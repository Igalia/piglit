/*!

[config]
name: amdgcn call clobbers
clc_version_min: 10
device_regex: gfx[\d]*

[test]
name: callee saved sgpr
kernel_name: call_clobber_s40
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] 0xabcd1234

[test]
name: callee saved vgpr
kernel_name: call_clobber_v40
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] 0xabcd1234

!*/

#ifndef __AMDGCN__
#error This test is only for amdgcn
#endif

__attribute__((noinline))
void clobber_s40()
{
    __asm volatile("s_mov_b32 s40, 0xdead" : : : "s40");
}

kernel void call_clobber_s40(__global int* ret)
{
    __asm volatile("s_mov_b32 s40, 0xabcd1234" : : : "s40");

    clobber_s40();

    int tmp;

    __asm volatile("v_mov_b32 %0, s40"
                  : "=v"(tmp)
                  :
                  : "s40");
    *ret = tmp;
}

__attribute__((noinline))
void clobber_v40()
{
    __asm volatile("v_mov_b32 v40, 0xdead" : : : "v40");
}

kernel void call_clobber_v40(__global int* ret)
{
    __asm volatile("v_mov_b32 v40, 0xabcd1234" : : : "v40");

    clobber_v40();

    int tmp;
    __asm volatile("v_mov_b32 %0, v40"
                  : "=v"(tmp)
                  :
                  : "v40");
    *ret = tmp;
}
