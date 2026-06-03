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

#ifndef INCLUDED_NACL_HASH_SHA3_IMPL_H
#define INCLUDED_NACL_HASH_SHA3_IMPL_H

#include <nacl/hash_sha3.h>
#include <pmt/pmt.h>

namespace gr {
namespace nacl {

class hash_sha3_impl : public hash_sha3
{
public:
    hash_sha3_impl(bool sha3_512);
    ~hash_sha3_impl();

    void handle_msg(pmt::pmt_t msg);

    pmt::pmt_t d_port_id_in, d_port_id_out;
    bool d_sha3_512;
};

} // namespace nacl
} // namespace gr

#endif /* INCLUDED_NACL_HASH_SHA3_IMPL_H */
