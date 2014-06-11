# Copyright 2014 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

"""
Parse gl.xml into Python objects.
"""

from __future__ import print_function


import os.path
import re
import sys

from collections import namedtuple
from copy import copy, deepcopy


# Export 'debug' so other Piglit modules can easily enable it.
debug = True


def _log_debug(msg):
    if debug:
        print('debug: {0}: {1}'.format(__name__, msg), file=sys.stderr)


# Prefer the external module 'lxml.etree' (it uses libxml2) over Python's
# builtin 'xml.etree.ElementTree'.  It's faster.
try:
    import lxml.etree as etree
    _log_debug('etree is lxml.etree')
except ImportError:
    import xml.etree.cElementTree as etree
    _log_debug('etree is xml.etree.cElementTree')


# Define a Python 2.6 compatibility wrapper for ElementTree.iterfind.
_etree_iterfind = None
if hasattr(etree.ElementTree(), 'iterfind'):
    _etree_iterfind = lambda elem, match: elem.iterfind(match)
    _log_debug('_etree_iterfind wraps ElementTree.iterfind')
else:
    _etree_iterfind = lambda elem, match: iter(elem.findall(match))
    _log_debug(('_etree_iterfind wraps ElementTree.findall for '
                'Python 2.6 compatibility'))


def parse():
    """Parse gl.xml and return a Registry object."""
    filename = os.path.join(os.path.dirname(__file__), 'gl.xml')
    xml_registry = etree.parse(filename).getroot()
    _repair_xml(xml_registry)
    return Registry(xml_registry)


def _repair_xml(xml_registry):
    fixes = set((
        'GL_ALL_ATTRIB_BITS',
        'glOcclusionQueryEventMaskAMD',
        'enums_SGI_0x8000_0x80BF',
        'enums_ARB_0x80000_0x80BF',
        'gles2_GL_ACTIVE_PROGRAM_EXT',
    ))

    remove_queue = []

    def defer_removal(parent, child):
        remove_queue.append((parent, child))

    for enums in _etree_iterfind(xml_registry, './enums'):
        if ('GL_ALL_ATTRIB_BITS' in fixes
            and enums.get('group') == 'AttribMask'):
                # The XML defines GL_ALL_ATTRIB_BITS incorrectly with all bits
                # set (0xFFFFFFFF). From the GL_ARB_multisample spec, v5:
                #
                #     In order to avoid incompatibility with GL implementations
                #     that do not support SGIS_multisample, ALL_ATTRIB_BITS
                #     does not include MULTISAMPLE_BIT_ARB.
                #
                enum = enums.find("./enum[@name='GL_ALL_ATTRIB_BITS']")
                enum.set('value', '0x000FFFFF')

                fixes.remove('GL_ALL_ATTRIB_BITS')
                continue

        if ('glOcclusionQueryEventMaskAMD' in fixes
            and enums.get('namespace') == 'OcclusionQueryEventMaskAMD'):
                # This tag's attributes are totally broken.
                enums.set('namespace', 'GL')
                enums.set('group', 'OcclusionQueryEventMaskAMD')
                enums.set('type', 'bitmask')

                fixes.remove('glOcclusionQueryEventMaskAMD')
                continue

        if ('enums_SGI_0x8000_0x80BF' in fixes
            and enums.get('vendor') == 'SGI'
            and enums.get('start') == '0x8000'
            and enums.get('end') == '0x80BF'):
                # This element is empty garbage that overlaps an ARB enum group
                # with the same range.
                defer_removal(xml_registry, enums)

                fixes.remove('enums_SGI_0x8000_0x80BF')
                continue

        if ('enums_ARB_0x80000_0x80BF' in fixes
            and enums.get('vendor') == 'ARB'
            and enums.get('group', None) is None
            and enums.get('start', None) is None):
                # This tag lacks 'start' and 'end' attributes.
                enums.set('start', '0x8000')
                enums.set('end', '0x80BF')

                fixes.remove('enums_ARB_0x80000_0x80BF')
                continue

        if ('gles2_GL_ACTIVE_PROGRAM_EXT' in fixes
            and enums.get('vendor') == 'ARB'
            and enums.get('start') <= '0x8259'
            and enums.get('end') >= '0x8259'):
                # GL_ACTIVE_PROGRAM_EXT has different numerical values in GL
                # (0x8B8D) and in GLES (0x8259). Remove the GLES value to avoid
                # redefinition collisions.
                bad_enum = enums.find(("./enum"
                                       "[@value='0x8259']"
                                       "[@name='GL_ACTIVE_PROGRAM_EXT']"
                                       "[@api='gles2']"))
                defer_removal(enums, bad_enum)

                fixes.remove('gles2_GL_ACTIVE_PROGRAM_EXT')
                continue

    for (parent, child) in remove_queue:
        parent.remove(child)

    if len(fixes) > 0:
        raise Exception('failed to apply some xml repairs: ' +
                        ', '.join(map(repr, fixes)))


