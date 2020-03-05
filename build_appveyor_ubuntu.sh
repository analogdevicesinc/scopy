#!/bin/bash

JOBS=$JOBS

cd ~/projects/scopy
mkdir build
cd build
cmake ../
make $JOBS

