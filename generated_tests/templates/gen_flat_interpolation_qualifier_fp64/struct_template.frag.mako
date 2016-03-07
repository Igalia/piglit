/* [config]
${config}\
 * [end config]
 *
 * Declare a ${mode} interpolation ${type_name} in a struct fragment input.
${comments}\
 */

${preprocessor}\

struct S {
    ${type_name} ${var_name};
};

${interpolation_mode}in S s;
out vec4 color;

void main()
{
    color = vec4(${var_as_vec4});
}
