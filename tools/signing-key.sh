#!/usr/bin/env bash
# Creates the release-signing key pair and compiles its public half into the firmware.
#
#   tools/signing-key.sh [private-key-path]
#
# The private key (default ~/.config/tft-panel/signing-key.pem, or $TFT_SIGNING_KEY) stays on the
# maintainer's machine: tools/release.sh signs every image with it. Lose it and panels running a
# build with this public key accept no update but a manual one (USB or the browser installer);
# keep a copy offline. The public key goes into src/net/signing_key.h, to be committed: from the
# first release built with it, panels refuse any image without a valid signature.
# Run it once. An existing private key is reused (the header is rewritten from it), never replaced.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd -P)"
key="${1:-${TFT_SIGNING_KEY:-$HOME/.config/tft-panel/signing-key.pem}}"
header="$root/src/net/signing_key.h"

mkdir -p "$(dirname "$key")"
case "$(cd "$(dirname "$key")" && pwd -P)/" in
    "$root"/*) echo "the private key must live outside the repository: $key" >&2; exit 1 ;;
esac

if [[ -e "$key" ]]; then
    echo "reusing $key"
else
    (umask 077 && openssl ecparam -name prime256v1 -genkey -noout | openssl pkcs8 -topk8 -nocrypt -out "$key")
    echo "created $key: keep a copy offline, it is the only way to sign updates for these panels"
fi

pub="$(openssl pkey -in "$key" -pubout)"
grep -q "BEGIN PUBLIC KEY" <<<"$pub" || { echo "cannot read a public key from $key" >&2; exit 1; }
openssl pkey -pubin -text_pub -noout <<<"$pub" | grep -q "NIST CURVE: P-256" \
    || { echo "$key is not a P-256 key" >&2; exit 1; }

{
    sed -n '1,/^namespace signature {$/p' "$header"
    echo
    printf 'constexpr const char PUBLIC_KEY[] =\n'
    while IFS= read -r line; do printf '    "%s\\n"\n' "$line"; done <<<"$pub"
    printf '    ;\n\n}  // namespace signature\n'
} > "$header.tmp"
mv "$header.tmp" "$header"
echo "wrote the public key into ${header#"$root"/}: commit it, the next release is signed"
