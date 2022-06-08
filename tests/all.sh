#!/usr/bin/env bash

set -xe
SCRIPT_DIR="$(dirname $0)"
SAT_SOLVER="$1"

for sample in $(find $SCRIPT_DIR/samples -name "*.cnf" -print -or -name "*.in" -print);
do
    python "$SCRIPT_DIR/sample_test.py" "$SAT_SOLVER" 240 "$sample"
done
python "$SCRIPT_DIR/generate_test.py" "$SAT_SOLVER" 10000 30 php 1 8 1 8
python "$SCRIPT_DIR/generate_test.py" "$SAT_SOLVER" 1000 30 count 7 9 3 5
python "$SCRIPT_DIR/generate_test.py" "$SAT_SOLVER" 50000 30 random 3 10 50
python "$SCRIPT_DIR/generate_test.py" "$SAT_SOLVER" 1000 30 random 4 10 100
python "$SCRIPT_DIR/generate_test.py" "$SAT_SOLVER" 1000 30 random 4 12 130