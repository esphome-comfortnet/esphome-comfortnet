#!/usr/bin/env bash

set -e
# set -x

mkdir -p lib
if [ ! -d "lib/esphome" ]; then
    git clone https://github.com/esphome/esphome.git lib/esphome
fi
script/setup.sh

cpp_json=.vscode/c_cpp_properties.json
if [ ! -f $cpp_json ]; then
    echo "Initializing PlatformIO..."
    pio init --ide vscode --silent
    sed -i "s%esphome-comfortnet/src%esphome-comfortnet/components/comfortnet/\*\*%g" $cpp_json
else
    echo "Cpp environment already configured. To reconfigure it you can run one the following commands:"
    echo "  pio init --ide vscode"
fi
