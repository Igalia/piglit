// [config]
// expect_result: fail
// glsl_version: 1.00
// [end config]

#version 100

/* Function using the float return type with no precision specified */
float
get_three()
{
        return 3.0;
}

void
main(void)
{
        gl_FragColor = vec4(get_three());
}
