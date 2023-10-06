#! /bin/bash

SCRIPT=$(realpath "$0")
SCRIPT_DIR=$(dirname "$SCRIPT")
PROJECT_DIR=$SCRIPT_DIR

check_install_package () {
	REQUIRED_PKG="$1"
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' ${REQUIRED_PKG} |grep "install ok installed")
	echo "Checking for package $REQUIRED_PKG"
	if [[ "" == "$PKG_OK" ]]; then
		echo "Package $REQUIRED_PKG not found. Installing $REQUIRED_PKG."
		sudo apt-get --yes install $REQUIRED_PKG
	else
		echo "Package $REQUIRED_PKG found"
	fi
}

PIO_CMD_PATH=$(which pio)
OPENSSL_CMD_PATH=$(which openssl)
MIC_RP2040_DIR=$(realpath secure-mic)
FWUPLOADER_DIR=$MIC_RP2040_DIR/tools/arduino-fwuploader
FWUPLOADER_URL=https://github.com/arduino/arduino-fwuploader/releases/download/2.2.0/arduino-fwuploader_2.2.0_Linux_64bit.tar.gz

if [[ $PIO_CMD_PATH == "" ]]; then
	echo "PlatformIO executable not found"
	echo "Installing PlatformIO"
	check_install_package "python3-pip"
	sudo pip3 install platformio
	sudo ~/.platformio/packages/framework-arduino-mbed/post_install.sh
else
	echo "Found PlatformIO executable: $PIO_CMD_PATH"
fi

if [[ $OPENSSL_CMD_PATH == "" ]]; then
	echo "OpenSSL executable not found"
	echo "Installing OpenSSL"
	check_install_package "libssl-dev"
else
	echo "Found OpenSSL executable: $OPENSSL_CMD_PATH"
fi

if [[ ! -f $FWUPLOADER_DIR/arduino-fwuploader ]]; then
	echo "Arduino firmware uploader executable not found"
	echo "Installing arduino firmware uploader into tools folder"
	if [[ ! -d "$FWUPLOADER_DIR" ]]; then
		mkdir -p $FWUPLOADER_DIR
	fi
	if [[ ! -f "$FWUPLOADER_DIR/arduino-fwuploader_2.2.0_Linux_64bit.tar.gz" ]]; then
		# cd $MIC_RP2040_DIR/tools
		# if [[ ! -d "$MIC_RP2040_DIR/arduino-fwuploader" ]]; then
		# 	mkdir -p $MIC_RP2040_DIR/arduino-fwuploader
		# fi
		cd $FWUPLOADER_DIR
		wget $FWUPLOADER_URL
		cd $PROJECT_DIR
	fi
	if [[ ! -f $FWUPLOADER_DIR/arduino-fwuploader ]]; then
		cd $FWUPLOADER_DIR
		tar -xzf arduino-fwuploader_2.2.0_Linux_64bit.tar.gz
		cd $PROJECT_DIR
	fi
else
	echo "Found arduino firmware uploader executable: $FWUPLOADER_DIR/arduino-fwuploader"
fi

