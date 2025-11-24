#!/bin/bash

# THIS IS MEANT TO BE RUN FROM build.sh
# THIS IS ALSO REALLY BAD
#   it should just be scanning the directory for .frag/.vert/.whatever files
#   im too lazy to do that though

THISPATH="data/eng/shaders/vulkan/"

SHADERS=(
    "tri.frag"
    "tri.vert"
    )

COMPILER=("glslc")

try_cache() {
    if command -v ccache >/dev/null 2>&1; then
        COMPILER=("ccache" "${COMPILER[@]}")
    fi
}

try_cache

for s in "${SHADERS[@]}"; do
    "${COMPILER[@]}" "$THISPATH$s" -o "$THISPATH$s.spv"
done
