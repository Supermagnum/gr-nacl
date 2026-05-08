// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/Message.hpp>
#include <gnuradio-4.0/Port.hpp>
#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/Tensor.hpp>
#include <gnuradio-4.0/Value.hpp>
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

const boost::ut::suite<"EncryptSecret"> EncryptSecretTests = [] {
    "crypto_secretbox output includes nonce and ciphertext"_test = [] {
        expect(::sodium_init() >= 0);
        const std::string           keyPath = temporaryPath("gr_nacl4_sk");
        writeSecretKeyMaterial(keyPath);

        gnuradio4::nacl::EncryptSecret block(
            gr::property_map{{"key_file_path", keyPath}, {"name", std::string("enc")}});
        block.init(std::make_shared<gr::Sequence>());
        block.start();

        gr::MsgPortOut toEncrypt;
        gr::MsgPortIn fromEncrypt;
        expect(toEncrypt.connect(block.msg_clear_in).has_value());
        expect(block.msg_cipher_out.connect(fromEncrypt).has_value());

        std::vector<std::uint8_t> payload{'d', 'a', 't', 'a'};
        gr::property_map          request;
        request[gr::convert_string_domain(std::string_view("msg_clear"))] = gr::pmt::Value(gr::Tensor<std::uint8_t>(payload));
        pushNotify(toEncrypt, std::move(request));
        block.processScheduledMessages();

        expect(eq(fromEncrypt.streamReader().available(), 1UZ));
        auto                 readerSpan = fromEncrypt.streamReader().template get<gr::SpanReleasePolicy::ProcessAll>(1UZ);
        const gr::Message& reply        = readerSpan[0];
        expect(reply.data.has_value());
        const gr::property_map&   body       = reply.data.value();
        const gr::pmt::Value&     nonceEntry = body.at(gr::convert_string_domain(std::string_view("nonce")));
        const gr::pmt::Value&     boxedEntry = body.at(gr::convert_string_domain(std::string_view("msg_encrypted")));
        const auto*               nonce      = nonceEntry.get_if<gr::Tensor<std::uint8_t>>();
        const auto*               boxed      = boxedEntry.get_if<gr::Tensor<std::uint8_t>>();
        expect(nonce != nullptr);
        expect(boxed != nullptr);
        expect(eq(nonce->size(), static_cast<std::size_t>(crypto_secretbox_NONCEBYTES)));
        expect(eq(boxed->size(), static_cast<std::size_t>(crypto_secretbox_MACBYTES + payload.size())));

        expect(readerSpan.consume(readerSpan.size()));

        std::remove(keyPath.c_str());
    };
};

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
