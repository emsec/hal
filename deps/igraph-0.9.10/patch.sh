#!/bin/bash
cp $1/IGRAPH_VERSION $2/
patch -u $2/src/CMakeLists.txt -i $1/patch.txt
