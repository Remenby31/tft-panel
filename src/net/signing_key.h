// The public half of the key that signs releases (tools/signing-key.sh writes this file).
// Empty: updates are checked against their published SHA-256 only, as before signing existed.
// Set: a panel installs only an image whose `.sig` this key verifies (src/net/signature.h).
#pragma once

namespace signature {

constexpr const char PUBLIC_KEY[] = "";

}  // namespace signature
