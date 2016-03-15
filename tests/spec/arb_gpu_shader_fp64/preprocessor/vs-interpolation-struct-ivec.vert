#version 150

struct C {
    ivec4 fs_color;
};

in vec4 piglit_vertex;
out C c;

void main() {
  gl_Position = piglit_vertex;
  c.fs_color = ivec4((piglit_vertex.x*2*piglit_vertex.y+3*piglit_vertex.z)*4.0, (piglit_vertex.x*20*piglit_vertex.y+3*piglit_vertex.z)*8.0, (piglit_vertex.x*2*piglit_vertex.y+3*piglit_vertex.z)*14.0, 1.0);
}
