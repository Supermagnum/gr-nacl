gr-nacl: GNU Radio data encryption module
========

**IMPORTANT NOTICE**: This is AI-generated code. The developer has a neurological condition that makes it impossible to use and learn traditional programming. The developer has put in a significant effort. This code might not work properly. Use at your own risk.

GNU Radio module for data encryption using NaCl-style primitives from **libsodium** (public-key and secret-key constructs, plus streamed PDUs). Symmetric vs public-key encryption is explained, for example, on Wikipedia [0].

This repository provides two build paths:

| Area | Branch | Path | GNU Radio |
|------|--------|------|-----------|
| Original out-of-tree module | `master` | repository root (`cmake` at top level) | **3.10+** |
| Modern C++23 blocks | `gnuradio4` | subdirectory **`gnuradio4/`** only | **4.0** (`gr::Block`, `find_package(gnuradio4)`) |

The **`gnuradio4`** branch keeps the 3.10 tree unchanged and adds the GR4 port under **`gnuradio4/`** (separate CMake project: header-only block library, Boost.UT tests, pkg-config and CMake package export `gr-nacl4` with namespace **`gnuradio4::`** targets such as **`gnuradio4::gr-nacl`**).

### GNU Radio 4.0 submodule (`gnuradio4/` branch, `gnuradio4/` directory)

**Blocks (namespace `gnuradio4::nacl`)**

- **CryptTaggedStream** — `crypto_stream_xor` on tagged PDUs; settings `symmetric_key`, `stream_nonce`, `rotate_nonce`, `length_tag_key` (default `packet_len`).
- **EncryptSecret** / **DecryptSecret** — `crypto_secretbox_*`; key file `key_file_path`; message maps use keys `msg_clear`, `nonce`, `msg_encrypted`, `msg_decrypted`.
- **EncryptPublic** / **DecryptPublic** — `crypto_box_*`; `public_key_path`, `secret_key_path`; same ciphertext map layout as secret path.
- **GenerateSymmetricKey** — random `crypto_secretbox` key file.
- **GeneratePublicKeypair** — `crypto_box_keypair` output files.

**Dependencies**

- Installed **GNU Radio 4.0** (`gnuradio4Config.cmake` on `CMAKE_PREFIX_PATH`).
- **libsodium** (pkg-config `libsodium`).
- C++23 toolchain as required by your GNU Radio build (upstream documents **GCC 15+** / **Clang 20+**).

**Configure, build, and test (GR4)**

From the repository root on branch **`gnuradio4`**, use a dedicated build directory (ignored by `gnuradio4/.gitignore` if you use `build/` or `build-*` under `gnuradio4/`). Point CMake at the **same prefix** where GNU Radio 4 is installed.

```bash
git checkout gnuradio4

# Example: GR4 installed under /opt/gnuradio4-gcc (adjust to your layout).
export GR4_PREFIX=/opt/gnuradio4-gcc

# Optional: if configure fails to find CPR (pulled in by gnuradio4), set explicitly:
# export CPR_DIR="${GR4_PREFIX}/lib/cmake/cpr"

rm -rf gnuradio4/build
cmake -S gnuradio4 -B gnuradio4/build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=g++-14 \
  -DCMAKE_PREFIX_PATH="${GR4_PREFIX}" \
  -Dgnuradio4_DIR="${GR4_PREFIX}/lib/cmake/gnuradio4" \
  -Dcpr_DIR="${GR4_PREFIX}/lib/cmake/cpr"

cmake --build gnuradio4/build -j"$(nproc)"
ctest --test-dir gnuradio4/build --output-on-failure -j"$(nproc)"
```

Install the OOT headers and CMake pkg files into your preferred prefix:

```bash
cmake --install gnuradio4/build --prefix "${GR4_PREFIX}"
# or another CMAKE_INSTALL_PREFIX, e.g. /usr/local
```

**Toolchain notes**

- Use a C++23 compiler compatible with **your** GNU Radio 4 binaries. On some distributions, **GCC 13** lacks `<print>` used by upstream headers; **`g++-14`** (or newer) often matches packaged GR4 GCC builds even when upstream docs mention GCC 15+.
- The GR4 subtree will not configure until `find_package(gnuradio4)` succeeds (**`gnuradio4Targets.cmake`** and related files must exist under `${GR4_PREFIX}/lib/cmake/gnuradio4`).

Installing with `--prefix` that matches how you consume GR4 installs **`gnuradio4-gr-nacl.pc`** under `${prefix}/lib/pkgconfig` alongside the CMake package.

Include in C++: `#include <gnuradio-4.0/nacl.hpp>` (or individual headers under `gnuradio-4.0/nacl/`).

---

### GNU Radio 3.10 module (original layout, `master`)

**Features**  
The gr-nacl module for GNU Radio 3.x exposes libsodium via **message ports** and **tagged stream** operations. See GNU Radio documentation on message passing and tagged streams [1].

**Testing (GR3)**  
Example flowgraphs live under `examples/`. From a top-level build directory:

```bash
ctest    # use -V for verbose output
```

If `qa_test_vectors` or `qa_libsodium_vectors` fail with “Permission denied” on the wrapper scripts, ensure the Python QA files under `python/` are executable (`chmod +x python/qa_*.py`) and rebuild or re-run `ctest`.

**Compatibility**  
GNU Radio 3.10+ with `std::shared_ptr` instead of `boost::shared_ptr` and `std::vector` instead of the deprecated `__GR_VLA` macro.

**Install guide (Linux, GR3)**

`git clone https://github.com/Supermagnum/gr-nacl.git`  
`cd gr-nacl/`  
`mkdir build && cd build/`  
`cmake ../`  
`make`  
`ctest`  
`sudo make install`

**Install guide (Mac OS X, GR3)**  
With GNU Radio and libsodium from MacPorts, for example:

`cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/local ../`  
then `make`, `ctest`, `sudo make install`.

**Development platform (historical)**  
Ubuntu 15.04, GNU Radio 3.7.6.1 (original), GNU Radio 3.10+ (updated), Python 3.x.

**Dependency**

The NaCl crypto library [2] has a maintained fork, **libsodium** [3].

**Minimum libsodium version**: 1.0.0 or later (recommended: 1.0.18+). The code uses `crypto_box_easy`, `crypto_secretbox_easy`, `crypto_stream_xor`, and related APIs.

Example build from source:

`git clone https://github.com/jedisct1/libsodium.git`  
`cd libsodium/`  
`git checkout 1.0.20`  # optional  
`./autogen.sh && ./configure && make && sudo make install`

**Contact**  
Stefan Wunsch  
stefan.wunsch[at]student.kit.edu

**Links**  
[0] https://en.wikipedia.org/wiki/Public-key_cryptography, https://en.wikipedia.org/wiki/Symmetric-key_algorithm  
[1] http://gnuradio.org/doc/doxygen/, http://gnuradio.org/doc/doxygen/page_msg_passing.html, http://gnuradio.org/doc/doxygen/page_tagged_stream_blocks.html  
[2] http://nacl.cr.yp.to/  
[3] http://doc.libsodium.org/, https://github.com/jedisct1/libsodium
