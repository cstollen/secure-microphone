#!/bin/bash

# Generate certificate
# openssl req -nodes -new -x509 -days 3650 -extensions v3_ca -keyout rp2040-key.pem -out rp2040-cert.pem -subj '/CN=hoegarden.techfak.uni-bielefeld.de'

FWUPLOADER_URL=https://github.com/arduino/arduino-fwuploader/releases/download/2.2.0/arduino-fwuploader_2.2.0_Linux_64bit.tar.gz
CERT_DIR=./cert
CERT_FILE=rp2040-cert.pem
# CERT_FILE=default-cert.pem
PLATFORM=arduino:mbed_nano:nanorp2040connect
PORT=/dev/ttyACM0

# Check for superuser
# if [[ $(id -u) -ne 0 ]]; then
# 	echo "Please run as root"
# 	exit 1
# fi

# Check for arduino-fwuploader command
# if ! [ -x "$(command -v arduino-fwuploader)" ]; then
# 	echo "Command arduino-fwuploader not found"
# 	exit 1
# fi

# Check for script directory
if [[ ! -f upload-certificates.sh ]]; then
	echo "Please execute script from same directory"
	exit 1
fi

# Check for certificate file
if [[ ! -f $CERT_DIR/$CERT_FILE ]]; then
	echo "Certificate file not found: $CERT_DIR/$CERT_FILE"
	exit 1
fi

# Create arduino-fwuploader directory
if [[ ! -d tools ]]; then
	echo "Fetching arduino firmware uploader"
	# su -c "mkdir -p tools" $USERNAME
	mkdir -p tools/arduino-fwuploader
	curl -L $FWUPLOADER_URL | tar -xzvf - -C ./tools/arduino-fwuploader
fi

if [[ -f ./tools/arduino-fwuploader/arduino-fwuploader ]]; then
	echo "Uploading certificate: $CERT_FILE"
	#./tools/arduino-fwuploader/arduino-fwuploader certificates flash --url arduino.cc:443 --url hoegarden.techfak.uni-bielefeld.de:4444 --file ./rp2040-cert.pem -b arduino:mbed_nano:nanorp2040connect -a /dev/ttyACM0
	#./tools/arduino-fwuploader/arduino-fwuploader certificates flash --url hoegarden.techfak.uni-bielefeld.de:4444 --file $CERT_DIR/$CERT_FILE -b $PLATFORM -a $PORT
	./tools/arduino-fwuploader/arduino-fwuploader certificates flash --file $CERT_DIR/$CERT_FILE -b $PLATFORM -a $PORT
else
	echo "Arduino firmware uploader not found"
	exit 1
fi

exit 0