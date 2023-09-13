// Arduino mbed core
// #include <Arduino.h>
#include <PDM.h>
// Arduino Pico SDK core
#include <pico/stdlib.h>
#include <pico/multicore.h>
// Project
#include "wifi_client.h"
#include "netlog.h"
// Hotword recognition
#include "main_functions.h"
#include "audio_provider.h"

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif
#ifndef NETLOG_PROTOCOL
#define NETLOG_PROTOCOL UDP
#endif
#ifndef NETLOG_URL
#define NETLOG_URL ""
#endif
#ifndef NETLOG_PORT
#define NETLOG_PORT 4444
#endif
#ifndef NETLOG_SSL
#define NETLOG_SSL 0
#endif
#ifndef SERVER_PORT
#define SERVER_PORT 55555
#endif
#ifndef SERVER_URL
#define SERVER_URL ""
#endif
#ifndef SERVER_SSL
#define SERVER_SSL 1
#endif
#ifndef SAMPLE_RATE
#define SAMPLE_RATE 16000
#endif
#ifndef SAMPLE_BUFFER_SIZE
#define SAMPLE_BUFFER_SIZE 64
#endif
#define RETRIES 10
#define VERBOSE 1

char* netlog_url = NETLOG_URL;
const int netlog_port = NETLOG_PORT;
const bool netlog_ssl = (NETLOG_SSL == 1) ? true : false;
const char* netlog_protocol = NETLOG_PROTOCOL;
char* server_url = SERVER_URL;
const int server_port = SERVER_PORT;
const int retries = RETRIES;

#define htons(x) (((x) << 8 & 0xFF00) | ((x) >> 8 & 0x00FF))
#define ntohs(x) htons(x)
#define htonl(x) \
	(((x) << 24 & 0xFF000000UL) | ((x) << 8 & 0x00FF0000UL) | ((x) >> 8 & 0x0000FF00UL) | ((x) >> 24 & 0x000000FFUL))
#define ntohl(x) htonl(x)

NetLog* netlog = nullptr;
WifiClient* wifi_client = nullptr;

std::string concatLog() { return std::string(); };
template <typename T, typename... Args>
std::string concatLog(T first, Args... args) {
	std::stringstream ss;
	ss << first << concatLog(args...);
	return ss.str();
}

template <typename... Args>
void nlog(Args... args) {
	netlog->nlog(args...);
}

// Flags
const bool verbose = false;
const bool enable_wifi = true;
const bool enable_network_stream = true;
const bool enable_ssl = (SERVER_SSL == 1) ? true : false;
const bool enable_testdata = false;
const bool enable_printdata = false;
const bool enable_hotword_recognition = true;
const bool enable_serial_connection = true;
const bool enable_serial_connection_wait = false;
const bool enable_remote_logging = true;
const bool enable_serial_logging = true;

// Server connection status
bool server_connected = false;

// Hotword recognition or audio streaming mode
bool hotword_mode = true;

// Audio sample buffer
// const int sample_buffer_size = 512;
const int sample_buffer_size = SAMPLE_BUFFER_SIZE;
// Number of output channels
static const int channels = 1;
// Number of available audio samples from PDM microphone
volatile int samples_available = 0;
// 16 bit audio sample buffer
int16_t sample_buffer[sample_buffer_size];

// Number of transmitted samples
unsigned int n_samples_sent = 0;
// Maximum amplitude
int16_t sample_amp_max = 0;
// Time interval for detecting silence
const int silence_time_ms = 1000;
// Threshold for detecting silence
const int silence_thresh = 50;

// PDM microphone sample rate
static const int sample_rate = SAMPLE_RATE;

// Micropohone gain
const int mic_gain = 50;

// Serial baud rate
// const int serial_baudrate = 9600;
// const int serial_baudrate = 19200;
const int serial_baudrate = 115200;

// Timings
volatile int rec_timestamp = 0;
volatile int rec_time = 0;
volatile int rec_interval = 0;
volatile int rec_samples = 0;
int write_timestamp = 0;
int serial_write_time = 0;
int write_interval = 0;
int loop_timestamp = 0;
int loop_interval = 0;

// Audio stream end bytes
uint8_t audio_end_bytes[4] = {0x0, 0xF, 0x0, 0xF};

// Minimum hotword recognition score
const uint8_t min_word_score = 155;

// Functions
void initPdmMic();
void capturePdmStreamSamples();
void writeSerialAudio();
void printRecTimings();
void printWriteTimings();
void sendAudio();
void pdmCallback();
void endAudioStream();

// Hotword recognition log function
void logFunction(const char* s) { nlog(s); }

void loop_core1() {
	// Heartbeat blink on second core
	while (true) {
		if (hotword_mode) {
			digitalWrite(LED_BUILTIN, HIGH);
			sleep_ms(100);
			digitalWrite(LED_BUILTIN, LOW);
			sleep_ms(900);
		} else {
			digitalWrite(LED_BUILTIN, HIGH);
			sleep_ms(1000);
		}
	}
}

