# Secure IP Microphone
This project enables an Arduino Nano RP2040 Connect to recognize hotwords and send audio data over WiFi to a server using SSL/TLS encryption. 
It is based on the [Arduino microphone example](https://docs.arduino.cc/tutorials/nano-rp2040-connect/rp2040-microphone-basics) and the [Tensorflow micro speech example](https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/micro_speech).

## Quick Start (tl;dr)
1. Install dependencies
  `$ ./install-dependencies.sh`
2. Run configuration script to generate config files  
  `$ ./configure.sh`
3. Set WiFi credentials and host URL in config file `secure-mic/config/config.ini`
4. Rerun configuration script
  `$ ./configure.sh`
5. Build project  
  `$ ./build-all.sh`
6. Start network logger  
  `$ ./start-netlog-server.sh`
7. Start microphone server  
  `$ ./start-mic-server.sh`

## Dependencies
- Build tools
  - [CMake](https://cmake.org/)
  - [Make](https://www.gnu.org/software/make)
- RP2040 secure microphone (secure-mic)
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
- Secure microphone audio stream server (secure-mic-monitor)
  - [OpenSSL](https://www.openssl.org) 1.1.1f  
- Wireless network logger (netlog-monitor)
  - [OpenSSL](https://www.openssl.org) 1.1.1f  

## Install dependencies
- Run install script
  `$ ./install-dependencies.sh`
- Script installs
  - Ubuntu packages with apt
    - `libssl-dev` (openssl command)
    - `python3-pip` (python3 package manager)
  - Python3 packages in current environment
    - `platformio` (pio command)

### Manual Installation of PlatformIO (optional)
- Install PlatformIO in a python 3 environment:  
  `$ pip3 install platformio`  
- Update udev rules:  
  `$ sudo ~/.platformio/packages/framework-arduino-mbed/post_install.sh`  

## Configure project
- First launch
  - Run configure script to generate new config files based on empty templates  
    `$ ./configure.sh`
  - Edit microphone config file `./secure-mic/config/config.ini`
    - Set SSID and password for WiFi (WPA2 personal)
    - Set URL (hostname) and port for the microphone monitor (SERVER) and network logger (NETLOG)
      - Default ports
        - Microphone: 55555 (TCP)
        - Network logger: 4444 (UDP)
      - Important: The microcontroller only connects to the set host, because the URL gets embedded in the SSL certificate
  - Edit microphone monitor config file `./secure-mic-monitor/config/config.ini`
    - Just needed if port differs from the default (55555)
  - Edit network logger config file `./secure-mic-monitor/config/config.ini`
    - Just needed if port differs from the default (4444)
  - Change the variable `UART_DEVICE` in the `configure.sh` script if the UART device path for the microcontroller differs from `/dev/ttyACM0`
  - Rerun the config script to generate SSL certificates based on the set configuration and upload them to the microcontroller  
    `$ ./configure.sh`
- Subsequent launches use the generated configuration files
  - The following configuration files get used, if they exist  
    `secure-mic/config/config.ini`  
    `secure-mic-monitor/config/config.ini`  
    `secure-mic-monitor/config/config.ini`  
  - The following certificate files get used, if they exist  
    `secure-mic/cert/ssl-cert.pem`  
    `secure-mic/cert/ssl-key.pem`  

## Build
- Build project including microcontroller PlatformIO project and host applications (secure microphone server and network logger)  
  `$ ./build-all.sh`

## Start
- Start network logger for debugging messages  
  `$ ./start-netlog-server.sh`
- Start microphone server  
  `$ ./start-mic-server.sh`  

## Usage
- The currently used model includes the 9 words: "yes", "no", "up", "down", "left", "right", "on", "off", "Marvin"
- The hotword *Marvin* is used for starting the audio stream.
- After one minute of silence the microphone switches back to hotword recognition mode.

## Appendix
### Serial Monitor
- A simple serial monitor using `cat` can be run to get the output of the hotword recognition:  
  `$ cat /dev/ttyACM0`  
- The device has initialized, when the onboard LED lights up and it is waiting for a serial connection to resume.  
- The output consists of the recognized word, a score and the time since the start of the device.  

### Changing Microphone Settings
- Important changeable parameters can be found in `secure-ip-mic-rp2040/src/hotword/config.h` including microphone and recognition configuration.

### Loading Test Data
- To load testdata instead of using the microphone, uncomment `#define LOADDATA` in `secure-mic/src/hotword/audio_provider.cpp`.  
- The example data consists of audio samples containing the words "yes" and "no".  
- Custom data was recorded containing the words "yes" and "no" in a 4 second audio clip. The custom data can be loaded by also uncommenting `#define CUSTOMDATA`.

### Tensorflow Model
- Tensorflow Lite Micro models were trained using the [speech commands dataset](https://www.tensorflow.org/datasets/catalog/speech_commands) by [Pete Warden](https://arxiv.org/abs/1804.03209) available as [download](http://download.tensorflow.org/data/speech_commands_v0.02.tar.gz). It contains 35 words, from which a subset is chosen as recognizable hotwords.  
- Included in the project are trained models (`training/models_archive`) for the word sets:  
  - 1 word: "Marvin" (marvin)
  - 8 words: "yes", "no", "up", "down", "left", "right", "on", "off" (demo)
  - 9 words: "yes", "no", "up", "down", "left", "right", "on", "off", "Marvin" (9words)
- For model training the jupyter notebook script `train_micro_speech_model.ipynb` in the `training` folder was used.
- Changing the model
  - Saved models are located in `training/models_archive`
  - Exchange the model files `micro_speech_model_data.cpp` and `micro_speech_model_data.h` in project folder `secure-mic/src/hotword` with the ones from the model archive
  - Modify `WORDCOUNT` define in `micro_model_settings.h` in project folder `secure-mic/src/hotword`
  - Modify `kCategoryLabels` variable in `micro_model_settings.cpp` in project folder `secure-mic/src/hotword`

#### Model Performance
8 words (3915 test samples):  
- Float model:  
  - Size: 164068 bytes  
  - Accuracy: 76.653895%  
- Quantized model:  
  - Size: 42736 bytes  
  - Accuracy: 76.704981%

### Links
[Arduino microphone example](https://docs.arduino.cc/tutorials/nano-rp2040-connect/rp2040-microphone-basics)
[Tensorflow micro speech example](https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/micro_speech)
[TensorFlow Lite Micro speech training example](https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/micro_speech/train)  
[TensorFlow example for simple audio recognition](https://www.tensorflow.org/tutorials/audio/simple_audio)
[Speech commands dataset](https://www.tensorflow.org/datasets/catalog/speech_commands)
