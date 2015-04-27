#!/bin/bash

python test.py

./gtsvm.py -v 3 -g 0.7 -c 3.0 heart_scale
./grid heart_scale
./easy heart_scale
