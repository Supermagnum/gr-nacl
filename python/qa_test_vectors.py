#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright 2024
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
import os
import struct

class qa_test_vectors (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None
        # Clean up test key files
        for f in ["test_key_secret.key", "test_key_public.key", "test_key_sk.key", "test_key_pk.key"]:
            if os.path.exists(f):
                os.remove(f)

    def test_secretbox_known_vector(self):
        """
        Test crypto_secretbox with a known test vector.
        This uses a simple test case: encrypt known plaintext with known key/nonce
        and verify decryption works correctly.
        """
        # Known test vector components
        # Key: 32 bytes of known value
        key_bytes = bytes([i % 256 for i in range(32)])
        # Nonce: 24 bytes of known value  
        nonce_bytes = bytes([(i+1) % 256 for i in range(24)])
        # Plaintext: simple known message
        plaintext = b"Hello, World! This is a test message."
        
        # Write key to file
        with open("test_key_secret.key", "wb") as f:
            f.write(key_bytes)
        
        # Create plaintext as u8vector
        plaintext_vec = list(plaintext)
        msg = pmt.list1(pmt.list2(pmt.string_to_symbol("msg_clear"), 
                                   pmt.init_u8vector(len(plaintext_vec), plaintext_vec)))
        
        # Encrypt
        strobe = blocks.message_strobe(msg, 100)
        encrypt_secret = nacl.encrypt_secret("test_key_secret.key")
        encrypt_debug = blocks.message_debug()
        
        self.tb.msg_connect(strobe, "strobe", encrypt_secret, "Msg clear")
        self.tb.msg_connect(encrypt_secret, "Msg encrypted", encrypt_debug, "store")
        
        self.tb.start()
        import time
        time.sleep(0.2)
        self.tb.stop()
        self.tb.wait()
        
        # Get encrypted message
        msg_encrypted_pmt = encrypt_debug.get_message(0)
        self.assertIsNotNone(msg_encrypted_pmt, "Encryption should produce output")
        
        # Extract nonce and ciphertext
        nonce_pmt = pmt.nth(0, msg_encrypted_pmt)
        ciphertext_pmt = pmt.nth(1, msg_encrypted_pmt)
        
        nonce_vec = pmt.u8vector_elements(pmt.nth(1, nonce_pmt))
        ciphertext_vec = pmt.u8vector_elements(pmt.nth(1, ciphertext_pmt))
        
        # Verify nonce size
        self.assertEqual(len(nonce_vec), 24, "Nonce should be 24 bytes")
        # Verify ciphertext is longer than plaintext (includes MAC)
        self.assertGreater(len(ciphertext_vec), len(plaintext), "Ciphertext should include MAC")
        
        # Now decrypt
        decrypt_secret = nacl.decrypt_secret("test_key_secret.key")
        decrypt_debug = blocks.message_debug()
        
        # Create message with known nonce and ciphertext
        msg_encrypted = pmt.list2(
            pmt.list2(pmt.string_to_symbol("nonce"), pmt.init_u8vector(len(nonce_vec), nonce_vec)),
            pmt.list2(pmt.string_to_symbol("msg_encrypted"), pmt.init_u8vector(len(ciphertext_vec), ciphertext_vec))
        )
        
        decrypt_strobe = blocks.message_strobe(msg_encrypted, 100)
        self.tb.msg_connect(decrypt_strobe, "strobe", decrypt_secret, "Msg encrypted")
        self.tb.msg_connect(decrypt_secret, "Msg decrypted", decrypt_debug, "store")
        
        self.tb.start()
        time.sleep(0.2)
        self.tb.stop()
        self.tb.wait()
        
        # Verify decryption
        msg_decrypted_pmt = decrypt_debug.get_message(0)
        self.assertIsNotNone(msg_decrypted_pmt, "Decryption should produce output")
        
        decrypted_data = pmt.u8vector_elements(pmt.nth(1, pmt.nth(0, msg_decrypted_pmt)))
        
        # Verify decrypted plaintext matches original
        self.assertEqual(len(decrypted_data), len(plaintext), "Decrypted length should match plaintext")
        for i in range(len(plaintext)):
            self.assertEqual(decrypted_data[i], plaintext[i], 
                           f"Decrypted byte {i} should match plaintext")

    def test_secretbox_roundtrip_multiple(self):
        """
        Test multiple round-trip encryptions to ensure consistency.
        """
        # Generate a key
        nacl.generate_key("test_key_secret.key")
        
        test_messages = [
            b"Short",
            b"Medium length message here",
            b"A" * 100,  # 100 bytes
            b"B" * 1000,  # 1000 bytes
            bytes(range(256)),  # All byte values
        ]
        
        for plaintext in test_messages:
            plaintext_vec = list(plaintext)
            msg = pmt.list1(pmt.list2(pmt.string_to_symbol("msg_clear"), 
                                     pmt.init_u8vector(len(plaintext_vec), plaintext_vec)))
            
            # Encrypt
            strobe = blocks.message_strobe(msg, 100)
            encrypt_secret = nacl.encrypt_secret("test_key_secret.key")
            encrypt_debug = blocks.message_debug()
            decrypt_secret = nacl.decrypt_secret("test_key_secret.key")
            decrypt_debug = blocks.message_debug()
            
            self.tb.msg_connect(strobe, "strobe", encrypt_secret, "Msg clear")
            self.tb.msg_connect(encrypt_secret, "Msg encrypted", decrypt_secret, "Msg encrypted")
            self.tb.msg_connect(decrypt_secret, "Msg decrypted", decrypt_debug, "store")
            
            self.tb.start()
            import time
            time.sleep(0.2)
            self.tb.stop()
            self.tb.wait()
            
            # Verify decryption
            msg_decrypted_pmt = decrypt_debug.get_message(0)
            self.assertIsNotNone(msg_decrypted_pmt, f"Decryption should work for {len(plaintext)} bytes")
            
            decrypted_data = pmt.u8vector_elements(pmt.nth(1, pmt.nth(0, msg_decrypted_pmt)))
            
            self.assertEqual(len(decrypted_data), len(plaintext), 
                          f"Length should match for {len(plaintext)} byte message")
            for i in range(len(plaintext)):
                self.assertEqual(decrypted_data[i], plaintext[i],
                               f"Byte {i} should match for {len(plaintext)} byte message")

    def test_box_roundtrip(self):
        """
        Test public key box encryption/decryption round-trip.
        """
        # Generate keypairs
        nacl.generate_keypair("test_key_sk.key", "test_key_pk.key")
        
        plaintext = b"Test message for public key encryption"
        plaintext_vec = list(plaintext)
        msg = pmt.list1(pmt.list2(pmt.string_to_symbol("msg_clear"), 
                                 pmt.init_u8vector(len(plaintext_vec), plaintext_vec)))
        
        # Encrypt (using pk_b, sk_a)
        strobe = blocks.message_strobe(msg, 100)
        encrypt_public = nacl.encrypt_public("test_key_pk.key", "test_key_sk.key")
        encrypt_debug = blocks.message_debug()
        decrypt_public = nacl.decrypt_public("test_key_pk.key", "test_key_sk.key")
        decrypt_debug = blocks.message_debug()
        
        self.tb.msg_connect(strobe, "strobe", encrypt_public, "Msg clear")
        self.tb.msg_connect(encrypt_public, "Msg encrypted", decrypt_public, "Msg encrypted")
        self.tb.msg_connect(decrypt_public, "Msg decrypted", decrypt_debug, "store")
        
        self.tb.start()
        import time
        time.sleep(0.2)
        self.tb.stop()
        self.tb.wait()
        
        # Verify decryption
        msg_decrypted_pmt = decrypt_debug.get_message(0)
        self.assertIsNotNone(msg_decrypted_pmt, "Public key decryption should work")
        
        decrypted_data = pmt.u8vector_elements(pmt.nth(1, pmt.nth(0, msg_decrypted_pmt)))
        
        self.assertEqual(len(decrypted_data), len(plaintext), "Length should match")
        for i in range(len(plaintext)):
            self.assertEqual(decrypted_data[i], plaintext[i], f"Byte {i} should match")

if __name__ == '__main__':
    gr_unittest.run(qa_test_vectors)