class OrderedKeyedSet(object):
    """A set with keyed elements that preserves order of element insertion.

    Why waste words? Let's document the class with example code.

    Example:
        Cheese = namedtuple('Cheese', ('name', 'flavor'))
        cheeses = OrderedKeyedSet(key='name')
        cheeses.add(Cheese(name='cheddar', flavor='good'))
        cheeses.add(Cheese(name='gouda', flavor='smells like feet'))
        cheeses.add(Cheese(name='romano', flavor='awesome'))

        # Elements are retrievable by key.
        assert(cheeses['gouda'].flavor == 'smells like feet')

        # On key collision, the old element is removed.
        cheeses.add(Cheese(name='gouda', flavor='ok i guess'))
        assert(cheeses['gouda'].flavor == 'ok i guess')

        # The set preserves order of insertion. Replacement does not alter
        # order.
        assert(list(cheeses)[2].name == 'romano')

        # The set is iterable.
        for cheese in cheeses:
            print(cheese.name)

        # Yet another example...
        Bread = namedtuple('Bread', ('name', 'smell'))
        breads = OrderedKeyedSet(key='name')
        breads.add(Bread(name='como', smell='subtle')
        breads.add(Bread(name='sourdough', smell='pleasant'))

        # The set supports some common set operations, such as union.
        breads_and_cheeses = breads | cheeses
        assert(len(breads_and_cheeses) == len(breads) + len(cheeses))
    """

    def __init__(self, key, elems=()):
        """Create a new set with the given key.

        The given 'key' defines how to calculate each element's key value.  If
        'key' is a string, then each key value is defined to be
        `getattr(elem, key)`.  If 'key' is a function, then the key value is
        `key(elem)`.
        """

        # A linked list contains the set's items. Each list node is a 4-tuple
        # [prev, next, key, value].  The root node is permanent.
        root = []
        root[:] = [root, root, None, None]
        self.__list_root = root

        # For quick retrieval, we map each key to its node. That is, each map
        # pair has form {key: [prev, next, key, value])}.
        self.__map = dict()

        if isinstance(key, str):
            self.__key_func = lambda elem: getattr(elem, key)
        else:
            self.__key_func = key

        for e in elems:
            self.add(e)

    def __or__(self, other):
        """Same as `union`."""
        return self.union(other)

    def __contains__(self, key):
        return key in self.__map

    def __copy__(self):
        return OrderedKeyedSet(key=deepcopy(self.__key_func),
                               elems=iter(self))

    def __getitem__(self, key):
        return self.__map[key][3]

    def __iter__(self):
        return self.itervalues()

    def __len__(self):
        return len(self.__map)

    def __repr__(self):
        templ = '{self.__class__.__name__}({self.name!r})'
        return templ.format(self=self)

    def add(self, value):
        key = self.__key_func(value)
        node = self.__map.get(key, None)
        if node is not None:
            node[3] = value
        else:
            root = self.__list_root
            old_tail = root[0]
            new_tail = [old_tail, root, key, value]
            new_tail[0][1] = new_tail
            new_tail[1][0] = new_tail
            self.__map[key] = new_tail

    def clear(self):
        self.__map.clear()
        root = self.__list_root
        root[:] = [root, root, None, None]

    def extend(self, elems):
        for e in elems:
            self.add(e)

    def get(self, key, default):
        node = self.__map.get(key, None)
        if node is not None:
            return node[3]
        else:
            return default

    def iteritems(self):
        root = self.__list_root
        node = root[1]
        while node is not root:
            yield (node[2], node[3])
            node = node[1]

    def iterkeys(self):
        return (i[0] for i in self.iteritems())

    def itervalues(self):
        return (i[1] for i in self.iteritems())

    def pop(self, key):
        node = self.__map.pop(key)
        node[0][1] = node[1]
        node[1][0] = node[0]
        return node[3]

    def sort_by_key(self):
        sorted_items = sorted(self.__map.iteritems(),
                              cmp=lambda x, y: cmp(x[0], y[0]))
        self.clear()
        for item in sorted_items:
            self.add(item[1])

    def sort_by_value(self):
        sorted_values = sorted(self.__map.itervalues())
        self.clear()
        for value in sorted_values:
            self.add(value)

    def union(self, other):
        """Return the union of two sets as a new set.

        In the new set, all elements of the self set precede those of the other
        set. The order of elements in the new set preserves the order of the
        original sets.

        The new set's key function is copied from self.  On key collisions, set
        y has precedence over x.
        """
        u = copy(self)
        u.extend(other)
        return u


