// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

struct s {
    int i;
} s1;

struct ss {
    int i;
} s2;

void main()
{
    s1 = s2;  // two different structures cannot be assigned to each other
}
