#include "ip_mic_monitor.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <limits>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <thread>
#include <csignal>
// #include <atomic>

using namespace std::chrono_literals;

#define BUFFER_SIZE 255
#ifndef SERVER_PORT
#define SERVER_PORT 55555
#endif
#ifndef SERVER_SSL
#define SERVER_SSL 1
#endif

bool is_exit = false;
void signal_handler(std::sig_atomic_t signal) {
	if (signal == SIGINT || signal == SIGTERM) {
		is_exit = true;
	}
	std::this_thread::sleep_for(1000ms);
	std::cout << "Exit" << std::endl;
	exit(0);
}

void run();
void run_check_pending();
void run_once();
void receive_audio();
void save_audio();

IpMicMonitor ip_mic_monitor;
bool receive_mode = true;
bool receive_mode_last = false;
bool enable_ssl = (SERVER_SSL == 1) ? true : false;
const int rec_frequency = 16000;
const int rec_time_s = 30;
const int rec_buffer_size = rec_frequency * rec_time_s;
int16_t sample_buffer[BUFFER_SIZE] = {0};
int16_t rec_buffer[rec_buffer_size] = {0};
int rec_idx = 0;
auto timestamp_audio_received = std::chrono::system_clock::now();

int main(int argc, char const* argv[]) {
	// Set signal handler
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	run();
}

void mode_watchdog() {
	while (!is_exit) {
		auto max_receive_interval_ms = 200;
		auto now = std::chrono::system_clock::now();
		auto receive_interval_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp_audio_received).count();
		// std::cout << "Receive interval: " << receive_interval_ms << " ms" << std::endl;
		if(receive_mode && receive_interval_ms > max_receive_interval_ms) {
			receive_mode = false;
			std::cout << "Receive mode" << std::endl;
			std::this_thread::sleep_for(10ms);
		} else {
			std::this_thread::sleep_for(100ms);
		}
	}
}

void receive_audio() {
	while (!is_exit) {
		// std::cout << "Receive audio" << std::endl;
		while (rec_idx < rec_buffer_size) {
			if (is_exit) break;
			int samples_read;
			if (enable_ssl) {
				samples_read = ip_mic_monitor.receiveSSL(sample_buffer);
			} else {
				samples_read = ip_mic_monitor.receive(sample_buffer);
			}
			timestamp_audio_received = std::chrono::system_clock::now();
			receive_mode = true;

			if (rec_idx + samples_read < rec_buffer_size) {
				// Fill recording buffer
				std::memcpy(rec_buffer + rec_idx, &sample_buffer, samples_read * sizeof(int16_t));
				rec_idx += samples_read;
				if (rec_idx % rec_frequency == 0) {
					std::cout << "recorded samples: " << rec_idx << std::endl;
				}
			} else {
				// Fill remainder of recording buffer
				std::memcpy(rec_buffer + rec_idx, &sample_buffer, (rec_buffer_size - rec_idx) * sizeof(int16_t));
				rec_idx += rec_buffer_size - rec_idx;
				std::cout << "recorded samples: " << rec_idx << std::endl;
				break;
			}

			// receive_mode_last = receive_mode;
		}
		std::cout << "Audio sample buffer overflow: Maximum record time: " << rec_time_s << " seconds" << std::endl;
	}
}

void save_audio() {
	/*
	// Dump audio to ascii file
	FILE* dump_file_ascii = fopen("./audio_stream_4s_acsii.pcm", "w");
	for (int i = 0; i < rec_buffer_size; i++) {
		// fprintf(dump_file, "%f\n", (float)sample_buffer[i] / std::numeric_limits<int16_t>::max());
		fprintf(dump_file_ascii, "%d\n", rec_buffer[i]);
	}
	fclose(dump_file_ascii);
	// Dump audio to binary file
	FILE* dump_file_binary = fopen("./audio_stream_4s_binary.pcm", "wb");
	fwrite(reinterpret_cast<char*>(rec_buffer), rec_buffer_size * sizeof(int16_t), 1, dump_file_binary);
	fclose(dump_file_binary);
	*/

	while (!is_exit) {
		if (!receive_mode && rec_idx > 0) {
			// Save audio to wav file
			auto now = std::chrono::system_clock::now();
			time_t local_time = std::chrono::system_clock::to_time_t(now);
			std::stringstream file_timestamp_ss;
			file_timestamp_ss << std::put_time(std::localtime(&local_time), "%Y%m%d-%H%M%S");
			std::string file_timestamp = file_timestamp_ss.str();
			std::string filename_wav = "audio_stream_4s" + file_timestamp + ".wav";
			std::cout << "Write audio to file: " << filename_wav << std::endl;
			ip_mic_monitor.writeWav(filename_wav.c_str(), rec_buffer, rec_idx);
			receive_mode = true;
			rec_idx = 0;
		} else {
			std::this_thread::sleep_for(1000ms);
		}
	}
}

