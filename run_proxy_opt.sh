#!/bin/bash

# Getting the root directory of your project
root_dir=$(pwd)

# Function to open a Terminal window
open_terminal() {
    local index="$1"
    local dir="$2"
    local url="$3"
    local plugins_folder_path="$4"
    local command_to_run="$5"

    osascript -e "tell application \"Terminal\" to do script \"cd '$dir'; export CONFIG_FILE_URL='$url'; export CONFIG_FILE_PATH='/tmp/config-$index.xml'; export PLUGINS_FOLDER_PATH='$plugins_folder_path'; echo 'CONFIG_FILE_URL=\$CONFIG_FILE_URL'; '$command_to_run';\""
}

# Specify the desired directory path and command
DIR="$root_dir/build"
plugins_folder_path="$root_dir/lib/asabru-handlers/build"
command_to_run="./Chista_Asabru"

# Array of URLs
URLS=(
    "https://pastebin.com/raw/jauuQcFL"
    "https://pastebin.com/raw/F3B2XFXL"
    "https://pastebin.com/raw/C3Fy5qG9"
    "https://pastebin.com/raw/BWv2jXWE"
)

# Loop through URLs and open Terminal windows
for ((i = 0; i < ${#URLS[@]}; i++)); do
    url="${URLS[i]}"
    open_terminal "$i" "$DIR" "$url" "$plugins_folder_path" "$command_to_run"
done
