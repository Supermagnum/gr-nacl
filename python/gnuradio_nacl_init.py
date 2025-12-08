# -*- coding: utf-8 -*-
"""
GNU Radio nacl module initialization

This module provides Python bindings for gr-nacl blocks.
"""

# Import GNU Radio base classes first to ensure types are registered
# This must happen before importing the pybind11 module
try:
    from gnuradio import gr
except ImportError:
    pass  # Will fail later if GNU Radio is not available

try:
    from nacl_python import *
except ImportError:
    import sys
    import os
    # Try to find the module in common locations
    possible_paths = [
        '/usr/local/lib/python3/dist-packages',
        '/usr/lib/python3/dist-packages',
    ]
    for path in possible_paths:
        if os.path.exists(os.path.join(path, 'nacl_python.so')):
            sys.path.insert(0, path)
            break
    try:
        from nacl_python import *
    except ImportError:
        raise ImportError(
            "Failed to import nacl_python. "
            "Make sure gr-nacl is built and installed correctly."
        )

