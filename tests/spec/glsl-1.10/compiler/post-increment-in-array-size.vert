// [config]
// expect_result: fail
// glsl_version: 1.10
// [end config]

void main()
{
        float[a+++4 ? 1:1] f;
}
