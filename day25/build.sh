#!/usr/bin/env bash
set -euo pipefail

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
CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release"

mkdir -p build
pushd build >& /dev/null
${COMMAND_PREFIX} cmake .. $CMAKE_ARGS
${COMMAND_PREFIX} make
popd >& /dev/null