void run() {
	std::cout << "Secure IP Microphone Monitor" << std::endl;
	int port = SERVER_PORT;

	if (enable_ssl) {
		ip_mic_monitor.openSSLConnection(port);
	} else {
		ip_mic_monitor.openConnection(port);
	}

	std::thread mode_watchdog_thread(mode_watchdog);
	std::thread receive_audio_thread(receive_audio);
	std::thread save_audio_thread(save_audio);

	save_audio_thread.join();
	receive_audio_thread.join();
	mode_watchdog_thread.join();

	ip_mic_monitor.closeSSL();


	exit(EXIT_SUCCESS);
}

void run_check_pending() {
	std::cout << "Secure IP Microphone Monitor" << std::endl;
	int port = SERVER_PORT;
	bool enable_ssl = (SERVER_SSL == 1) ? true : false;

	const int rec_frequency = 16000;
	const int rec_time_s = 30;
	const int rec_buffer_size = rec_frequency * rec_time_s;
	int16_t sample_buffer[BUFFER_SIZE] = {0};
	int16_t rec_buffer[rec_buffer_size] = {0};
	int rec_idx = 0;

	IpMicMonitor ip_mic_monitor;

	if (enable_ssl) {
		ip_mic_monitor.openSSLConnection(port);
	} else {
		ip_mic_monitor.openConnection(port);
	}

	// long long last_rec_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	bool receive_mode = true;
	bool receive_mode_last = false;

	while (true) {
		// Check if audio data is available
		int bytes_pending;
		if (enable_ssl) {
			bytes_pending = ip_mic_monitor.pendingSSL(sample_buffer);
		} else {
			bytes_pending = ip_mic_monitor.pending(sample_buffer);
		}
		// std::cout << "Bytes pending: " << bytes_pending << std::endl;
		// receive_mode = (bytes_pending > 0) ? true : false;
		if (bytes_pending > 0) {
			receive_mode = true;
			// std::cout << "Bytes pending: " << bytes_pending << std::endl;
		} else {
			receive_mode = false;
		}

		if (receive_mode != receive_mode_last) {
			if (receive_mode) {
				std::cout << "Receive mode" << std::endl;
			} else {
				std::cout << "Record mode" << std::endl;
			}
		}

		if (receive_mode) {
			while (rec_idx < rec_buffer_size) {
				// Receive audio data
				// if (receive_mode != receive_mode_last) {
				// 	std::cout << "Receive mode" << std::endl;
				// }
				int samples_read = 0;
				if (enable_ssl) {
					samples_read = ip_mic_monitor.receiveSSL(sample_buffer);
				} else {
					samples_read = ip_mic_monitor.receive(sample_buffer);
				}
				// last_rec_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

				if (samples_read == 0) {
					std::cout << "No samples read" << std::endl;
					break;
				}

				// Check for audio stream end bytes
				// if (rec_idx + samples_read < rec_buffer_size && samples_read > 3) {
				// 	// note: always reads 64 samples
				// 	for (int i = 0; i < samples_read - 4; i++) {
				// 		if (sample_buffer[i] == 0x0) {
				// 			if (sample_buffer[i + 0] == 0x0 && 
				// 				sample_buffer[i + 1] == 0xF && 
				// 				sample_buffer[i + 2] == 0x0 && 
				// 				sample_buffer[i + 3] == 0xF
				// 			) {
				// 				std::cout << "Received end of audio stream" << std::endl;
				// 			}
				// 		}
				// 	}
				// }

				if (rec_idx + samples_read < rec_buffer_size) {
					// Fill recording buffer
					std::memcpy(rec_buffer + rec_idx, &sample_buffer, samples_read * sizeof(int16_t));
					rec_idx += samples_read;
					if (rec_idx % rec_frequency == 0) {
						std::cout << "recorded samples: " << rec_idx << std::endl;
					}
				} else {
					// Fill remainder of recording buffer
					std::memcpy(rec_buffer + rec_idx, &sample_buffer, (rec_buffer_size - rec_idx) * sizeof(int16_t));
					rec_idx += rec_buffer_size - rec_idx;
					std::cout << "recorded samples: " << rec_idx << std::endl;
					break;
				}
			}
			std::cout << "Audio sample buffer overflow: Maximum record time: " << rec_time_s << " seconds" << std::endl;
		} else {
			// Record audio data
			// if (receive_mode != receive_mode_last) {
			// 	std::cout << "Record mode" << std::endl;
			// }

			// Check for received audio data
			if (rec_idx == 0) {
				continue;
			}

			// Get current time for file timestamp
			// auto const current_time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
			// std::string file_timestamp = std::format("{%Y%m%d-%H%M%S}", current_time);
			auto now = std::chrono::system_clock::now();
			auto local_time = std::chrono::system_clock::to_time_t(now);
			std::stringstream file_timestamp_ss;
			file_timestamp_ss << std::put_time(std::localtime(&local_time), "%Y%m%d-%H%M%S");
			std::string file_timestamp = file_timestamp_ss.str();
			std::string filename_wav = "audio_stream_4s" + file_timestamp + ".wav";

			// Save audio to wav file
			// ip_mic_monitor.writeWav(filename_wav.c_str(), rec_buffer, rec_buffer_size);
			std::cout << "Write to file: " << filename_wav << std::endl;

			// // Dump audio to ascii file
			// FILE* dump_file_ascii = fopen("./audio_stream_4s_acsii.pcm", "w");
			// for (int i = 0; i < rec_buffer_size; i++) {
			// 	// fprintf(dump_file, "%f\n", (float)sample_buffer[i] / std::numeric_limits<int16_t>::max());
			// 	fprintf(dump_file_ascii, "%d\n", rec_buffer[i]);
			// }
			// fclose(dump_file_ascii);
			// // Dump audio to binary file
			// FILE* dump_file_binary = fopen("./audio_stream_4s_binary.pcm", "wb");
			// fwrite(reinterpret_cast<char*>(rec_buffer), rec_buffer_size * sizeof(int16_t), 1, dump_file_binary);
			// fclose(dump_file_binary);

			rec_idx = 0;
		}
		receive_mode_last = receive_mode;

		std::this_thread::sleep_for(500ms);
		std::cout << "Loop end" << std::endl;
	}
	std::cout << "Finished" << std::endl;
}

