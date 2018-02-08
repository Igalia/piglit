/*!

[config]
name: MUBUF stack addressing behavior
clc_version_min: 10

[test]
name: MUBUF negative buffer offsets
kernel_name: negative_mubuf_vaddr
dimensions: 1
global_size: 16 0 0

arg_out: 0 buffer int[16]        \
  5 5 5 5 \
  5 5 5 5 \
  5 5 5 5 \
  5 5 5 5

!*/

// Prior to gfx9, MUBUF instructions with the vaddr offset enabled
// would always perform a range check. If a negative vaddr base index
// was used, this would fail the range check. The overall address
// computation would compute a valid address, but this doesn't happen
// due to the range check. For out-of-bounds MUBUF loads, a 0 is
// returned.
//
// Therefore it should be safe to fold any VGPR offset on gfx9 into
// the MUBUF vaddr, but not on older subtargets which can only do this
// if the sign bit is known 0.
kernel void negative_mubuf_vaddr(global int* out0)
{
    volatile int array[16];

    int id = get_global_id(0);
    for (int i = 0; i < 16; ++i)
    {
        array[i] = i + 1;
    }

    // Directly addressing the same buffer address works without using vaddr:
    //
    // buffer_load_dword v2, off, s[0:3], s11 offset:20
    // out0[id] = array[4];


    // But having a negative computed base index would fail:
    // v_mov_b32_e32 v0, -8
    // v_lshlrev_b32_e32 v0, 2, v0
    // v_add_i32_e32 v0, vcc, 4, v0
    // buffer_load_dword v2, v0, s[0:3], s11 offen offset:48

#ifdef __AMDGCN__
    // Obscure the value so it can't be folded with other constant or
    // make known bits assumptions.
    int offset;
    __asm volatile("v_mov_b32 %0, -8" : "=v"(offset));
#else
    int offset = -8;
#endif
    out0[id] = array[offset + 12];
}
