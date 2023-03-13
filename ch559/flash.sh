#!/bin/bash
set -euo pipefail

objcopy=$1
input=$2
output=$3

$objcopy -I ihex -O binary "$input" "$output"
./chflasher.py "$output"
