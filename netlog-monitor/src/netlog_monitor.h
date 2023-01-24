#pragma once
#include <stdlib.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFFER_SIZE 255

enum Protocol { TCP, UDP };

class NetlogMonitor {
 public:
	NetlogMonitor();
	NetlogMonitor(int port = 4444, Protocol protocol = TCP, bool enable_ssl = false);
	~NetlogMonitor();
	bool openConnection(int port = 4444, Protocol protocol = TCP, bool enable_ssl = false);
	void closeConnection();
	int readMessage(char* message);
	bool hasConnection() { return has_connection; };

 private:
	Protocol protocol = UDP;
	int tcp_socket;
	int udp_socket;
	int m_socket;
	struct sockaddr_in local_address;
	struct sockaddr_in remote_address;
	char buffer[BUFFER_SIZE] = {0};
	bool has_connection = false;

	bool openTCPConnection(int port);
	bool openUDPConnection(int port);
	bool openSocket(int& socket, int port);
	bool openTCPSocket(int& tcp_socket, const int port);
	bool openUDPSocket(int& udp_socket, const int port);
	int readTCPSocket(const int& tcp_socket, char* message);
	int readUDPSocket(const int& udp_socket, char* message);

	bool enable_ssl = false;
	SSL* ssl_connection;
	SSL_CTX* ssl_ctx;
	void createSSLContext(SSL_CTX*& ctx, const SSL_METHOD* method);
	void configureSSLContext(SSL_CTX* ctx);
	bool openSSLConnection();
};
