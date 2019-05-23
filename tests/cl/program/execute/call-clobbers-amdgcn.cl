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

[test]
name: Conditional call
kernel_name: conditional_call
dimensions: 1
local_size: 64 0 0
global_size: 64 0 0
arg_out: 0 buffer int[64] \
  0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 \
  0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 \
  0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 \
  0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 \
  0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 \
  0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 \
  0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 \
  0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234

[test]
name: Conditional call partial dispatch
kernel_name: conditional_call
dimensions: 1
local_size: 16 0 0
global_size: 16 0 0
arg_out: 0 buffer int[16] \
  0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 \
  0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234 0xabcd1234


[test]
name: Skip call no lanes
kernel_name: skip_call_no_lanes
dimensions: 1
local_size: 64 0 0
global_size: 64 0 0
arg_out: 0 buffer int[64] \
  123 123 123 123 123 123 123 123 \
  123 123 123 123 123 123 123 123 \
  123 123 123 123 123 123 123 123 \
  123 123 123 123 123 123 123 123 \
  123 123 123 123 123 123 123 123 \
  123 123 123 123 123 123 123 123 \
  123 123 123 123 123 123 123 123 \
  123 123 123 123 123 123 123 123

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

__attribute__((noinline))
void spill_sgpr_to_csr_vgpr()
{
    __asm volatile(
        "s_nop 1" :::
        "v0","v1","v2","v3","v4","v5","v6","v7",
        "v8","v9","v10","v11","v12","v13","v14","v15",
        "v16","v17","v18","v19","v20","v21","v22","v23",
        "v24","v25","v26","v27","v28","v29","v30","v31",

        "s0","s1","s2","s3","s4","s5","s6","s7",
        "s8","s9","s10","s11","s12","s13","s14","s15",
        "s16","s17","s18","s19","s20","s21","s22","s23",
        "s24","s25","s26","s27","s28","s29","s30","s31",
        "s32", "s33", "s34", "s35", "s36", "s37", "s38");
}

// A CSR VGPR needs to be spilled/restored in the prolog/epilog, but
// all lanes need to be made active to avoid clobbering lanes that did
// not enter the call.
kernel void conditional_call(global int* ret)
{
    __asm volatile("v_mov_b32 v32, 0xabcd1234" : : : "v32");

    int id = get_local_id(0);
    if (id == 0)
    {
        spill_sgpr_to_csr_vgpr();
    }

    int tmp;
    __asm volatile("v_mov_b32 %0, v32"
                   : "=v"(tmp)
                   :
                   : "v32");
    ret[id] = tmp;
}

__attribute__((noinline))
void hang_if_all_inactive()
{
    __builtin_amdgcn_s_sendmsghalt(0, 0);
}

// If all lanes could be dynamically false, the call must not be taken
// in case a side effecting scalar op is called inside.
kernel void skip_call_no_lanes(global int* ret)
{
    int divergent_false;
    __asm volatile("v_mov_b32 %0, 0" : "=v"(divergent_false));

    if (divergent_false)
    {
        hang_if_all_inactive();
    }

    ret[get_global_id(0)] = 123;
}
