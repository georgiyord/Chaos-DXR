#!/usr/bin/env bash
set -euo pipefail

SKIP_FETCH=0
SKIP_TEST=0

SCRIPT_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
. $SCRIPT_DIR/wine-env.sh

for arg in "$@"; do
    case $arg in
        --skip-fetch) SKIP_FETCH=1;;
        --skip-test) SKIP_TEST=1;;
    esac
done

if [[ $SKIP_FETCH -eq 0 ]]; then
    $SCRIPT_DIR/create-wine-prefix.sh
    $SCRIPT_DIR/install-MSVC.sh
fi

export PATH="$(realpath $SCRIPT_DIR/../../MSVC/bin/x64):$PATH"

# wineserver -k
# wineserver -p
# wine wineboot	

if [[ $SKIP_TEST -eq 0 ]]; then
    echo "Do test"
    $SCRIPT_DIR/test.sh
fi