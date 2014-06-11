/**
 * ${warning}
 *
 * Copyright 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

<%block filter='fake_whitespace'>\
#pragma once

#ifndef __PIGLIT_DISPATCH_GEN_H__
#define __PIGLIT_DISPATCH_GEN_H__

##
## Define function pointer typedefs, such as PFNGLACCUMPROC.
##
% for f in sorted(gl_registry.commands):
typedef ${f.c_return_type} (APIENTRY *PFN${f.name.upper()}PROC)(${f.c_named_param_list});
% endfor

##
## Emit macro wrapper for each dispatch function. For example,
##
##   #define glAttachObjectARB piglit_dispatch_glAttachShader
##
% for alias_set in gl_registry.command_alias_map:
<% f0 = alias_set.primary_command %>\
% for command in alias_set:
/* ${command.name}
% for requirement in command.requirements:
.................. (${requirement.provider.name})
% endfor
................................................. */
% endfor
extern PFN${f0.name.upper()}PROC piglit_dispatch_${f0.name};
% for command in alias_set:
#define ${command.name} piglit_dispatch_${f0.name}
% endfor

% endfor
##
## Define enums.
##
% for enum_group in gl_registry.enum_groups:
% if len(enum_group.enums) > 0:
/* Enum Group ${enum_group.name} */
% for enum in enum_group.enums:
#define ${enum.name} ${enum.c_num_literal}
% endfor

% endif
% endfor
/* Extensions */
% for extension in gl_registry.extensions:
#define ${extension.name} 1
% endfor

/* Versions */
% for feature in gl_registry.features:
#define ${feature.name} 1
% endfor

#endif /* __PIGLIT_DISPATCH_GEN_H__ */
</%block>\
