#pragma once
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
// #include <sys/socket.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFFER_SIZE 255

class IpMicMonitor {
 public:
	bool openConnection(int port = 4444);
	bool openSSLConnection(int port = 4444);
	bool connected() { return m_connected; }
	int hasPending(int16_t* sample_buffer, size_t sample_buffer_size = BUFFER_SIZE);
	int pending(int16_t* sample_buffer, size_t sample_buffer_size = BUFFER_SIZE);
	int receive(uint8_t* buffer, size_t buffer_size = BUFFER_SIZE);
	int receive(int16_t* sample_buffer, size_t sample_buffer_size = BUFFER_SIZE);
	// int hasPendingSSL();
	// int pendingSSL();
	int hasPendingSSL(int16_t* sample_buffer, size_t sample_buffer_size = BUFFER_SIZE);
	int pendingSSL(int16_t* sample_buffer, size_t sample_buffer_size = BUFFER_SIZE);
	int receiveSSL(uint8_t* buffer, size_t buffer_size = BUFFER_SIZE);
	int receiveSSL(int16_t* sample_buffer, size_t sample_buffer_size = BUFFER_SIZE);
	void printAudioStream(int port);
	void recordAudioStream(int rec_time_ms);
	char* readWav(const char* filename, short* nchannel, short* ssample, int* csample);
	void writeWav(const char* filename, int16_t* sample_buffer, int n_samples);
	void closeSSL();

 private:
	struct sockaddr_in m_remote_address;
	int m_socket;
	int openSocket(int port);
	bool m_connected = false;

	SSL* m_ssl_socket;
	SSL_CTX* m_ssl_ctx;
	void createSSLContext(SSL_CTX*& ctx, const SSL_METHOD* method);
	void configureSSLContext(SSL_CTX* ctx);
};