class ImmutableOrderedKeyedSet(OrderedKeyedSet):

    def __init__(self, key, elems):
        self.__is_frozen = False
        OrderedKeyedSet.__init__(self, key=key, elems=elems)
        self.__is_frozen = True

    def add(self, value):
        if self.__is_frozen:
            raise ImmutableError
        else:
            OrderedKeyedSet.add(self, value)

    def pop(self, key):
        raise ImmutableError

    def clear(self):
        raise ImmutableError


class ImmutableError:
    pass


# Values that may appear in the XML attributes 'api' and 'supported'.
VALID_APIS = frozenset(('gl', 'glcore', 'gles1', 'gles2'))


class Registry(object):
    """The toplevel <registry> element.

    Attributes:
        features: An OrderedKeyedSet that contains a Feature for each <feature>
        subelement.

        extensions: An OrderedKeyedSet that contains an Extension for each
        <extension> subelement.

        commands: An OrderedKeyedSet that contains a Command for each <command>
        subelement.

        command_alias_map: A CommandAliasMap that contains a CommandAliasSet
        for each equivalence class of commands.

        enum_groups: An OrderedKeyedSet that contains an EnumGroup for each
        <enums> subelement.

        enums: An OrderedKeyedSet that contains an Enum for each <enum>
        subelement.

        vendor_namespaces: A collection of all vendor prefixes and suffixes,
        such as "ARB", "EXT", "CHROMIUM", and "NV".
    """

    def __init__(self, xml_registry):
        """Parse the <registry> element."""

        assert(xml_registry.tag == 'registry')

        self.command_alias_map = CommandAliasMap()
        self.commands = OrderedKeyedSet(key='name')
        self.enum_groups = []
        self.enums = OrderedKeyedSet(key='name')
        self.extensions = OrderedKeyedSet(key='name')
        self.features = OrderedKeyedSet(key='name')
        self.vendor_namespaces = set()

        for xml_command in _etree_iterfind(xml_registry,
                                           './commands/command'):
            command = Command(xml_command)
            self.commands.add(command)
            self.command_alias_map.add(command)

        for xml_enums in _etree_iterfind(xml_registry, './enums'):
            enum_group = EnumGroup(xml_enums)
            self.enum_groups.append(enum_group)
            for enum in enum_group.enums:
                self.enums.add(enum)

        for xml_feature in _etree_iterfind(xml_registry, './feature'):
            feature = Feature(xml_feature, command_map=self.commands,
                              enum_map=self.enums)
            self.features.add(feature)

        for xml_ext in _etree_iterfind(xml_registry, './extensions/extension'):
            ext = Extension(xml_ext, command_map=self.commands,
                            enum_map=self.enums)
            self.extensions.add(ext)
            self.vendor_namespaces.add(ext.vendor_namespace)

        self.vendor_namespaces.remove(None)


