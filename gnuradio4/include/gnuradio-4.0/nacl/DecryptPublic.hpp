// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_NACL_DECRYPTPUBLIC_HPP
#define GNURADIO4_NACL_DECRYPTPUBLIC_HPP

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

GR_REGISTER_BLOCK(gnuradio4::nacl::DecryptPublic)

struct DecryptPublic : gr::Block<DecryptPublic, gr::NoTagPropagation> {
    using Description = gr::Doc<"Decrypts msg_encrypted tensors with crypto_box_open_easy using raw keys from public_key_path and secret_key_path.">;

    gr::MsgPortIn              msg_cipher_in{};
    gr::MsgPortOut             msg_plain_out{};
    gr::Annotated<std::string, "public_key_path"> public_key_path{};
    gr::Annotated<std::string, "secret_key_path"> secret_key_path{};

    GR_MAKE_REFLECTABLE(DecryptPublic, msg_cipher_in, msg_plain_out, public_key_path, secret_key_path);

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
        if (std::addressof(port) != std::addressof(msg_cipher_in)) [[unlikely]] {
            return;
        }
        for (const gr::Message& message : messages) {
            if (!message.data.has_value()) {
                continue;
            }
            const gr::property_map& body         = message.data.value();
            const auto* cipherTensor             = detail::tensorBytesFromMap(body, "msg_encrypted");
            const auto* nonceTensor              = detail::tensorBytesFromMap(body, "nonce");
            if (cipherTensor == nullptr || nonceTensor == nullptr || cipherTensor->size() <= crypto_box_MACBYTES //
                || nonceTensor->size() != crypto_box_NONCEBYTES) {
                continue;
            }
            const std::size_t          plainLen = cipherTensor->size() - crypto_box_MACBYTES;
            std::vector<unsigned char> plain(plainLen);
            const int                  err = ::crypto_box_open_easy(plain.data(),
                cipherTensor->data(),
                static_cast<unsigned long long>(cipherTensor->size()),
                nonceTensor->data(),
                _publicKey.data(),
                _secretKey.data());
            if (err != 0) [[unlikely]] {
                continue;
            }
            std::vector<std::uint8_t> plainBytes(plain.begin(), plain.end());
            gr::property_map          outBody;
            outBody[gr::convert_string_domain(std::string_view("msg_decrypted"))] = gr::pmt::Value(gr::Tensor<std::uint8_t>(plainBytes));
            gr::Message               reply;
            reply.cmd = gr::message::Command::Notify;
            reply.data = std::move(outBody);
            gr::WriterSpanLike auto outSpan = msg_plain_out.streamWriter().template reserve<gr::SpanReleasePolicy::ProcessAll>(1UZ);
            outSpan[0]                      = std::move(reply);
            outSpan.publish(1UZ);
        }
    }
};

} // namespace gnuradio4::nacl

#endif
