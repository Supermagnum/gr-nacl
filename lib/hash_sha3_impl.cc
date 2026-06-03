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
#include "hash_sha3_impl.h"

#include <functional>
#include <iostream>
#include <vector>
#include <sodium.h>

namespace gr {
namespace nacl {

hash_sha3::sptr hash_sha3::make(bool sha3_512) { return gnuradio::get_initial_sptr(new hash_sha3_impl(sha3_512)); }

hash_sha3_impl::hash_sha3_impl(bool sha3_512)
    : gr::block("hash_sha3",
                gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0)),
      d_sha3_512(sha3_512)
{
    d_port_id_in = pmt::mp("Msg in");
    message_port_register_in(d_port_id_in);
    set_msg_handler(d_port_id_in, [this](pmt::pmt_t msg) { this->handle_msg(msg); });

    d_port_id_out = pmt::mp("Msg hash");
    message_port_register_out(d_port_id_out);
}

hash_sha3_impl::~hash_sha3_impl() {}

void hash_sha3_impl::handle_msg(pmt::pmt_t msg)
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

    const size_t digest_len =
        d_sha3_512 ? crypto_hash_sha3512_BYTES : crypto_hash_sha3256_BYTES;
    std::vector<uint8_t> digest(digest_len);

    int hash_status;
    if (d_sha3_512) {
        hash_status = crypto_hash_sha3512(
            digest.data(), data.data(), data.size());
    } else {
        hash_status = crypto_hash_sha3256(
            digest.data(), data.data(), data.size());
    }

    if (hash_status != 0) {
        std::cout << "Failed to compute SHA3 hash." << std::endl;
        return;
    }

    const char* digest_tag = d_sha3_512 ? "hash_sha3_512" : "hash_sha3_256";
    pmt::pmt_t msg_out = pmt::list2(
        pmt::string_to_symbol(digest_tag),
        pmt::init_u8vector(digest.size(), digest));

    message_port_pub(d_port_id_out, pmt::list1(msg_out));
}

} /* namespace nacl */
} /* namespace gr */
