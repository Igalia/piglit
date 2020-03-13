// [config]
// expect_result: fail
// glsl_version: 1.10
// [end config]
//

void main()
{
    int i;
    for (i = 0; i < 10; i += j);
}
