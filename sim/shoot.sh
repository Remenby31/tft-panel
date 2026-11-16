#!/usr/bin/env bash
# Builds the simulator at both screen sizes and writes one PNG per screen into OUT_DIR.
# usage: sim/shoot.sh [OUT_DIR] [SCREEN,...] [en|fr]   (default: sim/out, the five main screens, en)
# The 800x480 build is the CrowPanel's, English only: fr changes the 1024x600 shots.
set -euo pipefail
cd "$(dirname "$0")/.."
out=${1:-sim/out}
shots=${2:-lyrics,cover,clock,library,settings}
lang=${3:-en}
pio=${PIO:-pio}
mkdir -p "$out"
for env in sim sim-1024; do
    "$pio" run -s -e "$env"
    ".pio/build/$env/program" --out "$out" --shots "$shots" --lang "$lang"
done
