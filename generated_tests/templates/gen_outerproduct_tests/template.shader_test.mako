## Copyright (c) 2014 Intel Corporation
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.

<%!
  from six.moves import range
%>

<%def name="spec()">
/* From page 43 (page 49 of the PDF) of the GLSL 1.20 spec:
 *
 *     "If an exact match is found, the other signatures are ignored, and the
 *     exact match is used. Otherwise, if no exact match is found, then the
 *     implicit conversions in Section 4.1.10 "Implicit Conversions" will be
 *     applied to the calling arguments if this can make their types match a
 *     signature."
 *
 * From page 20 (page 26 of the PDF) of the GLSL 1.20 spec:
 *
 *     "In some situations, an expression and its type will be implicitly
 *     converted to a different type. The following table shows all allowed
 *     implicit conversions:
 *
 *         Type of expression    Can be implicitly converted to
 *               int                         float
 *              ivec2                         vec2
 *              ivec3                         vec3
 *              ivec4                         vec4"
 */
</%def>

<%def name="const_shader_data()">
const ${params.vec_type}${params.columns} c = ${params.vec_type}${params.columns}(${", ".join(str(i + 1) for i in range(1, params.columns + 1))});
const ${params.vec_type}${params.rows} r = ${params.vec_type}${params.rows}(${", ".join(str(i + 1 + params.columns) for i in range(1, params.rows + 1))});
uniform ${params.matrix} expected = ${params.matrix}(${", ".join(str((i + 1 + params.columns) * (j + 1)) for i in range(1, params.rows + 1) for j in range(1, params.columns + 1))});
</%def>

<%def name="uniform_shader_data()">
uniform ${params.vec_type}${params.columns} c;
uniform ${params.vec_type}${params.rows} r;
uniform ${params.matrix} expected;
</%def>

[require]
GLSL >= 1.20
%if params.rows != params.columns:
# glUniformMatrix${params.rows}x${params.columns}fv only exists in OpenGL 2.1 or later.
GL >= 2.1
%endif

[vertex shader]
%if shader == 'vs' and params.vec_type == 'ivec':
${spec()}
%endif

%if shader == 'fs':
void main() { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; }
%else:
% if type == 'uniform':
${uniform_shader_data()}
% elif type == 'const':
${const_shader_data()}
% endif
varying vec4 color;

void main() {
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  ${params.matrix} result = outerProduct(c, r);
  color = (result == expected) ? vec4(0, 1, 0 ,1) : vec4(1, 0, 0, 1);
}
%endif

[fragment shader]
%if shader == 'fs' and params.vec_type == 'ivec':
${spec()}
%endif

%if shader == 'vs':
varying vec4 color;
void main() { gl_FragColor = color; }
%else:
% if type == 'uniform':
${uniform_shader_data()}
% elif type == 'const':
${const_shader_data()}
% endif

void main() {
  ${params.matrix} result = outerProduct(c, r);
  gl_FragColor = (result == expected) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
}
%endif

[test]
clear color 0.5 0.5 0.5 0.0
clear
ortho

%if type == 'uniform':
% for x in range(1, 5):
uniform ${params.vec_type}${params.columns} c ${" ".join(str(i + x) for i in range(1, params.columns + 1))}
uniform ${params.vec_type}${params.rows} r ${" ".join(str(i + x + params.columns) for i in range(1, params.rows + 1))}
uniform mat${params.rows}x${params.columns} expected ${" ".join(str((i + x + params.columns) * (j + x)) for i in range(1, params.rows + 1) for j in range(1, params.columns + 1))}
draw rect ${20 * x - 10} 10 10 10
probe rgb ${20 * x - 5} 15 0.0 1.0 0.0

% endfor
%elif type == 'const':
draw rect 10 10 10 10
probe rgb 15 15 0.0 1.0 0.0
%endif