// Setup function
void setup() {
	// Setup builtin led
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	if (enable_serial_connection) {
		// Init serial interface
		Serial.begin(serial_baudrate);
		// Wait for serial connection
		if (enable_serial_connection_wait) {
			while (!Serial)
				;
		}
	}

	// Turn off builtin led
	digitalWrite(LED_BUILTIN, LOW);

	// Network logging client
	static NetLog static_netlog;
	netlog = &static_netlog;
	netlog->setRemoteLogging(enable_remote_logging);
	netlog->setSerialLogging(enable_serial_logging);
	bool netlog_tcp = false;
	if (netlog_protocol == "TCP") {
		netlog_tcp = true;
	}

	// Wifi Client
	int wifi_timeout = 50000;
	char* wifi_ssid = WIFI_SSID;  // WPA2 network SSID
	char* wifi_pass = WIFI_PASS;  // WPA2 password
	static WifiClient static_wifi_client(netlog);
	wifi_client = &static_wifi_client;

	if (enable_wifi) {
		while (!wifi_client->connectToWiFi(wifi_ssid, wifi_pass, wifi_timeout, verbose)) {
			delay(2000);
		}
	}

	// Connect network logger
	if (netlog_ssl) {
		netlog->setEnableTcp(netlog_tcp);
		netlog->connectSSL(netlog_url, netlog_port);
	} else {
		netlog->openConnection(netlog_url, netlog_port, netlog_tcp, netlog_ssl, verbose, retries);
	}

	// PDM mic reinitialization test
	// PDM library can not be reinitialized once ended!
	// nlog("PDM reinitialization test start");
	// initPdmMic();
	// PDM.end();
	// PDM.begin(channels, sample_rate);
	// PDM.end();
	// nlog("PDM reinitialization test end");

	// Initialize audio processing
	initPdmMic();
	if (enable_hotword_recognition) {
		// Initialize hotword recognition
		setupHotwordRecognition(logFunction);
		nlog("Hotword recognition start");
	}

	// Lauch second core
	multicore_launch_core1(loop_core1);
}

void loop() {
	if (enable_network_stream) {
		// if (!wifi_client->connected()) {
		if (!server_connected) {
			if (enable_ssl) {
				wifi_client->disconnectSSL();
				wifi_client->connectSSL(server_url, server_port);
			} else {
				wifi_client->connectTCP(server_url, server_port, RETRIES, verbose);
			}
			server_connected = true;
		}
	}

	if (enable_hotword_recognition) {
		uint8_t score;
		const char* hotword = recognizeHotwords(score);
		if (hotword != "" && hotword != "silence") {
			nlog("Hotword: ", hotword);
			nlog("  Score: ", static_cast<unsigned>(score));
			if (hotword == "marvin" && score >= min_word_score) {
				nlog("Keyword \"marvin\"");
				nlog("Switching to audio streaming mode");
				hotword_mode = false;
			}
		}
	}

	if (enable_network_stream) {
		if (samples_available > 0) {
			if (enable_printdata) {
				writeSerialAudio();
			} else {
				sendAudio();
			}
			unsigned audio_stream_time_ms = n_samples_sent / (sample_rate / 1000.);
			if (audio_stream_time_ms % silence_time_ms == 0) {
				if ((sample_amp_max < silence_thresh)) {
					nlog("Silent for ", silence_time_ms / 1000, " s (max amplitude of ", sample_amp_max, " under threshold of ",
					     silence_thresh);
					// endAudioStream();
					nlog("Switching to hotword recognition mode");
					hotword_mode = true;
				}
				sample_amp_max = 0;
			}
		}
	}
}

void initPdmMic() {
	// Configure the data receive callback
	PDM.onReceive(pdmCallback);

	// Set buffer size in bytes (before PDM.begin, default: 512 -> 256 16bit samples)
	PDM.setBufferSize(sample_buffer_size * 2);

	// Optionally set the gain (should be set after PDM.begin?)
	// Defaults to 20 on the BLE Sense and -10 on the Portenta Vision Shields
	// gain: gain value to use, 0 - 255, defaults to 20 if not specified.
	PDM.setGain(mic_gain);

	// Initialize PDM with:
	// - one channel (mono mode)
	// - a 16 kHz sample rate for the Arduino Nano 33 BLE Sense
	// - a 32 kHz or 64 kHz sample rate for the Arduino Portenta Vision Shields
	if (!PDM.begin(channels, sample_rate)) {
		while (1) {
			nlog("Error initializing PDM microphone!");
			delay(1000);
		}
	}
}

