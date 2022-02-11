#!/bin/bash
if [ ! -d "build" ]; then
  mkdir -p "build";
fi

cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

read -p "Press enter to continue"
