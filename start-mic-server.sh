#!/bin/bash

SCRIPT=$(realpath "$0")
SCRIPT_DIR=$(dirname "$SCRIPT")
ROOT_DIR=$SCRIPT_DIR

MIC_SERVER_BIN=$(realpath $ROOT_DIR/secure-mic-monitor/build/secure-mic-monitor)

if [[ ! -f "$MIC_SERVER_BIN" ]]; then
	echo "Secure microphone server executable not found: $MIC_SERVER_BIN"
  echo "Run script to build: build-all.sh"
  exit 1
fi

$MIC_SERVER_BIN
