// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_NACL_ENCRYPTPUBLIC_HPP
#define GNURADIO4_NACL_ENCRYPTPUBLIC_HPP

#include <sodium.h>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/Message.hpp>
#include <gnuradio-4.0/Port.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/Tensor.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/meta/utils.hpp>
#include <gnuradio-4.0/nacl/detail/Helpers.hpp>

#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace gnuradio4::nacl {

GR_REGISTER_BLOCK(gnuradio4::nacl::EncryptPublic)

struct EncryptPublic : gr::Block<EncryptPublic, gr::NoTagPropagation> {
    using Description = gr::Doc<"Encrypts msg_clear byte tensors with crypto_box_easy using raw keys from public_key_path and secret_key_path.">;

    gr::MsgPortIn              msg_clear_in{};
    gr::MsgPortOut             msg_cipher_out{};
    gr::Annotated<std::string, "public_key_path"> public_key_path{};
    gr::Annotated<std::string, "secret_key_path"> secret_key_path{};

    GR_MAKE_REFLECTABLE(EncryptPublic, msg_clear_in, msg_cipher_out, public_key_path, secret_key_path);

    std::array<unsigned char, crypto_box_PUBLICKEYBYTES> _publicKey{};
    std::array<unsigned char, crypto_box_SECRETKEYBYTES> _secretKey{};

    [[nodiscard]] bool reloadKeysFromFiles() noexcept {
        if (public_key_path.value.empty() || secret_key_path.value.empty()) {
            return false;
        }
        return detail::readBinaryFile(std::string_view(public_key_path.value), std::span(_publicKey)) //
            && detail::readBinaryFile(std::string_view(secret_key_path.value), std::span(_secretKey));
    }

    void start() {
        if (::sodium_init() < 0) {
            this->requestStop();
            return;
        }
        if (!reloadKeysFromFiles()) {
            this->requestStop();
        }
    }

    void settingsChanged(const gr::property_map& /* oldSettings */, const gr::property_map& newSettings) {
        if (!newSettings.contains("public_key_path") && !newSettings.contains("secret_key_path")) [[likely]] {
            return;
        }
        if (!reloadKeysFromFiles()) [[unlikely]] {
            this->requestStop();
        }
    }

    [[nodiscard]] gr::work::Status processBulk() noexcept { return gr::work::Status::OK; }

    void processMessages(gr::MsgPortIn& port, std::span<const gr::Message> messages) {
        if (std::addressof(port) != std::addressof(msg_clear_in)) [[unlikely]] {
            return;
        }
        for (const gr::Message& message : messages) {
            if (!message.data.has_value()) {
                continue;
            }
            const gr::property_map&         body = message.data.value();
            const gr::Tensor<std::uint8_t>* clear = detail::tensorBytesFromMap(body, "msg_clear");
            if (clear == nullptr || clear->size() == 0ULL) {
                continue;
            }
            std::array<unsigned char, crypto_box_NONCEBYTES> nonce{};
            ::randombytes_buf(nonce.data(), nonce.size());
            const std::size_t           cipherLen = crypto_box_MACBYTES + clear->size();
            std::vector<unsigned char> cipher(cipherLen);
            const int                   err = ::crypto_box_easy(cipher.data(),
                clear->data(),
                static_cast<unsigned long long>(clear->size()),
                nonce.data(),
                _publicKey.data(),
                _secretKey.data());
            if (err != 0) [[unlikely]] {
                continue;
            }
            std::vector<std::uint8_t> nonceBytes(nonce.begin(), nonce.end());
            std::vector<std::uint8_t> cipherBytes(cipher.begin(), cipher.end());
            gr::property_map          outBody;
            outBody[gr::convert_string_domain(std::string_view("nonce"))]         = gr::pmt::Value(gr::Tensor<std::uint8_t>(nonceBytes));
            outBody[gr::convert_string_domain(std::string_view("msg_encrypted"))] = gr::pmt::Value(gr::Tensor<std::uint8_t>(cipherBytes));
            gr::Message               reply;
            reply.cmd = gr::message::Command::Notify;
            reply.data = std::move(outBody);
            gr::WriterSpanLike auto outSpan = msg_cipher_out.streamWriter().template reserve<gr::SpanReleasePolicy::ProcessAll>(1UZ);
            outSpan[0]                      = std::move(reply);
            outSpan.publish(1UZ);
        }
    }
};

} // namespace gnuradio4::nacl

#endif
