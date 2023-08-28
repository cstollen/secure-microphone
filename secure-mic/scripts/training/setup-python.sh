#!/bin/bash

# Time
SECONDS=0

UTIL_SCRIPT="common-util.sh"
if [[ ! -e $UTIL_SCRIPT ]]; then
  echo "Error: Could not find utility script $UTIL_SCRIPT"
  exit 1
fi
source $UTIL_SCRIPT

# Parameters
TF_DATASET_DIR=$ROOT_DIR/tf_train_speech_commands
PYTHON_VERSION="3.7.17"
LOG_FILE=$LOG_DIR/setup-python.log

# Paths
PYTHON_STR="Python-${PYTHON_VERSION}"
PYTHON_DIR=$ROOT_DIR/python
PYTHON_BUILD_DIR=$PYTHON_DIR/build
PYTHON_INSTALL_DIR=$PYTHON_DIR/localpython
PYTHON_VENV_DIR=$PYTHON_DIR/venv
PYTHON_LOCAL_CMD=$PYTHON_INSTALL_DIR/bin/python3

# Log header
echo -en "Setup Python environment ..."
echo "Setup python environment" &> $LOG_FILE
echo "────────────────────────" &>> $LOG_FILE

# Check python command
PY_CMD=$(which python3)
if [[ ! -z "$PY_CMD" ]]; then
  # echo "Error: Command python3 not found. Please install Python >= 3.8"
  # exit 1
  PY_VER_MAJOR=$($PY_CMD --version | sed 's/.* \([0-9]*\).\([0-9]*\).*/\1/')
  PY_VER_MINOR=$($PY_CMD --version | sed 's/.* \([0-9]*\).\([0-9]*\).*/\2/')
fi

# Check python version
PY_INSTALL=0
if [[ "${PY_VER_MINOR}" -lt "7" || "${PY_VER_MINOR}" -gt "7" ]]; then
  PY_INSTALL=1
  PY_CMD=$PYTHON_LOCAL_CMD
fi

# Install python locally
if [[ "$PY_INSTALL" -eq "1" && -z "$(which $PY_CMD)" ]]; then
  echo -e "\nInstalling python3 locally" &>> $LOG_FILE
  echo -e "──────────────────────────" &>> $LOG_FILE
  if [[ ! -d $PYTHON_BUILD_DIR ]]; then
    mkdir -p $PYTHON_BUILD_DIR
  fi
  if [[ ! -d $PYTHON_INSTALL_DIR ]]; then
    mkdir -p $PYTHON_INSTALL_DIR
  fi
  cd $PYTHON_BUILD_DIR
  echo -e "\nFetching python3 archive" &>> $LOG_FILE
  echo -e "────────────────────────" &>> $LOG_FILE
  wget http://www.python.org/ftp/python/${PYTHON_VERSION}/${PYTHON_STR}.tgz &>> $LOG_FILE
  tar -zxf ${PYTHON_STR}.tgz &>> $LOG_FILE
  cd ${PYTHON_STR}
  echo -e "\nBuilding python3" &>> $LOG_FILE
  echo -e "────────────────" &>> $LOG_FILE
  ./configure --prefix=${PYTHON_INSTALL_DIR} &>> $LOG_FILE
  make &>> $LOG_FILE
  make install &>> $LOG_FILE
else
  echo "Found python installation: ${PY_CMD}" &>> $LOG_FILE
fi

# Setup Python virtual environment
echo -e "\nSetup python virtual environment" &>> $LOG_FILE
echo -e "────────────────────────────────" &>> $LOG_FILE
TF_PACKAGE="tensorflow"
# TF_PACKAGE="tensorflow-gpu"
# TF_VERSION="2.13.0" # Current stable version
# TF_VERSION="2.12.0" # Current stable gpu version
TF_VERSION="1.15.5" # Version used in repo: 1.15
$PY_CMD -m venv $PYTHON_VENV_DIR &>> $LOG_FILE \
&& source $PYTHON_VENV_DIR/bin/activate &>> $LOG_FILE \
&& pip install notebook $TF_PACKAGE==$TF_VERSION &>> $LOG_FILE
# pip install jupyter_contrib_nbextensions ipykernel numpy matplotlib
SETUP_PY_VENV_RET=$?
deactivate &>> $LOG_FILE
echo -ne $REMOVE_THREE_DOTS
if [[ $SETUP_PY_VENV_RET -ne 0 ]]; then
  echo -e "\nSetup python successful" &>> $LOG_FILE
  echo -e  "${FAILED}  $(getElapsedTimeStr $SECONDS)"
  exit 1
fi
echo -e "\nSetup python failed" &>> $LOG_FILE
echo -e "${CHECK_MARK}  $(getElapsedTimeStr $SECONDS)"
exit 0