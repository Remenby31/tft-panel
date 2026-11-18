// Release signatures as tools/release.sh makes them (openssl, ECDSA P-256 over the image's
// SHA-256) must verify on the panel's mbedTLS, and nothing else may.
#include <string.h>
#include <unity.h>

#include "net/signature.h"
#include "net/signing_key.h"

// openssl ecparam -name prime256v1 -genkey; the image is the 20 bytes "tft-panel test image".
static const char KEY[] =
    "-----BEGIN PUBLIC KEY-----\n"
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEm+s6Fvf5XvE1sf1AhYOSEhRmeys/\n"
    "j/tOgyy4Fdv1OYueKxTf3C30+JrD/mXDs2NTNXJoOcNZg703ZHJE2VjwMg==\n"
    "-----END PUBLIC KEY-----\n";
static const char OTHER_KEY[] =
    "-----BEGIN PUBLIC KEY-----\n"
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEIl0bSmiIIdYq9wKJuAavgKz7Y8LR\n"
    "0Ymdc9+3jJNEuBvKLdQWPKbpm3lPSJi1wELQSrEShmE//vFoaMcQR1peZw==\n"
    "-----END PUBLIC KEY-----\n";
static const uint8_t DIGEST[32] = {
    0x0a, 0xce, 0x9a, 0x85, 0x55, 0x6b, 0xef, 0x35, 0x79, 0x5d, 0x63, 0x43, 0xaf, 0x85, 0x76, 0x5d,
    0x63, 0xe8, 0x13, 0xe6, 0x5a, 0xdc, 0x48, 0x20, 0x0f, 0x11, 0x79, 0x02, 0x2b, 0x7a, 0xb4, 0xab};
// openssl dgst -sha256 -sign key.pem -out image.sig image
static const uint8_t SIG[70] = {
    0x30, 0x44, 0x02, 0x20, 0x03, 0xb0, 0xd2, 0x9d, 0x76, 0x81, 0x05, 0x8f, 0xf3, 0x6a, 0xdc, 0xf6,
    0x12, 0x6c, 0x6e, 0x02, 0x0c, 0x47, 0x9b, 0x72, 0xdf, 0x9b, 0x40, 0xb1, 0xe8, 0x71, 0xa8, 0x6c,
    0x7b, 0x88, 0xf3, 0xcd, 0x02, 0x20, 0x79, 0xc7, 0x31, 0x78, 0x67, 0x1a, 0xfc, 0x44, 0xd1, 0x1e,
    0xdd, 0xb7, 0x95, 0xef, 0xcd, 0x79, 0xca, 0xb9, 0xda, 0x74, 0xd2, 0x4b, 0xd7, 0x09, 0x03, 0x01,
    0x84, 0x62, 0xa1, 0x97, 0x3e, 0xcc};
// The same image signed by a 256-bit key on another curve: valid, but not a P-256 key.
// openssl ecparam -name secp256k1 -genkey; openssl dgst -sha256 -sign k1.pem image
static const char K1_KEY[] =
    "-----BEGIN PUBLIC KEY-----\n"
    "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEcdN8xMRMoLQemPCEcB4NCYfunbb+tqMF\n"
    "6Ksh6f0yS52Zz3l5cuLdzf4A/3rQFQdx7EHyMnqztT24gO/Wx3H05w==\n"
    "-----END PUBLIC KEY-----\n";
static const uint8_t K1_SIG[72] = {
    0x30, 0x46, 0x02, 0x21, 0x00, 0xb1, 0x0d, 0x76, 0xc2, 0xfa, 0xa0, 0xee, 0x61, 0xef, 0xcd, 0x50,
    0x8e, 0x5b, 0x99, 0x5f, 0x5f, 0x99, 0x30, 0x0b, 0x30, 0xbe, 0xa4, 0xfb, 0x5e, 0xd4, 0x96, 0x8d,
    0x41, 0x5d, 0xe2, 0x63, 0xc8, 0x02, 0x21, 0x00, 0x85, 0x51, 0x22, 0xe0, 0xa9, 0x0b, 0xc2, 0x0e,
    0xaa, 0x38, 0x8d, 0x77, 0x69, 0x41, 0x59, 0xa1, 0x29, 0x81, 0xae, 0xb2, 0xc0, 0x48, 0x16, 0x17,
    0x4d, 0xc3, 0xb4, 0xc7, 0x18, 0x4f, 0x4f, 0x9c};
