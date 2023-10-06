#! /bin/bash

SCRIPT=$(realpath "$0")
SCRIPT_DIR=$(dirname "$SCRIPT")
PROJECT_DIR=$SCRIPT_DIR

MIC_RP2040_DIR=secure-mic
MIC_MONITOR_DIR=secure-mic-monitor
NETLOG_MONITOR_DIR=netlog-monitor

# Configure
cd $PROJECT_DIR
echo ""
./configure.sh

# Build secure ip microphone audio monitor
echo ""
echo "Building secure ip microphone audio monitor"
cd $MIC_MONITOR_DIR
if [[ ! -d "build" ]]; then
  mkdir build
fi
cd build
cmake ..
make -j$(nproc)
cd $PROJECT_DIR

# Build network logging monitor
echo ""
echo "Building network logging monitor"
cd $NETLOG_MONITOR_DIR
if [[ ! -d "build" ]]; then
  mkdir build
fi
cd build
cmake ..
make -j$(nproc)
cd $PROJECT_DIR

# Build and upload secure ip microphone RP2040
echo ""
echo "Building and uploading secure ip microphone RP2040"
cd $MIC_RP2040_DIR
pio run --environment debug --target upload

