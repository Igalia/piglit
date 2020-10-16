# coding=utf-8
#
# Copyright © 2020 Valve Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT


"""Tests for replayer's query_traces_yaml module."""

import pytest

from framework import exceptions

from framework.replay import query_traces_yaml as qty


YAML_DATA = [["", {}],
        ["""
- First
- Second
""",
         ["First", "Second"]],
        ["""
a: 1
b:
  c: 2
""",
         {"a": 1, "b": {"c": 2}}],
]

TRACES_DATA = {"traces":
              [{"path": "glmark2/desktop-blur-radius=5:effect=blur:passes=1:separable=true:windows=4.rdc",
                "expectations": [{"device": "gl-vmware-llvmpipe",
                                  "checksum": "8867f3a41f180626d0d4b7661ff5c0f4"}]},
               {"path": "glxgears/glxgears-2.trace",
                "expectations": [{"device": "gl-vmware-llvmpipe",
                                  "checksum": "f8eba0fec6e3e0af9cb09844bc73bdc7"},
                                 {"device": "gl-virgl",
                                  "checksum": "f8eba0fec6e3e0af9cb09844bc73bdc7"}]},
               {"path": "pathfinder/demo.trace",
                "expectations": [{"device": "gl-vmware-llvmpipe",
                                  "checksum": "e624d76c70cc3c532f4f54439e13659a"}]},
               {"path": "KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr",
                "expectations": [{"device": "vk-amd-polaris10",
                                  "checksum": "917cbbf4f09dd62ea26d247a1c70c16e"}]},]}

@pytest.mark.raises(exception=exceptions.PiglitFatalError)
def test_load_yaml_PiglitFatalError():
    """query_traces_yaml.load_yaml: Raise PiglitFatalError on invalid YAML"""
    y = qty.load_yaml("*** this is not YAML ***")


def test_load_yaml_basic():
    """query_traces_yaml.load_yaml: Load some basic YAML documents"""

    for i in YAML_DATA:
        y = qty.load_yaml(i[0])

        assert i[1] == y


@pytest.mark.raises(exception=TypeError)
@pytest.mark.parametrize("trace", [
    ([]),
    ({"expectations": "one"}),
    ({"expectations": ["one", "another"]}),
])
def test_trace_devices_TypeError(trace):
    """query_traces_yaml.trace_devicess: Raise TypeError on invalid trace"""
    d = qty.trace_devices(trace)


@pytest.mark.parametrize("trace, expected", [
    ({}, []),
    ({"one": 1}, []),
    ({"expectations": [{"one": 1}]}, []),
    ({"expectations": [{"device": "gl-vmware-lvmpipe"}]},
     ["gl-vmware-lvmpipe"]),
    ({"expectations": [{"device": "gl-vmware-lvmpipe"},
                       {"device": "vk-intel-anv", "checksum": "a checksum"}]},
     ["gl-vmware-lvmpipe", "vk-intel-anv"]),
])
def test_trace_devices_basic(trace, expected):
    """query_traces_yaml.trace_devicess: Get devices from some basic traces"""
    assert expected == qty.trace_devices(trace)


@pytest.mark.raises(exception=TypeError)
@pytest.mark.parametrize("trace, device", [
    ([], None),
    ({"expectations": "one"}, None),
    ({"expectations": ["one", "another"]}, None),
])
def test_trace_checksum_TypeError(trace, device):
    """query_traces_yaml.trace_checksum: Raise TypeError on invalid trace"""
    c = qty.trace_checksum(trace, device)


