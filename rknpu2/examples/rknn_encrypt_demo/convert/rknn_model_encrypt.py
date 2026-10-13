#!/usr/bin/env python3
"""
RKNN Model Encryption Tool

Encrypts a .rknn model file using AES-CTR, and wraps the user AES key
inside an RSA key envelope using RK's public key.

The RKNN model header (first 64 bytes) stays plaintext; the RKCE indicator
is written into the header's reserved region (offset 24). Only the model
body (offset 64+) is AES-CTR encrypted. The runtime auto-detects the
RKCE magic and decrypts transparently inside rknn_init().

Usage:
    python3 rknn_model_encrypt.py --model model.rknn \
                                  --rk-pubkey rk_public_key.pem \
                                  [--output-dir ./encrypted] \
                                  [--aes-bits 256]

Output files:
    <output_dir>/<model>.rknn        - Encrypted model (RKCE indicator + AES-CTR body)
    <output_dir>/<model>_key.bin     - RSA-encrypted key envelope (RKKE format)

File formats are compatible with the rknn_init() / rknn_init_extend API
defined in rknn_api.h.

Dependencies: pip install pycryptodome
"""

import argparse
import os
import struct
import sys

from Crypto.Cipher import AES, PKCS1_OAEP
from Crypto.PublicKey import RSA
from Crypto.Random import get_random_bytes

# Magic constants (must match rknn_crypto.h)
CRYPT_MODEL_MAGIC    = 0x45434B52  # "RKCE"
CRYPT_KEY_MAGIC      = 0x454B4B52  # "RKKE"
CRYPT_ALG_AES128_CTR = 0x01
CRYPT_ALG_AES256_CTR = 0x02

# RKNN header layout (must match rknn_model_data_struct.h / rknn_crypto.h)
RKNN_MAGIC_U64       = 0x4E4E4B52  # "RKNN" as uint64 LE
RKNN_MAGIC_BYTES     = struct.pack('<Q', RKNN_MAGIC_U64)  # b'RKNN\x00\x00\x00\x00'
RKNN_HEADER_SIZE     = 64   # magic(8) + version(8) + exportDataSize(8) + reserved(40)
RKNN_RESERVED_OFFSET = 24   # start of the 40-byte reserved region
RKNN_RESERVED_SIZE   = 40
RKCE_INDICATOR_SIZE  = 24   # bytes of reserved actually used

# RKCE indicator: magic(4) + version(4) + algorithm(4) + reserved(4) + original_size(8) = 24 bytes
RKCE_INDICATOR_FORMAT = '<IIII Q'

# RKKE key envelope header: magic(4) + version(4) + rsa_key_bits(4) + reserved(4) = 16 bytes
KEY_HEADER_FORMAT = '<IIII'

AES_IV_LEN = 16


def aes_algorithm(aes_bits: int) -> int:
    """Return the algorithm ID for the given AES key size."""
    if aes_bits == 128:
        return CRYPT_ALG_AES128_CTR
    return CRYPT_ALG_AES256_CTR


def aes_ctr_encrypt(key: bytes, iv: bytes, plaintext: bytes) -> bytes:
    """Encrypt data with AES-CTR (matches pycryptodome nonce=b'', initial_value=iv)."""
    cipher = AES.new(key, AES.MODE_CTR, nonce=b'', initial_value=iv)
    return cipher.encrypt(plaintext)


def aes_ctr_decrypt(key: bytes, iv: bytes, ciphertext: bytes) -> bytes:
    """Decrypt data with AES-CTR (CTR is symmetric: encrypt == decrypt)."""
    cipher = AES.new(key, AES.MODE_CTR, nonce=b'', initial_value=iv)
    return cipher.decrypt(ciphertext)


def build_rkce_indicator(original_size: int, aes_bits: int, version: int = 1) -> bytes:
    """Build the 24-byte RKCE indicator written into the RKNN header reserved region."""
    return struct.pack(
        RKCE_INDICATOR_FORMAT,
        CRYPT_MODEL_MAGIC,          # "RKCE"
        version,                    # format version
        aes_algorithm(aes_bits),    # 0x02 = AES-256-CTR
        0,                          # reserved0
        original_size,              # plaintext body length (file size - 64)
    )


def build_key_envelope(aes_key: bytes, aes_iv: bytes, rk_pubkey: RSA.RsaKey) -> bytes:
    """
    Build the key envelope file:
      RKKE header (16 bytes) + RSA-OAEP encrypted (AES key + IV)
    """
    rsa_cipher = PKCS1_OAEP.new(rk_pubkey)
    rsa_encrypted = rsa_cipher.encrypt(aes_key + aes_iv)

    header = struct.pack(
        KEY_HEADER_FORMAT,
        CRYPT_KEY_MAGIC,            # "RKKE"
        1,                          # version
        rk_pubkey.size_in_bits(),   # RSA key size in bits (e.g. 2048)
        0                           # reserved
    )
    return header + rsa_encrypted


def encrypt_model(model_data: bytes, aes_key: bytes, aes_iv: bytes, aes_bits: int) -> bytes:
    """
    Encrypt an RKNN model: plaintext header + RKCE indicator + AES-CTR body.

    The first 64 bytes (magic/version/exportDataSize) stay plaintext.
    The RKCE indicator is written into the reserved region (offset 24-47).
    Only the body (offset 64+) is AES-CTR encrypted.
    """
    body_plain = model_data[RKNN_HEADER_SIZE:]
    ciphertext = aes_ctr_encrypt(aes_key, aes_iv, body_plain)

    # Copy the plaintext header, then overwrite the reserved region with the RKCE indicator
    header = bytearray(model_data[:RKNN_HEADER_SIZE])
    rkce = build_rkce_indicator(len(body_plain), aes_bits)
    assert len(rkce) == RKCE_INDICATOR_SIZE
    header[RKNN_RESERVED_OFFSET:RKNN_RESERVED_OFFSET + RKCE_INDICATOR_SIZE] = rkce

    return bytes(header) + ciphertext


