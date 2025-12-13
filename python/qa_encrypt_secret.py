#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright 2015 Stefan Wunsch
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import nacl_python as nacl
import pmt
from time import sleep

class qa_encrypt_secret (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        data = [ord('t'),ord('e'),ord('s'),ord('t')]
        msg = pmt.list1(pmt.list2(pmt.string_to_symbol("msg_clear"),pmt.init_u8vector(len(data),data)))
        filename_key = "secret.key"
        nacl.generate_key(filename_key)
        
        strobe = blocks.message_strobe(msg, 100)
        encrypt_secret = nacl.encrypt_secret(filename_key)
        debug = blocks.message_debug()
        
        self.tb.msg_connect(strobe,"strobe",encrypt_secret,"Msg clear")
        self.tb.msg_connect(encrypt_secret,"Msg encrypted",debug,"store")
        
        self.tb.start()
        sleep(0.15)
        self.tb.stop()
        self.tb.wait()
        
        # check results
        msg_stored = debug.get_message(0)
        self.assertIsNotNone(msg_stored, "Encrypted message should be produced")
        
        nonce = pmt.nth(0,msg_stored)
        msg_encrypted = pmt.nth(1,msg_stored)
        
        # Verify nonce is present and has correct size
        self.assertEqual(pmt.symbol_to_string(pmt.nth(0,nonce)), "nonce", "Nonce should be tagged correctly")
        nonce_vec = pmt.u8vector_elements(pmt.nth(1,nonce))
        self.assertEqual(len(nonce_vec), 24, "Nonce should be 24 bytes (crypto_secretbox_NONCEBYTES)")
        
        # Verify encrypted message is present and different from original
        self.assertEqual(pmt.symbol_to_string(pmt.nth(0,msg_encrypted)), "msg_encrypted", "Message should be tagged as encrypted")
        encrypted_vec = pmt.u8vector_elements(pmt.nth(1,msg_encrypted))
        self.assertGreater(len(encrypted_vec), len(data), "Encrypted message should be longer than original (includes MAC)")
        # Encrypted data should be different from original
        self.assertNotEqual(encrypted_vec[:len(data)], data, "Encrypted data should differ from original")


if __name__ == '__main__':
    gr_unittest.run(qa_encrypt_secret)#, "qa_encrypt_secret.xml")
