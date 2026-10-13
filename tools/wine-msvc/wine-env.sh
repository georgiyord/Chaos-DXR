#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")/../.." ; pwd )
ENV_FILE="$PROJECT_DIR/.env"

set -a
source "$ENV_FILE"
set +a

: "${WINEPREFIX_NAME:?WINEPREFIX_NAME is not set}"

export WINEPREFIX=$PROJECT_DIR/$WINEPREFIX_NAME
export PATH=$PROJECT_DIR/MSVC/bin/x64:$PATH

echo "Using WINEPREFIX=$WINEPREFIX"
