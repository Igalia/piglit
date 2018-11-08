# coding=utf-8
import importlib
import os
import sys

sys.path.insert(0, os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', 'framework')))

importlib.import_module('compat')
