## Copyright (c) 2015-2016 Intel Corporation
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
  import textwrap

  from six.moves import range

  def clean_block(block):
      """Clean a block of text that is meant to have no indent or newlines.

      Removes blank lines and any leading or trailing whitespaces.
      
      """
      ret = []
      for l in block.splitlines():
          l = l.strip()
          if l:
              ret.append(l)

      return '\n'.join(ret)

  def trim_newlines(block):
      """Remove extra newlines."""
      return '\n'.join(l for l in block.splitlines() if l.strip())

  def replace_newline(block):
      """Replaces ">>newline" with a newline."""
      ret = []
      for l in block.splitlines():
          if ">>newline" in l:
              ret.append('')
          else:
              ret.append(l)
      return '\n'.join(ret)
              
%>

<%def name="license()" filter="textwrap.dedent">
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

<%def name="emit_globals(params)" filter="clean_block">
    uniform ${params.base_type} src_matrix;
    uniform vec${params.matrix_dim} v;
    uniform vec${params.matrix_dim} expect;

  % if params.array_dim != 0:
    uniform int index;
  % endif

  % if params.value_type == 'float':
    uniform int row;
  % endif

    uniform int col;
    uniform ${params.value_type} value;

  % if params.mode == 'varying':
    varying ${params.type} dst_matrix${params.dim};
  % endif
</%def>

<%def name="emit_distanceSqr_function(matrix_dim)" filter="clean_block">
  float distanceSqr(vec${matrix_dim} a, vec${matrix_dim} b) { vec${matrix_dim} diff = a - b; return dot(diff, diff); }
</%def>

<%def name="emit_set_matrix(params)" filter="clean_block">
% if params.array_dim != 0:
  % if int(params.version) >= 120:
    % if params.mode == 'temp':
      ${params.type} dst_matrix${params.dim} = ${params.type}(${', '.join('{}(0.0)'.format(params.base_type) for _ in range(1, params.array_dim + 1))});
    % else:
      dst_matrix = ${params.type}(${', '.join('{}(0.0)'.format(params.base_type) for _ in range(1, params.array_dim + 1))});
    % endif
  % else:
    % if params.mode == 'temp':
      ${params.type} dst_matrix${params.dim};
    % endif

    % for i in range(params.array_dim):
      dst_matrix[${i}] = ${params.base_type}(0.0);
    % endfor
  % endif
% elif params.mode == 'temp':
  ${params.type} dst_matrix${params.dim};
% endif
</%def>

<%def name="emit_transform(params)">
    /* Patch the supplied matrix with the supplied value.  If the resulting
     * matrix is correct, it will transform the input vector to the expected
     * value.  Verify that the distance between the result and the expected
     * vector is less than epsilon.
    % if params.array_dim != 0:
     *
     * NOTE: This test assumes that reads of arrays using non-constant
     * indicies works correctly.  If reads and writes happen to fail in an
     * identical manner, this test may give false positives.
    % endif
     */

    dst_matrix${params.idx} = src_matrix;
    dst_matrix${params.idx}${params.col}${params.row} = value;
</%def>

<%def name="emit_fs(params)">
[fragment shader]
${emit_globals(params)}

${emit_distanceSqr_function(params.matrix_dim)}

void main()
{
% if params.mode == 'temp':
  ${emit_set_matrix(params)}
  ${emit_transform(params)}
% endif

  gl_FragColor = (distanceSqr(dst_matrix${params.idx} * v, expect) < 4e-9)
    ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
}
</%def>

<%def name="emit_test_vectors(params)" filter="clean_block,replace_newline">
% for i in params.test_sizes:
  % if params.array_dim != 0 and params.index_value == 'index':
    uniform int index ${i - 1}
  % endif

  <% x_base = (i - 1) * (15 * params.matrix_dim + 10) %>
  % for c in params.test_columns:
    % if params.col == '[col]':
      uniform int col ${c - 1}
    % endif

    % for r in params.test_rows:
      % if params.value_type == 'float':
        uniform int row ${r - 1}
      % endif

      uniform vec${params.matrix_dim} v ${' '.join(params.test_vec)}
      uniform vec${params.matrix_dim} expect ${' '.join(params.test_exp)}
      uniform ${params.test_type} src_matrix ${params.test_matrix(c - 1, r - 1)}
      % if params.value_type == 'float':
        uniform ${params.value_type} value ${params.test_mat[c - 1][r - 1]}
      % else:
        uniform ${params.value_type} value ${' '.join(params.test_mat[c - 1])}
      % endif

      <%
        x = x_base + 15 * c - 10
        y = 15 * r - 10
      %>
      draw rect ${x} ${y} 10 10
      probe rgb ${x + 5} ${y + 5} 0.0 1.0 0.0

      ## This is replaced with a newline, after the formatting functions are applied
      >>newline
    % endfor
  % endfor
% endfor
</%def>
