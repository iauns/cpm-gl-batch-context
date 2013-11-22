#!/bin/bash
cd "$(dirname "$0")"

if [ ! -d ./bin ]; then
  mkdir -p ./bin
fi

pushd ./bin > /dev/null
  cmake -DUSE_OS_MESA=ON ..
  make
popd
