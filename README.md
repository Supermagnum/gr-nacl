gr-nacl: GNU Radio data encryption module
========

**IMPORTANT NOTICE**: This is AI-generated code. The developer has a neurological condition that makes it impossible to use and learn traditional programming. The developer has put in a significant effort. This code might not work properly. Use at your own risk.

GNU Radio **3.10+** out-of-tree module for data encryption using NaCl-style primitives from **libsodium** (public-key and secret-key constructs, streamed PDUs, post-quantum KEM, and SHA-3 hashing). Symmetric vs public-key encryption is explained, for example, on Wikipedia [0].

**Branch:** this is the **`master`** branch for **GNU Radio 3.x** only. For **GNU Radio 4.0**, use branch **`gnuradio4`** (separate development line).

```bash
git fetch origin gnuradio4
git checkout gnuradio4
```

See `README.md` on branch **`gnuradio4`** for the GR4 block library under `gnuradio4/`.

---

### libsodium requirement

gr-nacl is compiled and built against **[libsodium 1.0.22-RELEASE](https://github.com/jedisct1/libsodium/releases/tag/1.0.22-RELEASE)**. CMake enforces a minimum of 1.0.22 via `find_package(Sodium REQUIRED)` and the imported target `sodium::sodium`.

That release adds APIs used by this project:

- **X-Wing KEM** — hybrid ML-KEM768 + X25519 via `crypto_kem_*()`.
- **SHA-3** — `crypto_hash_sha3256_*()` and `crypto_hash_sha3512_*()`.

Install **libsodium** 1.0.22 before building gr-nacl (run these commands in a **libsodium** source tree, not in gr-nacl — gr-nacl has no `autogen.sh`):

```bash
git clone https://github.com/jedisct1/libsodium.git
cd libsodium
git checkout 1.0.22-RELEASE
./autogen.sh
./configure --prefix=/usr/local
make -j"$(nproc)"
sudo make install
sudo ldconfig    # Linux
```

If libsodium is already installed under `/usr/local`, skip the steps above.

Verify: `pkg-config --modversion libsodium` should print `1.0.22` or later.

---

### Blocks (namespace `gr::nacl`)

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

GRC block definitions live in `grc/` (legacy `.xml` plus YAML `.block.yml` for newer blocks). See GNU Radio documentation on message passing and tagged streams [1].

**KEM message PDU layout**

- **encrypt_kem** input: `msg_clear` on port `Msg clear`.
- **encrypt_kem** output: `kem_ciphertext`, `nonce`, `msg_encrypted` on port `Msg encrypted`.
- **decrypt_kem** reverses that layout and publishes `msg_decrypted` on port `Msg decrypted`.

**Python usage**

```python
from gnuradio import nacl

nacl.encrypt_public(pk_file, sk_file)
nacl.generate_kem_keypair(sk_file, pk_file)
nacl.encrypt_kem(pk_file)
nacl.decrypt_kem(sk_file)
nacl.hash_sha3(sha3_512=False)
```

---

### Configure, build, test, and install

From the **gr-nacl** repository root on branch **`master`** (use `cmake`, not `autogen.sh`):

```bash
git clone https://github.com/Supermagnum/gr-nacl.git
cd gr-nacl
git checkout master

mkdir -p build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/usr/local
cmake --build . -j"$(nproc)"
ctest --output-on-failure
sudo cmake --install .
```

CMake **3.5+** is required.

**Mac OS X**

With GNU Radio and libsodium from MacPorts:

```bash
cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/local ..
cmake --build . && ctest && sudo cmake --install .
```

**Testing**

Example flowgraphs live under `examples/`. If `qa_test_vectors` or `qa_libsodium_vectors` fail with "Permission denied", run `chmod +x python/qa_*.py` and re-run `ctest`.

**Compatibility**

GNU Radio 3.10+ with `std::shared_ptr` instead of `boost::shared_ptr` and `std::vector` instead of the deprecated `__GR_VLA` macro.

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
