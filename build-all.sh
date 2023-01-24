#! /bin/bash

if [[ ! -f "build-all.sh" ]]; then
  echo "Execute script from project root folder"
  exit 1
fi

PROJECT_DIR=$PWD
MIC_RP2040_DIR=secure-mic
MIC_MONITOR_DIR=secure-mic-monitor
NETLOG_MONITOR_DIR=netlog-monitor

# Configure
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

