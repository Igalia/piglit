/*!

[config]
name: calls
clc_version_min: 10

[test]
name: Call void_func_void
kernel_name: call_void_func_void
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] 12345

[test]
name: Call i32_func_void
kernel_name: call_i32_func_void
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] 0x12345

[test]
name: Call i64_func_void
kernel_name: call_i64_func_void
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer long[1] 0x100000000000


[test]
name: Call call_i32_func_void_callee_stack
kernel_name: call_i32_func_void_callee_stack
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] 290

[test]
name: Call call_i32_func_p0i32_i32_caller_stack
kernel_name: call_i32_func_p0i32_i32_caller_stack
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] 175

[test]
name: Call call_i32_func_p0i32_i32_indirect_kernel_stack
kernel_name: call_i32_func_p0i32_i32_indirect_kernel_stack
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] 241

[test]
name: Call call_i32_func_p0i32_i32_indirect_function_stack
kernel_name: call_i32_func_p0i32_i32_indirect_function_stack
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] 291

[test]
name: callee stack corruption
kernel_name: kernel_call_nested_stack_usage
dimensions: 1
global_size: 10 0 0

arg_out: 0 buffer int4[10] \
    53    48   156   160  \
    84   248   102   150  \
   102    56   217   106  \
   100   123   151   139  \
    80   150   135   163  \
   223    99   117   199  \
   187   262   223   169  \
   277   129    73   121  \
   162   165   138   137  \
   204   207   223   145  \


arg_in: 1 buffer int4[10] \
     0    13    76    46  \
     4    74    33    63  \
    26     9    95     7  \
    41    54    47    29  \
    15    68    38    39  \
    91    43    14    95  \
    44    83    69    70  \
    89    54    14    45  \
    77    63    21    21  \
    64    70    80    70

arg_in: 2 buffer int4[10] \
    53    22     4    68  \
    76   100    36    24  \
    50    38    27    92  \
    18    15    57    81  \
    50    14    59    85  \
    41    13    89     9  \
    99    96    85    29  \
    99    21    45    31  \
     8    39    96    95  \
    76    67    63     5

[test]
name: nested calls
kernel_name: kernel_nested_calls
dimensions: 1
global_size: 4 0 0

arg_out: 0 buffer int[4] \
  1    7   155     -4

arg_in: 1 buffer int[4] \
  0   100  1234  -912

arg_in: 2 buffer int[4] \
  1    4      2    45


[test]
name: Kernel call stack argument
kernel_name: kernel_call_stack_arg
dimensions: 1
global_size: 10 0 0


arg_out: 0 buffer int4[10] \
 11440  1348 29304 16698  \
 47975  3626 30850 13224  \
  8235 30495 31995  1455  \
 16048 40512 33992  7028  \
  9450  5356 21330 23130  \
 21120 35186 52896 49968  \
 34083 28520     0     0  \
 12384 41492  4420 17880  \
 37310 19320 37518 13175  \
 23852 16014 22734 24284  \


arg_in: 1 buffer int4[10] \
     0    13    76    46  \
    63    76   100    36  \
    27    92    53    46  \
    53    50    96    75  \
    99    41    14    57  \
    35    45    81    94  \
    80    71    74     1  \
    78    73    32    42  \
    60    17    83    15  \
    13    53    31    59

arg_in: 2 buffer int4[10] \
    53    22     4    68  \
    24    99    72    76  \
    95     5    76    77  \
    56    89    63    85  \
    25    49    46    97  \
    65    21    68    91  \
    89    53    46     6  \
    68    68    20    84  \
    99    25    23    10  \
    52    43    26    37

arg_in: 3 buffer int4[10] \
    68    94    38    52  \
    65     7    63    89  \
    83    12     1    69  \
    16    21    72    13  \
    12    20    32    63  \
    25    86    47    51  \
    72    49    67    68  \
    71    83     9     8  \
    22    64    70    80  \
    39    45    48    39

arg_in: 4 buffer int4[10] \
    83     3     5    53  \
    27    44    77    48  \
    87    63    74    73  \
     9    27     0    41  \
    12    65    62    81  \
    60    82    76    46  \
    20    92    87    89  \
    77    63    21    21  \
    70    76    67    63  \
    28     7    37    25

arg_in: 5 buffer int4[10] \
    67     0    38     6  \
    24    27    36    16  \
   100    89    23    30  \
     2    71    94    24  \
    25    48    39    20  \
    96    63    44    83  \
    54    14    45    99  \
     8    39    96    95  \
     5    60    22    32  \
    67    68    51    73

arg_in: 6 buffer int4[10] \
    42    69    59    93  \
    49    90    91     6  \
    35    51    59    85  \
    18    32    89    65  \
     2    91    43    14  \
    69    70    99    96  \
    21    45    31    51  \
    39    27    69    28  \
    70    11    77    53  \
    72    95    46    94

arg_in: 7 buffer int4[10] \
    85    53     9    66  \
    91    50    52    32  \
    41    84    27    41  \
    15    68    38    39  \
    95    41    13    89  \
    85    29    54    51  \
    89    44    47    81  \
    78    79    42    28  \
    55    59    33    71  \
    32    46    52    66

arg_in: 8 buffer int4[10] \
    42    70    91    76  \
    99    49    26     9  \
    54    47    29    18  \
    50    14    59    85  \
     9    16     7    36  \
    10    41    58    88  \
    36    21   100    15  \
    19     1    19    99  \
    14    16    49    86  \
    40    61    99    15

arg_in: 9 buffer int4[10] \
    26     4    74    33  \
    95     7    50    38  \
    15    57    81     3  \
    59    96    56    14  \
    25    13    79    45  \
    44    73    87    72  \
    63    62     0     0  \
    24    82    13    40  \
    82    56    74    31  \
    67    34    54    52

!*/

