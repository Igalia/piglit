# Section 4.3.7 (Interpolation) of the GLSL 1.30 spec says:
#
#     "If gl_Color is redeclared with an interpolation qualifier, then
#     gl_FrontColor and gl_BackColor (if they are written to) must
#     also be redeclared with the same interpolation qualifier, and
#     vice versa. If gl_SecondaryColor is redeclared with an
#     interpolation qualifier, then gl_FrontSecondaryColor and
#     gl_BackSecondaryColor (if they are written to) must also be
#     redeclared with the same interpolation qualifier, and vice
#     versa. This qualifier matching on predeclared variables is only
#     required for variables that are statically used within the
#     shaders in a program."
#
# Even though some of the other rules for interpolation qualifier
# matching changed in 4.x specifications, this rule has remained the
# same.
#
# We interpret the sentence "variables that are statically used within the
# shaders in a program" to mean static use of the variable in any shader in
# the program invokes the redeclaration requirement.  Since neither shader
# accesses any built-in variables, linking should succeed no matter what the
# interpolation qualifiers say.
[require]
GLSL >= 1.30

[vertex shader]
% if vs_mode != 'default':
${vs_mode} out vec4 ${vs_variable};
% endif
void main() { gl_Position = vec4(0); }

[fragment shader]
% if fs_mode != 'default':
${fs_mode} in vec4 ${fs_variable};
% endif
out vec4 c;
void main() { c = vec4(0); }

[test]
link success
