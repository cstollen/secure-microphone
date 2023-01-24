#include "netlog_monitor.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

NetlogMonitor::NetlogMonitor() {}

NetlogMonitor::NetlogMonitor(int port, Protocol protocol, bool enable_ssl)
    : protocol{protocol}, enable_ssl{enable_ssl} {
	// this->openConnection(port, protocol, enable_ssl);
}

NetlogMonitor::~NetlogMonitor() { closeConnection(); }

bool NetlogMonitor::openConnection(int port, Protocol protocol, bool enable_ssl) {
	bool connected = false;
	if (protocol == TCP) {
		connected = openTCPConnection(port);
	} else if (protocol == UDP) {
		connected = openUDPConnection(port);
	}

	bool ssl_connected = false;
	if (enable_ssl) {
		ssl_connected = openSSLConnection();
	}

	has_connection = enable_ssl ? (connected && ssl_connected) : connected;
	return enable_ssl ? (connected && ssl_connected) : connected;
}

void NetlogMonitor::closeConnection() {
	if (protocol == TCP) {
		close(udp_socket);
	} else if (protocol == UDP) {
		close(tcp_socket);
	}

	if (enable_ssl) {
		SSL_shutdown(ssl_connection);
		SSL_free(ssl_connection);
	}

	has_connection = false;
}

bool NetlogMonitor::openTCPConnection(int port) {
	printf("Listening for TCP connection on port %d\n", port);
	if (openTCPSocket(tcp_socket, port)) {
		printf("Connected to TCP client on port %d\n", port);
		return true;
	} else {
		printf("Cound not connect socket on port %d\n", port);
		return false;
	}
}

bool NetlogMonitor::openUDPConnection(int port) {
	std::cout << "Listening for UDP messages on port " << port << std::endl;
	if (openUDPSocket(udp_socket, port)) {
		return true;
	} else {
		std::cout << "Cound not open socket on port " << port << std::endl;
		return false;
	}
}

bool NetlogMonitor::openSocket(int& sock, int port) {
	int opt = 1;
	int addrlen = sizeof(local_address);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		printf("Could not create socket: (%s)\n", strerror(errno));
		return false;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		printf("Could not set socket options (%s)", strerror(errno));
		exit(EXIT_FAILURE);
	}
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = INADDR_ANY;
	local_address.sin_port = htons(port);

	if (bind(sock, (struct sockaddr*)&local_address, sizeof(local_address)) < 0) {
		printf("Could not bind socket on port %d (%s)\n", port, strerror(errno));
		return false;
	}
	if (listen(sock, 3) < 0) {
		printf("Could not listen to socket connection on port %d (%s)\n", port, strerror(errno));
		return false;
	}
	return true;
}

bool NetlogMonitor::openTCPSocket(int& tcp_socket, int port) {
	int opt = 1;
	int addrlen = sizeof(local_address);

	if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		printf("Could not create socket: (%s)\n", strerror(errno));
		return false;
	}

	if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		printf("Could not set socket options (%s)", strerror(errno));
		exit(EXIT_FAILURE);
	}
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = INADDR_ANY;
	local_address.sin_port = htons(port);

	if (bind(tcp_socket, (struct sockaddr*)&local_address, sizeof(local_address)) < 0) {
		printf("Could not bind socket on port %d (%s)\n", port, strerror(errno));
		return false;
	}
	if (listen(tcp_socket, 3) < 0) {
		printf("Could not listen to TCP connection on port %d (%s)\n", port, strerror(errno));
		return false;
	}
	if ((tcp_socket = accept(tcp_socket, (struct sockaddr*)&local_address, (socklen_t*)&addrlen)) < 0) {
		printf("Could not accept TCP connection on port %d (%s)\n", port, strerror(errno));
		return false;
	}
	return true;
}

