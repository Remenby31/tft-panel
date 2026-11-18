# PlatformIO pre-script: fails the build when src/ draws an LVGL symbol that the symbol-only
# fonts (src/app/symbol_fonts/) do not have, which would show as an empty box on the panel.
# The list is tools/fonts/symbols.txt; tools/fonts/build_fonts.sh regenerates the fonts from it.
import os
import re

Import("env")  # noqa: F821  (provided by PlatformIO/SCons)

root = env.subst("$PROJECT_DIR")  # noqa: F821
listed = [l.strip() for l in open(os.path.join(root, "tools", "fonts", "symbols.txt"))
          if l.strip() and not l.startswith("#")]

used = {}
for dirpath, _, files in os.walk(os.path.join(root, "src")):
    for f in files:
        if f.endswith((".c", ".cpp", ".h")) and "symbol_fonts" not in dirpath:
            path = os.path.join(dirpath, f)
            for name in re.findall(r"\bLV_SYMBOL_([A-Z0-9_]+)", open(path, errors="ignore").read()):
                used.setdefault(name, os.path.relpath(path, root))

problems = [f"LV_SYMBOL_{n} is used in {p} but not listed in tools/fonts/symbols.txt"
            for n, p in sorted(used.items()) if n not in listed]
fonts_dir = os.path.join(root, "src", "app", "symbol_fonts")
for f in sorted(f for f in os.listdir(fonts_dir) if f.endswith(".c")):   # not the licence texts
    m = re.search(r"^// symbols: (.*)$", open(os.path.join(fonts_dir, f)).read(), re.M)
    if not m or sorted(m.group(1).split()) != sorted(set(listed)):
        problems.append(f"src/app/symbol_fonts/{f} does not hold the symbols of tools/fonts/symbols.txt")
if problems:
    print("\n".join(problems) + "\nAdd it to tools/fonts/symbols.txt and run tools/fonts/build_fonts.sh.")
    env.Exit(1)  # noqa: F821
