gr-nacl: GNU Radio 4.0 data encryption module
========

**IMPORTANT NOTICE**: This is AI-generated code. The developer has a neurological condition that makes it impossible to use and learn traditional programming. The developer has put in a significant effort. This code might not work properly. Use at your own risk.

GNU Radio **4.0** out-of-tree block library using NaCl-style primitives from **libsodium**. Symmetric vs public-key encryption is explained, for example, on Wikipedia [0].

**Branch:** this is the **`gnuradio4`** branch for **GNU Radio 4.x** only. Build the project under **`gnuradio4/`** with `find_package(gnuradio4)`.

For **GNU Radio 3.x**, use branch **`master`** (separate development line):

```bash
git fetch origin master
git checkout master
```

See `README.md` on branch **`master`** for the GR3 module at the repository root.

---

### Blocks (namespace `gnuradio4::nacl`)

- **CryptTaggedStream** — `crypto_stream_xor` on tagged PDUs; settings `symmetric_key`, `stream_nonce`, `rotate_nonce`, `length_tag_key` (default `packet_len`).
- **EncryptSecret** / **DecryptSecret** — `crypto_secretbox_*`; key file `key_file_path`; message maps use keys `msg_clear`, `nonce`, `msg_encrypted`, `msg_decrypted`.
- **EncryptPublic** / **DecryptPublic** — `crypto_box_*`; `public_key_path`, `secret_key_path`; same ciphertext map layout as secret path.
- **GenerateSymmetricKey** — random `crypto_secretbox` key file.
- **GeneratePublicKeypair** — `crypto_box_keypair` output files.

Post-quantum KEM and SHA-3 blocks are on branch **`master`** (GR3) only, not in this GR4 port yet.

---

### Dependencies

- Installed **GNU Radio 4.0** (`gnuradio4Config.cmake` on `CMAKE_PREFIX_PATH`).
- **libsodium** via pkg-config (`libsodium`). Built and tested against [libsodium 1.0.22-RELEASE](https://github.com/jedisct1/libsodium/releases/tag/1.0.22-RELEASE).
- C++23 toolchain as required by your GNU Radio build (upstream documents **GCC 15+** / **Clang 20+**).

Install libsodium 1.0.22 if needed (in a **libsodium** source tree, not gr-nacl):

```bash
git clone https://github.com/jedisct1/libsodium.git
cd libsodium
git checkout 1.0.22-RELEASE
./autogen.sh
./configure --prefix=/usr/local
make -j"$(nproc)"
sudo make install
sudo ldconfig
```

---

### Configure, build, test, and install

From the repository root on branch **`gnuradio4`**, use a build directory under `gnuradio4/` (ignored by `gnuradio4/.gitignore`):

```bash
git clone https://github.com/Supermagnum/gr-nacl.git
cd gr-nacl
git checkout gnuradio4

export GR4_PREFIX=/opt/gnuradio4-gcc    # adjust to your GR4 install

rm -rf gnuradio4/build
cmake -S gnuradio4 -B gnuradio4/build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=g++-14 \
  -DCMAKE_PREFIX_PATH="${GR4_PREFIX}:/usr/local" \
  -Dgnuradio4_DIR="${GR4_PREFIX}/lib/cmake/gnuradio4" \
  -Dcpr_DIR="${GR4_PREFIX}/lib/cmake/cpr"

cmake --build gnuradio4/build -j"$(nproc)"
ctest --test-dir gnuradio4/build --output-on-failure -j"$(nproc)"
cmake --install gnuradio4/build --prefix "${GR4_PREFIX}"
```

**Toolchain notes**

- Use a C++23 compiler compatible with your GNU Radio 4 binaries. On some distributions, **GCC 13** lacks `<print>` used by upstream headers; **`g++-14`** (or newer) often matches packaged GR4 GCC builds.
- The GR4 subtree will not configure until `find_package(gnuradio4)` succeeds.

Installing with `--prefix` that matches how you consume GR4 installs **`gnuradio4-gr-nacl.pc`** under `${prefix}/lib/pkgconfig` alongside the CMake package **`gr-nacl4`**.

Include in C++: `#include <gnuradio-4.0/nacl.hpp>` (or individual headers under `gnuradio-4.0/nacl/`).

---

**Contact**

Stefan Wunsch  
stefan.wunsch[at]student.kit.edu

**Links**

[0] https://en.wikipedia.org/wiki/Public-key_cryptography, https://en.wikipedia.org/wiki/Symmetric-key_algorithm  
[1] http://gnuradio.org/doc/doxygen/, http://gnuradio.org/doc/doxygen/page_msg_passing.html  
[2] http://nacl.cr.yp.to/  
[3] http://doc.libsodium.org/, https://github.com/jedisct1/libsodium, https://github.com/jedisct1/libsodium/releases/tag/1.0.22-RELEASE