bool NetlogMonitor::openUDPSocket(int& udp_socket, int port) {
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	local_address.sin_port = htons(port);

	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket < 0) {
		printf("Could not create socket (%s)", strerror(errno));
		return false;
	}

	int opt = 1;
	if (setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		printf("Could not set socket options (%s)", strerror(errno));
		return false;
	}

	if (bind(udp_socket, (struct sockaddr*)&local_address, sizeof(local_address)) < 0) {
		printf("Could not bind socket on port %d (%s)\n", port, strerror(errno));
		return false;
	}

	return true;
}

int NetlogMonitor::readMessage(char* message) {
	if (protocol == TCP) {
		return readTCPSocket(tcp_socket, message);
	} else if (protocol == UDP) {
		return readUDPSocket(udp_socket, message);
	}
	return -1;
}

int NetlogMonitor::readTCPSocket(const int& tcp_socket, char* message) {
	int bytes_read;
	if (enable_ssl) {
		// int tcp_bytes_readable = read(tcp_socket, buffer, BUFFER_SIZE, MSG_PEEK);
		int bytes_pending = SSL_pending(ssl_connection);
		bytes_read = SSL_read(ssl_connection, buffer, bytes_pending);
	} else {
		bytes_read = read(tcp_socket, buffer, BUFFER_SIZE);
	}

	if (bytes_read < 0) {
		if (errno != ECONNRESET) {
			printf("Could not receive TCP message: (%s)\n", strerror(errno));
		} else if (errno == ECONNRESET) {
			closeConnection();
		} else {
		}
		return -1;
	}

	memset(message, 0, BUFFER_SIZE);
	memcpy(message, buffer, bytes_read);
	return bytes_read;
}

int NetlogMonitor::readUDPSocket(const int& udp_socket, char* message) {
	socklen_t remote_address_len = sizeof(remote_address);
	int bytes_read = 0;
	if (enable_ssl) {
		bytes_read = SSL_read(ssl_connection, buffer, BUFFER_SIZE);
	} else {
		bytes_read = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (sockaddr*)&remote_address, &remote_address_len);
	}

	if (bytes_read < 0) {
		printf("Could not receive UDP datagram (%s)\n", strerror(errno));
		return -1;
	}

	memset(message, 0, BUFFER_SIZE);
	memcpy(message, buffer, bytes_read);
	return bytes_read;
}

void NetlogMonitor::createSSLContext(SSL_CTX*& ctx, const SSL_METHOD* method) {
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
}

void NetlogMonitor::configureSSLContext(SSL_CTX* ctx) {
	// SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
	// assert_panic(ctx != 0, "unable to create SSL server context");
	// int uc = SSL_CTX_use_certificate_chain_file(ctx, SSL_CERT_FILE);
	// assert_panic(uc == 1, "unable to use cert.pem");
	// int up = SSL_CTX_use_PrivateKey_file(ctx, SSL_KEY_FILE, SSL_FILETYPE_PEM);
	// assert_panic(up == 1, "unable to use key.pem");

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

bool NetlogMonitor::openSSLConnection() {
	int ssl_ret = 0;
	if (protocol == TCP) {
		createSSLContext(ssl_ctx, TLS_server_method());
		configureSSLContext(ssl_ctx);
		ssl_connection = SSL_new(ssl_ctx);
		ssl_ret = SSL_set_fd(ssl_connection, tcp_socket);
	} else if (protocol == UDP) {
		createSSLContext(ssl_ctx, DTLS_server_method());
		configureSSLContext(ssl_ctx);
		ssl_connection = SSL_new(ssl_ctx);
		ssl_ret = SSL_set_fd(ssl_connection, udp_socket);
	}
	if (ssl_ret < 1) {
		std::cout << "SSL_set_fd error: " << ERR_reason_error_string(SSL_get_error(ssl_connection, ssl_ret)) << std::endl;
	}

	std::cout << "Waiting for ssl connection" << std::endl;
	ssl_ret = SSL_accept(ssl_connection);
	if (ssl_ret < 1) {
		std::cout << "SSL_accept error: " << ERR_reason_error_string(SSL_get_error(ssl_connection, ssl_ret)) << std::endl;
		return false;
	} else {
		std::cout << "SSL accepted" << std::endl;
		return true;
	}
}
