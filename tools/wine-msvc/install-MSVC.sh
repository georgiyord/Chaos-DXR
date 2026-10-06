#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
. $SCRIPT_DIR/wine-env.sh

PROJECT_DIR=$( cd $SCRIPT_DIR/../.. ; pwd -P )

if [ -d $PROJECT_DIR/MSVC ]; then
	while true; do
		read -p "MSVC is already installed. Do you want to reinstall it? (Y/*)" yn
		case $yn in
			[Yy]* ) break;;
			* ) exit;;
		esac
	done
fi
rm -rf $PROJECT_DIR/MSVC
EXT_TOOL_DIR="$SCRIPT_DIR/third-party/msvc-wine"
$EXT_TOOL_DIR/vsdownload.py --dest $PROJECT_DIR/MSVC
$EXT_TOOL_DIR/install.sh $PROJECT_DIR/MSVC
