## coding=utf-8
<%def name="versioning()"><%
    if ver == 'GL_ARB_vertex_attrib_64bit':
        glsl_version_int = '150'
    else:
        glsl_version_int = ver

    glsl_version = '{}.{}'.format(glsl_version_int[0], glsl_version_int[1:3])

    return (glsl_version, glsl_version_int)
%></%def>\
${next.body()}\
