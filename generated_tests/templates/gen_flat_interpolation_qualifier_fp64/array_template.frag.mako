/* [config]
${config}\
 * [end config]
 *
 * Declare a ${mode} interpolation ${type_name} array fragment input.
${comments}\
 */

${preprocessor}\

${interpolation_mode}in ${type_name} ${var_name}[4];
out vec4 color;

void main()
{
    color = vec4(${var_as_vec4});
}
