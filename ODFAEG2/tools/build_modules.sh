#!/bin/bash
set -e

# Racine du projet
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/src/odfaeg"
PCM="$ROOT/build/pcm"

mkdir -p "$PCM/Core" "$PCM/Math" "$PCM/Window" "$PCM/Graphics" "$PCM/Entity" "$PCM/Physics"

CLANG="clang++-17"
FLAGS="-std=c++20 -fmodules -stdlib=libc++ -I$ROOT/include"

echo "=== Compilation des modules ODFAEG ==="

compile_module() {
    local module_path="$1"
    local module_name="$(basename "$module_path" .cppm)"
    local module_dir="$(dirname "$module_path")"
    local category="$(basename "$module_dir")"

    echo "→ Module $category.$module_name"

    $CLANG $FLAGS -x c++-module "$module_path" -o "$PCM/$category/$module_name.pcm"
}

# 1) Core d'abord (base de tout)
for f in $SRC/Core/*.cppm; do
    compile_module "$f"
done

# 2) Math (dépend de Core)
for f in $SRC/Math/*.cppm; do
    compile_module "$f"
done

# 3) Window (dépend de Core)
for f in $SRC/Window/*.cppm; do
    compile_module "$f"
done

# 4) Graphics (dépend de Core + Math + Window)
for f in $SRC/Graphics/*.cppm; do
    compile_module "$f"
done

# 5) Entity (dépend de Core + Math)
for f in $SRC/Entity/*.cppm; do
    compile_module "$f"
done

# 6) Physics (dépend de Core + Math + Graphics)
for f in $SRC/Physics/*.cppm; do
    compile_module "$f"
done

echo "=== Modules compilés avec succès ==="