class Feature(object):
    """A <feature> XML element.

    Attributes:
        name: The XML element's 'name' attribute.

        api: The XML element's 'api' attribute.

        version_str:  The XML element's 'number' attribute. For example, "3.1".

        version_float: float(version_str)

        version_int: int(10 * version_float)

        requirements: A collection of Requirement for each Command and Enum
        this Feature requires.
    """

    def __init__(self, xml_feature, command_map, enum_map):
        """Parse a <feature> element."""

        # Example <feature> element:
        #
        #    <feature api="gles2" name="GL_ES_VERSION_3_1" number="3.1">
        #        <!-- arrays_of_arrays features -->
        #        <require/>
        #        <!-- compute_shader features -->
        #        <require>
        #            <command name="glDispatchCompute"/>
        #            <command name="glDispatchComputeIndirect"/>
        #            <enum name="GL_COMPUTE_SHADER"/>
        #            <enum name="GL_MAX_COMPUTE_UNIFORM_BLOCKS"/>
        #            ...
        #        </require>
        #        <!-- draw_indirect features -->
        #        <require>
        #            <command name="glDrawArraysIndirect"/>
        #            <command name="glDrawElementsIndirect"/>
        #            <enum name="GL_DRAW_INDIRECT_BUFFER"/>
        #            <enum name="GL_DRAW_INDIRECT_BUFFER_BINDING"/>
        #        </require>
        #        ...
        #    </feature>

        assert(xml_feature.tag == 'feature')

        # Parse the <feature> tag's attributes.
        self.name = xml_feature.get('name')
        self.api = xml_feature.get('api')
        self.is_gles = self.name.startswith('GL_ES')

        self.version_str = xml_feature.get('number')
        self.version_float = float(self.version_str)
        self.version_int = int(10 * self.version_float)

        self.__parse_requirements(xml_feature, command_map, enum_map)

        assert(self.api in VALID_APIS)
        assert(len(self.requirements) > 0)

    def __cmp__(self, other):
        if self is other:
            return 0

        # Sort features before extensions.
        if isinstance(other, Extension):
            return -1

        # Sort GL before GLES.
        diff = cmp(self.is_gles, other.is_gles)
        if diff != 0:
            return diff

        return cmp(self.name, other.name)

    def __repr__(self):
        templ = '{self.__class__.__name__}({self.name!r})'
        return templ.format(self=self)

    def __parse_requirements(self, xml_feature, command_map, enum_map):
        """For each <command> and <enum> under a <require>, create
        a Requirement that links this Feature to a Command or Enum.
        """
        self.requirements = set()

        def link(x):
            req = Requirement(provider=self, provided=x,
                              apis=frozenset((self.api,)))
            self.requirements.add(req)
            x.requirements.add(req)

        for xml_cmd in _etree_iterfind(xml_feature, './require/command'):
            cmd = command_map[xml_cmd.get('name')]
            link(cmd)
        for xml_enum in _etree_iterfind(xml_feature, './require/enum'):
            enum = enum_map[xml_enum.get('name')]
            link(enum)


class Extension(object):
    """An <extension> XML element.

    Attributes:
        name: The XML element's 'name' attribute.

        supported_apis: The set of api strings in the XML element's 'supported'
        attribute. For example, set('gl', 'glcore').

        vendor_namespace: For example, "AMD". May be None.

        requirements: A collection of Requirement for each Command and Enum
        this Extension requires.
    """

    __VENDOR_REGEX = re.compile(r'^GL_(?P<vendor_namespace>[A-Z]+)_')
    RATIFIED_NAMESPACES = ('KHR', 'ARB', 'OES')

    def __init__(self, xml_extension, command_map, enum_map):
        """Parse an <extension> element."""

        # Example <extension> element:
        #     <extension name="GL_ARB_ES2_compatibility" supported="gl|glcore">
        #         <require>
        #             <enum name="GL_FIXED"/>
        #             <enum name="GL_IMPLEMENTATION_COLOR_READ_TYPE"/>
        #             ...
        #             <command name="glReleaseShaderCompiler"/>
        #             <command name="glShaderBinary"/>
        #             ...
        #         </require>
        #     </extension>

        assert(xml_extension.tag == 'extension')

        self.name = xml_extension.get('name')

        self.vendor_namespace = None
        match = Extension.__VENDOR_REGEX.match(self.name)
        if match is not None:
            groups = match.groupdict()
            self.vendor_namespace = groups.get('vendor_namespace', None)

        self.supported_apis = xml_extension.get('supported').split('|')
        self.supported_apis = frozenset(self.supported_apis)
        assert(self.supported_apis <= VALID_APIS)

        self.__parse_requirements(xml_extension, command_map, enum_map)

    def __cmp__(self, other):
        if self is other:
            return 0

        # Sort features before extensions.
        if isinstance(other, Feature):
            return 1

        # Sort ratified before unratified.
        diff = cmp(other.is_ratified, self.is_ratified)
        if diff != 0:
            return diff

        # Sort EXT before others.
        diff = cmp(other.vendor_namespace == 'EXT',
                   self.vendor_namespace == 'EXT')
        if diff != 0:
            return diff

        return cmp(self.name, other.name)

    def __repr__(self):
        templ = '{self.__class__.__name__}(name={self.name!r})'
        return templ.format(self=self)

    @property
    def is_ratified(self):
        """True if the vendor namespace is one that traditionally requires
        ratification by Khronos.
        """
        return self.vendor_namespace in self.RATIFIED_NAMESPACES

    def __parse_requirements(self, xml_extension, command_map, enum_map):
        """For each <command> and <enum> under a <require>, create
        a Requirement that links this Extension to a Command or Enum.
        """
        self.requirements = set()

        def link(xml_require, x):
            api = xml_require.get('api', None)
            if api is not None:
                assert(api in self.supported_apis)
                apis = frozenset((api,))
            else:
                apis = frozenset(self.supported_apis)

            req = Requirement(provider=self, provided=x, apis=apis)
            self.requirements.add(req)
            x.requirements.add(req)

        for xml_req in _etree_iterfind(xml_extension, './require'):
            for xml_cmd in _etree_iterfind(xml_req, './command'):
                cmd = command_map[xml_cmd.get('name')]
                link(xml_req, cmd)
            for xml_enum in _etree_iterfind(xml_req, './enum'):
                enum = enum_map[xml_enum.get('name')]
                link(xml_req, enum)


