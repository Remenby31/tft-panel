#!/usr/bin/env bash
# The first-start walkthrough as a short film, from showcase screens already in docs/images:
#   tools/first-start.sh            → docs/media/first-start.mp4 and .gif
#
# Shot list, in the order a person meets them. Each still is held long enough to read, with the
# panel's device frame around it (tools/promo.py draws the frame; the stills come from
# tools/showcase.py, so every network, song and password in them is invented).
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"
screens=(wifi-setup wifi-list wifi-keyboard searching lyrics cover-view idle-clock)
holds=(4.5 3.5 3.5 3.0 4.5 3.5 4.0)

tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' EXIT
mkdir -p "$tmp/in"
i=0
for s in "${screens[@]}"; do
    cp "docs/images/$s.png" "$(printf '%s/in/f%03d.png' "$tmp" "$i")"
    i=$((i + 1))
done
uv run --quiet --with pillow==11.0.0 python tools/promo.py --screens docs/images --out "$tmp" --frames "$tmp/in" > /dev/null

list="$tmp/list.txt"
for i in "${!screens[@]}"; do
    printf "file '%s'\nduration %s\n" "$(printf '%s/framed/f%03d.png' "$tmp" "$i")" "${holds[$i]}" >> "$list"
done
printf "file '%s'\n" "$(printf '%s/framed/f%03d.png' "$tmp" "$((${#screens[@]} - 1))")" >> "$list"

out="$root/docs/media/first-start"
ffmpeg -y -loglevel error -f concat -safe 0 -i "$list" \
    -vf "fps=12,scale=1000:-2:flags=lanczos,format=yuv420p" -movflags +faststart "$out.mp4"
ffmpeg -y -loglevel error -f concat -safe 0 -i "$list" \
    -vf "fps=6,scale=640:-2:flags=lanczos,split[a][b];[a]palettegen=stats_mode=diff[p];[b][p]paletteuse=dither=bayer:bayer_scale=3" \
    "$out.gif"
ls -l "$out.mp4" "$out.gif"
