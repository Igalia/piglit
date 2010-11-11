// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

void function(const in int i);  

void main()
{
    int i;
    function(i);  
}

// function definition has different parameter qualifiers than function declaration
void function(in int i)  
{  
   i = 3;
}
