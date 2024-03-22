#!/bin/bash

# Getting the root directory of your project
root_dir=$(pwd)

list_of_directories=(
  ./lib/tinyxml2
  ./lib/libuv
  ./lib/asabru-commons
  ./lib/asabru-engine
  ./lib/asabru-parsers
  ./lib/asabru-handlers
  ./lib/asabru-client
  ./lib/asabru-ui
)
# For each subdirectory
# for dir in $(find ./lib -maxdepth 1 -type d)
for dir in "${list_of_directories[@]}"; do
  # Avoid the root directory itself ('.').
  if [[ $dir != "./lib" ]]; then
    if [[ $dir == "./lib/asabru-ui" ]]; then
      cd $dir
      npm install
      npm run build
      # Go back to the root directory
      cd $root_dir || exit
    else
      cd $dir
      rm -rf ./build
      # If the build directory doesn't exist, create it.
      if [[ ! -d build ]]; then
        mkdir build
      fi

      # Move into the build directory, run cmake and make.
      cd build || exit
      make clean

      if [[ $dir == "./lib/libuv" ]]; then
        cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_TESTING=OFF # -DCMAKE_BUILD_TYPE=Debug
      else
        cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON # -DCMAKE_BUILD_TYPE=Debug
      fi

      make
      # Go back to the root directory
      cd $root_dir || exit
    fi
  fi
done

# Build chista-asabru project
# rm -rf build
# mkdir build
# cd ./build
# cmake ..
# make
