#!/bin/sh
# Clean build of tred inside DOSBox-X with the DOS-hosted Open Watcom tools.
# usage: tools/dosbuild.sh   (WATCOM defaults to ~/develop/open-watcom-v2/rel)
# Always rebuilds everything: DOS timestamps are too coarse to trust.
cd "$(dirname "$0")/../src" || exit 1
SRC=$(pwd)
WAT=${WATCOM:-$HOME/develop/open-watcom-v2/rel}
DOSBOX=${DOSBOX:-/Applications/dosbox-x.app/Contents/MacOS/dosbox-x}
rm -f BUILD.LOG ./*.OBJ ./*.obj ./*.EXE ./*.exe ./*.ERR ./*.err
"$DOSBOX" -silent -nogui -nomenu -fastlaunch -exit -set "cpu cycles=max" \
  -c "mount c $SRC" -c "mount d $WAT" -c "c:" \
  -c "set WATCOM=D:\\" -c "set INCLUDE=D:\\H" -c "set PATH=D:\\BINW;Z:\\" \
  -c "wmake -h > BUILD.LOG" >/dev/null 2>&1
grep -E "Warning!|Error!|errors" BUILD.LOG | grep -v " 0 warnings, 0 errors"
cat ./*.ERR 2>/dev/null
[ -f tred.exe ] || [ -f TRED.EXE ] || { echo "build failed"; exit 1; }
echo "built src/tred.exe"
