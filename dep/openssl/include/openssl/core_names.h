/*
 * WARNING: do not edit!
 * Generated by Makefile from include/openssl/core_names.h.in
 *
 * Copyright 2019-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */


#ifndef OPENSSL_CORE_NAMES_H
# define OPENSSL_CORE_NAMES_H
# pragma once

# ifdef __cplusplus
extern "C" {
# endif

/* OSSL_CIPHER_PARAM_CTS_MODE Values */
# define OSSL_CIPHER_CTS_MODE_CS1 "CS1"
# define OSSL_CIPHER_CTS_MODE_CS2 "CS2"
# define OSSL_CIPHER_CTS_MODE_CS3 "CS3"

/* Known CIPHER names (not a complete list) */
# define OSSL_CIPHER_NAME_AES_128_GCM_SIV      "AES-128-GCM-SIV"
# define OSSL_CIPHER_NAME_AES_192_GCM_SIV      "AES-192-GCM-SIV"
# define OSSL_CIPHER_NAME_AES_256_GCM_SIV      "AES-256-GCM-SIV"

/* Known DIGEST names (not a complete list) */
# define OSSL_DIGEST_NAME_MD5            "MD5"
# define OSSL_DIGEST_NAME_MD5_SHA1       "MD5-SHA1"
# define OSSL_DIGEST_NAME_SHA1           "SHA1"
# define OSSL_DIGEST_NAME_SHA2_224       "SHA2-224"
# define OSSL_DIGEST_NAME_SHA2_256       "SHA2-256"
# define OSSL_DIGEST_NAME_SHA2_256_192   "SHA2-256/192"
# define OSSL_DIGEST_NAME_SHA2_384       "SHA2-384"
# define OSSL_DIGEST_NAME_SHA2_512       "SHA2-512"
# define OSSL_DIGEST_NAME_SHA2_512_224   "SHA2-512/224"
# define OSSL_DIGEST_NAME_SHA2_512_256   "SHA2-512/256"
# define OSSL_DIGEST_NAME_MD2            "MD2"
# define OSSL_DIGEST_NAME_MD4            "MD4"
# define OSSL_DIGEST_NAME_MDC2           "MDC2"
# define OSSL_DIGEST_NAME_RIPEMD160      "RIPEMD160"
# define OSSL_DIGEST_NAME_SHA3_224       "SHA3-224"
# define OSSL_DIGEST_NAME_SHA3_256       "SHA3-256"
# define OSSL_DIGEST_NAME_SHA3_384       "SHA3-384"
# define OSSL_DIGEST_NAME_SHA3_512       "SHA3-512"
# define OSSL_DIGEST_NAME_KECCAK_KMAC128 "KECCAK-KMAC-128"
# define OSSL_DIGEST_NAME_KECCAK_KMAC256 "KECCAK-KMAC-256"
# define OSSL_DIGEST_NAME_SM3            "SM3"

/* Known MAC names */
# define OSSL_MAC_NAME_BLAKE2BMAC    "BLAKE2BMAC"
# define OSSL_MAC_NAME_BLAKE2SMAC    "BLAKE2SMAC"
# define OSSL_MAC_NAME_CMAC          "CMAC"
# define OSSL_MAC_NAME_GMAC          "GMAC"
# define OSSL_MAC_NAME_HMAC          "HMAC"
# define OSSL_MAC_NAME_KMAC128       "KMAC128"
# define OSSL_MAC_NAME_KMAC256       "KMAC256"
# define OSSL_MAC_NAME_POLY1305      "POLY1305"
# define OSSL_MAC_NAME_SIPHASH       "SIPHASH"

/* Known KDF names */
# define OSSL_KDF_NAME_HKDF           "HKDF"
# define OSSL_KDF_NAME_TLS1_3_KDF     "TLS13-KDF"
# define OSSL_KDF_NAME_PBKDF1         "PBKDF1"
# define OSSL_KDF_NAME_PBKDF2         "PBKDF2"
# define OSSL_KDF_NAME_SCRYPT         "SCRYPT"
# define OSSL_KDF_NAME_SSHKDF         "SSHKDF"
# define OSSL_KDF_NAME_SSKDF          "SSKDF"
# define OSSL_KDF_NAME_TLS1_PRF       "TLS1-PRF"
# define OSSL_KDF_NAME_X942KDF_ASN1   "X942KDF-ASN1"
# define OSSL_KDF_NAME_X942KDF_CONCAT "X942KDF-CONCAT"
# define OSSL_KDF_NAME_X963KDF        "X963KDF"
# define OSSL_KDF_NAME_KBKDF          "KBKDF"
# define OSSL_KDF_NAME_KRB5KDF        "KRB5KDF"
# define OSSL_KDF_NAME_HMACDRBGKDF    "HMAC-DRBG-KDF"

/* RSA padding modes */
# define OSSL_PKEY_RSA_PAD_MODE_NONE    "none"
# define OSSL_PKEY_RSA_PAD_MODE_PKCSV15 "pkcs1"
# define OSSL_PKEY_RSA_PAD_MODE_OAEP    "oaep"
# define OSSL_PKEY_RSA_PAD_MODE_X931    "x931"
# define OSSL_PKEY_RSA_PAD_MODE_PSS     "pss"

/* RSA pss padding salt length */
# define OSSL_PKEY_RSA_PSS_SALT_LEN_DIGEST "digest"
# define OSSL_PKEY_RSA_PSS_SALT_LEN_MAX    "max"
# define OSSL_PKEY_RSA_PSS_SALT_LEN_AUTO   "auto"
# define OSSL_PKEY_RSA_PSS_SALT_LEN_AUTO_DIGEST_MAX "auto-digestmax"

/* OSSL_PKEY_PARAM_EC_ENCODING values */
# define OSSL_PKEY_EC_ENCODING_EXPLICIT  "explicit"
# define OSSL_PKEY_EC_ENCODING_GROUP     "named_curve"

# define OSSL_PKEY_EC_POINT_CONVERSION_FORMAT_UNCOMPRESSED "uncompressed"
# define OSSL_PKEY_EC_POINT_CONVERSION_FORMAT_COMPRESSED   "compressed"
# define OSSL_PKEY_EC_POINT_CONVERSION_FORMAT_HYBRID       "hybrid"

# define OSSL_PKEY_EC_GROUP_CHECK_DEFAULT     "default"
# define OSSL_PKEY_EC_GROUP_CHECK_NAMED       "named"
# define OSSL_PKEY_EC_GROUP_CHECK_NAMED_NIST  "named-nist"

/* OSSL_KEM_PARAM_OPERATION values */
#define OSSL_KEM_PARAM_OPERATION_RSASVE     "RSASVE"
#define OSSL_KEM_PARAM_OPERATION_DHKEM      "DHKEM"

/* Parameter name definitions - generated by util/perl/OpenSSL/paramnames.pm */
# define OSSL_ALG_PARAM_CIPHER "cipher"
# define OSSL_ALG_PARAM_DIGEST "digest"
# define OSSL_ALG_PARAM_ENGINE "engine"
# define OSSL_ALG_PARAM_MAC "mac"
# define OSSL_ALG_PARAM_PROPERTIES "properties"
# define OSSL_ASYM_CIPHER_PARAM_DIGEST OSSL_PKEY_PARAM_DIGEST
# define OSSL_ASYM_CIPHER_PARAM_ENGINE OSSL_PKEY_PARAM_ENGINE
# define OSSL_ASYM_CIPHER_PARAM_IMPLICIT_REJECTION "implicit-rejection"
# define OSSL_ASYM_CIPHER_PARAM_MGF1_DIGEST OSSL_PKEY_PARAM_MGF1_DIGEST
# define OSSL_ASYM_CIPHER_PARAM_MGF1_DIGEST_PROPS OSSL_PKEY_PARAM_MGF1_PROPERTIES
# define OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST OSSL_ALG_PARAM_DIGEST
# define OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST_PROPS "digest-props"
# define OSSL_ASYM_CIPHER_PARAM_OAEP_LABEL "oaep-label"
# define OSSL_ASYM_CIPHER_PARAM_PAD_MODE OSSL_PKEY_PARAM_PAD_MODE
# define OSSL_ASYM_CIPHER_PARAM_PROPERTIES OSSL_PKEY_PARAM_PROPERTIES
# define OSSL_ASYM_CIPHER_PARAM_TLS_CLIENT_VERSION "tls-client-version"
# define OSSL_ASYM_CIPHER_PARAM_TLS_NEGOTIATED_VERSION "tls-negotiated-version"
# define OSSL_CAPABILITY_TLS_GROUP_ALG "tls-group-alg"
# define OSSL_CAPABILITY_TLS_GROUP_ID "tls-group-id"
# define OSSL_CAPABILITY_TLS_GROUP_IS_KEM "tls-group-is-kem"
# define OSSL_CAPABILITY_TLS_GROUP_MAX_DTLS "tls-max-dtls"
# define OSSL_CAPABILITY_TLS_GROUP_MAX_TLS "tls-max-tls"
# define OSSL_CAPABILITY_TLS_GROUP_MIN_DTLS "tls-min-dtls"
# define OSSL_CAPABILITY_TLS_GROUP_MIN_TLS "tls-min-tls"
# define OSSL_CAPABILITY_TLS_GROUP_NAME "tls-group-name"
# define OSSL_CAPABILITY_TLS_GROUP_NAME_INTERNAL "tls-group-name-internal"
# define OSSL_CAPABILITY_TLS_GROUP_SECURITY_BITS "tls-group-sec-bits"
# define OSSL_CAPABILITY_TLS_SIGALG_CODE_POINT "tls-sigalg-code-point"
# define OSSL_CAPABILITY_TLS_SIGALG_HASH_NAME "tls-sigalg-hash-name"
# define OSSL_CAPABILITY_TLS_SIGALG_HASH_OID "tls-sigalg-hash-oid"
# define OSSL_CAPABILITY_TLS_SIGALG_IANA_NAME "tls-sigalg-iana-name"
# define OSSL_CAPABILITY_TLS_SIGALG_KEYTYPE "tls-sigalg-keytype"
# define OSSL_CAPABILITY_TLS_SIGALG_KEYTYPE_OID "tls-sigalg-keytype-oid"
# define OSSL_CAPABILITY_TLS_SIGALG_MAX_TLS "tls-max-tls"
# define OSSL_CAPABILITY_TLS_SIGALG_MIN_TLS "tls-min-tls"
# define OSSL_CAPABILITY_TLS_SIGALG_NAME "tls-sigalg-name"
# define OSSL_CAPABILITY_TLS_SIGALG_OID "tls-sigalg-oid"
# define OSSL_CAPABILITY_TLS_SIGALG_SECURITY_BITS "tls-sigalg-sec-bits"
# define OSSL_CAPABILITY_TLS_SIGALG_SIG_NAME "tls-sigalg-sig-name"
# define OSSL_CAPABILITY_TLS_SIGALG_SIG_OID "tls-sigalg-sig-oid"
# define OSSL_CIPHER_PARAM_AEAD "aead"
# define OSSL_CIPHER_PARAM_AEAD_IVLEN OSSL_CIPHER_PARAM_IVLEN
# define OSSL_CIPHER_PARAM_AEAD_MAC_KEY "mackey"
# define OSSL_CIPHER_PARAM_AEAD_TAG "tag"
# define OSSL_CIPHER_PARAM_AEAD_TAGLEN "taglen"
# define OSSL_CIPHER_PARAM_AEAD_TLS1_AAD "tlsaad"
# define OSSL_CIPHER_PARAM_AEAD_TLS1_AAD_PAD "tlsaadpad"
# define OSSL_CIPHER_PARAM_AEAD_TLS1_GET_IV_GEN "tlsivgen"
# define OSSL_CIPHER_PARAM_AEAD_TLS1_IV_FIXED "tlsivfixed"
# define OSSL_CIPHER_PARAM_AEAD_TLS1_SET_IV_INV "tlsivinv"
# define OSSL_CIPHER_PARAM_ALGORITHM_ID_PARAMS "alg_id_param"
# define OSSL_CIPHER_PARAM_BLOCK_SIZE "blocksize"
# define OSSL_CIPHER_PARAM_CTS "cts"
# define OSSL_CIPHER_PARAM_CTS_MODE "cts_mode"
# define OSSL_CIPHER_PARAM_CUSTOM_IV "custom-iv"
# define OSSL_CIPHER_PARAM_HAS_RAND_KEY "has-randkey"
# define OSSL_CIPHER_PARAM_IV "iv"
# define OSSL_CIPHER_PARAM_IVLEN "ivlen"
# define OSSL_CIPHER_PARAM_KEYLEN "keylen"
# define OSSL_CIPHER_PARAM_MODE "mode"
# define OSSL_CIPHER_PARAM_NUM "num"
# define OSSL_CIPHER_PARAM_PADDING "padding"
# define OSSL_CIPHER_PARAM_RANDOM_KEY "randkey"
# define OSSL_CIPHER_PARAM_RC2_KEYBITS "keybits"
# define OSSL_CIPHER_PARAM_ROUNDS "rounds"
# define OSSL_CIPHER_PARAM_SPEED "speed"
# define OSSL_CIPHER_PARAM_TLS1_MULTIBLOCK "tls-multi"
# define OSSL_CIPHER_PARAM_TLS1_MULTIBLOCK_AAD "tls1multi_aad"
# define OSSL_CIPHER_PARAM_TLS1_MULTIBLOCK_AAD_PACKLEN "tls1multi_aadpacklen"
# define OSSL_CIPHER_PARAM_TLS1_MULTIBLOCK_ENC "tls1multi_enc"
# define OSSL_CIPHER_PARAM_TLS1_MULTIBLOCK_ENC_IN "tls1multi_encin"
# define OSSL_CIPHER_PARAM_TLS1_MULTIBLOCK_ENC_LEN "tls1multi_enclen"
# define OSSL_CIPHER_PARAM_TLS1_MULTIBLOCK_INTERLEAVE "tls1multi_interleave"
# define OSSL_CIPHER_PARAM_TLS1_MULTIBLOCK_MAX_BUFSIZE "tls1multi_maxbufsz"
# define OSSL_CIPHER_PARAM_TLS1_MULTIBLOCK_MAX_SEND_FRAGMENT "tls1multi_maxsndfrag"
# define OSSL_CIPHER_PARAM_TLS_MAC "tls-mac"
# define OSSL_CIPHER_PARAM_TLS_MAC_SIZE "tls-mac-size"
# define OSSL_CIPHER_PARAM_TLS_VERSION "tls-version"
# define OSSL_CIPHER_PARAM_UPDATED_IV "updated-iv"
# define OSSL_CIPHER_PARAM_USE_BITS "use-bits"
# define OSSL_CIPHER_PARAM_XTS_STANDARD "xts_standard"
# define OSSL_DECODER_PARAM_PROPERTIES OSSL_ALG_PARAM_PROPERTIES
# define OSSL_DIGEST_PARAM_ALGID_ABSENT "algid-absent"
# define OSSL_DIGEST_PARAM_BLOCK_SIZE "blocksize"
# define OSSL_DIGEST_PARAM_MICALG "micalg"
# define OSSL_DIGEST_PARAM_PAD_TYPE "pad-type"
# define OSSL_DIGEST_PARAM_SIZE "size"
# define OSSL_DIGEST_PARAM_SSL3_MS "ssl3-ms"
# define OSSL_DIGEST_PARAM_XOF "xof"
# define OSSL_DIGEST_PARAM_XOFLEN "xoflen"
# define OSSL_DRBG_PARAM_CIPHER OSSL_ALG_PARAM_CIPHER
# define OSSL_DRBG_PARAM_DIGEST OSSL_ALG_PARAM_DIGEST
# define OSSL_DRBG_PARAM_ENTROPY_REQUIRED "entropy_required"
# define OSSL_DRBG_PARAM_MAC OSSL_ALG_PARAM_MAC
# define OSSL_DRBG_PARAM_MAX_ADINLEN "max_adinlen"
# define OSSL_DRBG_PARAM_MAX_ENTROPYLEN "max_entropylen"
# define OSSL_DRBG_PARAM_MAX_LENGTH "maxium_length"
# define OSSL_DRBG_PARAM_MAX_NONCELEN "max_noncelen"
# define OSSL_DRBG_PARAM_MAX_PERSLEN "max_perslen"
# define OSSL_DRBG_PARAM_MIN_ENTROPYLEN "min_entropylen"
# define OSSL_DRBG_PARAM_MIN_LENGTH "minium_length"
# define OSSL_DRBG_PARAM_MIN_NONCELEN "min_noncelen"
# define OSSL_DRBG_PARAM_PREDICTION_RESISTANCE "prediction_resistance"
# define OSSL_DRBG_PARAM_PROPERTIES OSSL_ALG_PARAM_PROPERTIES
# define OSSL_DRBG_PARAM_RANDOM_DATA "random_data"
# define OSSL_DRBG_PARAM_RESEED_COUNTER "reseed_counter"
# define OSSL_DRBG_PARAM_RESEED_REQUESTS "reseed_requests"
# define OSSL_DRBG_PARAM_RESEED_TIME "reseed_time"
# define OSSL_DRBG_PARAM_RESEED_TIME_INTERVAL "reseed_time_interval"
# define OSSL_DRBG_PARAM_SIZE "size"
# define OSSL_DRBG_PARAM_USE_DF "use_derivation_function"
# define OSSL_ENCODER_PARAM_CIPHER OSSL_ALG_PARAM_CIPHER
# define OSSL_ENCODER_PARAM_ENCRYPT_LEVEL "encrypt-level"
# define OSSL_ENCODER_PARAM_PROPERTIES OSSL_ALG_PARAM_PROPERTIES
# define OSSL_ENCODER_PARAM_SAVE_PARAMETERS "save-parameters"
# define OSSL_EXCHANGE_PARAM_EC_ECDH_COFACTOR_MODE "ecdh-cofactor-mode"
# define OSSL_EXCHANGE_PARAM_KDF_DIGEST "kdf-digest"
# define OSSL_EXCHANGE_PARAM_KDF_DIGEST_PROPS "kdf-digest-props"
# define OSSL_EXCHANGE_PARAM_KDF_OUTLEN "kdf-outlen"
# define OSSL_EXCHANGE_PARAM_KDF_TYPE "kdf-type"
# define OSSL_EXCHANGE_PARAM_KDF_UKM "kdf-ukm"
# define OSSL_EXCHANGE_PARAM_PAD "pad"
# define OSSL_GEN_PARAM_ITERATION "iteration"
# define OSSL_GEN_PARAM_POTENTIAL "potential"
# define OSSL_KDF_PARAM_ARGON2_AD "ad"
# define OSSL_KDF_PARAM_ARGON2_LANES "lanes"
# define OSSL_KDF_PARAM_ARGON2_MEMCOST "memcost"
# define OSSL_KDF_PARAM_ARGON2_VERSION "version"
# define OSSL_KDF_PARAM_CEK_ALG "cekalg"
# define OSSL_KDF_PARAM_CIPHER OSSL_ALG_PARAM_CIPHER
# define OSSL_KDF_PARAM_CONSTANT "constant"
# define OSSL_KDF_PARAM_DATA "data"
# define OSSL_KDF_PARAM_DIGEST OSSL_ALG_PARAM_DIGEST
# define OSSL_KDF_PARAM_EARLY_CLEAN "early_clean"
# define OSSL_KDF_PARAM_HMACDRBG_ENTROPY "entropy"
# define OSSL_KDF_PARAM_HMACDRBG_NONCE "nonce"
# define OSSL_KDF_PARAM_INFO "info"
# define OSSL_KDF_PARAM_ITER "iter"
# define OSSL_KDF_PARAM_KBKDF_R "r"
# define OSSL_KDF_PARAM_KBKDF_USE_L "use-l"
# define OSSL_KDF_PARAM_KBKDF_USE_SEPARATOR "use-separator"
# define OSSL_KDF_PARAM_KEY "key"
# define OSSL_KDF_PARAM_LABEL "label"
# define OSSL_KDF_PARAM_MAC OSSL_ALG_PARAM_MAC
# define OSSL_KDF_PARAM_MAC_SIZE "maclen"
# define OSSL_KDF_PARAM_MODE "mode"
# define OSSL_KDF_PARAM_PASSWORD "pass"
# define OSSL_KDF_PARAM_PKCS12_ID "id"
# define OSSL_KDF_PARAM_PKCS5 "pkcs5"
# define OSSL_KDF_PARAM_PREFIX "prefix"
# define OSSL_KDF_PARAM_PROPERTIES OSSL_ALG_PARAM_PROPERTIES
# define OSSL_KDF_PARAM_SALT "salt"
# define OSSL_KDF_PARAM_SCRYPT_MAXMEM "maxmem_bytes"
# define OSSL_KDF_PARAM_SCRYPT_N "n"
# define OSSL_KDF_PARAM_SCRYPT_P "p"
# define OSSL_KDF_PARAM_SCRYPT_R "r"
# define OSSL_KDF_PARAM_SECRET "secret"
# define OSSL_KDF_PARAM_SEED "seed"
# define OSSL_KDF_PARAM_SIZE "size"
# define OSSL_KDF_PARAM_SSHKDF_SESSION_ID "session_id"
# define OSSL_KDF_PARAM_SSHKDF_TYPE "type"
# define OSSL_KDF_PARAM_SSHKDF_XCGHASH "xcghash"
# define OSSL_KDF_PARAM_THREADS "threads"
# define OSSL_KDF_PARAM_UKM "ukm"
# define OSSL_KDF_PARAM_X942_ACVPINFO "acvp-info"
# define OSSL_KDF_PARAM_X942_PARTYUINFO "partyu-info"
# define OSSL_KDF_PARAM_X942_PARTYVINFO "partyv-info"
# define OSSL_KDF_PARAM_X942_SUPP_PRIVINFO "supp-privinfo"
# define OSSL_KDF_PARAM_X942_SUPP_PUBINFO "supp-pubinfo"
# define OSSL_KDF_PARAM_X942_USE_KEYBITS "use-keybits"
# define OSSL_KEM_PARAM_IKME "ikme"
# define OSSL_KEM_PARAM_OPERATION "operation"
# define OSSL_LIBSSL_RECORD_LAYER_PARAM_BLOCK_PADDING "block_padding"
# define OSSL_LIBSSL_RECORD_LAYER_PARAM_MAX_EARLY_DATA "max_early_data"
# define OSSL_LIBSSL_RECORD_LAYER_PARAM_MAX_FRAG_LEN "max_frag_len"
# define OSSL_LIBSSL_RECORD_LAYER_PARAM_MODE "mode"
# define OSSL_LIBSSL_RECORD_LAYER_PARAM_OPTIONS "options"
# define OSSL_LIBSSL_RECORD_LAYER_PARAM_READ_AHEAD "read_ahead"
# define OSSL_LIBSSL_RECORD_LAYER_PARAM_STREAM_MAC "stream_mac"
# define OSSL_LIBSSL_RECORD_LAYER_PARAM_TLSTREE "tlstree"
# define OSSL_LIBSSL_RECORD_LAYER_PARAM_USE_ETM "use_etm"
# define OSSL_LIBSSL_RECORD_LAYER_READ_BUFFER_LEN "read_buffer_len"
# define OSSL_MAC_PARAM_BLOCK_SIZE "block-size"
# define OSSL_MAC_PARAM_CIPHER OSSL_ALG_PARAM_CIPHER
# define OSSL_MAC_PARAM_CUSTOM "custom"
# define OSSL_MAC_PARAM_C_ROUNDS "c-rounds"
# define OSSL_MAC_PARAM_DIGEST OSSL_ALG_PARAM_DIGEST
# define OSSL_MAC_PARAM_DIGEST_NOINIT "digest-noinit"
# define OSSL_MAC_PARAM_DIGEST_ONESHOT "digest-oneshot"
# define OSSL_MAC_PARAM_D_ROUNDS "d-rounds"
# define OSSL_MAC_PARAM_IV "iv"
# define OSSL_MAC_PARAM_KEY "key"
# define OSSL_MAC_PARAM_PROPERTIES OSSL_ALG_PARAM_PROPERTIES
# define OSSL_MAC_PARAM_SALT "salt"
# define OSSL_MAC_PARAM_SIZE "size"
# define OSSL_MAC_PARAM_TLS_DATA_SIZE "tls-data-size"
# define OSSL_MAC_PARAM_XOF "xof"
# define OSSL_OBJECT_PARAM_DATA "data"
# define OSSL_OBJECT_PARAM_DATA_STRUCTURE "data-structure"
# define OSSL_OBJECT_PARAM_DATA_TYPE "data-type"
# define OSSL_OBJECT_PARAM_DESC "desc"
# define OSSL_OBJECT_PARAM_REFERENCE "reference"
# define OSSL_OBJECT_PARAM_TYPE "type"
# define OSSL_PASSPHRASE_PARAM_INFO "info"
# define OSSL_PKEY_PARAM_BITS "bits"
# define OSSL_PKEY_PARAM_CIPHER OSSL_ALG_PARAM_CIPHER
# define OSSL_PKEY_PARAM_DEFAULT_DIGEST "default-digest"
# define OSSL_PKEY_PARAM_DHKEM_IKM "dhkem-ikm"
# define OSSL_PKEY_PARAM_DH_GENERATOR "safeprime-generator"
# define OSSL_PKEY_PARAM_DH_PRIV_LEN "priv_len"
# define OSSL_PKEY_PARAM_DIGEST OSSL_ALG_PARAM_DIGEST
# define OSSL_PKEY_PARAM_DIGEST_SIZE "digest-size"
# define OSSL_PKEY_PARAM_DIST_ID "distid"
# define OSSL_PKEY_PARAM_EC_A "a"
# define OSSL_PKEY_PARAM_EC_B "b"
# define OSSL_PKEY_PARAM_EC_CHAR2_M "m"
# define OSSL_PKEY_PARAM_EC_CHAR2_PP_K1 "k1"
# define OSSL_PKEY_PARAM_EC_CHAR2_PP_K2 "k2"
# define OSSL_PKEY_PARAM_EC_CHAR2_PP_K3 "k3"
# define OSSL_PKEY_PARAM_EC_CHAR2_TP_BASIS "tp"
# define OSSL_PKEY_PARAM_EC_CHAR2_TYPE "basis-type"
# define OSSL_PKEY_PARAM_EC_COFACTOR "cofactor"
# define OSSL_PKEY_PARAM_EC_DECODED_FROM_EXPLICIT_PARAMS "decoded-from-explicit"
# define OSSL_PKEY_PARAM_EC_ENCODING "encoding"
# define OSSL_PKEY_PARAM_EC_FIELD_TYPE "field-type"
# define OSSL_PKEY_PARAM_EC_GENERATOR "generator"
# define OSSL_PKEY_PARAM_EC_GROUP_CHECK_TYPE "group-check"
# define OSSL_PKEY_PARAM_EC_INCLUDE_PUBLIC "include-public"
# define OSSL_PKEY_PARAM_EC_ORDER "order"
# define OSSL_PKEY_PARAM_EC_P "p"
# define OSSL_PKEY_PARAM_EC_POINT_CONVERSION_FORMAT "point-format"
# define OSSL_PKEY_PARAM_EC_PUB_X "qx"
# define OSSL_PKEY_PARAM_EC_PUB_Y "qy"
# define OSSL_PKEY_PARAM_EC_SEED "seed"
# define OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY "encoded-pub-key"
# define OSSL_PKEY_PARAM_ENGINE OSSL_ALG_PARAM_ENGINE
# define OSSL_PKEY_PARAM_FFC_COFACTOR "j"
# define OSSL_PKEY_PARAM_FFC_DIGEST OSSL_PKEY_PARAM_DIGEST
# define OSSL_PKEY_PARAM_FFC_DIGEST_PROPS OSSL_PKEY_PARAM_PROPERTIES
# define OSSL_PKEY_PARAM_FFC_G "g"
# define OSSL_PKEY_PARAM_FFC_GINDEX "gindex"
# define OSSL_PKEY_PARAM_FFC_H "hindex"
# define OSSL_PKEY_PARAM_FFC_P "p"
# define OSSL_PKEY_PARAM_FFC_PBITS "pbits"
# define OSSL_PKEY_PARAM_FFC_PCOUNTER "pcounter"
# define OSSL_PKEY_PARAM_FFC_Q "q"
# define OSSL_PKEY_PARAM_FFC_QBITS "qbits"
# define OSSL_PKEY_PARAM_FFC_SEED "seed"
# define OSSL_PKEY_PARAM_FFC_TYPE "type"
# define OSSL_PKEY_PARAM_FFC_VALIDATE_G "validate-g"
# define OSSL_PKEY_PARAM_FFC_VALIDATE_LEGACY "validate-legacy"
# define OSSL_PKEY_PARAM_FFC_VALIDATE_PQ "validate-pq"
# define OSSL_PKEY_PARAM_GROUP_NAME "group"
# define OSSL_PKEY_PARAM_IMPLICIT_REJECTION "implicit-rejection"
# define OSSL_PKEY_PARAM_MANDATORY_DIGEST "mandatory-digest"
# define OSSL_PKEY_PARAM_MASKGENFUNC "mgf"
# define OSSL_PKEY_PARAM_MAX_SIZE "max-size"
# define OSSL_PKEY_PARAM_MGF1_DIGEST "mgf1-digest"
# define OSSL_PKEY_PARAM_MGF1_PROPERTIES "mgf1-properties"
# define OSSL_PKEY_PARAM_PAD_MODE "pad-mode"
# define OSSL_PKEY_PARAM_PRIV_KEY "priv"
# define OSSL_PKEY_PARAM_PROPERTIES OSSL_ALG_PARAM_PROPERTIES
# define OSSL_PKEY_PARAM_PUB_KEY "pub"
# define OSSL_PKEY_PARAM_RSA_BITS OSSL_PKEY_PARAM_BITS
# define OSSL_PKEY_PARAM_RSA_COEFFICIENT "rsa-coefficient"
# define OSSL_PKEY_PARAM_RSA_COEFFICIENT1 "rsa-coefficient1"
# define OSSL_PKEY_PARAM_RSA_COEFFICIENT2 "rsa-coefficient2"
# define OSSL_PKEY_PARAM_RSA_COEFFICIENT3 "rsa-coefficient3"
# define OSSL_PKEY_PARAM_RSA_COEFFICIENT4 "rsa-coefficient4"
# define OSSL_PKEY_PARAM_RSA_COEFFICIENT5 "rsa-coefficient5"
# define OSSL_PKEY_PARAM_RSA_COEFFICIENT6 "rsa-coefficient6"
# define OSSL_PKEY_PARAM_RSA_COEFFICIENT7 "rsa-coefficient7"
# define OSSL_PKEY_PARAM_RSA_COEFFICIENT8 "rsa-coefficient8"
# define OSSL_PKEY_PARAM_RSA_COEFFICIENT9 "rsa-coefficient9"
# define OSSL_PKEY_PARAM_RSA_D "d"
# define OSSL_PKEY_PARAM_RSA_DIGEST OSSL_PKEY_PARAM_DIGEST
# define OSSL_PKEY_PARAM_RSA_DIGEST_PROPS OSSL_PKEY_PARAM_PROPERTIES
# define OSSL_PKEY_PARAM_RSA_E "e"
# define OSSL_PKEY_PARAM_RSA_EXPONENT "rsa-exponent"
# define OSSL_PKEY_PARAM_RSA_EXPONENT1 "rsa-exponent1"
# define OSSL_PKEY_PARAM_RSA_EXPONENT10 "rsa-exponent10"
# define OSSL_PKEY_PARAM_RSA_EXPONENT2 "rsa-exponent2"
# define OSSL_PKEY_PARAM_RSA_EXPONENT3 "rsa-exponent3"
# define OSSL_PKEY_PARAM_RSA_EXPONENT4 "rsa-exponent4"
# define OSSL_PKEY_PARAM_RSA_EXPONENT5 "rsa-exponent5"
# define OSSL_PKEY_PARAM_RSA_EXPONENT6 "rsa-exponent6"
# define OSSL_PKEY_PARAM_RSA_EXPONENT7 "rsa-exponent7"
# define OSSL_PKEY_PARAM_RSA_EXPONENT8 "rsa-exponent8"
# define OSSL_PKEY_PARAM_RSA_EXPONENT9 "rsa-exponent9"
# define OSSL_PKEY_PARAM_RSA_FACTOR "rsa-factor"
# define OSSL_PKEY_PARAM_RSA_FACTOR1 "rsa-factor1"
# define OSSL_PKEY_PARAM_RSA_FACTOR10 "rsa-factor10"
# define OSSL_PKEY_PARAM_RSA_FACTOR2 "rsa-factor2"
# define OSSL_PKEY_PARAM_RSA_FACTOR3 "rsa-factor3"
# define OSSL_PKEY_PARAM_RSA_FACTOR4 "rsa-factor4"
# define OSSL_PKEY_PARAM_RSA_FACTOR5 "rsa-factor5"
# define OSSL_PKEY_PARAM_RSA_FACTOR6 "rsa-factor6"
# define OSSL_PKEY_PARAM_RSA_FACTOR7 "rsa-factor7"
# define OSSL_PKEY_PARAM_RSA_FACTOR8 "rsa-factor8"
# define OSSL_PKEY_PARAM_RSA_FACTOR9 "rsa-factor9"
# define OSSL_PKEY_PARAM_RSA_MASKGENFUNC OSSL_PKEY_PARAM_MASKGENFUNC
# define OSSL_PKEY_PARAM_RSA_MGF1_DIGEST OSSL_PKEY_PARAM_MGF1_DIGEST
# define OSSL_PKEY_PARAM_RSA_N "n"
# define OSSL_PKEY_PARAM_RSA_PRIMES "primes"
# define OSSL_PKEY_PARAM_RSA_PSS_SALTLEN "saltlen"
# define OSSL_PKEY_PARAM_RSA_TEST_P1 "p1"
# define OSSL_PKEY_PARAM_RSA_TEST_P2 "p2"
# define OSSL_PKEY_PARAM_RSA_TEST_Q1 "q1"
# define OSSL_PKEY_PARAM_RSA_TEST_Q2 "q2"
# define OSSL_PKEY_PARAM_RSA_TEST_XP "xp"
# define OSSL_PKEY_PARAM_RSA_TEST_XP1 "xp1"
# define OSSL_PKEY_PARAM_RSA_TEST_XP2 "xp2"
# define OSSL_PKEY_PARAM_RSA_TEST_XQ "xq"
# define OSSL_PKEY_PARAM_RSA_TEST_XQ1 "xq1"
# define OSSL_PKEY_PARAM_RSA_TEST_XQ2 "xq2"
# define OSSL_PKEY_PARAM_SECURITY_BITS "security-bits"
# define OSSL_PKEY_PARAM_USE_COFACTOR_ECDH OSSL_PKEY_PARAM_USE_COFACTOR_FLAG
# define OSSL_PKEY_PARAM_USE_COFACTOR_FLAG "use-cofactor-flag"
# define OSSL_PROV_PARAM_BUILDINFO "buildinfo"
# define OSSL_PROV_PARAM_CORE_MODULE_FILENAME "module-filename"
# define OSSL_PROV_PARAM_CORE_PROV_NAME "provider-name"
# define OSSL_PROV_PARAM_CORE_VERSION "openssl-version"
# define OSSL_PROV_PARAM_DRBG_TRUNC_DIGEST "drbg-no-trunc-md"
# define OSSL_PROV_PARAM_NAME "name"
# define OSSL_PROV_PARAM_SECURITY_CHECKS "security-checks"
# define OSSL_PROV_PARAM_SELF_TEST_DESC "st-desc"
# define OSSL_PROV_PARAM_SELF_TEST_PHASE "st-phase"
# define OSSL_PROV_PARAM_SELF_TEST_TYPE "st-type"
# define OSSL_PROV_PARAM_STATUS "status"
# define OSSL_PROV_PARAM_TLS1_PRF_EMS_CHECK "tls1-prf-ems-check"
# define OSSL_PROV_PARAM_VERSION "version"
# define OSSL_RAND_PARAM_MAX_REQUEST "max_request"
# define OSSL_RAND_PARAM_STATE "state"
# define OSSL_RAND_PARAM_STRENGTH "strength"
# define OSSL_RAND_PARAM_TEST_ENTROPY "test_entropy"
# define OSSL_RAND_PARAM_TEST_NONCE "test_nonce"
# define OSSL_SIGNATURE_PARAM_ALGORITHM_ID "algorithm-id"
# define OSSL_SIGNATURE_PARAM_CONTEXT_STRING "context-string"
# define OSSL_SIGNATURE_PARAM_DIGEST OSSL_PKEY_PARAM_DIGEST
# define OSSL_SIGNATURE_PARAM_DIGEST_SIZE OSSL_PKEY_PARAM_DIGEST_SIZE
# define OSSL_SIGNATURE_PARAM_INSTANCE "instance"
# define OSSL_SIGNATURE_PARAM_KAT "kat"
# define OSSL_SIGNATURE_PARAM_MGF1_DIGEST OSSL_PKEY_PARAM_MGF1_DIGEST
# define OSSL_SIGNATURE_PARAM_MGF1_PROPERTIES OSSL_PKEY_PARAM_MGF1_PROPERTIES
# define OSSL_SIGNATURE_PARAM_NONCE_TYPE "nonce-type"
# define OSSL_SIGNATURE_PARAM_PAD_MODE OSSL_PKEY_PARAM_PAD_MODE
# define OSSL_SIGNATURE_PARAM_PROPERTIES OSSL_PKEY_PARAM_PROPERTIES
# define OSSL_SIGNATURE_PARAM_PSS_SALTLEN "saltlen"
# define OSSL_STORE_PARAM_ALIAS "alias"
# define OSSL_STORE_PARAM_DIGEST "digest"
# define OSSL_STORE_PARAM_EXPECT "expect"
# define OSSL_STORE_PARAM_FINGERPRINT "fingerprint"
# define OSSL_STORE_PARAM_INPUT_TYPE "input-type"
# define OSSL_STORE_PARAM_ISSUER "name"
# define OSSL_STORE_PARAM_PROPERTIES "properties"
# define OSSL_STORE_PARAM_SERIAL "serial"
# define OSSL_STORE_PARAM_SUBJECT "subject"

# ifdef __cplusplus
}
# endif

#endif
