// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_NACL_GENERATEPUBLICKEYPAIR_HPP
#define GNURADIO4_NACL_GENERATEPUBLICKEYPAIR_HPP

#include <sodium.h>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

#include <fstream>
#include <string>

namespace gnuradio4::nacl {

GR_REGISTER_BLOCK(gnuradio4::nacl::GeneratePublicKeypair)

struct GeneratePublicKeypair : gr::Block<GeneratePublicKeypair, gr::NoTagPropagation> {
    using Description = gr::Doc<"Generates a libsodium crypto_box keypair via crypto_box_keypair and writes raw bytes to secret_key_path and public_key_path on start.">;

    gr::Annotated<std::string, "secret_key_path"> secret_key_path{};
    gr::Annotated<std::string, "public_key_path"> public_key_path{};

    GR_MAKE_REFLECTABLE(GeneratePublicKeypair, secret_key_path, public_key_path);

    void start() {
        if (::sodium_init() < 0) {
            this->requestStop();
            return;
        }
        if (secret_key_path.value.empty() || public_key_path.value.empty()) {
            this->requestStop();
            return;
        }
        unsigned char pk[crypto_box_PUBLICKEYBYTES]{};
        unsigned char sk[crypto_box_SECRETKEYBYTES]{};
        ::crypto_box_keypair(pk, sk);
        {
            std::ofstream secretFile(secret_key_path.value, std::ios::binary);
            if (!secretFile) {
                this->requestStop();
                return;
            }
            secretFile.write(reinterpret_cast<const char*>(sk), static_cast<std::streamsize>(sizeof sk));
            if (!secretFile) {
                this->requestStop();
                return;
            }
        }
        {
            std::ofstream publicFile(public_key_path.value, std::ios::binary);
            if (!publicFile) {
                this->requestStop();
                return;
            }
            publicFile.write(reinterpret_cast<const char*>(pk), static_cast<std::streamsize>(sizeof pk));
            if (!publicFile) {
                this->requestStop();
            }
        }
    }

    [[nodiscard]] gr::work::Status processBulk() noexcept { return gr::work::Status::OK; }
};

} // namespace gnuradio4::nacl

#endif
