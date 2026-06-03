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
#include "encrypt_kem_impl.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <vector>
#include <sodium.h>

namespace gr {
namespace nacl {

encrypt_kem::sptr encrypt_kem::make(std::string filename_pk)
{
    return gnuradio::get_initial_sptr(new encrypt_kem_impl(filename_pk));
}

encrypt_kem_impl::encrypt_kem_impl(std::string filename_pk)
    : gr::block("encrypt_kem",
                gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0))
{
    d_filename_pk = filename_pk;
    d_pk = new unsigned char[crypto_kem_PUBLICKEYBYTES];

    std::ifstream file_pk(filename_pk.c_str(), std::ios::binary);
    if (!file_pk.is_open()) {
        throw std::runtime_error("Public-key file not found.");
    }
    file_pk.read(reinterpret_cast<char*>(d_pk), crypto_kem_PUBLICKEYBYTES);
    if (!file_pk) {
        throw std::runtime_error("Public-key file is too short for KEM key.");
    }
    file_pk.close();

    d_port_id_in = pmt::mp("Msg clear");
    message_port_register_in(d_port_id_in);
    set_msg_handler(d_port_id_in, [this](pmt::pmt_t msg) { this->handle_msg(msg); });

    d_port_id_out = pmt::mp("Msg encrypted");
    message_port_register_out(d_port_id_out);
}

void encrypt_kem_impl::handle_msg(pmt::pmt_t msg)
{
    size_t msg_size = pmt::length(msg);

    std::vector<uint8_t> data;
    for (size_t k = 0; k < msg_size; k++) {
        if (pmt::symbol_to_string(pmt::nth(0, pmt::nth(k, msg))) == "msg_clear") {
            if (pmt::is_u8vector(pmt::nth(1, pmt::nth(k, msg)))) {
                data = pmt::u8vector_elements(pmt::nth(1, pmt::nth(k, msg)));
            }
        }
    }

    if (data.empty()) {
        return;
    }

    unsigned char kem_ct[crypto_kem_CIPHERTEXTBYTES];
    unsigned char ss[crypto_kem_SHAREDSECRETBYTES];
    if (crypto_kem_enc(kem_ct, ss, d_pk) != 0) {
        std::cout << "Failed to perform KEM encapsulation." << std::endl;
        sodium_memzero(ss, sizeof ss);
        return;
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    std::vector<unsigned char> data_char(data.size());
    const size_t data_char_sz = data.size();
    for (size_t k = 0; k < data.size(); k++) {
        data_char[k] = static_cast<unsigned char>(data[k]);
    }

    const size_t ciphertext_len = crypto_secretbox_MACBYTES + data_char_sz;
    std::vector<unsigned char> ciphertext(ciphertext_len);
    const int encrypt_status = crypto_secretbox_easy(
        ciphertext.data(), data_char.data(), data_char_sz, nonce, ss);
    sodium_memzero(ss, sizeof ss);

    if (encrypt_status != 0) {
        std::cout << "Failed to encrypt message." << std::endl;
        return;
    }

    std::vector<uint8_t> kem_ct_vec(crypto_kem_CIPHERTEXTBYTES);
    for (size_t k = 0; k < crypto_kem_CIPHERTEXTBYTES; k++) {
        kem_ct_vec[k] = static_cast<uint8_t>(kem_ct[k]);
    }

    std::vector<uint8_t> msg_encrypted(ciphertext_len);
    for (size_t k = 0; k < ciphertext_len; k++) {
        msg_encrypted[k] = static_cast<uint8_t>(ciphertext[k]);
    }

    std::vector<uint8_t> nonce_vec(crypto_secretbox_NONCEBYTES);
    for (size_t k = 0; k < crypto_secretbox_NONCEBYTES; k++) {
        nonce_vec[k] = static_cast<uint8_t>(nonce[k]);
    }

    pmt::pmt_t msg_out_kem_ct = pmt::list2(
        pmt::string_to_symbol("kem_ciphertext"),
        pmt::init_u8vector(kem_ct_vec.size(), kem_ct_vec));
    pmt::pmt_t msg_out_nonce = pmt::list2(
        pmt::string_to_symbol("nonce"), pmt::init_u8vector(nonce_vec.size(), nonce_vec));
    pmt::pmt_t msg_out_data = pmt::list2(
        pmt::string_to_symbol("msg_encrypted"),
        pmt::init_u8vector(msg_encrypted.size(), msg_encrypted));

    message_port_pub(d_port_id_out, pmt::list3(msg_out_kem_ct, msg_out_nonce, msg_out_data));
}

encrypt_kem_impl::~encrypt_kem_impl() { delete[] d_pk; }

} /* namespace nacl */
} /* namespace gr */
