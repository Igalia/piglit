## Copyright (c) 2015 Intel Corporation
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

  def dedent(text):
      return '\n'.join(l.lstrip() for l in text.splitlines())

  def newlines(text):
      return '\n'.join(l for l in text.splitlines() if l.strip())
%>

<%def name="license()">
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
</%def>

<%def name="emit_header(params)" filter="dedent,newlines">
  ## Generated test, do not edit
  [require]
  GLSL >= ${params.formated_version}
  % if params.mode == 'varying':
    GL_MAX_VARYING_COMPONENTS >= ${params.varying_comps}
  % endif
</%def>

<%def name="matrix_data(first, dim, delim=', ')" filter="trim,dedent,newlines">
  ${delim.join(str(float(x)) for x in range(first, first + dim**2))}
</%def>

<%def name="emit_matrix_array_initializer(matrix_dim, array_dim, base_type)" filter="trim,newlines">
  % for c in range(array_dim):
${base_type}(${matrix_data(c * matrix_dim**2 + 1, matrix_dim)})\
    % if c < array_dim - 1:
, \
    % endif
  % endfor
</%def>

<%def name="emit_set_matrix(params)" filter="dedent,newlines">
  % if params.array_dim != 0:
    % if params.mode == 'temp':
      % if params.glsl_version == 120:
        ${params.type} m${params.dim} = ${params.type}(${emit_matrix_array_initializer(params.matrix_dim, params.array_dim, params.base_type)});
      % else:
        ${params.type} m${params.dim};
      % endif
    % endif
    % if params.glsl_version == 110 or params.mode == 'varying':
      % for i in range(params.array_dim):
        m[${i}] = mat${params.matrix_dim}(${matrix_data(1 + i * params.matrix_dim**2, params.matrix_dim)});
      % endfor
    % endif
  % else:
    % if params.mode == 'temp':
      ${params.type} m = ${params.type}(${matrix_data(1, params.matrix_dim)});
    % else:
      m = ${params.type}(${matrix_data(1, params.matrix_dim)});
    % endif
  % endif
</%def>

<%def name="emit_globals(params)" filter="dedent,newlines">
  % if params.array_dim != 0 and params.index_value == 'index':
    uniform int index;
  % endif

  % if params.col == 'col':
    uniform int col;
  % endif

  % if params.expect_type == 'float':
    uniform int row;
  % endif

  uniform ${params.expect_type} expect;

  % if params.glsl_version == 120 and params.mode == 'uniform':
    % if params.array_dim == 0:
      ${params.mode} ${params.type} m = ${params.type}(${matrix_data(1, params.matrix_dim)});
    % else:
      ${params.mode} ${params.type} m${params.dim} = ${params.type}(${emit_matrix_array_initializer(params.matrix_dim, params.array_dim, params.base_type)});
    % endif
  % elif params.mode != 'temp':
    ${params.mode} ${params.type} m${params.dim};
  % endif
  varying vec4 color;
</%def>

## TODO: convert do_compare into a bool
<%def name="emit_vs(params, do_compare)" filter="newlines">
[vertex shader]
${emit_globals(params)}

void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  % if params.mode == 'varying' or (params.mode == 'temp' and do_compare != 0):
    ${emit_set_matrix(params)}
  % endif

  % if do_compare != 0:
    % if params.mode == 'varying':
      /* From page 23 (page 30 of the PDF) of the GLSL 1.10 spec:
       *
       *     "A vertex shader may also read varying variables, getting back the
       *     same values it has written. Reading a varying variable in a vertex
       *     shader returns undefined values if it is read before being
       *     written."
       */
    % endif
    ## TODO: Could probably simplify this with the use of params.row
    % if params.expect_type == 'float':
      color = (m${params.idx}[${params.col}][row] == expect) \
    % else:
      color = (m${params.idx}[${params.col}] == expect) \
    % endif
    ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
  % endif
}
</%def>

## TODO: convert do_compare into a bool
<%def name="emit_fs(params, do_compare)" filter="newlines">
[fragment shader]
${emit_globals(params)}

void main()
{
  % if do_compare == 0 and params.mode == 'varying':
    /* There is some trickery here.  The fragment shader has to actually use
     * the varyings generated by the vertex shader, or the compiler (more
     * likely the linker) might demote the varying outputs to just be vertex
     * shader global variables.  Since the point of the test is the vertex
     * shader reading from a varying, that would defeat the test.
     */
  % endif
  % if do_compare != 0 or params.mode == 'varying':
    % if params.mode == 'temp':
      ${emit_set_matrix(params)}
    % endif
    gl_FragColor = (m${params.idx}[${params.col}]${params.row} == expect) \
    % if do_compare == 0:
      ? color : vec4(1.0, 0.0, 0.0, 1.0);
    % else:
      ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
    % endif
  % else:
    gl_FragColor = color;
  % endif
}
</%def>

<%def name="emit_test_vectors(params)" filter="dedent">
  <%block filter="newlines">
  [test]
  clear color 0.5 0.5 0.5 0.5
  clear
  ortho
  % if params.mode == 'uniform' and params.glsl_version == 110 and params.test_array_dim == 0:
    uniform ${params.cxr_type} m ${matrix_data(1, params.matrix_dim, delim=' ')}
  % endif

  </%block>
  % for size in params.test_sizes:
    <%block filter="newlines">
    % if params.mode == 'uniform' and params.glsl_version == 110 and params.test_array_dim != 0:
      % for c in range(params.test_array_dim):
        uniform ${params.cxr_type} m[${c}] ${matrix_data(1 + c * params.matrix_dim**2, params.matrix_dim, delim=' ')}
      % endfor
    % endif
    % if params.test_array_dim != 0 and params.index_value == 'index':
      uniform int index ${size - 1}
    % endif
    </%block>
    <% x_base = ((size - 1) * (15 * params.matrix_dim + 10)) %>
    % for column in params.test_columns:
      <%block filter="newlines">
      % if params.col == 'col':
        uniform int col ${column - 1}
      % endif
      </%block>

      % for row in params.test_rows:
        <%block filter="newlines">
        <% expect = (size - 1) * params.matrix_dim**2 + (column - 1) * params.matrix_dim + row %>
        % if params.expect_type == 'float':
          uniform int row ${row - 1}
          uniform float expect ${expect}
        % else:
          uniform ${params.expect_type} expect ${' '.join(str(i) for i in range(expect, expect + params.matrix_dim))}
        % endif

        <%
          x = x_base + 15 * column - 10
          y = 15 * row - 10
        %>
        draw rect ${x} ${y} 10 10
        probe rgb ${x + 5} ${y + 5} 0.0 1.0 0.0
        </%block>

      % endfor
    % endfor
  % endfor
</%def>
