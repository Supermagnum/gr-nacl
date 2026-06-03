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
#include "generate_kem_keypair_impl.h"

#include <fstream>
#include <iostream>
#include <sodium.h>

namespace gr {
namespace nacl {

generate_kem_keypair::sptr
generate_kem_keypair::make(std::string filename_sk, std::string filename_pk)
{
    return gnuradio::get_initial_sptr(
        new generate_kem_keypair_impl(filename_sk, filename_pk));
}

generate_kem_keypair_impl::generate_kem_keypair_impl(std::string filename_sk,
                                                     std::string filename_pk)
    : gr::block("generate_kem_keypair",
                gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0))
{
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium.");
    }

    unsigned char pk[crypto_kem_PUBLICKEYBYTES];
    unsigned char sk[crypto_kem_SECRETKEYBYTES];

    if (crypto_kem_keypair(pk, sk) != 0) {
        throw std::runtime_error("Failed to generate KEM keypair.");
    }

    std::cout << "KEM keypair [" << filename_sk << ", " << filename_pk
              << "] generated successfully." << std::endl;

    std::ofstream file_sk(filename_sk.c_str(), std::ios::binary);
    if (!file_sk.is_open()) {
        throw std::runtime_error("Failed to open secret-key file for writing.");
    }
    file_sk.write(reinterpret_cast<const char*>(sk), crypto_kem_SECRETKEYBYTES);
    file_sk.close();

    std::ofstream file_pk(filename_pk.c_str(), std::ios::binary);
    if (!file_pk.is_open()) {
        throw std::runtime_error("Failed to open public-key file for writing.");
    }
    file_pk.write(reinterpret_cast<const char*>(pk), crypto_kem_PUBLICKEYBYTES);
    file_pk.close();

    std::cout << "KEM keypair [" << filename_sk << ", " << filename_pk
              << "] saved to file." << std::endl;

    sodium_memzero(pk, sizeof pk);
    sodium_memzero(sk, sizeof sk);
}

generate_kem_keypair_impl::~generate_kem_keypair_impl() {}

} /* namespace nacl */
} /* namespace gr */
