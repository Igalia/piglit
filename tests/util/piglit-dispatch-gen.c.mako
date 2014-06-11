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
% for alias_set in gl_registry.command_alias_map:
<% f0 = alias_set.primary_command %>\
static void*
resolve_${f0.name}(void)
{
% for req in alias_set.requirements:
>-------/* ${req.command.name} (${req.provider.name}) */
% for api in sorted(set(dispatch.APIS[x] for x in req.apis)):
% if loop.first:
>-------if ((
% else:
>-------     .
% endif
.............(dispatch_api == ${api.c_piglit_token}
% if req.has_feature and req.feature.version_int > api.base_version_int:
................................................... && check_version(${req.feature.version_int})
% endif
................................................................................................)
% if not loop.last:
................................................................................................. ||
% endif
% endfor
....................................................................................................)
% if req.has_extension:
>-------    && check_extension("${req.extension.name}")
% endif
...............................................................) {
% if req.has_feature:
>------->-------return get_core_proc("${req.command.name}", ${api.base_version_int});
% elif req.has_extension:
>------->-------return get_ext_proc("${req.command.name}");
% endif
>-------}

% endfor
>-------unsupported("${f0.name}");
>-------return piglit_dispatch_${f0.name};
}

static ${f0.c_return_type} APIENTRY
stub_${f0.name}(${f0.c_named_param_list})
{
>-------check_initialized();
>-------piglit_dispatch_${f0.name} = resolve_${f0.name}();
>-------
% if f0.c_return_type != 'void':
........return .
% endif
...............piglit_dispatch_${f0.name}(${f0.c_untyped_param_list});
}

PFN${f0.name.upper()}PROC piglit_dispatch_${f0.name} = stub_${f0.name};

% endfor
static void reset_dispatch_pointers(void)
{
% for alias_set in gl_registry.command_alias_map:
<% f0 = alias_set.primary_command %>\
>-------piglit_dispatch_${f0.name} = stub_${f0.name};
% endfor
}

static const char * function_names[] = {
% for command in gl_registry.commands:
>-------"${command.name}",
% endfor
};

static void* (*const function_resolvers[])(void) = {
% for alias_set in gl_registry.command_alias_map:
<% f0 = alias_set.primary_command %>\
>-------resolve_${f0.name},
% endfor
};
</%block>\
