#!/bin/bash

snr=10.0
ampl=0.01
dump=ber.dat

size=14
num=10000

interp=16
freq=2.45G

txaddr=
rxaddr=

TX=

log=

bitrate=1

while [ $# -gt 0 ]; do
  case $1 in
    -a|--amp|--snr) ampl=$2; snr=$2; shift ;;
    -i|--interp) interp=$2; shift ;;
    -f|--freq) freq=$2; shift ;;
    -u|--usrp) txaddr=$2; rxaddr=$3; shift 2 ;;
    -s|--size) size=$2; shift ;;
    -n|--num)  num=$2; shift ;;
    -b|--bitrate) bitrate=$2; shift ;;
    -d) dump=$2; shift ;;
    -x) TX="--txdata $2"; shift ;;
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
    -b|--bitrate)      select bitrate 1-8 ($bitrate)
    -d)                dump raw snr to ($dump)
    -x)                packet data to be transmitted (leave empty to use random)
    --log)             enable flowgraph logging
EOF
      exit 1
      ;;
  esac
  shift
done

QAM="-s ${size} --bitrate ${bitrate}"

$PYTHON ./qam_tx.py --outfile TX.dat ${QAM} -N ${num} ${TX} --burst ${num} ${log}

if [ -z $rxaddr ]; then
  # simulated channel
  $PYTHON ./channel.py --infile TX.dat --outfile RX.dat --snr ${snr} #--frequency-offset 0.35
else
  echo "USRP ${freq} ${interp}"
  # actual channel
  $PYTHON ./ofdm_rx.py -a d -f ${freq} -i ${interp} --gain 35 --outfile RX.dat &
  pid=$!
  sleep 1
  $PYTHON ./ofdm_tx.py -a c -f ${freq} -i ${interp} --amp ${ampl} --infile TX.dat
  sleep 1
  kill -SIGINT $pid
  sleep 2
fi

# per-packet BER
$PYTHON ./qam_rx.py --infile RX.dat --berdata ${dump} --bermode 0 ${QAM} ${TX} ${log}

$PYTHON ./berstats.py < ${dump}

