try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

import os
import platform
import sys
from distutils.core import Extension

import numpy as np

if platform.system() == 'Windows':
    extra_compile_args = []
    library_dirs = ['../LeddarConfigurator4/x64/Release']
    libraries = ['LeddarConfigurator4']
else:
    extra_compile_args = ['-std=c++11', '-Wno-write-strings', '-Wno-unused-function', '-Wno-unused-variable']
    library_dirs = ['../release']
    libraries = ['LeddarConfigurator4','usb-1.0']
    
# define 'LEDDAR_DEBUG' using e.g. 
# user@machine:~/LC4/main/src/LeddarPy$ (export LEDDAR_DEBUG=1; python setup.py install  --user)
print("LEDDAR_DEBUG = " + str(os.environ.get('LEDDAR_DEBUG', 0)))
debug = ['-g', '-DPy_DEBUG'] if os.environ.get('LEDDAR_DEBUG', 0) == "1" else []

# to debug, you can use 
# gdbserver localhost:1234 python-dbg my_script_that_imports_leddar.py

# then, from another window:
# gdb --args python-dbg my_script_that_imports_leddar.py
# (gdb) target remote localhost:1234
# (gdb) break LeddarPy.cpp:initleddar //FOR EXAMPLE

# you can also connect from Eclipse
# (you may want to add LC4/main/src as a "Makefile Project with Existing Code" and use a 'make' command inspired from the one found in ../build.sh)
# C/C++ Remote Application
# C/C++ Application : /usr/bin/python-dbg
# "Select other" link -> "GDB (DSF) Manual Remote Debugguing Launcher"
# "Debugger" tab -> "Connection" sub-tab -> fill in connection info (e.g. localhost:1234)
# Add a breakpoint! (e.g. in LeddarPy.cpp::initleddar())
# Run!

module1 = Extension('leddar',
                    include_dirs = ['./','../Leddar','../LeddarTech','../shared/','../shared/comm','../../libs/RapidJson', np.get_include()],
                    libraries = libraries,
                    library_dirs = library_dirs,
                    sources = ['./LeddarPy.cpp', './LeddarPyDevice.cpp', './Connecters.cpp'],
                    extra_compile_args= extra_compile_args + debug)

setup (name = 'leddar',
       version = '1.0',
       description = 'Python wrapper for LeddarTech SDK.',
       author = 'David Levy, Maxime Lemonnier',
       author_email = 'maxime.lemonnier@leddartech.com',
       ext_modules = [module1])
