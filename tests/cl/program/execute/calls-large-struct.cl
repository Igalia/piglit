/*!

[config]
name: calls with large structs
clc_version_min: 10

[test]
name: byval struct
kernel_name: call_i32_func_byval_Char_IntArray
dimensions: 1
global_size: 16 0 0

arg_out: 0 buffer int[16]        \
 1021 1022 1023 1024 1025 1026 1027 1028 \
 1029 1030 1031 1032 1033 1034 1035 1036

arg_out: 1 buffer int[16] \
  14   14   14   14 \
  14   14   14   14 \
  14   14   14   14 \
  14   14   14   14 \

arg_in: 2 buffer int[16] \
 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15


[test]
name: sret struct
kernel_name: call_sret_Char_IntArray_func
dimensions: 1
global_size: 16 0 0

arg_out: 0 buffer int[16]        \
 921 922 923 924 925 926 927 928 \
 929 930 931 932 933 934 935 936

arg_in: 1 buffer int[16] \
 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

!*/

#define NOINLINE __attribute__((noinline))

typedef struct ByVal_Char_IntArray {
    char c;
    int i32_arr[32];
} ByVal_Char_IntArray;

NOINLINE
int i32_func_byval_Char_IntArray(ByVal_Char_IntArray st)
{
    st.i32_arr[0] += 100;

    int sum = 0;
    for (int i = 0; i < 32; ++i)
    {
        sum += st.i32_arr[i];
    }

    sum += st.c;
    return sum;
}

kernel void call_i32_func_byval_Char_IntArray(global int* out0,
                                              global int* out1,
                                              global int* input)
{
    ByVal_Char_IntArray st;
    st.c = 15;

    int id = get_global_id(0);

    int val = input[id];


    st.i32_arr[0] = 14;
    st.i32_arr[1] = -8;
    st.i32_arr[2] = val;
    st.i32_arr[3] = 900;

    for (int i = 4; i < 32; ++i)
    {
        st.i32_arr[i] = 0;
    }

    volatile int stack_object[16];
    for (int i = 0; i < 16; ++i)
    {
        const int test_val = 0x07080900 | i;
        stack_object[i] = test_val;
    }

    int result = i32_func_byval_Char_IntArray(st);

    // Check for stack corruption
    for (int i = 0; i < 16; ++i)
    {
        const int test_val = 0x07080900 | i;
        if (stack_object[i] != test_val)
            result = -1;
    }

    out0[id] = result;
    out1[id] = st.i32_arr[0];
}

NOINLINE
ByVal_Char_IntArray sret_Char_IntArray_func(global int* input, int id)
{
    ByVal_Char_IntArray st;
    st.c = 15;

    int val = input[id];
    st.i32_arr[0] = 14;
    st.i32_arr[1] = -8;
    st.i32_arr[2] = val;
    st.i32_arr[3] = 900;

    for (int i = 4; i < 32; ++i)
    {
        st.i32_arr[i] = 0;
    }

    return st;
}

kernel void call_sret_Char_IntArray_func(global int* output, global int* input)
{
    volatile int stack_object[16];
    for (int i = 0; i < 16; ++i)
    {
        const int test_val = 0x04030200 | i;
        stack_object[i] = test_val;
    }

    int id = get_global_id(0);
    ByVal_Char_IntArray st = sret_Char_IntArray_func(input, id);

    int sum = 0;
    for (int i = 0; i < 32; ++i)
    {
        sum += st.i32_arr[i];
    }

    sum += st.c;

    // Check for stack corruption
    for (int i = 0; i < 16; ++i)
    {
        const int test_val = 0x04030200 | i;
        if (stack_object[i] != test_val)
            sum = -1;
    }

    output[id] = sum;
}
