// [config]
// expect_result: pass
// glsl_version: 4.30
// [end config]
//
// The tests for a compiler which leads to mismatch between max_array_access
// and the var type when the compiler tries to optimize unused ssbo instances.
// This mismatch leads to crash because in some cases the type length is less
// than the max_array_access field.

#version 430

layout(packed) buffer BlockA {
   float a;
} blockA[3];

layout(packed) buffer BlockB {
   float a;
} blockB[2];

layout(packed) buffer BlockC {
   float a;
} blockC[2];

out vec4 color;

void main()
{
   color = vec4(blockA[1].a, blockB[0].a, blockC[1].a, 1.0);
}
