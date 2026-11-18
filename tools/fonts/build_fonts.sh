#!/usr/bin/env bash
# Rebuilds the embedded fonts in assets/fonts/ from Noto Sans (SIL Open Font License 1.1), and the
# symbol-only fonts in src/app/symbol_fonts/ from LVGL's built-in Montserrat (tools/fonts/symbols.txt).
#
# LVGL's built-in Montserrat is ASCII only; lyrics need accents, Latin Extended-A
# (Polish, Czech, Turkish, œ…) and typographic punctuation (’ “ ” – — …).
# The firmware renders these TTFs at run time with LVGL's tiny_ttf, at any size.
#
# LVGL's Montserrat stays only for its symbols (the play, volume, settings... icons), which Noto Sans
# does not have: tiny_ttf falls back to it per glyph. Its letters are never drawn, so only the
# symbols the firmware uses are kept, byte for byte (subset_lvgl_font.py): ~60 KB less flash on the
# CrowPanel. tools/fonts/check_symbols.py fails a build that uses a symbol not kept.
#
# Requirements: gh (GitHub CLI), uv (runs a pinned fonttools), python3.
set -euo pipefail

GOOGLE_FONTS_SHA=8b0a1d0f5983c89bc2b93f1b5fb55f9e252744b5   # google/fonts, ofl/notosans
FONTTOOLS=fonttools==4.55.0
LVGL_SHA=80ca777e37a2b176770726a02e07a6fb79ef0b39   # lvgl/lvgl v9.6.0, the version platformio.ini pins

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT="$ROOT/assets/fonts"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

if [ "${1:-}" != symbols ]; then   # `build_fonts.sh symbols`: the symbol fonts only
    echo "Downloading Noto Sans (google/fonts@${GOOGLE_FONTS_SHA:0:7})"
    for f in 'NotoSans[wdth,wght].ttf' OFL.txt; do
        gh api "repos/google/fonts/contents/ofl/notosans/$f?ref=$GOOGLE_FONTS_SHA" \
            -H 'Accept: application/vnd.github.raw' > "$WORK/$f"
    done
    test -s "$WORK/NotoSans[wdth,wght].ttf" || { echo "font download failed" >&2; exit 1; }

    # Basic Latin, Latin-1 Supplement, Latin Extended-A, the General Punctuation block (all
    # hyphens and dashes: metadata uses U+2010 in "Jean‐Luc"; quotes, ellipsis, primes,
    # guillemets ‹ ›), fraction slash, euro, and the replacement character.
    UNICODES='U+0020-007E,U+00A0-017F,U+2010-2027,U+2030-203A,U+2044,U+20AC,U+FFFD'

    mkdir -p "$OUT"
    for w in 400 600; do
        echo "  NotoSans-$w.ttf"
        uvx --quiet --from "$FONTTOOLS" fonttools varLib.instancer \
            "$WORK/NotoSans[wdth,wght].ttf" wght=$w wdth=100 -o "$WORK/inst-$w.ttf"
        # stb_truetype reads glyf/cmap/hmtx (+ legacy kern); drop hinting and layout tables.
        uvx --quiet --from "$FONTTOOLS" pyftsubset "$WORK/inst-$w.ttf" \
            --unicodes="$UNICODES" --no-hinting --layout-features='' --drop-tables+=GSUB,GPOS,GDEF \
            --name-IDs='*' --output-file="$OUT/NotoSans-$w.ttf"
    done
    cp "$WORK/OFL.txt" "$OUT/OFL.txt"
    ls -l "$OUT"
fi

echo "Symbol fonts from LVGL's Montserrat (lvgl/lvgl@${LVGL_SHA:0:7}), keeping tools/fonts/symbols.txt"
SYM_OUT="$ROOT/src/app/symbol_fonts"
mkdir -p "$SYM_OUT"
lvgl_file() {   # path in the lvgl repo
    gh api "repos/lvgl/lvgl/contents/$1?ref=$LVGL_SHA" -H 'Accept: application/vnd.github.raw' > "$WORK/${1##*/}"
    test -s "$WORK/${1##*/}" || { echo "download of $1 failed" >&2; exit 1; }
}
lvgl_file include/lvgl/font/lv_symbol_def.h
# 14/20/28 on the 800x480 screens, 18/24/36 on the Waveshare 7B (src/app/fonts.cpp).
for size in 14 18 20 24 28 36; do
    lvgl_file "src/font/lv_font_montserrat_$size.c"
    python3 -I "$ROOT/tools/fonts/subset_lvgl_font.py" "$WORK/lv_font_montserrat_$size.c" "$WORK/lv_symbol_def.h" \
        "$ROOT/tools/fonts/symbols.txt" "$SYM_OUT/symbols_$size.c"
done
