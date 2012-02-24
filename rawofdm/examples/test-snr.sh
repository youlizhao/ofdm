#!/bin/bash

snr=10.0
ampl=0.01
dump=snr.dat

size=14
num=10000

interp=16
freq=2.45G

txaddr=
rxaddr=

TX=

log=

while [ $# -gt 0 ]; do
  case $1 in
    -a|--amp|--snr) ampl=$2; snr=$2; shift ;;
    -i|--interp) interp=$2; shift ;;
    -f|--freq) freq=$2; shift ;;
    -u|--usrp) txaddr=$2; rxaddr=$3; shift 2 ;;
    -s|--size) size=$2; shift ;;
    -n|--num)  num=$2; shift ;;
    -d) dump=$2; shift ;;
    -x) TX="--txdata $2 --char 4"; shift ;;
    --log) log="--log" ;;
    -h|--help|\?)
      cat >&2 <<EOF
Usage:
    $0 [options]
    -a|--amp|--snr)    amplitude (over USRP) or snr (simulated channel) ($ampl/$snr)
    -i|--interp)       interpolation rate ($interp)
    -f|--freq)         center frequency ($freq)
    -u|--usrp) TX RX   send via usrp (leave empty to use simulated channel)
    -s|--size)         number of symbols per packet ($size)
    -n|--num)          number of packets ($num)
    -d)                dump raw snr to ($dump)
    -x)                packet data to be transmitted (leave empty to use random)
    --log)             enable flowgraph logging
EOF
      exit 1
      ;;
  esac
  shift
done

$PYTHON ./ofdm_tx.py --outfile TX.dat -s ${size} -N ${num} ${TX} --burst ${num} ${log}

if [ -z "$rxaddr" ]; then
  # simulated channel
  $PYTHON ./channel.py --infile TX.dat --outfile RX.dat --snr ${snr} #--frequency-offset 0.35
else
  echo "USRP ${freq} ${interp}"
  # actual channel
  $PYTHON ./ofdm_rx.py -a ${rxaddr} -f ${freq}  -i ${interp} --gain 35 --outfile RX.dat &
  pid=$!
  sleep 1
  $PYTHON ./ofdm_tx.py -a ${txaddr} -f ${freq}  -i ${interp} --amp ${ampl} --infile TX.dat
  sleep 1
  kill -SIGINT $pid
  sleep 2
fi

$PYTHON ./ofdm_rx.py --infile RX.dat --snrdata ${dump} --snrmode 1 -s ${size} -N ${num} ${TX} ${log}

$PYTHON ./snrstats.py < ${dump}

