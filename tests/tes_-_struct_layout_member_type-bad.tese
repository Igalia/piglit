// [config]
// expect_result: fail
// glsl_version: 4.20
// [end config]

#version 420

layout(isolines, point_mode) in;

in  vec4 tcs_tes_result[];
out vec4 tes_gs_result;

struct BasicStructure {
    vec4 member_a;
    vec3 member_b;
};

struct StructureWithStructure {
    BasicStructure member_a;
    vec4           member_b;
};


void main()
{
    vec4 result = vec4(0, 1, 0, 1);

    StructureWithStructure variable = { { {0, 1, 0, 1}, vec4(0, 1, 0, 1) }, vec4(1, 0, 1, 0) };

    float sum = variable.member_a.member_a.x + variable.member_a.member_a.y + variable.member_a.member_a.z + variable.member_a.member_a.w + variable.member_a.member_b.x + variable.member_a.member_b.y + variable.member_a.member_b.z + variable.member_b.x + variable.member_b.y + variable.member_b.z + variable.member_b.w;

    if (0 != sum)
    {
        result = vec4(1, 0, 0, 1);
    }
    else if (vec4(0, 1, 0, 1) != tcs_tes_result[0])
    {
         result = vec4(1, 0, 0, 1);
    }

    tes_gs_result = result;
}
