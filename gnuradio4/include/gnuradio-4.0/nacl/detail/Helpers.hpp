// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_NACL_DETAIL_HELPERS_HPP
#define GNURADIO4_NACL_DETAIL_HELPERS_HPP

#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/Tensor.hpp>
#include <gnuradio-4.0/Value.hpp>

#include <cstddef>
#include <fstream>
#include <memory_resource>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace gnuradio4::nacl::detail {

[[nodiscard]] inline const gr::Tensor<std::uint8_t>* tensorBytesFromMap(const gr::property_map& map, std::string_view keyView) {
    const std::pmr::string key(keyView.begin(), keyView.end());
    const auto            it = map.find(key);
    if (it == map.end()) {
        return nullptr;
    }
    return it->second.get_if<gr::Tensor<std::uint8_t>>();
}

[[nodiscard]] inline std::optional<gr::Size_t> readPduLength(const gr::property_map& map, std::string_view keyView) {
    const std::pmr::string key(keyView.begin(), keyView.end());
    const auto            it = map.find(key);
    if (it == map.end()) {
        return std::nullopt;
    }
    const gr::pmt::Value& v = it->second;
    if (const auto* p = v.get_if<std::uint64_t>()) {
        return static_cast<gr::Size_t>(*p);
    }
    if (const auto* p = v.get_if<std::int64_t>()) {
        return static_cast<gr::Size_t>(*p);
    }
    if (const auto* p = v.get_if<std::uint32_t>()) {
        return static_cast<gr::Size_t>(*p);
    }
    if (const auto* p = v.get_if<std::int32_t>()) {
        return static_cast<gr::Size_t>(*p);
    }
    return std::nullopt;
}

[[nodiscard]] inline bool readBinaryFile(std::string_view path, std::span<unsigned char> out) {
    std::ifstream file(std::string(path), std::ios::binary);
    if (!file) {
        return false;
    }
    file.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size()));
    return static_cast<std::size_t>(file.gcount()) == out.size();
}

inline void rotateNonceLikeGr3(std::span<unsigned char> nonce) noexcept {
    if (nonce.empty()) {
        return;
    }
    const unsigned char storeBit = static_cast<unsigned char>(nonce[0] & 0x80u);
    for (unsigned char& b : nonce) {
        b = static_cast<unsigned char>(b << 1U);
    }
    if (storeBit == 0) {
        nonce.back() = static_cast<unsigned char>(nonce.back() & static_cast<unsigned char>(~0x01u));
    } else {
        nonce.back() = static_cast<unsigned char>(nonce.back() | 0x01u);
    }
}

} // namespace gnuradio4::nacl::detail

#endif
