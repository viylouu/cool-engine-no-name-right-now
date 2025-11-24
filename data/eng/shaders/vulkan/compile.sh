#!/bin/bash

# THIS IS MEANT TO BE RUN FROM build.sh

THISPATH="data/eng/shaders/vulkan/"

SHADERS=(
    "tri.frag"
    "tri.vert"
    )

for s in "${SHADERS[@]}"; do
    glslc "$THISPATH$s" -o "$THISPATH$s.spv"
done
