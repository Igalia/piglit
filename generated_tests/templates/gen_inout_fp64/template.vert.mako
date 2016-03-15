/* [config]
${config}\
 * [end config]
${comments}\
 */

${preprocessor}\

in ${type_name} vertex;

void main()
{
  gl_Position = vec4(vertex${extra_params});
}

