#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
. $SCRIPT_DIR/wine-env.sh

$SCRIPT_DIR/create-wine-prefix.sh
$SCRIPT_DIR/install-MSVC.sh

# wineserver -k
# wineserver -p
# wine wineboot	

$SCRIPT_DIR/test.sh