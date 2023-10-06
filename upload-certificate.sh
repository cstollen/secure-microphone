#! /bin/bash

SCRIPT=$(realpath "$0")
SCRIPT_DIR=$(dirname "$SCRIPT")
ROOT_DIR=$SCRIPT_DIR

# Parameters
MIC_DIR=$ROOT_DIR/secure-mic
CERT_DIR=$MIC_DIR/cert
CERT_FILENAME=ssl-cert.pem
CERT_FILE=$(realpath $CERT_DIR/$CERT_FILENAME)
ARDUINO_UPLOADER_BIN=$(realpath secure-mic/tools/arduino-fwuploader/arduino-fwuploader)
UART_DEVICE=/dev/ttyACM0

# Check for certificate file
if [[ ! -f "$CERT_FILE" ]]; then
  echo -e "Certificate files not found: $CERT_FILE"
  exit 1
fi

# Check for arduino uploader executable
if [[ ! -f "$ARDUINO_UPLOADER_BIN" ]]; then
  echo -e "Arduino uploader not found: $ARDUINO_UPLOADER_BIN"
  exit 1
fi

# Upload certificate using arduino uploader
echo "Uploading certificate to microcontroller"
$ARDUINO_UPLOADER_BIN certificates flash --file $CERT_FILE -b arduino:mbed_nano:nanorp2040connect -a $UART_DEVICE

# Check return code
UPLOAD_RET=$?
if [[ $UPLOAD_RET -eq 1 ]]; then
  echo "Certificate upload failed"
else
  echo "Certificate upload finished"
fi
