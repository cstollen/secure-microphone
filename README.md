# Secure IP Microphone

This project enables an Arduino Nano RP2040 Connect to recognize hotwords and send audio data over WiFi to a server using SSL/TLS encryption. 
It is based on the [Arduino microphone example](https://docs.arduino.cc/tutorials/nano-rp2040-connect/rp2040-microphone-basics) and the [Tensorflow micro speech example](https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/micro_speech).

## Dependencies

- RP2040 secure microphone  
  - [PlatformIO](https://platformio.org) 
    - PlatformIO packages:  
      - framework-arduino-mbed 3.0.1   
      - tool-rp2040tools 1.0.2   
      - toolchain-gccarmnoneeabi 1.90301.200702 (9.3.1)  
    - PlatformIO libraries:  
      - [PDM](https://docs.arduino.cc/learn/built-in-libraries/pdm) 1.0
      - [SPI](https://www.arduino.cc/reference/en/language/functions/communication/spi) 1.0
      - [WiFiNINA](https://github.com/arduino-libraries/WiFiNINA) 1.8.13
      - [Tensorflow Lite Micro](https://github.com/tensorflow/tflite-micro) (bundled in lib folder)  
- Secure microphone audio stream server  
  - [OpenSSL](https://www.openssl.org) 1.1.1f  

## Install dependencies

`$ ./install-dependencies.sh`

### Manual Installation of Platformio

Installing PlatformIO in a python 3 environment:  
`$ pip3 install platformio`  
Update udev rules:  
`$ sudo ~/.platformio/packages/framework-arduino-mbed/post_install.sh`  

## Configure project

Set configuration variables in `config.ini` and run config script afterwards:  
`$ ./configure.sh`  
The SSL certificate and key are generated based on the server URL given in `config.ini` and uploaded to the microcontroller. 

## Build Project

`$ ./build-all.sh`

## Start

Start server: `$ ./start-server.sh`  
Start client by connecting to serial interface: `$ ./start-client.sh`

## Serial Monitor

A simple serial monitor using `cat` can be run to get the output of the hotword recognition:  
`$ cat /dev/ttyACM0`  
The device has initialized, when the onboard LED lights up and it is waiting for a serial connection to resume.  
The output consists of the recognized word, a score and the time since the start of the device.  

## Changing Parameters

Important changeable parameters can be found in `secure-ip-mic-rp2040/src/hotword/config.h` including microphone and recognition configuration.

## Loading Test Data

To load testdata instead of using the microphone, uncomment `#define LOADDATA` in `src/audio_provider.cpp`.  
The example data consists of audio samples containing the words "yes" and "no".  
Custom data was recorded containing the words "yes" and "no" in a 4 second audio clip. The custom data can be loaded by also uncommenting `#define CUSTOMDATA`.

## Tensorflow Model

Tensorflow Lite Micro models were trained using the [speech commands dataset](https://www.tensorflow.org/datasets/catalog/speech_commands) by [Pete Warden](https://arxiv.org/abs/1804.03209) available as [download](http://download.tensorflow.org/data/speech_commands_v0.02.tar.gz). It contains 35 words, from which a subset is chosen as recognizable hotwords.  
Included in the project is a trained model for the word set:  

- 8 words: "yes", "no", "up", "down", "left", "right", "on", "off"  

For the training a jupyter notebook script found in `scripts/training` was used.

### Model Performance

8 words (3915 test samples):  

- Float model:  
  - Size: 164068 bytes  
  - Accuracy: 76.653895%  
- Quantized model:  
  - Size: 42736 bytes  
  - Accuracy: 76.704981%

[tflite-micro/tensorflow/lite/micro/examples/micro_speech/train at main · tensorflow/tflite-micro · GitHub](https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/micro_speech/train)

[Simple audio recognition: Recognizing keywords &nbsp;|&nbsp; TensorFlow Core](https://www.tensorflow.org/tutorials/audio/simple_audio)
