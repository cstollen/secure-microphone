// #include <stdio.h>
// #include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <iostream>
#include <fstream>
#include "ip_mic_monitor.hpp"

bool IpMicMonitor::openConnection(int port) {
	m_socket = openSocket(port);
	return (m_socket > 0) ? true : false;
}

int IpMicMonitor::openSocket(int port) {
	sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	int socket_fd = -1;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 1) {
		printf("Could not create socket: (%s)\n", strerror(errno));
		return socket_fd;
	}

	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		printf("Could not set socket options (%s)", strerror(errno));
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		printf("Could not bind socket on port %d (%s)\n", port, strerror(errno));
		return socket_fd;
	}
	if (listen(socket_fd, 3) < 0) {
		printf("Could not listen to socket connection on port %d (%s)\n", port, strerror(errno));
		return socket_fd;
	}
	if ((socket_fd = accept(socket_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
		printf("Could not accept TCP connection on port %d (%s)\n", port, strerror(errno));
	}
	// std::cout << "Accepted TCP connection on port " << port << std::endl;
	m_connected = true;
	return socket_fd;
}

void IpMicMonitor::createSSLContext(SSL_CTX*& ctx, const SSL_METHOD* method) {
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
}

void IpMicMonitor::configureSSLContext(SSL_CTX* ctx) {
	// SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
	// int uc = SSL_CTX_use_certificate_chain_file(ctx, SSL_CERT_FILE);
	// int up = SSL_CTX_use_PrivateKey_file(ctx, SSL_KEY_FILE, SSL_FILETYPE_PEM);

	// if (SSL_CTX_use_certificate_file(ctx, SSL_CERT_FILE, SSL_FILETYPE_PEM) <= 0) {
	if (SSL_CTX_use_certificate_chain_file(ctx, SSL_CERT_FILE) < 1) {
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, SSL_KEY_FILE, SSL_FILETYPE_PEM) < 1) {
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
}

bool IpMicMonitor::openSSLConnection(int port) {
	std::cout << "Waiting for SSL connection on port: " << port << std::endl;
	openConnection(port);

	// // Set socket mode to non blocking
	// unsigned long int non_block_mode = 1;
	// int ret = ioctl(m_socket, FIONBIO, &non_block_mode);

	int ssl_ret = 0;
	// createSSLContext(m_ssl_ctx, TLS_server_method());
	m_ssl_ctx = SSL_CTX_new(TLS_server_method());
	if (!m_ssl_ctx) {
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	configureSSLContext(m_ssl_ctx);
	m_ssl_socket = SSL_new(m_ssl_ctx);
	ssl_ret = SSL_set_fd(m_ssl_socket, m_socket);
	if (ssl_ret < 1) {
		std::cout << "SSL_set_fd error: " << ERR_reason_error_string(SSL_get_error(m_ssl_socket, ssl_ret)) << std::endl;
	}

	// // Set SSL mode to non blocking
	// int set_blocking_mode_success = SSL_set_blocking_mode(m_ssl_socket, 0);
	// if (set_blocking_mode_success < 1) {
	// 	std::cout << "Could not set SSL blocking mode" << std::endl;
	// 	return false;
	// }	

	SSL_set_accept_state(m_ssl_socket);
	ssl_ret = SSL_accept(m_ssl_socket);
	if (ssl_ret < 1) {
		std::cout << "SSL accept error: " << ERR_reason_error_string(SSL_get_error(m_ssl_socket, ssl_ret)) << std::endl;
		return false;
	} else {
		std::cout << "Accepted SSL connection on port " << port << std::endl;
		return true;
	}
}

int IpMicMonitor::hasPending(int16_t* sample_buffer, size_t sample_buffer_size) {
	int bytes_pending = pending(sample_buffer, sample_buffer_size);
	if (bytes_pending > 0) {
		return 1;
	} else {
		return 0;
	}
}

int IpMicMonitor::pending(int16_t* sample_buffer, size_t sample_buffer_size) {
	uint8_t* sample_buffer_bytes_ptr = reinterpret_cast<uint8_t*>(sample_buffer);
	int sample_buffer_bytes_size = sample_buffer_size * 2;
	// int bytes_read = receive(sample_buffer_bytes_ptr, sample_buffer_size * sizeof(int16_t));
	int bytes_pending = recv(m_socket, sample_buffer_bytes_ptr, sample_buffer_bytes_size, MSG_PEEK | MSG_DONTWAIT);
	return bytes_pending;
}

int IpMicMonitor::receive(uint8_t* buffer, size_t buffer_size) {
	return read(m_socket, buffer, buffer_size);
	// return recv(m_socket, buffer, buffer_size, 0);
}

int IpMicMonitor::receive(int16_t* sample_buffer, size_t sample_buffer_size) {
	// uint8_t* sample_buffer_bytes_ptr = reinterpret_cast<uint8_t*>(sample_buffer);
	// int bytes_read = receive(sample_buffer_bytes_ptr, sample_buffer_size);
	// return bytes_read / 2;

	// int16_t num = (uint8_t)buffer[1] << 8 | (uint8_t)buffer[0];

	uint8_t* sample_buffer_bytes_ptr = reinterpret_cast<uint8_t*>(sample_buffer);
	int bytes_read = receive(sample_buffer_bytes_ptr, sample_buffer_size * sizeof(int16_t));
	int samples_read = bytes_read / 2;
	// for (int i = 0; i < samples_read; i++) {
	// 	sample_buffer[i] = ntohs(sample_buffer[i]);
	// }
	return samples_read;
}

// int IpMicMonitor::hasPendingSSL() {
// 	return SSL_has_pending(m_ssl_socket);
// }

// int IpMicMonitor::pendingSSL() {
// 	return SSL_pending(m_ssl_socket);
// }

int IpMicMonitor::hasPendingSSL(int16_t* sample_buffer, size_t sample_buffer_size) {
	int bytes_pending = pendingSSL(sample_buffer, sample_buffer_size);
	if (bytes_pending > 0) {
		return 1;
	} else {
		return 0;
	}
}

int IpMicMonitor::pendingSSL(int16_t* sample_buffer, size_t sample_buffer_size) {
	uint8_t* sample_buffer_bytes_ptr = reinterpret_cast<uint8_t*>(sample_buffer);
	int sample_buffer_bytes_size = sample_buffer_size * 2;
	return SSL_peek(m_ssl_socket, sample_buffer_bytes_ptr, sample_buffer_bytes_size);
}

int IpMicMonitor::receiveSSL(uint8_t* buffer, size_t buffer_size) {
	// if (m_ssl_socket->hs_status != SSL_OK) {
	// 	std::cout << "" << std::endl;
	// }
	return SSL_read(m_ssl_socket, buffer, buffer_size);
}

int IpMicMonitor::receiveSSL(int16_t* sample_buffer, size_t sample_buffer_size) {
	uint8_t* sample_buffer_bytes_ptr = reinterpret_cast<uint8_t*>(sample_buffer);
	int bytes_read = receiveSSL(sample_buffer_bytes_ptr, sample_buffer_size);
	return bytes_read / 2;
}

void IpMicMonitor::printAudioStream(int port) {
	std::cout << "Starting audio stream output" << std::endl;
	int16_t sample_buffer[BUFFER_SIZE] = {0};
	// uint8_t* sample_buffer_bytes_ptr = reinterpret_cast<uint8_t*>(sample_buffer);

	while (true) {
		if (!connected()) {
			sleep(1);
			std::cout << "Waiting for connection on port " << port << std::endl;
			openConnection(port);
		}

		// int bytes_read = receive(sample_buffer_bytes_ptr);
		int n_samples_read = receive(sample_buffer);

		if (n_samples_read < 1) {
			continue;
		}

		// std::cout << "bytes read: " << bytes_read << std::endl;
		// std::cout << "samples read: " << bytes_read / 2 << std::endl;
		for (int i = 0; i < n_samples_read; i++) {
			std::cout << sample_buffer[i] << std::endl;
		}
	}
}

void IpMicMonitor::recordAudioStream(int rec_time_ms) {}

char* IpMicMonitor::readWav(const char* filename, short* nchannel, short* ssample, int* csample) {
	// Reading the file.
	FILE* fp = fopen(filename, "rb");

	if (!fp) {
		fprintf(stderr, "Couldn't open the file \"%s\"\n", filename);
		exit(0);
	}

	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	printf("The file \"%s\" has %d bytes\n\n", filename, file_size);

	char* buffer = (char*)malloc(sizeof(char) * file_size);
	fread(buffer, file_size, 1, fp);

	// Dump the buffer info.
	*nchannel = *(short*)&buffer[22];
	*ssample = *(short*)&buffer[34] / 8;
	*csample = *(int*)&buffer[40] / *ssample;

	printf("ChunkSize :\t %u\n", *(int*)&buffer[4]);
	printf("Format :\t %u\n", *(short*)&buffer[20]);
	printf("NumChannels :\t %u\n", *(short*)&buffer[22]);
	printf("SampleRate :\t %u\n", *(int*)&buffer[24]);  // number of samples per second
	printf("ByteRate :\t %u\n", *(int*)&buffer[28]);    // number of bytes per second
	printf("BitsPerSample :\t %u\n", *(short*)&buffer[34]);
	printf("Subchunk2ID :\t \"%c%c%c%c\"\n", buffer[36], buffer[37], buffer[38],
	       buffer[39]);                                    // marks beginning of the data section
	printf("Subchunk2Size :\t %u\n", *(int*)&buffer[40]);  // size of data (byte)
	printf("Duration :\t %fs\n\n", (float)(*(int*)&buffer[40]) / *(int*)&buffer[28]);

	fclose(fp);
	return buffer;
}

// PCM to WAV function copied from https://gist.github.com/csukuangfj/c1d1d769606260d436f8674c30662450
void IpMicMonitor::writeWav(const char* filename, int16_t* sample_buffer, int n_samples) {
	typedef struct WAV_HEADER {
		/* RIFF Chunk Descriptor */
		uint8_t RIFF[4] = {'R', 'I', 'F', 'F'};  // RIFF Header Magic header
		uint32_t ChunkSize;                      // RIFF Chunk Size
		uint8_t WAVE[4] = {'W', 'A', 'V', 'E'};  // WAVE Header
		/* "fmt" sub-chunk */
		uint8_t fmt[4] = {'f', 'm', 't', ' '};  // FMT header
		uint32_t Subchunk1Size = 16;            // Size of the fmt chunk
		uint16_t AudioFormat = 1;               // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM
		                                        // Mu-Law, 258=IBM A-Law, 259=ADPCM
		uint16_t NumOfChan = 1;                 // Number of channels 1=Mono 2=Sterio
		uint32_t SamplesPerSec = 16000;         // Sampling Frequency in Hz
		uint32_t bytesPerSec = 16000 * 2;       // bytes per second
		uint16_t blockAlign = 2;                // 2=16-bit mono, 4=16-bit stereo
		uint16_t bitsPerSample = 16;            // Number of bits per sample
		/* "data" sub-chunk */
		uint8_t Subchunk2ID[4] = {'d', 'a', 't', 'a'};  // "data"  string
		uint32_t Subchunk2Size;                         // Sampled data length
	} WavHdr;

	static_assert(sizeof(WavHdr) == 44, "");

	WavHdr wav;
	wav.ChunkSize = n_samples * sizeof(int16_t) + sizeof(WavHdr) - 8;
	wav.Subchunk2Size = n_samples * sizeof(int16_t);

	std::ofstream out(filename, std::ios::binary);
	// Write header
	out.write(reinterpret_cast<const char*>(&wav), sizeof(wav));

	// Write data
	for (int i = 0; i < n_samples; ++i) {
		out.write(reinterpret_cast<char*>(&sample_buffer[i]), sizeof(int16_t));
	}

	// FILE* fp = fopen(filename, "wb");
	// if (!fp) {
	// 	fprintf(stderr, "Couldn't open the file \"%s\"\n", filename);
	// 	exit(0);
	// }
	// fwrite(data, len, 1, fp);
	// fclose(fp);
}

void IpMicMonitor::closeSSL() {
	SSL_free(m_ssl_socket);
	close(m_socket);
	SSL_CTX_free(m_ssl_ctx);
}