// Release signatures: ECDSA P-256 over the image's SHA-256, the digest the update paths already
// compute while the image arrives. The `.sig` beside each release binary is the DER signature
// `openssl dgst -sha256 -sign` writes (tools/release.sh).
// ECDSA and not Ed25519: the mbedTLS of ESP-IDF 5.5 has no EdDSA, and ECDSA P-256 is already in
// the image for TLS (GitHub's certificates), so checking a signature costs almost no flash.
// Pure: mbedTLS only, so the host tests run it as is.
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace signature {

constexpr size_t MAX_SIG = 72;   // DER ECDSA P-256: 70-72 bytes

// A key is compiled in (signing_key.h): unsigned or badly signed images are refused.
bool enforced();

// `sig` is a valid signature of `digest` by `pemKey` (a P-256 public key, PEM).
bool verify(const char *pemKey, const uint8_t digest[32], const uint8_t *sig, size_t sigLen);

// verify() with the compiled-in key; false when there is none.
bool verifyRelease(const uint8_t digest[32], const uint8_t *sig, size_t sigLen);

// Hex text → bytes; the byte count, 0 if it is not even-length hex or does not fit in `cap`.
size_t fromHex(const char *hex, uint8_t *out, size_t cap);

}  // namespace signature
