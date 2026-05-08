// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/Message.hpp>
#include <gnuradio-4.0/Port.hpp>
#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/Tensor.hpp>
#include <gnuradio-4.0/Value.hpp>
#include <gnuradio-4.0/nacl/DecryptPublic.hpp>
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

const boost::ut::suite<"DecryptPublic"> DecryptPublicTests = [] {
    "crypto_box round-trip over message ports"_test = [] {
        expect(::sodium_init() >= 0);

        unsigned char pk[crypto_box_PUBLICKEYBYTES]{};
        unsigned char sk[crypto_box_SECRETKEYBYTES]{};
        ::crypto_box_keypair(pk, sk);

        const std::string pkPath = temporaryPath("gr_nacl4_pk_d");
        const std::string skPath = temporaryPath("gr_nacl4_sk_d");
        {
            std::ofstream pkFile(pkPath, std::ios::binary);
            std::ofstream skFile(skPath, std::ios::binary);
            expect(static_cast<bool>(pkFile && skFile));
            pkFile.write(reinterpret_cast<const char*>(pk), static_cast<std::streamsize>(sizeof pk));
            skFile.write(reinterpret_cast<const char*>(sk), static_cast<std::streamsize>(sizeof sk));
        }

        gnuradio4::nacl::EncryptPublic encryptor(
            gr::property_map{{"public_key_path", pkPath}, {"secret_key_path", skPath}, {"name", std::string("penc")}});
        gnuradio4::nacl::DecryptPublic decryptor(
            gr::property_map{{"public_key_path", pkPath}, {"secret_key_path", skPath}, {"name", std::string("pdec")}});
        encryptor.init(std::make_shared<gr::Sequence>());
        decryptor.init(std::make_shared<gr::Sequence>());
        encryptor.start();
        decryptor.start();

        gr::MsgPortOut toEncrypt;
        gr::MsgPortIn  fromEncrypt;
        gr::MsgPortOut toDecrypt;
        gr::MsgPortIn  fromDecrypt;
        expect(toEncrypt.connect(encryptor.msg_clear_in).has_value());
        expect(encryptor.msg_cipher_out.connect(fromEncrypt).has_value());
        expect(toDecrypt.connect(decryptor.msg_cipher_in).has_value());
        expect(decryptor.msg_plain_out.connect(fromDecrypt).has_value());

        std::vector<std::uint8_t> plain{11U, 12U};
        gr::property_map          req;
        req[gr::convert_string_domain(std::string_view("msg_clear"))] =
            gr::pmt::Value(gr::Tensor<std::uint8_t>(plain));
        pushNotify(toEncrypt, std::move(req));
        encryptor.processScheduledMessages();

        auto               packReader = fromEncrypt.streamReader().template get<gr::SpanReleasePolicy::ProcessAll>(1UZ);
        expect(packReader[0].data.has_value());
        gr::property_map boxedBody = packReader[0].data.value();
        expect(packReader.consume(packReader.size()));

        pushNotify(toDecrypt, std::move(boxedBody));
        decryptor.processScheduledMessages();

        expect(eq(fromDecrypt.streamReader().available(), 1UZ));
        auto plainReader = fromDecrypt.streamReader().template get<gr::SpanReleasePolicy::ProcessAll>(1UZ);
        expect(plainReader[0].data.has_value());
        const gr::property_map& out    = plainReader[0].data.value();
        const auto* tensor =
            out.at(gr::convert_string_domain(std::string_view("msg_decrypted"))).get_if<gr::Tensor<std::uint8_t>>();
        expect(tensor != nullptr && tensor->size() == plain.size());
        for (std::size_t i = 0; i < plain.size(); ++i) {
            expect(eq((*tensor)[i], plain[i]));
        }
        expect(plainReader.consume(plainReader.size()));

        std::remove(pkPath.c_str());
        std::remove(skPath.c_str());
    };
};

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