class Requirement(object):
    """A <require> XML element, which links a provider (Feature or Extension)
    to a provided (Command or Enum) for a set of apis.
    """

    def __init__(self, provider, provided, apis):
        self.provider = provider
        self.provided = provided
        self.apis = frozenset(apis)

        def choose_if(condition, obj):
            if condition:
                return obj
            else:
                return None

        self.has_feature = isinstance(provider, Feature)
        self.has_extension = isinstance(provider, Extension)
        self.has_command = isinstance(provided, Command)
        self.has_enum = isinstance(provided, Enum)

        self.feature = choose_if(self.has_feature, self.provider)
        self.extension = choose_if(self.has_extension, self.provider)
        self.command = choose_if(self.has_command, self.provided)
        self.enum = choose_if(self.has_enum, self.provided)

        assert(self.has_feature + self.has_extension == 1)
        assert(self.has_command + self.has_enum == 1)
        assert(self.apis <= VALID_APIS)

        _log_debug('created {0}'.format(self))

    def __cmp__(self, other):
        """Sort by 'provider', then by 'provided'."""

        diff = cmp(self.provider, other.provider)
        if diff != 0:
            return diff

        diff = cmp(self.provided, other.provided)
        if diff != 0:
            return diff

        return 0

    def __repr__(self):
        templ = ('{self.__class__.__name__}'
                 '(provider={self.provider.name!r},'
                 ' provided={self.provided.name!r},'
                 ' apis={api_tuple})')
        return templ.format(self=self, api_tuple=tuple(self.apis))


class CommandParam(object):
    """A <param> XML element at path command/param.

    Attributes:
        name
        c_type
    """

    __PARAM_NAME_FIXES = {'near': 'hither', 'far': 'yon'}

    def __init__(self, xml_param, log=None):
        """Parse a <param> element."""

        # Example <param> elements:
        #
        #    <param>const <ptype>GLchar</ptype> *<name>name</name></param>
        #    <param len="1"><ptype>GLsizei</ptype> *<name>length</name></param>
        #    <param len="bufSize"><ptype>GLint</ptype> *<name>values</name></param>
        #    <param><ptype>GLenum</ptype> <name>shadertype</name></param>
        #    <param group="sync"><ptype>GLsync</ptype> <name>sync</name></param>

        assert(xml_param.tag == 'param')

        self.name = xml_param.find('./name').text

        # Rename the parameter if its name is a reserved keyword in MSVC.
        self.name = self.__PARAM_NAME_FIXES.get(self.name, self.name)

        # Pare the C type.
        c_type_text = list(xml_param.itertext())
        c_type_text.pop(-1)  # Pop off the text from the <name> subelement.
        c_type_text = (t.strip() for t in c_type_text)
        self.c_type = ' '.join(c_type_text).strip()

        _log_debug('parsed {0}'.format(self))

    def __repr__(self):
        templ = ('{self.__class__.__name__}'
                 '(name={self.name!r}, type={self.c_type!r})')
        return templ.format(self=self)


