/* -*- c++ -*- */
/*
 * Copyright 2026 gr-nacl contributors
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "decrypt_kem_impl.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <vector>
#include <sodium.h>

namespace gr {
namespace nacl {

decrypt_kem::sptr decrypt_kem::make(std::string filename_sk)
{
    return gnuradio::get_initial_sptr(new decrypt_kem_impl(filename_sk));
}

decrypt_kem_impl::decrypt_kem_impl(std::string filename_sk)
    : gr::block("decrypt_kem",
                gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0))
{
    d_filename_sk = filename_sk;
    d_sk = new unsigned char[crypto_kem_SECRETKEYBYTES];

    std::ifstream file_sk(filename_sk.c_str(), std::ios::binary);
    if (!file_sk.is_open()) {
        throw std::runtime_error("Secret-key file not found.");
    }
    file_sk.read(reinterpret_cast<char*>(d_sk), crypto_kem_SECRETKEYBYTES);
    if (!file_sk) {
        throw std::runtime_error("Secret-key file is too short for KEM key.");
    }
    file_sk.close();

    d_port_id_in = pmt::mp("Msg encrypted");
    message_port_register_in(d_port_id_in);
    set_msg_handler(d_port_id_in, [this](pmt::pmt_t msg) { this->handle_msg(msg); });

    d_port_id_out = pmt::mp("Msg decrypted");
    message_port_register_out(d_port_id_out);
}

decrypt_kem_impl::~decrypt_kem_impl() { delete[] d_sk; }

void decrypt_kem_impl::handle_msg(pmt::pmt_t msg)
{
    size_t msg_size = pmt::length(msg);

    std::vector<uint8_t> kem_ct, data, nonce;
    bool kem_ciphertext_found = false;
    bool msg_encrypted_found = false;
    bool nonce_found = false;

    for (size_t k = 0; k < msg_size; k++) {
        const std::string tag = pmt::symbol_to_string(pmt::nth(0, pmt::nth(k, msg)));
        if (tag == "kem_ciphertext") {
            if (pmt::is_u8vector(pmt::nth(1, pmt::nth(k, msg)))) {
                kem_ct = pmt::u8vector_elements(pmt::nth(1, pmt::nth(k, msg)));
                kem_ciphertext_found = true;
            }
        }
        if (tag == "msg_encrypted") {
            if (pmt::is_u8vector(pmt::nth(1, pmt::nth(k, msg)))) {
                data = pmt::u8vector_elements(pmt::nth(1, pmt::nth(k, msg)));
                msg_encrypted_found = true;
            }
        }
        if (tag == "nonce") {
            if (pmt::is_u8vector(pmt::nth(1, pmt::nth(k, msg)))) {
                nonce = pmt::u8vector_elements(pmt::nth(1, pmt::nth(k, msg)));
                nonce_found = true;
            }
        }
    }

    if (!kem_ciphertext_found || !msg_encrypted_found || !nonce_found) {
        return;
    }

    if (kem_ct.size() != crypto_kem_CIPHERTEXTBYTES) {
        std::cout << "Invalid KEM ciphertext length." << std::endl;
        return;
    }

    unsigned char ss[crypto_kem_SHAREDSECRETBYTES];
    if (crypto_kem_dec(ss, kem_ct.data(), d_sk) != 0) {
        std::cout << "Failed to perform KEM decapsulation." << std::endl;
        sodium_memzero(ss, sizeof ss);
        return;
    }

    std::vector<unsigned char> data_char(data.size());
    std::vector<unsigned char> nonce_char(nonce.size());
    const size_t data_char_sz = data.size();
    for (size_t k = 0; k < data.size(); k++) {
        data_char[k] = static_cast<unsigned char>(data[k]);
    }
    for (size_t k = 0; k < nonce.size(); k++) {
        nonce_char[k] = static_cast<unsigned char>(nonce[k]);
    }

    const size_t msg_len = data_char_sz - crypto_secretbox_MACBYTES;
    std::vector<unsigned char> msg_decrypted(msg_len);
    const int msg_status = crypto_secretbox_open_easy(msg_decrypted.data(),
                                                      data_char.data(),
                                                      data_char_sz,
                                                      nonce_char.data(),
                                                      ss);
    sodium_memzero(ss, sizeof ss);

    if (msg_status == 0) {
        std::vector<uint8_t> msg_decrypted_vec(msg_len);
        for (size_t k = 0; k < msg_len; k++) {
            msg_decrypted_vec[k] = static_cast<uint8_t>(msg_decrypted[k]);
        }

        pmt::pmt_t msg_out = pmt::list2(
            pmt::string_to_symbol("msg_decrypted"),
            pmt::init_u8vector(msg_decrypted_vec.size(), msg_decrypted_vec));

        message_port_pub(d_port_id_out, pmt::list1(msg_out));
    } else {
        std::cout << "Failed to decrypt message." << std::endl;
        std::cout << "KEM ciphertext found: " << kem_ciphertext_found << std::endl;
        std::cout << "Nonce found: " << nonce_found << std::endl;
        std::cout << "Encrypted message found: " << msg_encrypted_found << std::endl;
        std::cout << "Message decryption status: " << msg_status << std::endl;
    }
}

} /* namespace nacl */
} /* namespace gr */
