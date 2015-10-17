// [config]
// expect_result: pass
// glsl_version: 1.20
// require_extensions: GL_ARB_shader_storage_buffer_object
// check_link: true
// [end config]

#version 120
#extension GL_ARB_shader_storage_buffer_object: require

buffer bufblock {
  ivec4 iface_var;
};

void main()
{
  atomicMin(iface_var.x, 2);
  gl_Position = vec4(0.0);
}