class Command(object):
    """A <command> XML element.

    Attributes:
        name: The XML element's 'name' attribute, which is also the function
        name.

        c_return_type: For example, "void *".

        alias: The XML element's 'alias' element. May be None.

        param_list: List of that contains a CommandParam for each <param>
        subelement.

        requirements: A collection of each Requirement that exposes this
        Command.
    """

    def __init__(self, xml_command):
        """Parse a <command> element."""

        # Example <command> element:
        #
        #    <command>
        #        <proto>void <name>glTexSubImage2D</name></proto>
        #        <param group="TextureTarget"><ptype>GLenum</ptype> <name>target</name></param>
        #        <param group="CheckedInt32"><ptype>GLint</ptype> <name>level</name></param>
        #        <param group="CheckedInt32"><ptype>GLint</ptype> <name>xoffset</name></param>
        #        <param group="CheckedInt32"><ptype>GLint</ptype> <name>yoffset</name></param>
        #        <param><ptype>GLsizei</ptype> <name>width</name></param>
        #        <param><ptype>GLsizei</ptype> <name>height</name></param>
        #        <param group="PixelFormat"><ptype>GLenum</ptype> <name>format</name></param>
        #        <param group="PixelType"><ptype>GLenum</ptype> <name>type</name></param>
        #        <param len="COMPSIZE(format,type,width,height)">const void *<name>pixels</name></param>
        #        <glx type="render" opcode="4100"/>
        #        <glx type="render" opcode="332" name="glTexSubImage2DPBO" comment="PBO protocol"/>
        #    </command>
        #

        assert(xml_command.tag == 'command')
        xml_proto = xml_command.find('./proto')
        self.name = xml_proto.find('./name').text
        _log_debug('start parsing Command(name={0!r})'.format(self.name))

        self.requirements = set()
        self.__vendor_namespace = None

        # Parse the return type from the <proto> element.
        #
        # Example of a difficult <proto> element:
        #     <proto group="String">const <ptype>GLubyte</ptype> *<name>glGetStringi</name></proto>
        c_return_type_text = list(xml_proto.itertext())
        c_return_type_text.pop(-1)  # Pop off the text from the <name> subelement.
        c_return_type_text = (t.strip() for t in c_return_type_text)
        self.c_return_type = ' '.join(c_return_type_text).strip()

        # Parse alias info, if any.
        xml_alias = xml_command.find('./alias')
        if xml_alias is None:
            self.alias = None
        else:
            self.alias = xml_alias.get('name')

        self.param_list = [
            CommandParam(xml_param)
            for xml_param in _etree_iterfind(xml_command, './param')
        ]

        _log_debug(('parsed {self.__class__.__name__}('
                    'name={self.name!r}, '
                    'alias={self.alias!r}, '
                    'prototype={self.c_prototype!r})').format(self=self))

    def __cmp__(self, other):
        return cmp(self.name, other.name)

    def __repr__(self):
        templ = '{self.__class__.__name__}({self.name!r})'
        return templ.format(self=self)

    @property
    def vendor_namespace(self):
        if self.__vendor_namespace is None:
            for req in self.__requirements:
                ext = req.extension
                if ext is None:
                    continue

                if ext.vendor_namespace is None:
                    continue

                if self.name.endswith('_' + ext.vendor_namespace):
                    self.__vendor_namespace = ext.vendor_namespace

        return self.__vendor_namespace

    @property
    def c_prototype(self):
        """For example, "void glAccum(GLenum o, GLfloat value)"."""
        return '{self.c_return_type} {self.name}({self.c_named_param_list})'.format(self=self)

    @property
    def c_funcptr_typedef(self):
        """For example, "PFNGLACCUMROC" for glAccum."""
        return 'PFN{0}PROC'.format(self.name).upper()

    @property
    def c_named_param_list(self):
        """For example, "GLenum op, GLfloat value" for glAccum."""
        return ', '.join(
            '{param.c_type} {param.name}'.format(param=param)
            for param in self.param_list
        )

    @property
    def c_unnamed_param_list(self):
        """For example, "GLenum, GLfloat" for glAccum."""
        return ', '.join(
            param.c_type
            for param in self.param_list
        )

    @property
    def c_untyped_param_list(self):
        """For example, "op, value" for glAccum."""
        return ', '.join(
            param.name
            for param in self.param_list
        )


