#!/usr/bin/env bash

set -xe
SAT_SOLVER="$1"

for sample in $(find samples -type f -print);
do
    python ./sample_test.py "$SAT_SOLVER" 180 "$sample"
done
python ./generate_test.py "$SAT_SOLVER" 10000 30 php 1 8 1 8
python ./generate_test.py "$SAT_SOLVER" 1000 30 count 7 9 3 5
python ./generate_test.py "$SAT_SOLVER" 50000 30 random 3 10 50
python ./generate_test.py "$SAT_SOLVER" 1000 30 random 4 10 100
python ./generate_test.py "$SAT_SOLVER" 1000 30 random 4 12 130