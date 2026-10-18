#!/bin/sh
# Times tred on a few key scripts at a fixed DOSBox-X speed.
# usage: tools/dosbench.sh RUNDIR FILE [cycles]   (default 300, roughly a 4.77 MHz XT)
T=$(cd "$(dirname "$0")" && pwd)
RUN=$1; FILE=$2; CYC=${3:-300}
DOWN=$(printf '{Down}%.0s' $(seq 60))
TYPE='{Down}{Down}{End}'$(printf ' Tieengs Vieetj%.0s' $(seq 4))
PAGE=$(printf '{PgDn}%.0s' $(seq 10))$(printf '{PgUp}%.0s' $(seq 10))
for name in DOWN TYPE PAGE; do
  eval keys=\$$name
  DOSBOX_SET="cpu cycles=fixed $CYC" "$T/dosrun.sh" "$RUN" "$keys" "$FILE" >/dev/null || { echo "$name: failed"; continue; }
  set -- $(cat "$RUN/TIMING.TXT")
  n=$(printf '%s' "$keys" | sed 's/{[^}]*}/K/g' | wc -c | tr -d ' ')
  echo "$name: $n keys, $(( $1 * 1000 * 10 / 182 )) ms, $(( $1 * 1000 * 10 / 182 / n )) ms/key, $2 cells written"
done
