#!/bin/bash

# Source utility script
UTIL_SCRIPT="common-util.sh"
if [[ ! -e $UTIL_SCRIPT ]]; then
  echo "Error: Could not find utility script $UTIL_SCRIPT"
  exit 1
fi
source $UTIL_SCRIPT

# Check for python virtual environment
VENV_DIR="$ROOT_DIR/python/venv"
VENV_ACTIVATE_SCRIPT="$VENV_DIR/bin/activate"
if [[ ! -e $VENV_ACTIVATE_SCRIPT ]]; then
  echo "Error: Python virtual environment not found. Install with setup-python.sh script."
  exit 1
fi

# Activate python virtual environment
source $VENV_ACTIVATE_SCRIPT

# Start jupyter notebook server
TRAIN_SCRIPT="$ROOT_DIR/train_micro_speech_model.ipynb"
jupyter notebook --port 9999 --no-browser $TRAIN_SCRIPT