@pytest.mark.parametrize("trace, device, expected", [
    ({}, None, ''),
    ({"one": 1}, None, ''),
    ({"expectations": [{"one": 1}]}, None, ''),
    ({"expectations": [{"device": "gl-vmware-lvmpipe"}]}, "gl-vmware-lvmpipe", ''),
    ({"expectations": [{"device": "gl-vmware-lvmpipe", "checksum": "a checksum"}]},
     "gl-vmware-lvmpipe", "a checksum"),
    ({"expectations": [{"device": "gl-vmware-lvmpipe", "checksum": "a checksum"},
                       {"device": "vk-intel-anv", "checksum": "another checksum"}]},
     "vk-intel-anv", "another checksum"),
    ({"expectations": [{"device": "gl-vmware-lvmpipe", "checksum": "a checksum"},
                       {"device": "vk-intel-anv", "checksum": "another checksum"},
                       {"device": "vk-intel-anv", "checksum": "yet another checksum"}]},
     "vk-intel-anv", "another checksum"),
])
def test_trace_checksum_basic(trace, device, expected):
    """query_traces_yaml.trace_checksum: Get checksum from some basic traces"""
    assert expected == qty.trace_checksum(trace, device)


@pytest.mark.raises(exception=TypeError)
@pytest.mark.parametrize("yaml", [
    (8),
    ({"traces-db": "one"}),
])
def test_download_url_TypeError(yaml):
    """query_traces_yaml.download_url: Raise TypeError on invalid YAML"""
    u = qty.download_url(yaml)


@pytest.mark.parametrize("yaml, expected", [
    ({}, None),
    ({"traces-db": {"one": 1}}, None),
    ({"traces-db": {"download-url": "an url"}}, "an url"),
    ({"traces-db": {"one": 1, "download-url": "an url"}}, "an url"),
])
def test_download_url_basic(yaml, expected):
    """query_yamls_yaml.download_url: Get download url from some basic YAML with a trace-db entry"""
    assert expected == qty.download_url(yaml)


@pytest.mark.raises(exception=AttributeError)
@pytest.mark.parametrize("yaml, ext, device, checksum", [
    (8, ".trace", "gl-vmware-llvmpipe", True),
    (TRACES_DATA, {}, None, False),
])
def test_traces_AttributeError(yaml, ext, device, checksum):
    """query_traces_yaml.traces: Raise AttributeError on invalid parameters"""
    t = list(qty.traces(yaml, ext, device, checksum))


@pytest.mark.raises(exception=TypeError)
@pytest.mark.parametrize("yaml, ext, device, checksum", [
    ({"traces": "one"}, ".trace", "gl-vmware-llvmpipe", True),
    ({"traces": ["one"]}, ".trace", "gl-vmware-llvmpipe", True),
    ({"traces": [{"path": 8}]}, ".trace", "gl-vmware-llvmpipe", True),
])
def test_traces_TypeError(yaml, ext, device, checksum):
    """query_traces_yaml.traces: Raise TypeError on invalid YAML"""
    t = list(qty.traces(yaml, ext, device, checksum))


@pytest.mark.parametrize("yaml, ext, device, checksum, expected", [
    ({}, ".trace", "gl-vmware-llvmpipe", True, []),
    ({"traces": [{}]}, ".trace", "gl-vmware-llvmpipe", True, []),
    (TRACES_DATA, "", "gl-virgl", False, []),
    (TRACES_DATA, ".rdc", "gl-virgl", False, []),
    (TRACES_DATA, ".trace", "gl-virgl", False,
     [{"path": "glxgears/glxgears-2.trace"}]),
    (TRACES_DATA, ".rdc,.trace", "gl-vmware-llvmpipe", False,
     [{"path": "glmark2/desktop-blur-radius=5:effect=blur:passes=1:separable=true:windows=4.rdc"},
      {"path": "glxgears/glxgears-2.trace"},
      {"path": "pathfinder/demo.trace"}]),
    (TRACES_DATA, ".trace", None, False,
     [{"path": "glxgears/glxgears-2.trace"},
      {"path": "pathfinder/demo.trace"}]),
    (TRACES_DATA, ".gfxr", "vk-amd-polaris10", True,
     [{"checksum": "917cbbf4f09dd62ea26d247a1c70c16e",
       "path": "KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr"}]),
    (TRACES_DATA, ".gfxr", None, True,
     [{"checksum": "",
       "path": "KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr"}]),
])
def test_traces_basic(yaml, ext, device, checksum, expected):
    """query_traces_yaml.traces: Get traces lists from some basic yamls"""
    assert expected == list(qty.traces(yaml, ext, device, checksum))
