#!/bin/bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

ls ch559/src/* example/src/* | grep -v "CH559.h" | xargs clang-format -i
