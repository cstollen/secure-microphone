#!/bin/bash

SCRIPT=$(realpath "$0")
SCRIPT_DIR=$(dirname "$SCRIPT")
ROOT_DIR=$SCRIPT_DIR

NETLOG_SERVER_BIN=$(realpath $ROOT_DIR/netlog-monitor/build/netlog-monitor)

if [[ ! -f "$NETLOG_SERVER_BIN" ]]; then
	echo "Network logging server executable not found: $NETLOG_SERVER_BIN"
  echo "Run script to build: build-all.sh"
  exit 1
fi

$NETLOG_SERVER_BIN
