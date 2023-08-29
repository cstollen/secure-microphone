#!/usr/bin/bash -i

# Source utility script
UTIL_SCRIPT="common-util.sh"
if [[ ! -e $UTIL_SCRIPT ]]; then
  echo "Error: Could not find utility script $UTIL_SCRIPT"
  exit 1
fi
source $UTIL_SCRIPT

if false; then
  # Check for python virtual environment
  VENV_DIR="$ROOT_DIR/python/venv"
  VENV_ACTIVATE_SCRIPT="$VENV_DIR/bin/activate"
  if [[ ! -e $VENV_ACTIVATE_SCRIPT ]]; then
    echo "Error: Python virtual environment not found. Install with setup-python.sh script."
    exit 1
  fi

  # Activate python virtual environment
  source $VENV_ACTIVATE_SCRIPT
fi

# Activate anaconda python virtual environment
VENV_DIR="$ROOT_DIR/venv"
# conda init bash
# eval $(conda shell.bash hook)
# source ~/.bashrc
# conda shell.bash activate $VENV_DIR
conda activate $VENV_DIR

# Start jupyter notebook server
# TRAIN_SCRIPT="$ROOT_DIR/train_micro_speech_model.ipynb"
cd $ROOT_DIR
TRAIN_SCRIPT="train_micro_speech_model.ipynb"
jupyter notebook --no-browser --port 9999
# conda shell.bash deactivate
conda deactivate

# Appendix
# Check for tensorflow microfrontend
# python3 -c "import tensorflow.lite.experimental.microfrontend.python.ops"