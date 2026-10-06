#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
. $SCRIPT_DIR/wine-env.sh

if [ -d $WINEPREFIX ]; then
	while true; do
		read -p "Prefix $WINEPREFIX_NAME already exists. Do you want to recreate it? (Y/*)" yn
		case $yn in
			[Yy]* ) break;;
			* ) exit;;
		esac
	done
	rm -rf $WINEPREFIX
fi

export WINEARCH=win64
unset DISPLAY

export WINEDEBUG=-all
wineboot -i

echo "Created wine prefix in current directory: $WINEPREFIX_NAME"