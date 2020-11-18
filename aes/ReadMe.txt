Doxygen Format
/** @pagegroup{pageExamples,pageExamples-aes,aes}

A basic description of AES can be obtained from Wikipedia, and if
 you are new to AES, you should definitely do some research to
 understand the implementation. The description below is from
 Wikipedia:
 
 In cryptography, a block cipher is a symmetric key cipher which
 operates on fixed-length groups of bits, termed blocks, with an
 unvarying transformation. When encrypting, a block cipher might
 take a (for example) 16-bit block of plaintext as input, and
 output a corresponding 16-bit block of ciphertext. The exact
 transformation is controlled using a second input - the secret key.
 Decryption is similar: the decryption algorithm takes, in this example,
 a 16-bit block of ciphertext together with the secret key, and
 yields the original 16-bit block of plaintext.
 
 To encrypt messages longer than the block size, a mode of operation is used.
 Block ciphers can be contrasted with stream ciphers; a stream cipher
 operates on individual digits one at a time, and the transformation
 varies during the encryption. The distinction between the two types is
 not always clear-cut: a block cipher, when used in certain modes of
 operation, acts effectively as a stream cipher.
 
 This example demonstrates how to use the AES key, encryption, and
 decryption function calls on a 16 byte block of data. Since AES is
 a block cipher, all data must be encrypted in 16 byte blocks. If you
 have more than 16 bytes to transfer, you must break the data into
 16 byte blocks, encrypt them (you can use the same key for all
 blocks), send them, then decrypt them in 16 byte blocks on the other
 end.

*/