/*!

[config]
name: tail calls
clc_version_min: 10
dimensions: 1

[test]
name: Basic tail call
kernel_name: kernel_call_tailcall
global_size: 4 0 0

arg_out: 0 buffer int[4] \
  4    11   107     -12

arg_in: 1 buffer int[4] \
  0   100  1234  -912

arg_in: 2 buffer int[4] \
  1    4      2    45

[test]
name: Tail call with more arguments than caller
kernel_name: kernel_call_tailcall_extra_arg
global_size: 4 0 0

arg_out: 0 buffer int[4] \
  2    112   1340   -882

arg_in: 1 buffer int[4] \
  0   100  1234  -912

arg_in: 2 buffer int[4] \
  1    4      2    45

[test]
name: Tail call with fewer arguments than acller
kernel_name: kernel_call_tailcall_fewer_args
global_size: 4 0 0

arg_out: 0 buffer int[4] \
  4    8   81   -10

arg_in: 1 buffer int[4] \
  0   100  1234  -912

arg_in: 2 buffer int[4] \
  1    4      2    45

arg_in: 3 buffer int[4] \
  3    8      4    9

[test]
name: Tail call with stack passed argument
kernel_name: kernel_call_tailcall_stack_passed_args
global_size: 10 0 0

arg_out: 0 buffer int4[10] \
 11440  8762 10296 13156  \
 19649 31311 18081 24745  \
 10476 11772 17766 11070  \
 22165 18005 28665 35945  \
   624   938   768   990  \
 30618 28791 30240 31815  \
 49851 47676 46806 47676  \
  4400  4272  3392  2632  \
 10582  8712  8514  7854  \
 19737 21199 23865 18533  \


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

arg_in: 3 buffer int4[10] \
    68    94    38    52  \
    99    72    76    65  \
    53    46    95     5  \
     3    53    50    96  \
    59    96    56    14  \
    16     7    36    25  \
    54    51    10    41  \
    51    89    44    47  \
    39    27    69    28  \
    60    22    32    70

arg_in: 4 buffer int4[10] \
    83     3     5    53  \
     7    63    89    27  \
    76    77    83    12  \
    75    56    89    63  \
    99    41    14    57  \
    13    79    45    35  \
    58    88    44    73  \
    81    36    21   100  \
    78    79    42    28  \
    11    77    53    55

arg_in: 5 buffer int4[10] \
    67     0    38     6  \
    44    77    48    24  \
     1    69    87    63  \
    85    16    21    72  \
    25    49    46    97  \
    45    81    94    65  \
    87    72    80    71  \
    15    63    62     0  \
    19     1    19    99  \
    59    33    71    14

arg_in: 6 buffer int4[10] \
    42    69    59    93  \
    27    36    16    49  \
    74    73   100    89  \
    13     9    27     0  \
    12    20    32    63  \
    21    68    91    25  \
    74     1    89    53  \
     0    78    73    32  \
    24    82    13    40  \
    16    49    86    82

arg_in: 7 buffer int4[10] \
    85    53     9    66  \
    90    91     6    91  \
    23    30    35    51  \
    41     2    71    94  \
    12    65    62    81  \
    86    47    51    60  \
    46     6    72    49  \
    42    68    68    20  \
    60    17    83    15  \
    56    74    31    13

arg_in: 8 buffer int4[10] \
    42    70    91    76  \
    50    52    32    99  \
    59    85    41    84  \
    24    18    32    89  \
    25    48    39    20  \
    82    76    46    96  \
    67    68    20    92  \
    84    71    83     9  \
    99    25    23    10  \
    53    31    59    52

arg_in: 9 buffer int[10] \
   26  \
   49  \
   27  \
   65  \
    2  \
   63  \
   87  \
    8  \
   22  \
   43

!*/

#define NOINLINE __attribute__((noinline))

NOINLINE
int i32_func_i32_i32(int x, int y)
{
    return x / y + 4;
}

NOINLINE
int i32_func_i32_i32_i32(int x, int y, int z)
{
    return x / y + z;
}

// Test a basic tail call
NOINLINE
int tailcall_i32_func_i32_i32(int x, int y)
{
    x += 5;
    y += 10;
    return i32_func_i32_i32(x, y);
}

// Test a basic tail call with more arguments in the callee than
// caller.
NOINLINE
int tailcall_i32_func_i32_i32_extra_arg(int x, int y)
{
    int z = x + y + 1;
    x += 5;
    y += 10;
    return i32_func_i32_i32_i32(x, y, z);
}

// Test a basic tail call with fewere arguments in the callee than
// caller.
NOINLINE
int tailcall_i32_func_i32_i32_i32_fewer_args(int x, int y, int z)
{
    x += 5;
    y += 10;
    return i32_func_i32_i32(x, y + z);
}

kernel void kernel_call_tailcall(global int* output,
                                 global int* input0,
                                 global int* input1)
{
    int id = get_global_id(0);
    output[id] = tailcall_i32_func_i32_i32(input0[id], input1[id]);
}

kernel void kernel_call_tailcall_extra_arg(global int* output,
                                           global int* input0,
                                           global int* input1)
{
    int id = get_global_id(0);
    output[id] = tailcall_i32_func_i32_i32_extra_arg(input0[id], input1[id]);
}

kernel void kernel_call_tailcall_fewer_args(global int* output,
                                            global int* input0,
                                            global int* input1,
                                            global int* input2)
{
    int id = get_global_id(0);
    output[id] = tailcall_i32_func_i32_i32_i32_fewer_args(input0[id], input1[id], input2[id]);
}

NOINLINE
int4 v4i32_func_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_i32(
    int4 arg0, int4 arg1, int4 arg2, int4 arg3,
    int4 arg4, int4 arg5, int4 arg6, int4 arg7,
    int arg8)
{
    // Try to make sure we can't clobber the incoming stack arguments
    // with local stack objects.
    volatile int4 args[8] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7 };
    volatile int scalar_arg = arg8;

    int4 total = 0;
    for (int i = 0; i < 8; ++i)
    {
        total += args[i];
    }

    return total * scalar_arg;
}

// Test a basic tail call
NOINLINE
int4 tailcall_v4i32_func_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_i32(
    int4 arg0, int4 arg1, int4 arg2, int4 arg3,
    int4 arg4, int4 arg5, int4 arg6, int4 arg7,
    int arg8)
{
    arg0 *= 2;
    return v4i32_func_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_i32(
        arg0, arg1, arg2, arg3, arg4,
        arg5, arg6, arg7, arg8);
}

kernel void kernel_call_tailcall_stack_passed_args(global int4* output,
                                                   global int4* input0,
                                                   global int4* input1,
                                                   global int4* input2,
                                                   global int4* input3,
                                                   global int4* input4,
                                                   global int4* input5,
                                                   global int4* input6,
                                                   global int4* input7,
                                                   global int* input8)
{
    int id = get_global_id(0);
    output[id] = tailcall_v4i32_func_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_v4i32_i32(
        input0[id],
        input1[id],
        input2[id],
        input3[id],
        input4[id],
        input5[id],
        input6[id],
        input7[id],
        input8[id]);
}
