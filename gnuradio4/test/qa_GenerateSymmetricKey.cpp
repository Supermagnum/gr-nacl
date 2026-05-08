// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/nacl/GenerateSymmetricKey.hpp>
#include <gnuradio-4.0/Sequence.hpp>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <sodium.h>

using namespace boost::ut;

namespace {

[[nodiscard]] std::string temporaryPath(const char* prefix) {
    static int counter = 0;
    return std::string(prefix) + "_" + std::to_string(++counter) + ".bin";
}

} // namespace

const boost::ut::suite<"GenerateSymmetricKey"> GenerateSymmetricKeyTests = [] {
    "writes a secretbox-length key material file on start"_test = [] {
        expect(::sodium_init() >= 0);
        const std::string path = temporaryPath("gr_nacl4_gen_sk");
        gnuradio4::nacl::GenerateSymmetricKey generator(
            gr::property_map{{"key_file_path", path}, {"name", std::string("gen")}});
        generator.init(std::make_shared<gr::Sequence>());
        generator.start();

        std::ifstream file(path, std::ios::binary);
        std::vector<char> buffer(static_cast<std::size_t>(crypto_secretbox_KEYBYTES));
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        expect(static_cast<bool>(file));
        expect(eq(static_cast<std::size_t>(file.gcount()), static_cast<std::size_t>(crypto_secretbox_KEYBYTES)));

        std::remove(path.c_str());
    };
};

int main() { return boost::ut::cfg<boost::ut::override>.run(); }