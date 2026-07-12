#!/bin/bash
set -e

git config --global http.postBuffer 524288000
git config --global http.maxRequests 10
git config --global core.compression 0

git submodule update --init --recursive

if git describe --tags --exact-match >/dev/null 2>&1; then
  export SEMVER="$(git describe --tags --exact-match)"
fi

export GITHUB_SHA_SHORT="$(git rev-parse --short HEAD)"

### --- Export env vars for CMake ------------------------------------------
export HL2SDKCS2="/opt/sdk/hl2sdk-cs2"
export MMSOURCE_DEV="/opt/sdk/metamod-source"
export CSGO_PROTO="/opt/sdk/Protobufs/csgo"

echo "Using HL2SDKCS2=$HL2SDKCS2"
echo "Using MMSOURCE_DEV=$MMSOURCE_DEV"
echo "Using CSGO_PROTO=$CSGO_PROTO"

### --- Build ---------------------------------------------------------------
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
