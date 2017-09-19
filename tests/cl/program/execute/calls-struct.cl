/*!

[config]
name: calls with structs
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


[test]
name: byval struct and sret struct
kernel_name: call_sret_Char_IntArray_func_byval_Char_IntArray
dimensions: 1
global_size: 16 0 0

arg_out: 0 buffer int[16]        \
  86 87 88 89   \
  90 91 92 93   \
  94 95 96 97   \
  98 99 100 101

arg_out: 1 buffer int[16]        \
  134  135  136  137  \
  138  139  140  141  \
  142  143  144  145  \
  146  147  148  149

arg_in: 2 buffer int[16] \
 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

!*/

#define NOINLINE __attribute__((noinline))

typedef struct ByVal_Char_IntArray {
    char c;
    int i[4];
} ByVal_Char_IntArray;

NOINLINE
int i32_func_byval_Char_IntArray(ByVal_Char_IntArray st)
{
    st.i[0] += 100;

    int sum = 0;
    for (int i = 0; i < 4; ++i)
    {
        sum += st.i[i];
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
    st.i[0] = 14;
    st.i[1] = -8;
    st.i[2] = val;
    st.i[3] = 900;

    int result = i32_func_byval_Char_IntArray(st);
    out0[id] = result;
    out1[id] = st.i[0];
}

NOINLINE
ByVal_Char_IntArray sret_Char_IntArray_func(global int* input, int id)
{
    ByVal_Char_IntArray st;
    st.c = 15;

    int val = input[id];
    st.i[0] = 14;
    st.i[1] = -8;
    st.i[2] = val;
    st.i[3] = 900;

    return st;
}

kernel void call_sret_Char_IntArray_func(global int* output, global int* input)
{
    int id = get_global_id(0);
    ByVal_Char_IntArray st = sret_Char_IntArray_func(input, id);

    int sum = 0;
    for (int i = 0; i < 4; ++i)
    {
        sum += st.i[i];
    }

    sum += st.c;
    output[id] = sum;
}

NOINLINE
ByVal_Char_IntArray sret_Char_IntArray_func_byval_Char_IntArray(ByVal_Char_IntArray st)
{
    st.c += 15;

    st.i[0] += 14;
    st.i[1] -= 8;
    st.i[2] += 9;
    st.i[3] += 18;

    return st;
}

kernel void call_sret_Char_IntArray_func_byval_Char_IntArray(global int* output0,
                                                             global int* output1,
                                                             global int* input)
{
    int id = get_global_id(0);

    volatile ByVal_Char_IntArray st0;
    st0.c = -20;

    int val = input[id];
    st0.i[0] = 14;
    st0.i[1] = -8;
    st0.i[2] = val;
    st0.i[3] = 100;

    ByVal_Char_IntArray st1 = sret_Char_IntArray_func_byval_Char_IntArray(st0);

    int sum0 = 0;
    int sum1 = 0;
    for (int i = 0; i < 4; ++i)
    {
        sum0 += st0.i[i];
        sum1 += st1.i[i];
    }

    sum0 += st0.c;
    sum1 += st1.c;

    output0[id] = sum0;
    output1[id] = sum1;
}
