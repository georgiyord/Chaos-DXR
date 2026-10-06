#!/usr/bin/env bash
set euxo -pipefail

PROJECT_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")/../.." ; pwd )
ENV_FILE="$PROJECT_DIR/.env"

set -a
source "$ENV_FILE"
set +a

: "${WINEPREFIX_NAME:?WINEPREFIX_NAME is not set}"

export WINEPREFIX=$(realpath ./$WINEPREFIX_NAME)

echo "Using WINEPREFIX=$WINEPREFIX"
