/* FAIL
 *
 * Specifying a bad type as the return of a function should cleanly fail.
 *
 * Bug #29608
 */

foo bar()
{
}

void main()
{
    gl_Position = vec4(0.0);
}
