/* -*- c++ -*- */
/*
 * Copyright 2024
 *
 * This file is part of gr-nacl.
 *
 * gr-nacl is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * gr-nacl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gr-nacl; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <nacl/encrypt_public.h>
#include <nacl/decrypt_public.h>
#include <nacl/encrypt_secret.h>
#include <nacl/decrypt_secret.h>
#include <nacl/generate_keypair.h>
#include <nacl/generate_key.h>
#include <nacl/crypt_tagged_stream.h>

namespace py = pybind11;

void bind_encrypt_public(py::module& m)
{
    using encrypt_public = gr::nacl::encrypt_public;

    py::class_<encrypt_public, gr::block, std::shared_ptr<encrypt_public>>(
        m, "encrypt_public")
        .def(py::init(&encrypt_public::make),
             py::arg("filename_pk"),
             py::arg("filename_sk"));
}

void bind_decrypt_public(py::module& m)
{
    using decrypt_public = gr::nacl::decrypt_public;

    py::class_<decrypt_public, gr::block, std::shared_ptr<decrypt_public>>(
        m, "decrypt_public")
        .def(py::init(&decrypt_public::make),
             py::arg("filename_pk"),
             py::arg("filename_sk"));
}

void bind_encrypt_secret(py::module& m)
{
    using encrypt_secret = gr::nacl::encrypt_secret;

    py::class_<encrypt_secret, gr::block, std::shared_ptr<encrypt_secret>>(
        m, "encrypt_secret")
        .def(py::init(&encrypt_secret::make),
             py::arg("filename_key"));
}

void bind_decrypt_secret(py::module& m)
{
    using decrypt_secret = gr::nacl::decrypt_secret;

    py::class_<decrypt_secret, gr::block, std::shared_ptr<decrypt_secret>>(
        m, "decrypt_secret")
        .def(py::init(&decrypt_secret::make),
             py::arg("filename_key"));
}

void bind_generate_keypair(py::module& m)
{
    using generate_keypair = gr::nacl::generate_keypair;

    py::class_<generate_keypair, gr::block, std::shared_ptr<generate_keypair>>(
        m, "generate_keypair")
        .def(py::init(&generate_keypair::make),
             py::arg("filename_sk"),
             py::arg("filename_pk"));
}

void bind_generate_key(py::module& m)
{
    using generate_key = gr::nacl::generate_key;

    py::class_<generate_key, gr::block, std::shared_ptr<generate_key>>(
        m, "generate_key")
        .def(py::init(&generate_key::make),
             py::arg("filename_key"));
}

void bind_crypt_tagged_stream(py::module& m)
{
    using crypt_tagged_stream = gr::nacl::crypt_tagged_stream;

    py::class_<crypt_tagged_stream, gr::tagged_stream_block, std::shared_ptr<crypt_tagged_stream>>(
        m, "crypt_tagged_stream")
        .def(py::init(&crypt_tagged_stream::make),
             py::arg("key"),
             py::arg("nonce"),
             py::arg("rotate_nonce") = false,
             py::arg("len_key") = "packet_len");
}

PYBIND11_MODULE(nacl_python, m)
{
    m.doc() = "Python bindings for gr-nacl";

    bind_encrypt_public(m);
    bind_decrypt_public(m);
    bind_encrypt_secret(m);
    bind_decrypt_secret(m);
    bind_generate_keypair(m);
    bind_generate_key(m);
    bind_crypt_tagged_stream(m);
}

