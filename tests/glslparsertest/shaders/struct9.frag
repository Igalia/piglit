// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

const struct s {
    int i;
} s1 = s(1);

void main()
{
   s1.i = 1;  // const struct members cannot be modified
}
