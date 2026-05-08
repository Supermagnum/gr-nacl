// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/nacl/GeneratePublicKeypair.hpp>
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

const boost::ut::suite<"GeneratePublicKeypair"> GeneratePublicKeypairTests = [] {
    "writes boxing public and secret key files on start"_test = [] {
        expect(::sodium_init() >= 0);
        const std::string skPath = temporaryPath("gr_nacl4_gen_bs");
        const std::string pkPath = temporaryPath("gr_nacl4_gen_bp");

        gnuradio4::nacl::GeneratePublicKeypair generator(
            gr::property_map{{"secret_key_path", skPath}, {"public_key_path", pkPath}, {"name", std::string("kgen")}});
        generator.init(std::make_shared<gr::Sequence>());
        generator.start();

        std::vector<char> skBuf(static_cast<std::size_t>(crypto_box_SECRETKEYBYTES));
        std::vector<char> pkBuf(static_cast<std::size_t>(crypto_box_PUBLICKEYBYTES));

        std::ifstream skFile(skPath, std::ios::binary);
        std::ifstream pkFile(pkPath, std::ios::binary);
        skFile.read(skBuf.data(), static_cast<std::streamsize>(skBuf.size()));
        pkFile.read(pkBuf.data(), static_cast<std::streamsize>(pkBuf.size()));
        expect(static_cast<bool>(skFile && pkFile));
        expect(eq(static_cast<std::size_t>(skFile.gcount()), static_cast<std::size_t>(crypto_box_SECRETKEYBYTES)));
        expect(eq(static_cast<std::size_t>(pkFile.gcount()), static_cast<std::size_t>(crypto_box_PUBLICKEYBYTES)));

        std::remove(skPath.c_str());
        std::remove(pkPath.c_str());
    };
};

int main() { return boost::ut::cfg<boost::ut::override>.run(); }