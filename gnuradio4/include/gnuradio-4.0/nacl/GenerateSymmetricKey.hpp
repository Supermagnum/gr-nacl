// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_NACL_GENERATESYMMETRICKEY_HPP
#define GNURADIO4_NACL_GENERATESYMMETRICKEY_HPP

#include <sodium.h>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

#include <array>
#include <fstream>
#include <string>

namespace gnuradio4::nacl {

GR_REGISTER_BLOCK(gnuradio4::nacl::GenerateSymmetricKey)

struct GenerateSymmetricKey : gr::Block<GenerateSymmetricKey, gr::NoTagPropagation> {
    using Description =
        gr::Doc<"Generates crypto_secretbox_KEYBYTES random bytes via randombytes_buf and writes them to key_file_path when the block starts.">;

    gr::Annotated<std::string, "key_file_path"> key_file_path{};
    GR_MAKE_REFLECTABLE(GenerateSymmetricKey, key_file_path);

    void start() {
        if (::sodium_init() < 0) {
            this->requestStop();
            return;
        }
        if (key_file_path.value.empty()) {
            this->requestStop();
            return;
        }
        std::array<unsigned char, crypto_secretbox_KEYBYTES> material{};
        ::randombytes_buf(material.data(), material.size());
        std::ofstream file(key_file_path.value, std::ios::binary);
        if (!file) {
            this->requestStop();
            return;
        }
        file.write(reinterpret_cast<const char*>(material.data()), static_cast<std::streamsize>(material.size()));
        if (!file) {
            this->requestStop();
        }
    }

    [[nodiscard]] gr::work::Status processBulk() noexcept { return gr::work::Status::OK; }
};

} // namespace gnuradio4::nacl

#endif
