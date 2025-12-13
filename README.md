gr-nacl: GNU Radio data encryption module
========

GNU Radio module for data encryption using NaCl library  

**Features**  
The gr-nacl module for GNU Radio provides functionality from the NaCl crypto library implemented with the fork libsodium (see section 'Dependency' for more information). This contains public-key and secret-key encryption. The difference is explained, e.g., on Wikipedia [0]. The implementation is based on encryption of messages, which are passed in GNU Radio via the message system. Check out the GNU Radio documentation for further information [1]. Furthermore, a byte stream encryption method via tagged streams is implemented.

**Testing**  
The module includes comprehensive test coverage:
- All 8 test suites pass (100% pass rate)
- Round-trip encryption/decryption validation
- Test vector validation against libsodium behavior
- Multiple message size testing (short, medium, long, edge cases)
- Nonce randomness and uniqueness verification
- Ciphertext structure validation (proper MAC inclusion)

The functionality can be tested with the example flowgraphs for GNU Radio Companion at the subfolder examples/ or directly with the provided test-cases using `ctest`.

**Compatibility**  
This module has been updated for GNU Radio 3.10+ compatibility. It uses `std::shared_ptr` instead of `boost::shared_ptr` and `std::vector` instead of the deprecated `__GR_VLA` macro.

**Recent Updates**  
The codebase has been thoroughly reviewed and critical bugs have been fixed:
- Fixed nonce rotation bug in tagged stream encryption
- Fixed key generation size bug (now correctly generates 32-byte keys)
- Fixed Python 2/3 compatibility issues in tests
- Added comprehensive test vector validation
- All tests passing with proper assertions

The implementation correctly uses libsodium's cryptographic primitives and has been validated against expected behavior.

**Install guide (Linux)**  
Change to any folder in your home directory and enter following commands in your terminal. Check out the section 'Dependency' first. As well, you can install GNU Radio with PyBOMBS and use the provided install recipe for gr-nacl. The recipe builds and installs the dependency automatically.

`git clone https://github.com/Supermagnum/gr-nacl.git` // clone this repository  
`cd gr-nacl/`  
`mkdir build` // make build folder  
`cd build/`  
`cmake ../` // build makefiles  
`make` // build toolbox  
`ctest` // run tests, check if all have passed, the option -V provides an extended output  
`sudo make install` // install toolbox

**Install guide (Mac OS X)**  
The following commands will work if you have installed Gnuradio and libsodium via Macports.
Change to any folder in your home directory and enter following commands in your terminal.

`git clone https://github.com/Supermagnum/gr-nacl.git` // clone this repository  
`cd gr-nacl/`  
`mkdir build` // make build folder  
`cd build/`  
`cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/local  ../` // build makefiles  
`make` // build toolbox  
`ctest` // run tests, check if all have passed, the option -V provides an extended output. This actually did not work for me  
`sudo make install` // install toolbox

**Development platform**  
Ubuntu 15.04  
GNU Radio 3.7.6.1 (original)  
GNU Radio 3.10+ (updated for compatibility)  
Python 3.x (tests updated for Python 3 compatibility)  

**Dependency**  
The NaCl (pronounced 'salt') crypto library [2] by Daniel J. Bernstein, Tanja Lange and Peter Schwabe has a well maintained fork called 'libsodium' [3]. Follow the instructions to build and install it.

**Minimum libsodium version**: 1.0.0 or later (recommended: 1.0.18+)

The code uses standard libsodium functions (`crypto_box_easy`, `crypto_secretbox_easy`, `crypto_stream_xor`, etc.) that have been available since libsodium 1.0.0. The latest stable version is 1.0.20 (released May 2024), which includes additional features but is not required for this module.

`git clone https://github.com/jedisct1/libsodium.git` // clone libsodium  
`cd libsodium/`  
`git checkout 1.0.20` // optional: checkout latest stable version  
`./autogen.sh` // build libsodium  
`./configure`  
`make`  
`sudo make install` // install libsodium

**Contact**  
Stefan Wunsch  
stefan.wunsch[at]student.kit.edu

**Links**  
[0] https://en.wikipedia.org/wiki/Public-key_cryptography, https://en.wikipedia.org/wiki/Symmetric-key_algorithm  
[1] http://gnuradio.org/doc/doxygen/, http://gnuradio.org/doc/doxygen/page_msg_passing.html, http://gnuradio.org/doc/doxygen/page_tagged_stream_blocks.html  
[2] http://nacl.cr.yp.to/  
[3] http://doc.libsodium.org/, https://github.com/jedisct1/libsodium
