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

#ifndef INCLUDED_NACL_DECRYPT_KEM_H
#define INCLUDED_NACL_DECRYPT_KEM_H

#include <nacl/api.h>
#include <gnuradio/block.h>
#include <memory>
#include <string>

namespace gr {
namespace nacl {

/*!
 * \brief Decrypt a KEM-encrypted PDU message with an X-Wing secret key.
 * \ingroup nacl
 */
class NACL_API decrypt_kem : virtual public gr::block
{
public:
    typedef std::shared_ptr<decrypt_kem> sptr;

    static sptr make(std::string filename_sk);
};

} // namespace nacl
} // namespace gr

#endif /* INCLUDED_NACL_DECRYPT_KEM_H */
