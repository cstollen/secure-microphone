#include "ip_mic_monitor.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <limits>

#define BUFFER_SIZE 255
#ifndef SERVER_PORT
#define SERVER_PORT 55555
#endif
#ifndef SERVER_SSL
#define SERVER_SSL 1
#endif

int main(int argc, char const* argv[]) {
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
