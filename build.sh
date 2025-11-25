#!/bin/bash

#### SETUP

start=$(date +%s.%N)
mkdir -p build

#### VARS
COMPILER_C=("gcc")

C_COMPFLAGS="-std=c99 -pedantic -Wall -Wextra  -Iengine -I. -Ideps -isystem"
C_LINKFLAGS=""

OBJDIR="build/obj"
OBJS=()

#### FLAGS
BUILD_TEST=false
EXAMPLE=""

[[ "$OSTYPE" == "linux-gnu" ]] && BUILD_WINDOWS=false || BUILD_WINDOWS=true

for arg in "$@"; do
    if [ "$arg" = "-t" ]; then
        BUILD_TEST=true
    #elif [ "$arg" = "-w" ]; then
    #    BUILD_WINDOWS=true
    #else
    #    EXAMPLE="$arg"
    fi
done

#### VERIFY COMPILERS

#### #### C COMPILER
if $BUILD_WINDOWS && command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    COMPILER_C=("x86_64-w64-mingw32-gcc")
#elif $BUILD_TEST && command -v tcc --version &> /dev/null; then
#    COMPILER_C=("tcc")
else
    if command -v gcc &> /dev/null; then
        COMPILER_C=("gcc")
    elif command -v clang &> /dev/null; then
        COMPILER_C=("clang")
    elif command -v cc &> /dev/null; then
        COMPILER_C=("cc")
    else
        echo "failed to find c compiler! (checked gcc, clang, cc)"
        exit 1
    fi
fi

echo "using c compiler $COMPILER_C"

#### FLAGS AGAIN
#if [ -n "$EXAMPLE" ]; then
#    SRC_DIRS=("." "examples/$EXAMPLE")
#else
    SRC_DIRS=(".")
#fi


if $BUILD_WINDOWS; then
    C_LINKFLAGS+=" -Ldeps/VULKAN -Lengine/deps/VULKAN -lvulkan-1 -Ldeps/GLFW -Lengine/deps/GLFW -lglfw3 -lgdi32"
    #C_COMPFLAGS+=" -fno-sanitize=undefined"
else
    C_LINKFLAGS+=" -lvulkan -lglfw -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lm"
fi

if $BUILD_TEST; then
    C_COMPFLAGS+=" -O0 -g -fsanitize=address"
    C_LINKFLAGS+=" -g -fno-lto -fsanitize=address"
else
    C_COMPFLAGS+=" -O3 -fno-sanitize=address"
    C_LINKFLAGS+=" -flto -fno-sanitize=address"
fi

#### FUNCS
FILES_C=()

find_files() {
    for dir in "${SRC_DIRS[@]}"; do
        while IFS= read -r file; do
            [[ -f "$file" ]] || continue
            case "${file##*.}" in
                c) FILES_C+=("$file") ;;
            esac
        done < <(find "$dir" -type f ! -path "*/examples/*")
    done
}

gen_compcmds() {
    OUT="compile_commands.json"
    FIRST=true
    
    echo "[" > "$OUT"

    for file in  "${FILES_C[@]}"; do
        if [[ $file == *.c ]]; then
            COMP="gcc"
            FLAGS=$C_COMPFLAGS
        fi

        CMD="$COMP $FLAGS -c \\\"$file\\\""

        if $FIRST; then
            FIRST=false
        else
            echo "," >> "$OUT"
        fi

        echo -ne "\t{
            \"directory\": \"$(pwd)\",
            \"command\": \"$CMD\",
            \"file\": \"$(pwd)/${file#./}\"
        }" >> "$OUT"
    done

    echo -e "\n]" >> "$OUT"
}

try_cache() {
    if command -v ccache >/dev/null 2>&1; then
        echo "ccache enabled!"
        COMPILER_C=("ccache" "${COMPILER_C[@]}")
    fi
    #echo "hi :)"
}

C_compile_file() {
    local file="$1"

    cleanpath="${file#./}"
    obj="$OBJDIR/${cleanpath%.c}.o"
    mkdir -p "$(dirname "$obj")"
    echo "$obj" >> "$OBJDIR/objs.tmp"

    #skip_compile=false
    "${COMPILER_C[@]}" $C_COMPFLAGS \
        -c "$file" -o "$obj"
}

#### COMPILATION
find_files
gen_compcmds
try_cache

> "$OBJDIR/objs.tmp"
rm -rf "$OBJDIR"
mkdir -p "$OBJDIR"

max_jobs=$(nproc)

for file in "${FILES_C[@]}"; do
    C_compile_file "$file" &
    while [ $(jobs -r | wc -l) -ge $max_jobs ]; do
        sleep 0.1
    done
done

wait

./data/eng/shaders/vulkan/compile.sh

#### LINKING

mapfile -t OBJS < "$OBJDIR/objs.tmp"
rm -f "$OBJDIR/objs.tmp"

#if $BUILD_WINDOWS; then
    #"${COMPILER_C[@]}" "${OBJS[@]}" $C_LINKFLAGS -o build/out.exe
    
#else
    #"${COMPILER_C[@]}" -fuse-ld=lld "${OBJS[@]}" $C_LINKFLAGS -o build/out.game
#fi

rm -r intf/lib/ceng.lib 2>/dev/null
mkdir -p intf/lib

if $BUILD_WINDOWS; then
    ar rcs intf/lib/ceng.lib "${OBJS[@]}"
else
    ar rcs intf/lib/libceng.a "${OBJS[@]}"
fi

wait

odin build src -out:"build/out.game" -extra-linker-flags:"$C_LINKFLAGS"

#### RUNNING

end=$(date +%s.%N)
elapsed=$(echo "$end - $start" | bc)
printf "TOOK %.3f SECONDS\n\n" "$elapsed"

if $BUILD_WINDOWS; then
    ( [[ "$OSTYPE" == "linux-gnu" ]] && wine ./build/out.exe || ./build/out.exe)
else
    ./build/out.game
fi
