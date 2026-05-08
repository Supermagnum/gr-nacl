// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/nacl/detail/Helpers.hpp>

#include <cstdint>
#include <string_view>
#include <vector>

#include <sodium.h>

using namespace boost::ut;

namespace {

[[nodiscard]] gr::property_map makePacketLenMap(gr::Size_t pduBytes) {
    gr::property_map map;
    map.insert_or_assign(gr::convert_string_domain(std::string_view("packet_len")),
        gr::pmt::Value(static_cast<std::uint64_t>(pduBytes)));
    return map;
}

} // namespace

const boost::ut::suite<"CryptTaggedStream"> CryptTaggedStreamTests = [] {
    "Helpers readPduLength extracts packet_len from tag maps"_test = [] {
        const gr::property_map empty{};
        expect(!gnuradio4::nacl::detail::readPduLength(empty, std::string_view("packet_len")).has_value());
        auto m = makePacketLenMap(static_cast<gr::Size_t>(64));
        const auto len =
            gnuradio4::nacl::detail::readPduLength(m, std::string_view("packet_len"));
        expect(len.has_value());
        expect(eq(len.value(), 64UZ));
    };

    "Libsodium stream XOR is self-inverse with identical parameters"_test = [] {
        expect(::sodium_init() >= 0);
        std::vector<unsigned char> key(static_cast<std::size_t>(crypto_stream_KEYBYTES), 7U);
        std::vector<unsigned char> nonce(static_cast<std::size_t>(crypto_stream_NONCEBYTES), 3U);
        std::vector<unsigned char> plain{10U, 20U, 30U};
        std::vector<unsigned char> buf0(plain.size());
        std::vector<unsigned char> buf1(plain.size());

        expect(eq(::crypto_stream_xor(buf0.data(), plain.data(), plain.size(), nonce.data(), key.data()),
            static_cast<int>(0)));
        expect(eq(::crypto_stream_xor(buf1.data(), buf0.data(), plain.size(), nonce.data(), key.data()),
            static_cast<int>(0)));
        for (std::size_t i = 0; i < plain.size(); ++i) {
            expect(eq(buf1[i], plain[i]));
        }
    };

    "rotateNonceLikeGr3 matches legacy left-rotate semantics on small fixture"_test = [] {
        std::vector<unsigned char> nonce = {0b10000001U};
        gnuradio4::nacl::detail::rotateNonceLikeGr3(std::span<unsigned char>(nonce.data(), nonce.size()));
        expect(eq(nonce[0], static_cast<unsigned char>(0b00000011U)));
        gnuradio4::nacl::detail::rotateNonceLikeGr3(std::span<unsigned char>(nonce.data(), nonce.size()));
        expect(eq(nonce[0], static_cast<unsigned char>(0b00000110U)));
    };
};

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
