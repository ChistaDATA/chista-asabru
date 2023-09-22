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
)
# For each subdirectory
# for dir in $(find ./lib -maxdepth 1 -type d)
for dir in ${list_of_directories[@]}
do
  # Avoid the root directory itself ('.'). 
  if [[ $dir != "./lib" ]]
  then
    # if [[ $dir == "./lib/libuv" ]]; then
    #   continue
    # fi
    cd $dir
    # rm -rf ./build
    # If the build directory doesn't exist, create it.
    if [[ ! -d build ]]
    then
      mkdir build
    fi

    # Move into the build directory, run cmake and make.
    cd build
    make clean
    cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    make

    # Go back to the root directory
    cd $root_dir
  fi
done