/**
   Callback function to process the data from the PDM microphone.
   NOTE: This callback is executed as part of an ISR.
   Therefore using `Serial` to print messages inside this function isn't supported.
 **/
void capturePdmStreamSamples() {
	// int start_time = micros();
	// Query the number of available bytes
	int bytes_available = PDM.available();

	// Read into the sample buffer
	// int bytes_read = PDM.read(sample_buffer, sample_buffer_size);
	int bytes_read = PDM.read(sample_buffer, bytes_available);

	// 16-bit, 2 bytes per sample
	// 64 samples available -> 4 ms at 16 kHz
	samples_available = bytes_read / 2;

	// Test data
	if (enable_testdata) {
		int test_data_factor = 65536 / samples_available;
		for (int i = 0; i < samples_available; i++) {
			sample_buffer[i] = -32768 + (i * test_data_factor);
		}
	}

	// Maximum amplitude of recorded samples
	for (int i = 0; i < samples_available; i++) {
		if (sample_buffer[i] > sample_amp_max) {
			sample_amp_max = sample_buffer[i];
		}
	}
}

// PDM microphone callback
void pdmCallback() {
	if (hotword_mode) {
		captureHotwordSamples();
	} else {
		capturePdmStreamSamples();
	}
}

// Write mic sample buffer
void writeSerialAudio() {
	int start_time = micros();
	if (samples_available > 0) {
		for (int i = 0; i < samples_available; i++) {
			Serial.println(sample_buffer[i]);
		}
		samples_available = 0;

		int end_time = micros();
		serial_write_time = end_time - start_time;
		write_interval = end_time - write_timestamp;
		write_timestamp = end_time;
	}
}

// Send audio data over wifi
void sendAudio() {
	if (samples_available > 0) {
		wifi_client->send(reinterpret_cast<uint8_t*>(sample_buffer), samples_available * sizeof(int16_t));
		n_samples_sent += samples_available;
		samples_available = 0;
	}
}

void endAudioStream() {
	nlog("End audio stream");
	wifi_client->send(audio_end_bytes, 4);
	// samples_available = 0;
}

// Print recording timings
void printRecTimings() {
	// Clock: 133 MHz
	// plain loop interval: ~0.13 ms with 0.5 ms peaks
	// rec interval: 4 ms, time since rec: 30 microsec
	// 16000 Hz: samples_available: 64, write time: 10 ms, write interval: ~12 ms
	// 8000 Hz: samples_available: 32, write time: 5 ms, write interval: ~5-8 ms
	// writing in thread
	// 8000 Hz: write time: 90-160 ms, write interval: 100-170 ms
	// println: 6 bytes / sample -> 6 * 8000 = 48000 byte/s = 384000 bit/s
	nlog("rec time: ", rec_time);
	nlog("rec interval: ", rec_interval);
	// nlog("time since rec: ", micros() - rec_timestamp);
	nlog("rec samples: ", rec_samples, "\n");
	// rec_samples = 0;
	// samples_available = 0;
}

void printWriteTimings() {
	nlog("write time: ", serial_write_time);
	nlog("write interval: ", write_interval, "\n");
}

/*
// Pico SDK PDM microphone library
// Microphone Library for Pico
// #include <pico/pdm_microphone.h>

// void initPdmMicrophone();
// void capturePdmStreamSamples();

// Arduino Nano RP2040 Connect PDM microphone configuration
// const struct pdm_microphone_config config = {
//     .gpio_data = 22,
//     .gpio_clk = 23,
//     .pio = pio0,
//     .pio_sm = 0,
//     .sample_rate = SAMPLE_RATE,
//     .sample_buffer_size = SAMPLE_BUFFER_SIZE,
// };

void initPdmMicrophone() {
  uint8_t mic_filter_gain = 16;        // default: 16
  uint8_t mic_filter_max_volume = 64;  // default: 64
  uint16_t mic_filter_volume = 64;     // default: 64

  // initialize and start the PDM microphone
  pdm_microphone_init(&config);
  pdm_microphone_set_filter_gain(mic_filter_gain);
  pdm_microphone_set_filter_max_volume(mic_filter_max_volume);
  pdm_microphone_set_filter_volume(mic_filter_volume);
  pdm_microphone_set_samples_ready_handler(capturePdmStreamSamples);
  pdm_microphone_start();
}

void capturePdmStreamSamples() {
  // Read into the sample buffer
  pdm_microphone_read(sample_buffer, SAMPLE_BUFFER_SIZE);

  // 16-bit, 2 bytes per sample
  // 64 samples available -> 4 ms at 16 kHz
  samples_available = SAMPLE_BUFFER_SIZE;

  // Inject test data
  if (enable_testdata) {
    int test_data_factor = 65536 / samples_available;
    for (int i = 0; i < samples_available; i++) {
      sample_buffer[i] = -32768 + (i * test_data_factor);
    }
  }
}
*/
