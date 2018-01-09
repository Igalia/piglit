#version 450
#extension GL_ARB_enhanced_layouts: require

layout(location = 0, xfb_offset = 0) out double x1_out;
layout(location = 1, xfb_offset = 8) out dvec2 x2_out;
layout(location = 2, xfb_offset = 24) out dvec4 x3_out;

void main() {
        gl_Position = vec4(0.0);
        x1_out = 1.0lf;
        x2_out = dvec2(2.0lf, 3.0lf);
        x3_out = dvec4(4.0lf, 5.0lf, 6.0lf, 7.0lf);
}
