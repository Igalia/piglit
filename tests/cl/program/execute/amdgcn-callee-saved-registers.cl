/*!

[config]
name: amdgcn call clobbers
clc_version_min: 10
device_regex: gfx[\d]*

[test]
name: CSR VGPR for SGPR spilling
kernel_name: kernel_call_need_spill_vgpr_for_csr_sgpr_spills_no_calls
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[2] \
  0x1337  0xabcd1234

!*/

#ifndef __AMDGCN__
#error This test is only for amdgcn
#endif

__attribute__((noinline))
int need_spill_vgpr_for_csr_sgpr_spills_no_calls()
{
    int sgpr_val;
    __asm volatile("s_mov_b32 %0, 0x1337" : "=s"(sgpr_val));

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

    return sgpr_val;
}


kernel void kernel_call_need_spill_vgpr_for_csr_sgpr_spills_no_calls(global int* ret)
{
    int v32;
    __asm volatile("v_mov_b32 %0, 0xabcd1234" : "={v32}"(v32));
    ret[0] = need_spill_vgpr_for_csr_sgpr_spills_no_calls();
    __asm volatile ("s_nop 0" :: "{v32}"(v32));
    ret[1] = v32;
}
