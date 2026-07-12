#!/bin/bash
set -e

SDK=/opt/sdk

mkdir -p "$SDK"

echo "=== Updating HL2SDK ==="

if [ ! -d "$SDK/hl2sdk-cs2/.git" ]; then
    git clone \
      --branch cs2 \
      https://github.com/alliedmodders/hl2sdk \
      "$SDK/hl2sdk-cs2"
else
    cd "$SDK/hl2sdk-cs2"
    git pull --ff-only
fi


echo "=== Updating Metamod ==="

if [ ! -d "$SDK/metamod-source/.git" ]; then
    git clone --recursive \
      https://github.com/alliedmodders/metamod-source \
      "$SDK/metamod-source"
else
    cd "$SDK/metamod-source"
    git pull --ff-only
    git submodule update --init --recursive
fi


echo "=== Updating Protobufs ==="

if [ ! -d "$SDK/Protobufs/.git" ]; then
    git clone \
      https://github.com/SteamDatabase/Protobufs \
      "$SDK/Protobufs"
else
    cd "$SDK/Protobufs"
    git pull --ff-only
fi

export HL2SDKCS2="/opt/sdk/hl2sdk-cs2"
export MMSOURCE_DEV="/opt/sdk/metamod-source"
export CSGO_PROTO="/opt/sdk/Protobufs/csgo"

echo "Using HL2SDKCS2=$HL2SDKCS2"
echo "Using MMSOURCE_DEV=$MMSOURCE_DEV"
echo "Using CSGO_PROTO=$CSGO_PROTO"


echo "=== SDK ready ==="

### --- Build ---------------------------------------------------------------

cd /app/source

echo "=== Starting build ==="

rm -rf build
mkdir build
cd build

cmake .. -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++

echo "=== Building with GCC | Release | All ==="
cmake --build . -j"$(nproc)"

echo "=== Done Building ==="