// The inline asm is necessary to defeat interprocedural sparse
// conditional constant propagation eliminating some of the trivial
// calls.
#ifdef __AMDGCN__
#define USE_ASM 1
#endif

#define NOINLINE __attribute__((noinline))

NOINLINE
void void_func_void(void)
{
#if USE_ASM
  __asm("");
#endif
}

kernel void call_void_func_void(__global int* ret)
{
  void_func_void();
  *ret = 12345;
}

NOINLINE
int i32_func_void(void)
{
    int ret;
#if USE_ASM
    __asm("v_mov_b32 %0, 0x12345" : "=v"(ret));
#else
    ret = 0x12345;
#endif

    return ret;
}

kernel void call_i32_func_void(__global int* ret)
{
    *ret = i32_func_void();
}

NOINLINE
long i64_func_void(void)
{
    long ret;
#if USE_ASM
    __asm("v_lshlrev_b64 %0, 44, 1" : "=v"(ret));
#else
    ret = 1ul << 44;
#endif
    return ret;
}

kernel void call_i64_func_void(__global long* ret)
{
    *ret = i64_func_void();
}


NOINLINE
int i32_func_void_callee_stack(void)
{
    int ret;
#if USE_ASM
    __asm("v_mov_b32 %0, 0x64" : "=v"(ret));
#else
    ret = 0x64;
#endif

    volatile int alloca[20];

    for (int i = 0; i < 20; ++i)
    {
        alloca[i] = i;
    }

    for (int i = 0; i < 20; ++i)
    {
        ret += alloca[i];
    }

    return ret;
}

kernel void call_i32_func_void_callee_stack(__global int* ret)
{
    volatile int alloca[10];

    for (int i = 0; i < 10; ++i)
    {
        alloca[i] = 0xffff;
    }


    *ret = i32_func_void_callee_stack();
}

NOINLINE
int i32_func_p0i32_i32_caller_stack(volatile int* stack, int n)
{
    int ret;
#if USE_ASM
    __asm("v_mov_b32 %0, 0x64" : "=v"(ret));
#else
    ret = 0x64;
#endif

    for (int i = 0; i < n; ++i)
    {
        ret += stack[i];
    }

    return ret;
}

kernel void call_i32_func_p0i32_i32_caller_stack(__global int* ret)
{
    volatile int alloca[10];

    for (int i = 0; i < 10; ++i)
    {
        alloca[i] = 3 + i;
    }

    *ret = i32_func_p0i32_i32_caller_stack(alloca, 10);
}

NOINLINE
int i32_func_p0i32_i32_indirect_stack(volatile int* stack, int n)
{
    int ret;
#if USE_ASM
    __asm("v_mov_b32 %0, 0x64" : "=v"(ret));
#else
    ret = 0x64;
#endif
    for (int i = 0; i < n; ++i)
    {
        ret += stack[i];
    }

    return ret;
}

// Access a stack object in the parent kernel's frame.
NOINLINE
int i32_func_p0i32_i32_pass_kernel_stack(volatile int* stack, int n)
{
    int ret;
#if USE_ASM
    __asm("v_mov_b32 %0, 0x42" : "=v"(ret));
#else
    ret = 0x42;
#endif

    volatile int local_object[10];
    for (int i = 0; i < 10; ++i)
        local_object[i] = -1;

    ret += i32_func_p0i32_i32_indirect_stack(stack, n);

    return ret;
}

kernel void call_i32_func_p0i32_i32_indirect_kernel_stack(volatile __global int* ret)
{
    volatile int alloca[10];

    for (int i = 0; i < 10; ++i)
    {
        alloca[i] = 3 + i;
    }

    *ret = i32_func_p0i32_i32_pass_kernel_stack(alloca, 10);
}

