## coding=utf-8
<%def name="versioning()"><%
    if ver == 'GL_ARB_vertex_attrib_64bit':
        glsl_version_int = '150'
    else:
        glsl_version_int = ver

    glsl_version = '{}.{}'.format(glsl_version_int[0], glsl_version_int[1:3])

    return (glsl_version, glsl_version_int)
%></%def>\
<%def name="cols(in_type)"><%
    if 'mat' in in_type:
        if 'x' in in_type:
            return int(in_type[-3:][:1])
        else:
            return int(in_type[-1:])
    else:
        return 1
%></%def>\
<%def name="rows(in_type)"><%
    if 'vec' in in_type or 'mat' in in_type:
        return int(in_type[-1:])
    else:
        return 1
%></%def>\
${next.body()}\
