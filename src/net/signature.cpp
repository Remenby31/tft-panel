#include "signature.h"

#include <mbedtls/pk.h>
#include <string.h>

#include "signing_key.h"

namespace signature {

bool enforced() { return PUBLIC_KEY[0] != 0; }

bool verify(const char *pemKey, const uint8_t digest[32], const uint8_t *sig, size_t sigLen) {
    if (!pemKey || !*pemKey || !sig || !sigLen || sigLen > MAX_SIG) return false;
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    // PEM parsing wants the terminating NUL counted in the length.
    bool ok = mbedtls_pk_parse_public_key(&pk, (const unsigned char *)pemKey, strlen(pemKey) + 1) == 0 &&
              mbedtls_pk_get_type(&pk) == MBEDTLS_PK_ECKEY && mbedtls_pk_get_bitlen(&pk) == 256 &&
              mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, digest, 32, sig, sigLen) == 0;
    mbedtls_pk_free(&pk);
    return ok;
}

bool verifyRelease(const uint8_t digest[32], const uint8_t *sig, size_t sigLen) {
    return enforced() && verify(PUBLIC_KEY, digest, sig, sigLen);
}

size_t fromHex(const char *hex, uint8_t *out, size_t cap) {
    size_t len = hex ? strlen(hex) : 0;
    if (!len || len % 2 || len / 2 > cap) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = hex[i];
        int v = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
        if (v < 0) return 0;
        if (i % 2) out[i / 2] |= v;
        else out[i / 2] = v << 4;
    }
    return len / 2;
}

}  // namespace signature
