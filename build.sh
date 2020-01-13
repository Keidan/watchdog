#!/bin/bash

echo "To change the path of the configuration file (default: /etc) please use the -DCONFIG_DIR=<path> parameter."
echo "e.g.: ./build.sh -DCONFIG_DIR=/etc/watchdog"

cmake -DDISTRIBUTION=debug -DCMAKE_BUILD_TYPE=$(uname -m) ${*}
make
cmake -DDISTRIBUTION=release -DCMAKE_BUILD_TYPE=$(uname -m) ${*}
make