class CommandAliasSet(ImmutableOrderedKeyedSet):

    def __init__(self, commands):
        ImmutableOrderedKeyedSet.__init__(self, key='name',
                                          elems=sorted(commands))
        self.__primary_command = None
        self.__requirements = None

    def __cmp__(self, other):
        return cmp(self.name, other.name)

    def __repr__(self):
        templ = '{self.__class__.__name__}({self.name!r})'
        return templ.format(self=self)

    @property
    def name(self):
        return self.primary_command.name

    @property
    def primary_command(self):
        """The set's first command when sorted by name."""
        for command in self:
            return command

    @property
    def requirements(self):
        """A sorted iterator over each Requirement that exposes this
        CommandAliasSet.
        """
        if self.__requirements is None:
            self.__requirements = sorted(
                req
                for command in self
                for req in command.requirements
            )
            _log_debug('{0} sorted requirements: {1}'.format(
                self, self.__requirements))

        return iter(self.__requirements)


class CommandAliasMap(object):

    def __init__(self):
        self.__map = dict()
        self.__sorted_unique_values = None

    def __getitem__(self, command_name):
        return self.__map[command_name]

    def __iter__(self):
        """A sorted iterator over the map's unique CommandAliasSet values."""
        if self.__sorted_unique_values is None:
            self.__sorted_unique_values = sorted(set(self.__map.itervalues()))

        return iter(self.__sorted_unique_values)

    def get(self, command_name, default):
        return self.__map.get(command_name, default)

    def add(self, command):
        assert(isinstance(command, Command))
        _log_debug('adding command {0!r} to CommandAliasMap'.format(command.name))

        name = command.name
        name_set = self.get(name, None)
        assert(self.__is_set_mapping_complete(name_set))

        alias = command.alias
        alias_set = self.get(command.alias, None)
        assert(self.__is_set_mapping_complete(alias_set))

        if name_set is alias_set and name_set is not None:
            return

        # After modifying the contained alias sets, the mapping will no longer
        # be sorted.
        self.__sorted_unique_values = None

        new_set_elems = set((command,))
        if name_set is not None:
            new_set_elems.update(name_set)
        if alias_set is not None:
            new_set_elems.update(alias_set)

        new_set = CommandAliasSet(new_set_elems)
        for other_command in new_set:
            self.__map[other_command.name] = new_set
            if other_command.alias is not None:
                self.__map[other_command.alias] = new_set

    def __is_set_mapping_complete(self, alias_set):
        if alias_set is None:
            return True

        for command in alias_set:
            if self[command.name] is not alias_set:
                return False
            if command.alias is None:
                continue
            if self[command.alias] is not alias_set:
                return False

        return True