def verify_roundtrip(model_data: bytes, enc_model: bytes, aes_key: bytes, aes_iv: bytes) -> bool:
    ciphertext = enc_model[RKNN_HEADER_SIZE:]
    dec_body = aes_ctr_decrypt(aes_key, aes_iv, ciphertext)

    # Header prefix stays plaintext; body must decrypt back losslessly
    return enc_model[:RKNN_RESERVED_OFFSET] == model_data[:RKNN_RESERVED_OFFSET] \
        and dec_body == model_data[RKNN_HEADER_SIZE:]


def validate_rknn_header(model_data: bytes) -> int:
    """
    Validate the RKNN model header. Returns the model version on success,
    exits on error.
    """
    if model_data[:8] != RKNN_MAGIC_BYTES:
        print(f'Error: invalid RKNN magic: {model_data[:8]!r} (expected {RKNN_MAGIC_BYTES!r})',
              file=sys.stderr)
        sys.exit(1)

    version = struct.unpack('<Q', model_data[8:16])[0]
    if version <= 1:
        print(f'Error: RKNN model version={version} <= 1, no reserved region for RKCE indicator',
              file=sys.stderr)
        sys.exit(1)

    if len(model_data) < RKNN_HEADER_SIZE:
        print(f'Error: model file too small: {len(model_data)} bytes (header needs {RKNN_HEADER_SIZE})',
              file=sys.stderr)
        sys.exit(1)

    return version


def model_stem(path: str) -> str:
    """Extract the base name without any extension (e.g. 'model.rknn' -> 'model')."""
    name = os.path.basename(path)
    while '.' in name:
        name = os.path.splitext(name)[0]
    return name


def main():
    parser = argparse.ArgumentParser(
        description='RKNN Model Encryption Tool',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument('-m', '--model', required=True,
                        help='Path to plaintext .rknn model file')
    parser.add_argument('-k', '--rk-pubkey', required=True,
                        help='Path to RK RSA public key (PEM)')
    parser.add_argument('--aes-bits', type=int, choices=(128, 256), default=256,
                        help='AES key size in bits (default: 256)')
    parser.add_argument('-o', '--output-dir', default='.',
                        help='Output directory (default: current)')
    args = parser.parse_args()

    # --- Validate inputs ---
    if not os.path.isfile(args.model):
        print(f'Error: model file not found: {args.model}', file=sys.stderr)
        sys.exit(1)
    if not os.path.isfile(args.rk_pubkey):
        print(f'Error: RK public key not found: {args.rk_pubkey}', file=sys.stderr)
        sys.exit(1)

    # --- 1. Read and validate the plaintext model ---
    with open(args.model, 'rb') as f:
        model_data = f.read()
    rknn_version = validate_rknn_header(model_data)

    # --- 2. Load the RK RSA public key ---
    with open(args.rk_pubkey, 'rb') as f:
        rk_pubkey = RSA.import_key(f.read())

    # --- 3. Generate random AES key and IV ---
    aes_key = get_random_bytes(args.aes_bits // 8)
    aes_iv  = get_random_bytes(AES_IV_LEN)

    # --- 4. Encrypt the model ---
    enc_model = encrypt_model(model_data, aes_key, aes_iv, args.aes_bits)

    # --- 5. Build the key envelope ---
    key_envelope = build_key_envelope(aes_key, aes_iv, rk_pubkey)

    # --- 6. Verify round-trip ---
    if not verify_roundtrip(model_data, enc_model, aes_key, aes_iv):
        print('Error: verification failed - decrypted model does not match original',
              file=sys.stderr)
        sys.exit(1)

    # --- 7. Write output files ---
    os.makedirs(args.output_dir, exist_ok=True)
    stem = model_stem(args.model)
    enc_model_path = os.path.join(args.output_dir, stem + '.rknn')
    key_env_path   = os.path.join(args.output_dir, stem + '_key.bin')

    # Guard against overwriting the input plaintext model
    if os.path.abspath(enc_model_path) == os.path.abspath(args.model):
        print(f'Error: output path matches input, would overwrite plaintext model: {enc_model_path}',
              file=sys.stderr)
        print('       Use -o to specify a different output directory', file=sys.stderr)
        sys.exit(1)

    with open(enc_model_path, 'wb') as f:
        f.write(enc_model)
    with open(key_env_path, 'wb') as f:
        f.write(key_envelope)

    body_len = len(model_data) - RKNN_HEADER_SIZE
    print('Encryption complete:')
    print(f'  Input model:  {args.model} ({len(model_data)} bytes, version={rknn_version})')
    print(f'  RK public key: {args.rk_pubkey} (RSA-{rk_pubkey.size_in_bits()})')
    print(f'  AES:          {args.aes_bits}-bit CTR')
    print(f'  Encrypted model: {enc_model_path} '
          f'(header {RKNN_HEADER_SIZE} + body {body_len} = {len(enc_model)} bytes)')
    print(f'  Key envelope: {key_env_path} '
          f'(RKKE header 16 + RSA {len(key_envelope) - 16} = {len(key_envelope)} bytes)')
    print(f'  Verify:       OK (AES-CTR round-trip passed)')
    


if __name__ == '__main__':
    main()
