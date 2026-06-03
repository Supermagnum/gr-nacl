gr-nacl: GNU Radio data encryption module
========

**IMPORTANT NOTICE**: This is AI-generated code. The developer has a neurological condition that makes it impossible to use and learn traditional programming. The developer has put in a significant effort. This code might not work properly. Use at your own risk.

GNU Radio module for data encryption using NaCl-style primitives from **libsodium** (public-key and secret-key constructs, streamed PDUs, post-quantum KEM, and SHA-3 hashing). Symmetric vs public-key encryption is explained, for example, on Wikipedia [0].

### libsodium requirement

gr-nacl is compiled and built against **[libsodium 1.0.22-RELEASE](https://github.com/jedisct1/libsodium/releases/tag/1.0.22-RELEASE)**. CMake enforces a minimum of 1.0.22 via `find_package(Sodium REQUIRED)` and the imported target `sodium::sodium`.

That release adds APIs used by this project:

- **X-Wing KEM** — hybrid ML-KEM768 + X25519 via `crypto_kem_*()` (recommended post-quantum key encapsulation in libsodium).
- **SHA-3** — `crypto_hash_sha3256_*()` and `crypto_hash_sha3512_*()` with one-shot and streaming forms.

Install **libsodium** 1.0.22 before building gr-nacl (run these commands in a
**libsodium** source tree, not in the gr-nacl directory — gr-nacl has no
`autogen.sh`):

```bash
git clone https://github.com/jedisct1/libsodium.git
cd libsodium
git checkout 1.0.22-RELEASE
./autogen.sh
./configure --prefix=/usr/local
make -j"$(nproc)"
sudo make install
sudo ldconfig    # Linux: refresh shared-library cache if needed
```

If libsodium is already installed (for example under `/usr/local` from a prior
build), skip the steps above and build gr-nacl with CMake only (see below).

Verify: `pkg-config --modversion libsodium` should print `1.0.22` or later.

---

### Repository layout and branches

| Area | Branch | Path | GNU Radio |
|------|--------|------|-----------|
| Out-of-tree module (GR3) | `master` or `gnuradio4` | repository root | **3.10+** |
| Modern C++23 blocks | `gnuradio4` | subdirectory `gnuradio4/` | **4.0** |

The **`gnuradio4`** branch contains the full GR3 module at the repository root **and** the GR4 port under `gnuradio4/` (separate CMake project: header-only block library, Boost.UT tests, pkg-config and CMake package export `gr-nacl4` with namespace **`gnuradio4::`**).

Use **`master`** for GR3-only work. Use **`gnuradio4`** when you need both GR3 (including KEM and SHA-3 blocks) and the GR4 subtree.

---

### GNU Radio 3.10 module (repository root)

**Features**

The GR3 module exposes libsodium via **message ports** and **tagged stream** blocks. GRC block definitions live in `grc/` (legacy `.xml` plus YAML `.block.yml` for newer blocks). See GNU Radio documentation on message passing and tagged streams [1].

**Blocks (namespace `gr::nacl`)**

| Block | libsodium API | Notes |
|-------|---------------|-------|
| `generate_key` | random key file | Symmetric key generation |
| `generate_keypair` | `crypto_box_keypair` | Public-key keypair files |
| `encrypt_secret` / `decrypt_secret` | `crypto_secretbox_*` | Message PDUs |
| `encrypt_public` / `decrypt_public` | `crypto_box_*` | Message PDUs |
| `crypt_tagged_stream` | `crypto_stream_xor` | Tagged byte streams |
| `generate_kem_keypair` | `crypto_kem_keypair` | X-Wing KEM key files (**1.0.22+**) |
| `encrypt_kem` / `decrypt_kem` | `crypto_kem_*` + `crypto_secretbox_*` | KEM + secretbox PDUs (**1.0.22+**) |
| `hash_sha3` | `crypto_hash_sha3256_*` / `crypto_hash_sha3512_*` | SHA3-256 or SHA3-512 digest PDUs (**1.0.22+**) |

**KEM message PDU layout**

- **encrypt_kem** input: `msg_clear` on port `Msg clear`.
- **encrypt_kem** output: `kem_ciphertext`, `nonce`, `msg_encrypted` on port `Msg encrypted`.
- **decrypt_kem** reverses that layout and publishes `msg_decrypted` on port `Msg decrypted`.

**Python usage**

```python
from gnuradio import nacl

# Existing blocks
nacl.encrypt_public(pk_file, sk_file)
nacl.generate_kem_keypair(sk_file, pk_file)   # 1.0.22+
nacl.encrypt_kem(pk_file)                     # 1.0.22+
nacl.decrypt_kem(sk_file)                     # 1.0.22+
nacl.hash_sha3(sha3_512=False)                # 1.0.22+
```

**Configure, build, test, and install (GR3)**

From the **gr-nacl** repository root (no `autogen.sh` here — use `cmake`):

```bash
git clone https://github.com/Supermagnum/gr-nacl.git
cd gr-nacl
git checkout gnuradio4    # or master for GR3-only tree

mkdir -p build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/usr/local
cmake --build . -j"$(nproc)"
ctest --output-on-failure
sudo cmake --install .
```

CMake **3.5+** is required. If libsodium is installed under `/usr/local`, ensure `PKG_CONFIG_PATH` and `CMAKE_PREFIX_PATH` include that prefix so `find_package(Sodium REQUIRED)` locates version 1.0.22.

**Mac OS X (GR3)**

With GNU Radio and libsodium from MacPorts, for example:

```bash
cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/local ..
make && ctest && sudo make install
```

**Testing (GR3)**

Example flowgraphs live under `examples/`. From the build directory:

```bash
ctest    # use -V for verbose output
```

If `qa_test_vectors` or `qa_libsodium_vectors` fail with "Permission denied" on wrapper scripts, run `chmod +x python/qa_*.py` and re-run `ctest`.

**Compatibility**

GNU Radio 3.10+ with `std::shared_ptr` instead of `boost::shared_ptr` and `std::vector` instead of the deprecated `__GR_VLA` macro.

---

### GNU Radio 4.0 submodule (`gnuradio4/` directory)

**Blocks (namespace `gnuradio4::nacl`)**

- **CryptTaggedStream** — `crypto_stream_xor` on tagged PDUs; settings `symmetric_key`, `stream_nonce`, `rotate_nonce`, `length_tag_key` (default `packet_len`).
- **EncryptSecret** / **DecryptSecret** — `crypto_secretbox_*`; key file `key_file_path`; message maps use keys `msg_clear`, `nonce`, `msg_encrypted`, `msg_decrypted`.
- **EncryptPublic** / **DecryptPublic** — `crypto_box_*`; `public_key_path`, `secret_key_path`; same ciphertext map layout as secret path.
- **GenerateSymmetricKey** — random `crypto_secretbox` key file.
- **GeneratePublicKeypair** — `crypto_box_keypair` output files.

KEM and SHA-3 blocks are currently available in the **GR3** root module only, not yet in the GR4 subtree.

**Dependencies**

- Installed **GNU Radio 4.0** (`gnuradio4Config.cmake` on `CMAKE_PREFIX_PATH`).
- **libsodium 1.0.22** or later (pkg-config `libsodium`). Built and tested against [1.0.22-RELEASE](https://github.com/jedisct1/libsodium/releases/tag/1.0.22-RELEASE).
- C++23 toolchain as required by your GNU Radio build (upstream documents **GCC 15+** / **Clang 20+**).

**Configure, build, and test (GR4)**

From the repository root on branch **`gnuradio4`**, use a dedicated build directory under `gnuradio4/` (ignored by `gnuradio4/.gitignore`):

```bash
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
```

Install:

```bash
cmake --install gnuradio4/build --prefix "${GR4_PREFIX}"
```

**Toolchain notes**

- Use a C++23 compiler compatible with your GNU Radio 4 binaries. On some distributions, **GCC 13** lacks `<print>` used by upstream headers; **`g++-14`** (or newer) often matches packaged GR4 GCC builds.
- The GR4 subtree will not configure until `find_package(gnuradio4)` succeeds.

Include in C++: `#include <gnuradio-4.0/nacl.hpp>` (or individual headers under `gnuradio-4.0/nacl/`).

---

**Development platform (historical)**

Ubuntu 15.04, GNU Radio 3.7.6.1 (original), GNU Radio 3.10+ (updated), Python 3.x.

**Contact**

Stefan Wunsch  
stefan.wunsch[at]student.kit.edu

**Links**

[0] https://en.wikipedia.org/wiki/Public-key_cryptography, https://en.wikipedia.org/wiki/Symmetric-key_algorithm  
[1] http://gnuradio.org/doc/doxygen/, http://gnuradio.org/doc/doxygen/page_msg_passing.html, http://gnuradio.org/doc/doxygen/page_tagged_stream_blocks.html  
[2] http://nacl.cr.yp.to/  
[3] http://doc.libsodium.org/, https://github.com/jedisct1/libsodium, https://github.com/jedisct1/libsodium/releases/tag/1.0.22-RELEASE
