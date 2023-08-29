#!/usr/bin/bash

# Source utility script
UTIL_SCRIPT="common-util.sh"
if [[ ! -e $UTIL_SCRIPT ]]; then
  echo "Error: Could not find utility script $UTIL_SCRIPT"
  exit 1
fi
source $UTIL_SCRIPT

# Activate anaconda python virtual environment
PYTHON_VENV_DIR="$ROOT_DIR/venv"
# conda activate $VENV_DIR
export PATH=$PATH:$(realpath $(dirname $(which conda))/../bin)
source activate $PYTHON_VENV_DIR

# Start tensorboard
# TRAIN_SCRIPT="$ROOT_DIR/train_micro_speech_model.ipynb"
cd $ROOT_DIR
TF_LOGS_DIR=$ROOT_DIR/logs
TB_PORT=9998
tensorboard --logdir $TF_LOGS_DIR --port $TB_PORT

conda deactivate

# Tunnel tensorboard port to localhost
# ssh twix -L 9998:localhost:9998
