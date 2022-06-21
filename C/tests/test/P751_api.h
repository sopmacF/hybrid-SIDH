/********************************************************************************************
* SIDH: an efficient supersingular isogeny-based cryptography library
*
*    Copyright (c) Microsoft Corporation. All rights reserved.
*
*
* Abstract: API header file for P751
*
*********************************************************************************************/  

#ifndef __P751_API_H__
#define __P751_API_H__

#include "../config.h"
    

/*********************** Key encapsulation mechanism API ***********************/

#define CRYPTO_SECRETKEYBYTES     660    // CRYPTO_BYTES + SECRETKEY_B_BYTES + CRYPTO_PUBLICKEYBYTES bytes
#define CRYPTO_PUBLICKEYBYTES     564
#define CRYPTO_BYTES               48
#define CRYPTO_CIPHERTEXTBYTES    612    // CRYPTO_PUBLICKEYBYTES + CRYPTO_BYTES bytes

// SIKE's key generation
// It produces a private key sk and computes the public key pk.
// Outputs: secret key sk (CRYPTO_SECRETKEYBYTES = 660 bytes)
//          public key pk (CRYPTO_PUBLICKEYBYTES = 564 bytes) 
int crypto_kem_keypair(unsigned char *pk, unsigned char *sk);

// SIKE's encapsulation
// Input:   public key pk         (CRYPTO_PUBLICKEYBYTES = 564 bytes)
// Outputs: shared secret ss      (CRYPTO_BYTES = 48 bytes)
//          ciphertext message ct (CRYPTO_CIPHERTEXTBYTES = 612 bytes) 
int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk);

// SIKE's decapsulation
// Input:   secret key sk         (CRYPTO_SECRETKEYBYTES = 660 bytes)
//          ciphertext message ct (CRYPTO_CIPHERTEXTBYTES = 410 bytes) 
// Outputs: shared secret ss      (CRYPTO_BYTES = 32 bytes)
int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk);


// Encoding of keys for KEM-based isogeny system "SIKEp751" (wire format):
// ----------------------------------------------------------------------
// Elements over GF(p751) are encoded in 94 octets in little endian format (i.e., the least significant octet is located in the lowest memory address). 
// Elements (a+b*i) over GF(p751^2), where a and b are defined over GF(p751), are encoded as {a, b}, with a in the lowest memory portion.
//
// Private keys sk consist of the concatenation of a 48-byte random value and a value in the range [0, 2^378-1]. In the SIDH API, private keys are 
// encoded in 96 octets in little endian format (the top 6 bits are zeroes). 
// Public keys pk consist of 3 elements in GF(p751^2). In the SIDH API, they are encoded in 564 octets. 
// Ciphertexts ct consist of the concatenation of a public key value and a 48-byte value. In the SIDH API, ct is encoded in 564 + 48 = 612 octets.  
// Shared keys ss consist of a value of 48 octets.


/*********************** Ephemeral key exchange API ***********************/

#define SIDH_SECRETKEYBYTES      48
#define SIDH_PUBLICKEYBYTES     564
#define SIDH_BYTES              188 

// SECURITY NOTE: SIDH supports ephemeral Diffie-Hellman key exchange. It is NOT secure to use it with static keys.
// See "On the Security of Supersingular Isogeny Cryptosystems", S.D. Galbraith, C. Petit, B. Shani and Y.B. Ti, in ASIACRYPT 2016, 2016.
// Extended version available at: http://eprint.iacr.org/2016/859     

// Generation of Alice's secret key 
// Outputs random value in [0, 2^eA - 1] to be used as Alice's private key
void random_mod_order_A(unsigned char* random_digits);

// Generation of Bob's secret key 
// Outputs random value in [0, 2^Floor(Log(2, oB)) - 1] to be used as Bob's private key
void random_mod_order_B(unsigned char* random_digits);

// Alice's ephemeral public key generation
// Input:  a private key PrivateKeyA in the range [0, oA-1], where oA = 2^372, stored in 47 bytes. 
// Output: the public key PublicKeyA consisting of 3 GF(p751^2) elements encoded in 564 bytes.
int EphemeralKeyGeneration_A(const unsigned char* PrivateKeyA, unsigned char* PublicKeyA);

// Bob's ephemeral key-pair generation
// It produces a private key PrivateKeyB and computes the public key PublicKeyB.
// The private key is an integer in the range [0, 2^Floor(Log(2,oB)) - 1], where oB = 3^239, stored in 48 bytes.  
// The public key consists of 3 GF(p751^2) elements encoded in 564 bytes.
int EphemeralKeyGeneration_B(const unsigned char* PrivateKeyB, unsigned char* PublicKeyB);

// Alice's ephemeral shared secret computation
// It produces a shared secret key SharedSecretA using her secret key PrivateKeyA and Bob's public key PublicKeyB
// Inputs: Alice's PrivateKeyA is an integer in the range [0, oA-1], where oA = 2^372, stored in 47 bytes. 
//         Bob's PublicKeyB consists of 3 GF(p751^2) elements encoded in 564 bytes.
// Output: a shared secret SharedSecretA that consists of one element in GF(p751^2) encoded in 188 bytes.
int EphemeralSecretAgreement_A(const unsigned char* PrivateKeyA, const unsigned char* PublicKeyB, unsigned char* SharedSecretA);

// Bob's ephemeral shared secret computation
// It produces a shared secret key SharedSecretB using his secret key PrivateKeyB and Alice's public key PublicKeyA
// Inputs: Bob's PrivateKeyB is an integer in the range [0, 2^Floor(Log(2,oB)) - 1], where oB = 3^239, stored in 48 bytes.  
//         Alice's PublicKeyA consists of 3 GF(p751^2) elements encoded in 564 bytes.
// Output: a shared secret SharedSecretB that consists of one element in GF(p751^2) encoded in 188 bytes. 
int EphemeralSecretAgreement_B(const unsigned char* PrivateKeyB, const unsigned char* PublicKeyA, unsigned char* SharedSecretB);


// Encoding of keys for KEX-based isogeny system "SIDHp751" (wire format):
// ----------------------------------------------------------------------
// Elements over GF(p751) are encoded in 94 octets in little endian format (i.e., the least significant octet is located in the lowest memory address). 
// Elements (a+b*i) over GF(p751^2), where a and b are defined over GF(p751), are encoded as {a, b}, with a in the lowest memory portion.
//
// Private keys PrivateKeyA and PrivateKeyB can have values in the range [0, 2^372-1] and [0, 2^378-1], resp. In the SIDH API, private keys are encoded 
// in 48 octets in little endian format. 
// Public keys PublicKeyA and PublicKeyB consist of 3 elements in GF(p751^2). In the SIDH API, they are encoded in 564 octets. 
// Shared keys SharedSecretA and SharedSecretB consist of one element in GF(p751^2). In the SIDH API, they are encoded in 188 octets.


#endif