void run_once() {
	std::cout << "Secure IP Microphone Monitor" << std::endl;
	int port = SERVER_PORT;
	bool enable_ssl = (SERVER_SSL == 1) ? true : false;

	IpMicMonitor ip_mic_monitor;

	if (enable_ssl) {
		ip_mic_monitor.openSSLConnection(port);
	} else {
		ip_mic_monitor.openConnection(port);
	}

	const int rec_frequency = 16000;
	const int rec_time_s = 4;
	const int rec_buffer_size = rec_frequency * rec_time_s;
	int16_t sample_buffer[BUFFER_SIZE] = {0};
	int16_t rec_buffer[rec_buffer_size] = {0};
	int rec_idx = 0;
	std::cout << "recording samples for " << rec_time_s << " seconds" << std::endl;
	while (rec_idx < rec_buffer_size) {
		int samples_read;
		if (enable_ssl) {
			samples_read = ip_mic_monitor.receiveSSL(sample_buffer);
		} else {
			samples_read = ip_mic_monitor.receive(sample_buffer);
		}
		if (rec_idx + samples_read < rec_buffer_size) {
			// Fill recording buffer
			std::memcpy(rec_buffer + rec_idx, &sample_buffer, samples_read * sizeof(int16_t));
			rec_idx += samples_read;
			if (rec_idx % rec_frequency == 0) {
				std::cout << "recorded samples: " << rec_idx << std::endl;
			}
		} else {
			// Fill remainder of recording buffer
			std::memcpy(rec_buffer + rec_idx, &sample_buffer, (rec_buffer_size - rec_idx) * sizeof(int16_t));
			rec_idx += rec_buffer_size - rec_idx;
			std::cout << "recorded samples: " << rec_idx << std::endl;
			break;
		}
	}

	ip_mic_monitor.closeSSL();

	// Dump audio to ascii file
	FILE* dump_file_ascii = fopen("./audio_stream_4s_acsii.pcm", "w");
	for (int i = 0; i < rec_buffer_size; i++) {
		// fprintf(dump_file, "%f\n", (float)sample_buffer[i] / std::numeric_limits<int16_t>::max());
		fprintf(dump_file_ascii, "%d\n", rec_buffer[i]);
	}
	fclose(dump_file_ascii);
	// Dump audio to binary file
	FILE* dump_file_binary = fopen("./audio_stream_4s_binary.pcm", "wb");
	fwrite(reinterpret_cast<char*>(rec_buffer), rec_buffer_size * sizeof(int16_t), 1, dump_file_binary);
	fclose(dump_file_binary);

	// Save audio to wav file
	ip_mic_monitor.writeWav("./audio_stream_4s.wav", rec_buffer, rec_buffer_size);

	exit(EXIT_SUCCESS);
}
