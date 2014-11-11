/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 *
 * Page 32 (page 38 of the PDF) of the GLSL 1.10 spec says:
 *
 *     "Variables that are built-in types, entire structures, structure
 *     fields, l-values with the field selector ( . ) applied to select
 *     components or swizzles without repeated fields, and l-values
 *     dereferenced with the array subscript operator ( [ ] ) are all
 *     l-values. Other binary or unary expressions, non-dereferenced arrays,
 *     function names, swizzles with repeated fields, and constants cannot be
 *     l-values.  The ternary operator (?:) is also not allowed as an
 *     l-value."
 */
uniform ${type_name} u;
${mode} vec4 v;

void main()
{
    ${type_name} t = u;

    ${op} = ${type_name}(v${components});
    ${dest} = ${var_as_vec4};
}