// Access a stack object in a parent non-kernel function's stack frame.
NOINLINE
int i32_func_void_pass_function_stack()
{
    int ret;
#if USE_ASM
    __asm("v_mov_b32 %0, 0x42" : "=v"(ret));
#else
    ret = 0x42;
#endif

    volatile int local_object[10];
    for (int i = 0; i < 10; ++i)
        local_object[i] = 8 + i;

    ret += i32_func_p0i32_i32_indirect_stack(local_object, 10);
    return ret;
}

kernel void call_i32_func_p0i32_i32_indirect_function_stack(__global int* ret)
{
  *ret = i32_func_void_pass_function_stack();
}

NOINLINE
int4 v4i32_func_v4i32_v4i32_stack(int4 arg0, int4 arg1)
{
    // Force stack usage.
    volatile int4 args[8] = { arg0, arg1 };

    int4 total = 0;
    for (int i = 0; i < 8; ++i)
    {
        total += args[i];
    }

    return total;
}

// Make sure using stack in a callee function from a callee function
// doesn't corrupt caller's stack objects.
NOINLINE
int4 nested_stack_usage_v4i32_func_v4i32_v4i32(int4 arg0, int4 arg1)
{
    volatile int stack_object[4];
    for (int i = 0; i < 4; ++i) {
        const int test_val = 0x04030200 | i;
        stack_object[i] = test_val;
    }

    arg0 *= 2;

    int4 result = v4i32_func_v4i32_v4i32_stack(arg0, arg1);

    // Check for stack corruption
    for (int i = 0; i < 4; ++i)
    {
        const int test_val = 0x04030200 | i;
        if (stack_object[i] != test_val)
            result = -1;
    }

    return result;
}

kernel void kernel_call_nested_stack_usage(global int4* output,
                                           global int4* input0,
                                           global int4* input1)
{
    int id = get_global_id(0);
    output[id] = nested_stack_usage_v4i32_func_v4i32_v4i32(
        input0[id],
        input1[id]);
}

NOINLINE
int func_div_add(int x, int y)
{
    return x / y + 4;
}

NOINLINE
int call_i32_func_i32_i32(int x, int y, volatile int* ptr)
{
    int tmp = func_div_add(x, y) >> 2;
    return tmp + *ptr;
}

kernel void kernel_nested_calls(global int* output,
                                global int* input0,
                                global int* input1)
{
    int id = get_global_id(0);
    volatile int zero = 0;
    output[id] = call_i32_func_i32_i32(input0[id], input1[id], &zero);
}

NOINLINE
int4 v4i32_func_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32(
    int4 arg0, int4 arg1, int4 arg2, int4 arg3,
    int4 arg4, int4 arg5, int4 arg6, int4 arg7,
    int4 arg8)
{
    // Try to make sure we can't clobber the incoming stack arguments
    // with local stack objects.
    volatile int4 args[8] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7 };
    volatile int4 last_arg = arg8;

    int4 total = 0;
    for (int i = 0; i < 8; ++i)
    {
        total += args[i];
    }

    return total * last_arg;
}

 // Test argument passed on stack, but doesn't use byval.
NOINLINE
int4 stack_arg_v4i32_func_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32(
    int4 arg0, int4 arg1, int4 arg2, int4 arg3,
    int4 arg4, int4 arg5, int4 arg6, int4 arg7,
    int4 arg8)
{
    volatile int stack_object[8];
    for (int i = 0; i < 8; ++i) {
        const int test_val = 0x04030200 | i;
        stack_object[i] = test_val;
    }

    arg0 *= 2;

    int4 result = v4i32_func_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32(
        arg0, arg1, arg2, arg3, arg4,
        arg5, arg6, arg7, arg8);

    // Check for stack corruption.
    for (int i = 0; i < 8; ++i)
    {
        const int test_val = 0x04030200 | i;
        if (stack_object[i] != test_val)
            result = -1;
    }

    return result;
}

kernel void kernel_call_stack_arg(global int4* output,
                                  global int4* input0,
                                  global int4* input1,
                                  global int4* input2,
                                  global int4* input3,
                                  global int4* input4,
                                  global int4* input5,
                                  global int4* input6,
                                  global int4* input7,
                                  global int4* input8)
{
    int id = get_global_id(0);

    volatile int stack_object[8];
    for (int i = 0; i < 8; ++i) {
        const int test_val = 0x05060700 | i;
        stack_object[i] = test_val;
    }

    output[id] = stack_arg_v4i32_func_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32(
        input0[id],
        input1[id],
        input2[id],
        input3[id],
        input4[id],
        input5[id],
        input6[id],
        input7[id],
        input8[id]);

    // Check for stack corruption.
    for (int i = 0; i < 8; ++i)
    {
        const int test_val = 0x05060700 | i;
        if (stack_object[i] != test_val)
            output[id] = -1;
    }

}
