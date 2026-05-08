// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/Message.hpp>
#include <gnuradio-4.0/Port.hpp>
#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/Tensor.hpp>
#include <gnuradio-4.0/Value.hpp>
#include <gnuradio-4.0/nacl/EncryptPublic.hpp>

#include <cstdio>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include <sodium.h>

using namespace boost::ut;

namespace {

[[nodiscard]] std::string temporaryPath(const char* prefix) {
    static int counter = 0;
    return std::string(prefix) + "_" + std::to_string(++counter) + ".bin";
}

void pushNotify(gr::MsgPortOut& downstream, gr::property_map body) {
    gr::Message message;
    message.cmd  = gr::message::Command::Notify;
    message.data = std::move(body);
    auto writer  = downstream.streamWriter().template reserve<gr::SpanReleasePolicy::ProcessAll>(1UZ);
    writer[0]    = std::move(message);
    writer.publish(1UZ);
}

} // namespace

const boost::ut::suite<"EncryptPublic"> EncryptPublicTests = [] {
    "crypto_box output contains nonce and ciphertext"_test = [] {
        expect(::sodium_init() >= 0);
        unsigned char pk[crypto_box_PUBLICKEYBYTES]{};
        unsigned char sk[crypto_box_SECRETKEYBYTES]{};
        ::crypto_box_keypair(pk, sk);

        const std::string pkPath = temporaryPath("gr_nacl4_pk");
        const std::string skPath = temporaryPath("gr_nacl4_sk");
        {
            std::ofstream pkFile(pkPath, std::ios::binary);
            std::ofstream skFile(skPath, std::ios::binary);
            expect(static_cast<bool>(pkFile && skFile));
            pkFile.write(reinterpret_cast<const char*>(pk), static_cast<std::streamsize>(sizeof pk));
            skFile.write(reinterpret_cast<const char*>(sk), static_cast<std::streamsize>(sizeof sk));
        }

        gnuradio4::nacl::EncryptPublic block(
            gr::property_map{{"public_key_path", pkPath}, {"secret_key_path", skPath}, {"name", std::string("pubenc")}});
        block.init(std::make_shared<gr::Sequence>());
        block.start();

        gr::MsgPortOut toEncrypt;
        gr::MsgPortIn fromEncrypt;
        expect(toEncrypt.connect(block.msg_clear_in).has_value());
        expect(block.msg_cipher_out.connect(fromEncrypt).has_value());

        std::vector<std::uint8_t> payload{7U, 8U, 9U};
        gr::property_map          inbound;
        inbound[gr::convert_string_domain(std::string_view("msg_clear"))] =
            gr::pmt::Value(gr::Tensor<std::uint8_t>(payload));
        pushNotify(toEncrypt, std::move(inbound));
        block.processScheduledMessages();

        expect(eq(fromEncrypt.streamReader().available(), 1UZ));
        auto reader = fromEncrypt.streamReader().template get<gr::SpanReleasePolicy::ProcessAll>(1UZ);
        expect(reader[0].data.has_value());
        const gr::property_map& map    = reader[0].data.value();
        const auto*              nonce =
            map.at(gr::convert_string_domain(std::string_view("nonce"))).get_if<gr::Tensor<std::uint8_t>>();
        const auto* cipher =
            map.at(gr::convert_string_domain(std::string_view("msg_encrypted"))).get_if<gr::Tensor<std::uint8_t>>();
        expect(nonce != nullptr && cipher != nullptr);
        expect(eq(nonce->size(), static_cast<std::size_t>(crypto_box_NONCEBYTES)));
        expect(eq(cipher->size(), static_cast<std::size_t>(crypto_box_MACBYTES + payload.size())));
        expect(reader.consume(reader.size()));

        std::remove(pkPath.c_str());
        std::remove(skPath.c_str());
    };
};

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
