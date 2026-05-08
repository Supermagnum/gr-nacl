// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/Message.hpp>
#include <gnuradio-4.0/Port.hpp>
#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/Tensor.hpp>
#include <gnuradio-4.0/nacl/DecryptSecret.hpp>
#include <gnuradio-4.0/nacl/EncryptSecret.hpp>

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

void writeSecretKeyMaterial(const std::string& path) {
    std::vector<unsigned char> key(static_cast<std::size_t>(crypto_secretbox_KEYBYTES));
    ::randombytes_buf(key.data(), key.size());
    std::ofstream file(path, std::ios::binary);
    expect(static_cast<bool>(file));
    file.write(reinterpret_cast<const char*>(key.data()), static_cast<std::streamsize>(key.size()));
    expect(static_cast<bool>(file));
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

const boost::ut::suite<"DecryptSecret"> DecryptSecretTests = [] {
    "encrypt notify round-trips through decrypt block"_test = [] {
        expect(::sodium_init() >= 0);
        const std::string keyPath = temporaryPath("gr_nacl4_sk_rd");
        writeSecretKeyMaterial(keyPath);

        gnuradio4::nacl::EncryptSecret encryptor(
            gr::property_map{{"key_file_path", keyPath}, {"name", std::string("enc")}});
        gnuradio4::nacl::DecryptSecret decryptor(
            gr::property_map{{"key_file_path", keyPath}, {"name", std::string("dec")}});
        encryptor.init(std::make_shared<gr::Sequence>());
        decryptor.init(std::make_shared<gr::Sequence>());
        encryptor.start();
        decryptor.start();

        gr::MsgPortOut             toEncrypt;
        gr::MsgPortIn              fromEncrypt;
        gr::MsgPortOut             toDecrypt;
        gr::MsgPortIn              fromDecrypt;
        expect(toEncrypt.connect(encryptor.msg_clear_in).has_value());
        expect(encryptor.msg_cipher_out.connect(fromEncrypt).has_value());
        expect(toDecrypt.connect(decryptor.msg_cipher_in).has_value());
        expect(decryptor.msg_plain_out.connect(fromDecrypt).has_value());

        std::vector<std::uint8_t> plainIn{42U, 43U, 44U};
        gr::property_map          inbound;
        inbound[gr::convert_string_domain(std::string_view("msg_clear"))] =
            gr::pmt::Value(gr::Tensor<std::uint8_t>(plainIn));
        pushNotify(toEncrypt, std::move(inbound));
        encryptor.processScheduledMessages();

        expect(eq(fromEncrypt.streamReader().available(), 1UZ));
        auto                 cipherSpan   = fromEncrypt.streamReader().template get<gr::SpanReleasePolicy::ProcessAll>(1UZ);
        const gr::Message& packaged       = cipherSpan[0];
        expect(packaged.data.has_value());
        gr::property_map ciphertextBody = packaged.data.value();
        expect(cipherSpan.consume(cipherSpan.size()));

        pushNotify(toDecrypt, std::move(ciphertextBody));
        decryptor.processScheduledMessages();

        expect(eq(fromDecrypt.streamReader().available(), 1UZ));
        auto                 plainSpan    = fromDecrypt.streamReader().template get<gr::SpanReleasePolicy::ProcessAll>(1UZ);
        const gr::Message&   decryptedMsg = plainSpan[0];
        expect(decryptedMsg.data.has_value());
        const gr::property_map&   outMap         = decryptedMsg.data.value();
        const gr::pmt::Value&     decryptedField = outMap.at(gr::convert_string_domain(std::string_view("msg_decrypted")));
        const auto*               payload        = decryptedField.get_if<gr::Tensor<std::uint8_t>>();
        expect(payload != nullptr);
        expect(eq(payload->size(), plainIn.size()));
        for (std::size_t i = 0; i < plainIn.size(); ++i) {
            expect(eq((*payload)[i], plainIn[i]));
        }
        expect(plainSpan.consume(plainSpan.size()));

        std::remove(keyPath.c_str());
    };
};

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
