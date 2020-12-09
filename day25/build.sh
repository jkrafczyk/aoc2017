#!/usr/bin/env bash
set -euo pipefail
if [[ -z "${BUILD_TYPE-}" ]]; then
  BUILD_TYPE="Release"
fi

if [[ -d /usr/local/opt/llvm/bin ]]; then
    PATH="/usr/local/opt/llvm/bin:$PATH"
fi

if which scan-build >& /dev/null; then
  COMMAND_PREFIX=scan-build
  CMAKE_ARGS="-DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang"
else
  COMMAND_PREFIX=
  CMAKE_ARGS=
fi
CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_BUILD_TYPE=$BUILD_TYPE"

mkdir -p "build-${BUILD_TYPE}"
pushd "build-${BUILD_TYPE}" >& /dev/null
${COMMAND_PREFIX} cmake .. $CMAKE_ARGS
${COMMAND_PREFIX} make -j
popd >& /dev/null
