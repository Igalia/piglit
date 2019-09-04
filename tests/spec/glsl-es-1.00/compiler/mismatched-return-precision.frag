// [config]
// expect_result: fail
// glsl_version: 1.00
// [end config]

#version 100

precision mediump float;

/* The prototype uses the default (mediump) precision for the return type. */
float
get_three();

/* The definition use an explicit highp precision. */
highp float
get_three()
{
        return 3.0;
}

void
main(void)
{
        gl_FragColor = vec4(get_three());
}