// SIG as the upload page sends it (?sig=).
static const char SIG_HEX[] =
    "3044022003b0d29d7681058ff36adcf6126c6e020c479b72df9b40b1e871a86c7b88f3cd022079c73178671afc44d1"
    "1eddb795efcd79cab9da74d24bd70903018462a1973ecc";

void setUp() {}
void tearDown() {}

void test_release_signature_verifies() { TEST_ASSERT_TRUE(signature::verify(KEY, DIGEST, SIG, sizeof SIG)); }

void test_one_flipped_bit_in_the_image_is_refused() {
    uint8_t digest[32];
    memcpy(digest, DIGEST, 32);
    digest[31] ^= 0x01;
    TEST_ASSERT_FALSE(signature::verify(KEY, digest, SIG, sizeof SIG));
}

void test_one_flipped_bit_in_the_signature_is_refused() {
    uint8_t sig[sizeof SIG];
    memcpy(sig, SIG, sizeof SIG);
    sig[40] ^= 0x01;
    TEST_ASSERT_FALSE(signature::verify(KEY, DIGEST, sig, sizeof sig));
}

void test_another_key_is_refused() { TEST_ASSERT_FALSE(signature::verify(OTHER_KEY, DIGEST, SIG, sizeof SIG)); }

void test_a_valid_signature_on_another_curve_is_refused() {
    TEST_ASSERT_FALSE(signature::verify(K1_KEY, DIGEST, K1_SIG, sizeof K1_SIG));
}

void test_no_key_or_no_signature_is_refused() {
    TEST_ASSERT_FALSE(signature::verify("", DIGEST, SIG, sizeof SIG));
    TEST_ASSERT_FALSE(signature::verify(KEY, DIGEST, SIG, 0));
}

// The repository ships without a key: every build behaves as before signing existed. Once the
// maintainer commits a key, there is nothing left to check here.
void test_the_repository_build_does_not_enforce() {
    if (signature::PUBLIC_KEY[0]) TEST_IGNORE_MESSAGE("a signing key is compiled in");
    TEST_ASSERT_FALSE(signature::enforced());
    TEST_ASSERT_FALSE(signature::verifyRelease(DIGEST, SIG, sizeof SIG));
}

void test_the_upload_page_hex_decodes_to_the_signature() {
    uint8_t sig[signature::MAX_SIG];
    size_t n = signature::fromHex(SIG_HEX, sig, sizeof sig);
    TEST_ASSERT_EQUAL_UINT(70, n);
    TEST_ASSERT_TRUE(signature::verify(KEY, DIGEST, sig, n));
    TEST_ASSERT_EQUAL_UINT(0, signature::fromHex("30zz", sig, sizeof sig));
    TEST_ASSERT_EQUAL_UINT(0, signature::fromHex("", sig, sizeof sig));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_release_signature_verifies);
    RUN_TEST(test_one_flipped_bit_in_the_image_is_refused);
    RUN_TEST(test_one_flipped_bit_in_the_signature_is_refused);
    RUN_TEST(test_another_key_is_refused);
    RUN_TEST(test_a_valid_signature_on_another_curve_is_refused);
    RUN_TEST(test_no_key_or_no_signature_is_refused);
    RUN_TEST(test_the_repository_build_does_not_enforce);
    RUN_TEST(test_the_upload_page_hex_decodes_to_the_signature);
    return UNITY_END();
}
