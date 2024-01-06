#!/bin/bash
openssl req -nodes -new -x509 -days 3650 -extensions v3_ca -keyout tls-key-hoegarden.pem -out tls-cert-hoegarden.pem -subj /CN=hoegarden.techfak.uni-bielefeld.de

./tools/arduino-fwuploader/arduino-fwuploader certificates flash --file ./cert/tls-cert-hoegarden.pem -b arduino:mbed_nano:nanorp2040connect -a /dev/ttyACM0