class EnumGroup(object):
    """An <enums> element at path registry/enums.

    Attributes:
        name: The XML element's 'group' attribute. If the XML does not define
        'group', then this class invents one.

        type: The XML element's 'type' attribute. If the XML does not define
        'type', then this class invents one.

        start, end: The XML element's 'start' and 'end' attributes. Each may be
        None.

        enums: An OrderedKeyedSet of Enum that contains each <enum> subelement
        in this <enums>.
    """

    # Each EnumGroup belongs to exactly one member of EnumGroup.TYPES.
    #
    # Some members in EnumGroup.TYPES are invented and not present in gl.xml.
    # The only enum type defined explicitly in gl.xml is "bitmask", which
    # occurs as <enums type="bitmask">.  However, in gl.xml each block of
    # non-bitmask enums is introduced by a comment that describes the block's
    # "type", even if the <enums> tag lacks a 'type' attribute. (Thanks,
    # Khronos, for encoding data in XML comments rather than the XML itself).
    # EnumGroup.TYPES lists such implicit comment-only types, with invented
    # names, alongside the types explicitly defined by <enums type=>.
    TYPES = (
        # Type 'default_namespace' is self-explanatory. It indicates the large
        # set of enums from 0x0000 to 0xffff that includes, for example,
        # GL_POINTS and GL_TEXTURE_2D.
        'default_namespace',

        # Type 'bitmask' is self-explanatory.
        'bitmask',

        # Type 'small_index' indicates a small namespace of non-bitmask enums.
        # As of Khronos revision 26792, 'small_index' groups generally contain
        # small numbers used for indexed access.
        'small_index',

        # Type 'special' is used only for the group named "SpecialNumbers". The
        # group contains enums such as GL_FALSE, GL_ZERO, and GL_INVALID_INDEX.
        'special',
    )

    def __init__(self, xml_enums):
        """Parse an <enums> element."""

        # Example of a bitmask group:
        #
        #     <enums namespace="GL" group="SyncObjectMask" type="bitmask">
        #         <enum value="0x00000001" name="GL_SYNC_FLUSH_COMMANDS_BIT"/>
        #         <enum value="0x00000001" name="GL_SYNC_FLUSH_COMMANDS_BIT_APPLE"/>
        #     </enums>
        #
        # Example of a group that resides in OpenGL's default enum namespace:
        #
        #     <enums namespace="GL" start="0x0000" end="0x7FFF" vendor="ARB" comment="...">
        #         <enum value="0x0000" name="GL_POINTS"/>
        #         <enum value="0x0001" name="GL_LINES"/>
        #         <enum value="0x0002" name="GL_LINE_LOOP"/>
        #         ...
        #     </enums>
        #
        # Example of a non-bitmask group that resides outside OpenGL's default
        # enum namespace:
        #
        #     <enums namespace="GL" group="PathRenderingTokenNV" vendor="NV">
        #         <enum value="0x00" name="GL_CLOSE_PATH_NV"/>
        #         <enum value="0x02" name="GL_MOVE_TO_NV"/>
        #         <enum value="0x03" name="GL_RELATIVE_MOVE_TO_NV"/>
        #         ...
        #     </enums>
        #

        self.name = xml_enums.get('group', None)
        _log_debug('start parsing {0}'.format(self))

        self.type = xml_enums.get('type', None)
        self.start = xml_enums.get('start', None)
        self.end = xml_enums.get('end', None)
        self.enums = []

        self.__invent_name_and_type()
        assert(self.name is not None)
        assert(self.type in self.TYPES)

        _log_debug('start parsing <enum> subelements of {0}'.format(self))
        self.enums = OrderedKeyedSet(key='name')
        for xml_enum in _etree_iterfind(xml_enums, './enum'):
            self.enums.add(Enum(self, xml_enum))
        _log_debug('parsed {0}'.format(self))

    def __repr__(self):
        templ = '{self.__class__.__name__}({self.name!r})'
        return templ.format(self=self)

    def __invent_name_and_type(self):
        """If the XML didn't define a name or type, invent one."""
        if self.name is None:
            assert(self.type is None)
            assert(self.start is not None)
            assert(self.end is not None)
            self.name = 'range_{self.start}_{self.end}'.format(self=self)
            self.type = 'default_namespace'
        elif self.type is None:
            self.type = 'small_index'
        elif self.name == 'SpecialNumbers':
            assert(self.type is None)
            self.type = 'special'


class Enum(object):
    """An <enum> XML element.

    Attributes:
        name, api: The XML element's 'name' and 'api' attributes.

        str_value, c_num_literal: Equivalent attributes. The XML element's
        'value' attribute.

        num_value: The long integer for str_value.

        requirements: A collection of each Requirement that exposes this Enum.
    """

    def __init__(self, enum_group, xml_enum):
        """Parse an <enum> tag located at path registry/enums/enum."""

        # Example <enum> element:
        #     <enum value="0x0000" name="GL_POINTS"/>

        assert(isinstance(enum_group, EnumGroup))
        assert(xml_enum.tag == 'enum')

        self.requirements = set()
        self.__vendor_namespace = None

        self.group = enum_group
        self.name = xml_enum.get('name')
        self.api = xml_enum.get('api')
        self.str_value = xml_enum.get('value')
        self.c_num_literal = self.str_value

        if '0x' in self.str_value.lower():
            base = 16
        else:
            base = 10
        self.num_value = long(self.str_value, base)

        _log_debug('parsed {0}'.format(self))

    def __repr__(self):
        templ = ('{self.__class__.__name__}'
                 '(name={self.name!r},'
                 ' value={self.str_value!r})')
        return templ.format(self=self)

    @property
    def vendor_namespace(self):
        if self.__vendor_namespace is None:
            for req in self.requirements:
                ext = req.extension
                if ext is None:
                    continue

                if ext.vendor_namespace is None:
                    continue

                if self.name.endswith('_' + ext.vendor_namespace):
                    self.__vendor_namespace = ext.vendor_namespace

        return self.__vendor_namespace
