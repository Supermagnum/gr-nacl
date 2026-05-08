// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_NACL_CRYPTTAGGEDSTREAM_HPP
#define GNURADIO4_NACL_CRYPTTAGGEDSTREAM_HPP

#include <array>
#include <optional>
#include <span>
#include <string>
#include <sodium.h>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/Tensor.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/meta/utils.hpp>
#include <gnuradio-4.0/nacl/detail/Helpers.hpp>


namespace gnuradio4::nacl {

GR_REGISTER_BLOCK(gnuradio4::nacl::CryptTaggedStream)

struct CryptTaggedStream : gr::Block<CryptTaggedStream> {
    using Description = gr::Doc<"Symmetric stream XOR on tagged PDUs via crypto_stream_xor (libsodium), matching GNU Radio tagged-stream packet boundaries.">;

    template<typename U, gr::meta::fixed_string description = "", typename... Arguments>
    using A = gr::Annotated<U, description, Arguments...>;

    gr::PortIn<std::uint8_t>               in{};
    gr::PortOut<std::uint8_t>              out{};
    A<gr::Tensor<std::uint8_t>, "symmetric_key">      symmetric_key{};
    A<gr::Tensor<std::uint8_t>, "stream_nonce">      stream_nonce{};
    A<bool, "rotate_nonce", gr::Doc<"apply legacy gr-nacl nonce bit rotation after each PDU">> rotate_nonce       = false;
    A<std::string, "length_tag_key", gr::Doc<"tag key storing PDU byte length">>              length_tag_key     = std::string("packet_len");

    GR_MAKE_REFLECTABLE(CryptTaggedStream, in, out, symmetric_key, stream_nonce, rotate_nonce, length_tag_key);

    std::array<unsigned char, crypto_stream_KEYBYTES>  _streamKey{};
    std::array<unsigned char, crypto_stream_NONCEBYTES> _nonceBytes{};

    [[nodiscard]] bool refreshKeyMaterialFromSettings() noexcept {
        if (symmetric_key.value.size() != crypto_stream_KEYBYTES) {
            return false;
        }
        if (stream_nonce.value.size() != crypto_stream_NONCEBYTES) {
            return false;
        }
        for (std::size_t i = 0; i < crypto_stream_KEYBYTES; ++i) {
            _streamKey[i] = symmetric_key.value[i];
        }
        for (std::size_t i = 0; i < crypto_stream_NONCEBYTES; ++i) {
            _nonceBytes[i] = stream_nonce.value[i];
        }
        return true;
    }

    void start() {
        if (::sodium_init() < 0) {
            this->requestStop();
            return;
        }
        if (!refreshKeyMaterialFromSettings()) {
            this->requestStop();
        }
    }

    void settingsChanged(const gr::property_map& /* oldSettings */, const gr::property_map& newSettings) {
        if (!newSettings.contains("symmetric_key") && !newSettings.contains("stream_nonce")) [[likely]] {
            return;
        }
        if (!refreshKeyMaterialFromSettings()) [[unlikely]] {
            this->requestStop();
        }
    }

    gr::work::Status processBulk(gr::InputSpanLike auto& inSamples, gr::OutputSpanLike auto& outSamples) {
        std::optional<gr::Size_t> pduLength;
        for (const auto& [relIndex, mapRef] : inSamples.tags()) {
            (void)relIndex;
            pduLength = detail::readPduLength(mapRef.get(), std::string_view(length_tag_key));
            if (pduLength) {
                break;
            }
        }
        if (!pduLength) [[unlikely]] {
            return gr::work::Status::ERROR;
        }
        if (*pduLength != inSamples.size()) [[unlikely]] {
            return gr::work::Status::ERROR;
        }
        const std::span<const unsigned char> keySpan(_streamKey.data(), _streamKey.size());
        const std::span<const unsigned char> nonceSpan(_nonceBytes.data(), _nonceBytes.size());
        unsigned char*     outPtr = reinterpret_cast<unsigned char*>(&outSamples[0]);
        const unsigned char* inPtr  = reinterpret_cast<const unsigned char*>(&inSamples[0]);
        int                                   status  = ::crypto_stream_xor(outPtr, inPtr,
            static_cast<unsigned long long>(inSamples.size()),
            nonceSpan.data(),
            keySpan.data());
        if (status != 0) [[unlikely]] {
            return gr::work::Status::ERROR;
        }
        if (rotate_nonce.value) {
            detail::rotateNonceLikeGr3(std::span<unsigned char>(_nonceBytes.data(), _nonceBytes.size()));
        }
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::nacl

#endif
