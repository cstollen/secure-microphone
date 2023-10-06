#! /bin/bash

SCRIPT=$(realpath "$0")
SCRIPT_DIR=$(dirname "$SCRIPT")
PROJECT_DIR=$SCRIPT_DIR

MIC_RP2040_DIR=secure-mic
MIC_MONITOR_DIR=secure-mic-monitor
NETLOG_MONITOR_DIR=netlog-monitor
FWUPLOADER_URL=https://github.com/arduino/arduino-fwuploader/releases/download/2.2.0/arduino-fwuploader_2.2.0_Linux_64bit.tar.gz
UART_DEVICE=/dev/ttyACM0

if [[ ! -f "$MIC_RP2040_DIR/config/config.ini" ]]; then
	cp "$MIC_RP2040_DIR/config/config-template.ini" "$MIC_RP2040_DIR/config/config.ini"
  echo "Config file created in $MIC_RP2040_DIR/config"
fi

if [[ ! -f "$MIC_MONITOR_DIR/config/config.ini" ]]; then
	cp "$MIC_MONITOR_DIR/config/config-template.ini" "$MIC_MONITOR_DIR/config/config.ini"
  echo "Config file created in $MIC_MONITOR_DIR/config"
fi

if [[ ! -f "$NETLOG_MONITOR_DIR/config/config.ini" ]]; then
	cp "$NETLOG_MONITOR_DIR/config/config-template.ini" "$NETLOG_MONITOR_DIR/config/config.ini"
  echo "Config file created in $NETLOG_MONITOR_DIR/config"
fi

source "$MIC_RP2040_DIR/config/config.ini"

if [[ "$WIFI_SSID" == "" ]] || [[ "$WIFI_PASS" == "" ]] || [[ "$SERVER_URL" == "" ]]; then
	echo "Please set configuration variables in $MIC_RP2040_DIR/config/config.ini"
	exit 1
fi

if [[ ! -f "$MIC_RP2040_DIR/tools/arduino-fwuploader/arduino-fwuploader" ]]; then
	# Download arduino firmware wuploader
	if [[ ! -d $MIC_RP2040_DIR/tools/arduino-fwuploader ]]; then
		echo "Fetching arduino firmware uploader"
		mkdir -p $MIC_RP2040_DIR/tools/arduino-fwuploader
		curl -L $FWUPLOADER_URL | tar -xzvf - -C $MIC_RP2040_DIR/tools/arduino-fwuploader
	fi
fi

if [[ ! -d "$MIC_RP2040_DIR/cert" ]]; then
	mkdir -p "$MIC_RP2040_DIR/cert"
fi
cd $MIC_RP2040_DIR/cert
if [[ ! -f "ssl-cert.pem" ]] || [[ ! -f "ssl-key.pem" ]]; then
	echo "Generating SSL certificate and key"
	openssl req -nodes -new -x509 -days 3650 -extensions v3_ca -keyout ssl-key.pem -out ssl-cert.pem -subj /CN=$SERVER_URL
	cd ..
	echo ""
	echo "Uploading certificate to microcontroller"
	tools/arduino-fwuploader/arduino-fwuploader certificates flash --file cert/ssl-cert.pem -b arduino:mbed_nano:nanorp2040connect -a $UART_DEVICE
fi

cd $PROJECT_DIR
if [[ ! -f $MIC_MONITOR_DIR/cert/ssl-cert.pem ]]; then
	cp $MIC_RP2040_DIR/cert/ssl-cert.pem $MIC_MONITOR_DIR/cert
fi
if [[ ! -f $MIC_MONITOR_DIR/cert/ssl-key.pem ]]; then
	cp $MIC_RP2040_DIR/cert/ssl-key.pem $MIC_MONITOR_DIR/cert
fi

echo "Configuration"
echo "WiFi SSID: $WIFI_SSID"
echo "Server URL: $SERVER_URL"
echo "Server port: $SERVER_PORT"

