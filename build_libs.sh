#!/bin/bash

# Getting the root directory of your project
root_dir=$(pwd)

TOOLCHAIN_ARG=""

if [ "$1" = "amd64" ]; then
  TOOLCHAIN_ARG="-DCMAKE_TOOLCHAIN_FILE=$root_dir/dist/ubuntu/amd64/toolchain-amd64.cmake"
fi
echo "Toolchain ARG : ${TOOLCHAIN_ARG}"

list_of_directories=(
  #  ./lib/tinyxml2
  #  ./lib/libuv
  # ./lib/asabru-commons
  # ./lib/asabru-engine
  # ./lib/asabru-parsers
  ./lib/asabru-handlers
  #  ./lib/asabru-client
  ./lib/asabru-ui
)
# For each subdirectory
# for dir in $(find ./lib -maxdepth 1 -type d)
for dir in "${list_of_directories[@]}"; do
  # Avoid the root directory itself ('.').
  if [[ $dir != "./lib" ]]; then
    if [[ $dir == "./lib/asabru-ui" ]]; then
      cd $dir || exit
      npm install
      npm run build
      # Go back to the root directory
      cd $root_dir || exit
    else
      cd $dir || exit
      rm -rf ./build
      # If the build directory doesn't exist, create it.
      if [[ ! -d build ]]; then
        mkdir build
      fi

      # Move into the build directory, run cmake and make.
      cd build || exit
      make clean

      if [[ $dir == "./lib/libuv" ]]; then
        cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_TESTING=OFF ${TOOLCHAIN_ARG} # -DCMAKE_BUILD_TYPE=Debug
      elif [[ $dir == "./lib/asabru-handlers" ]]; then
        cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DASABRU_COMMONS_BUILD=LOCAL_DIR -DASABRU_ENGINE_BUILD=LOCAL_DIR -DASABRU_PARSERS_BUILD=LOCAL_DIR ${TOOLCHAIN_ARG} # -DCMAKE_BUILD_TYPE=Debug
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
