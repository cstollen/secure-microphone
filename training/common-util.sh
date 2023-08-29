#!/bin/bash

# Colors
WHITE='\033[0;37m'
GRAY='\033[0;90m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NOCOLOR='\033[0m'

# Print beautification for maximizing developer experience 
CHECK_MARK="\033[0;32m\xE2\x9C\x94\033[0m"
CROSS="\u274c"
FAILED=" ${CROSS}  ${RED}failed!${NOCOLOR}"
CLEAR_LINE="\033[2K"
REMOVE_THREE_DOTS="\b\b\b\\u0020\\u0020\\u0020\b\b\b"

function ensureFileExists() {
  FP=$1

  if [[ ! -e $FP ]]; then
    echo "Error: Could not find file $FP"
    exit 1
  fi
}

function ensureDirExists() {
  DP=$1

  if [[ ! -d $DP ]]; then
    echo "Error: Could not find folder $DP"
    exit 1
  fi
}

function getElapsedTimeStr() {
  echo $(date -ud "@$1" "+%H:%M:%S")
}

# directory structure
CWD=$(pwd)
CWD_PARENT=$(dirname ${CWD})
CWD_NAME=$(basename ${CWD})
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT_DIR=$SCRIPT_DIR
LOG_DIR=$ROOT_DIR/log

# Create log dir if non existant
if [[ ! -d "$LOG_DIR" ]]; then
  mkdir -p $LOG_DIR
fi
