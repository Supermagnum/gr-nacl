function(gr_nacl4_add_ut name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE gnuradio4::gnuradio-options gnuradio4::gnuradio-core gnuradio4::gnuradio-blocklib-core
                                          gnuradio4::gr-nacl ut)
  add_test(NAME ${name} COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:${name}>)
endfunction()

gr_nacl4_add_ut(qa_CryptTaggedStream)
gr_nacl4_add_ut(qa_EncryptSecret)
gr_nacl4_add_ut(qa_DecryptSecret)
gr_nacl4_add_ut(qa_EncryptPublic)
gr_nacl4_add_ut(qa_DecryptPublic)
gr_nacl4_add_ut(qa_GenerateSymmetricKey)
gr_nacl4_add_ut(qa_GeneratePublicKeypair)
