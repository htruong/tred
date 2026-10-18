#!/bin/sh
# Runs src/tred.exe in DOSBox-X with a key script and dumps the screen.
# usage: tools/dosrun.sh RUNDIR "keys" [file]   e.g. tools/dosrun.sh /tmp/run '{Down}abc' TEST.TXT
# RUNDIR gets KEYS.TXT and the dump; then use tools/vgarender.py RUNDIR out.png|--text
T=$(cd "$(dirname "$0")" && pwd)
RUN=$1
WAT=${WATCOM:-$HOME/develop/open-watcom-v2/rel}
DOSBOX=${DOSBOX:-/Applications/dosbox-x.app/Contents/MacOS/dosbox-x}
mkdir -p "$RUN"
rm -f "$RUN/TEXT.BIN" "$RUN/FONT.BIN" "$RUN/REGS.TXT"
cp "$T/../src/tred.exe" "$RUN/TRED.EXE"
if [ ! -f "$RUN/VGADUMP.EXE" ]; then
  cp "$T/vgadump.c" "$RUN/VGADUMP.C"
  "$DOSBOX" -silent -nogui -nomenu -fastlaunch -exit -c "mount c $RUN" -c "mount d $WAT" -c "c:" \
    -c "set WATCOM=D:\\" -c "set INCLUDE=D:\\H" -c "set PATH=D:\\BINW;Z:\\" -c "wcl -bt=dos -ml vgadump.c > NUL" >/dev/null 2>&1
fi
printf '%s' "$2" > "$RUN/KEYS.TXT"
printf '@echo off\r\ntred -k KEYS.TXT %s\r\nvgadump\r\n' "$3" > "$RUN/GO.BAT"
"$DOSBOX" -silent -nogui -nomenu -fastlaunch -exit -set "cpu cycles=max" -c "mount c $RUN" -c "c:" -c "GO.BAT" >/dev/null 2>&1
[ -f "$RUN/TEXT.BIN" ] || { echo "no dump (tred hung or crashed?)"; exit 1; }
