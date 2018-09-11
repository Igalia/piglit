/*!

[config]
name: call with stack realignment

[test]
name: call stack realignment 16
kernel_name: kernel_call_stack_realign16_func
dimensions: 1
global_size: 1 0 0

arg_out: 0 buffer int[1] 1


[test]
name: call stack realignment 32
kernel_name: kernel_call_stack_realign32_func
dimensions: 1
global_size: 1 0 0

arg_out: 0 buffer int[1] 1

[test]
name: call stack realignment 64
kernel_name: kernel_call_stack_realign64_func
dimensions: 1
global_size: 1 0 0

arg_out: 0 buffer int[1] 1

[test]
name: call stack realignment 128
kernel_name: kernel_call_stack_realign128_func
dimensions: 1
global_size: 1 0 0

arg_out: 0 buffer int[1] 1


!*/

// Make sure the absolute private address of stack objects in callee
// functions is properly aligned.

#define NOINLINE __attribute__((noinline))

NOINLINE
int test_stack_object_alignment16() {
    volatile int4 requires_align16 = 0;
    volatile uintptr_t addr = (uintptr_t)&requires_align16;
    return (addr & 15) == 0;
}

NOINLINE
int test_stack_object_alignment32() {
    volatile int8 requires_align32 = 0;
    volatile uintptr_t addr = (uintptr_t)&requires_align32;
    return (addr & 31) == 0;
}

NOINLINE
int test_stack_object_alignment64() {
    volatile int16 requires_align64 = 0;
    volatile uintptr_t addr = (uintptr_t)&requires_align64;
    return (addr & 63) == 0;
}

NOINLINE
int test_stack_object_alignment128() {
    volatile long16 requires_align128 = 0;
    volatile uintptr_t addr = (uintptr_t)&requires_align128;
    return (addr & 127) == 0;
}

kernel void kernel_call_stack_realign16_func(global int* out) {
    volatile int misalign_stack = 0;
    *out = test_stack_object_alignment16();
}

kernel void kernel_call_stack_realign32_func(global int* out) {
    volatile int misalign_stack = 0;
    *out = test_stack_object_alignment32();
}

kernel void kernel_call_stack_realign64_func(global int* out) {
    volatile int misalign_stack = 0;
    *out = test_stack_object_alignment64();
}

kernel void kernel_call_stack_realign128_func(global int* out) {
    volatile int misalign_stack = 0;
    *out = test_stack_object_alignment128();
}
