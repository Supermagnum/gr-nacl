# gr-nacl GNU Radio 4 port: build and test fixes

Reference for aligning the GR4 subtree (`gnuradio4/`) with an installed `gnuradio4` toolchain (GCC 14+ for `<print>`, CMake prefix `/opt/gnuradio4-gcc` or equivalent).

## Configure / link

- Use **g++-14** (or another compiler matching upstream GR4): `CMAKE_CXX_COMPILER=g++-14`
- **`find_package(gnuradio4)`** may pull CPR: pass `-Dcpr_DIR=/path/to/install/lib/cmake/cpr`
- Typical prefix: `-DCMAKE_PREFIX_PATH=<install>` and `-Dgnuradio4_DIR=<install>/lib/cmake/gnuradio4`

## Block code

- **`property_map` keys**: Prefer `gr::convert_string_domain(std::string_view("key"))` to avoid ambiguity with string overloads (`std::pmr::string`, `std::string`, `std::string_view`).
- **Helpers**: Use `std::pmr::string` for lookups in `gr::property_map` (with `#include <memory_resource>`); `gr::pmr::string` is wrong.
- **Sodium macros in headers**: Include `<sodium.h>` before member types that depend on constants like `crypto_*_KEYBYTES` in layouts.
- **CryptTaggedStream**: Accept the first inbound tag map that contains `packet_len` (indices are relative; not always `relIndex == 0`).

## Lifecycle and settings on standalone blocks

- Constructor **`Block(property_map)`** only stages parameters; **`block.init(std::make_shared<gr::Sequence>())`** merges them into `Annotated<>` fields before **`block.start()`** (user `start()` that loads keys/files).
- Omitting **`init()`** leaves paths empty: generator tests saw no files; EncryptPublic saw no ciphertext.

## Message-port tests (`qa_*`)

- **MsgPortOut** drives a block **input**; connect `toBlock.connect(block.msg_*_in)`
- **MsgPortIn** receives from a block **output**; connect `block.msg_*_out.connect(fromBlock)`
- Use **`streamWriter()`** only on **`MsgPortOut`**; **`streamReader()`** only on **`MsgPortIn`**.
- Replace string UDLs **`"..."s`** with **`std::string("...")`** and include `<string>` as needed.

## Boost.UT

- With **`BOOST_UT_DISABLE_MODULE`**, add:
  **`int main() { return boost::ut::cfg<boost::ut::override>.run(); }`**

## `qa_CryptTaggedStream`

- The earlier graph + `scheduler::Simple` + TagSource regression did not reliably deliver samples in this OOT link setup even for Source-only-to-Sink sanity checks (after trying template `connect<>`, `GrTestingBlocksShared`, `runAndWait().has_value()` checks).
- Replaced by **deterministic unit tests**: `readPduLength`, XOR self-inverse (`crypto_stream_xor`), and **`rotateNonceLikeGr3`** fixtures (expects **LSB-or-1 merge** after left shift: `0x81` then `0x03`, then `0x06`).

## CTest

```bash
cd gnuradio4/build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-14 \
  -DCMAKE_PREFIX_PATH=/opt/gnuradio4-gcc \
  -Dgnuradio4_DIR=/opt/gnuradio4-gcc/lib/cmake/gnuradio4 \
  -Dcpr_DIR=/opt/gnuradio4-gcc/lib/cmake/cpr
cmake --build . -j"$(nproc)"
ctest --test-dir . -j"$(nproc)" --output-on-failure
```
