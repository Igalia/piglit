// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS - the while loop has a single statement: the if block. */
uniform bool condition;
void main()
{
    while (condition)
        if (condition)
            continue;
}
