// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

uniform int uniformInt;

void function(out int i)  
{
    i = 1; 
}

void main()
{
    function(uniformInt);  // out and inout parameters cannot be uniform since uniforms cannot be modified
}

