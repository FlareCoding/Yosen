#!/bin/bash
cd ..

if [ ! -d "build" ]; then
  mkdir -p "build";
fi

cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
