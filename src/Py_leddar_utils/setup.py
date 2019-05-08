#!/usr/bin/env python

import os
import sys
try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

setup(name="leddar_utils",

      version = '0.1',
      description='Python support module for "leddar" module',
      author='Maxime Lemonnier',
      author_email='maxime.lemonnier@leddartech.com',
      packages=['leddar_utils'],
      install_requires = ['future', 'transforms3d', 'matplotlib', 'numpy'], #output from pipreqs ./leddar_utils
    )
