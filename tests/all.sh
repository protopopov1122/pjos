#!/usr/bin/env bash

set -xe

python ./generate_test.py ../bin/sat_solver 10000 30 php 1 8 1 8
python ./generate_test.py ../bin/sat_solver 1000 30 count 8 10 3 5
python ./generate_test.py ../bin/sat_solver 50000 30 random 4 20 200