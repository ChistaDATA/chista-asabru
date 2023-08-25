#!/bin/bash

# Getting the root directory of your project
root_dir=$(pwd)

# For each subdirectory
for dir in $(find ./lib -maxdepth 1 -type d)
do
  # Avoid the root directory itself ('.'). 
  if [[ $dir != "./lib" ]]
  then
    cd $dir

    # If the build directory doesn't exist, create it.
    if [[ ! -d build ]]
    then
      mkdir build
    fi

    # Move into the build directory, run cmake and make.
    cd build
    make clean
    cmake ..
    make

    # Go back to the root directory
    cd $root_dir
  fi
done