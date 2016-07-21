// [config]
// expect_result: fail
// glsl_version: 4.20
// [end config]

#version 420

out vec4 vs_tcs_result;

struct BasicStructure {
    vec3 member_a;
};

void main()
{
    BasicStructure variable = { vec4(0, 1, 0, 1) };

    float sum = variable.member_a.x + variable.member_a.y + variable.member_a.z;

    if (0 != sum)
    {
        vs_tcs_result = vec4(1, 0, 0, 1);
    }

    vs_tcs_result = vec4(0, 1, 1, 0);
}
