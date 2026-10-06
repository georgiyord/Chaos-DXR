#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
. $SCRIPT_DIR/wine-env.sh

PROJECT_DIR=$( cd $SCRIPT_DIR/../.. ; pwd -P )

TMP_DIR=$(mktemp -d)
cd $TMP_DIR

echo "Test 1: WinMain return 0"
$PROJECT_DIR/MSVC/bin/x64/cl $PROJECT_DIR/tests/wine/winmain.cpp /Fe:$TMP_DIR/winmain.exe
echo "Test 1: Compiled"
wine $TMP_DIR/winmain.exe
echo "Test 1: Passed"

echo "Test 2: Window creation"
$PROJECT_DIR/MSVC/bin/x64/cl $PROJECT_DIR/tests/wine/window.cpp user32.lib /Fe:$TMP_DIR/window.exe
echo "Test 2: Compiled"
wine $TMP_DIR/window.exe
echo "Test 2: Passed"

echo "Test 3: DXGI 1.1 factory"
$PROJECT_DIR/MSVC/bin/x64/cl $PROJECT_DIR/tests/wine/dxgi.cpp user32.lib dxgi.lib /Fe:$TMP_DIR/dxgi.exe /std:c++17
echo "Test 3: Compiled"
wine $TMP_DIR/dxgi.exe
echo "Test 3: Passed"

rm -rf $TMP_DIR