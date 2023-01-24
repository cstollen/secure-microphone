#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "netlog.h"

NetLog::~NetLog() { closeConnection(); }

bool NetLog::openConnection(char* url, int port, bool use_tcp, bool enable_ssl, bool verbose, int retries) {
	this->use_tcp = use_tcp;
	if (use_tcp) {
		nlog("Logging messages to TCP server: ", url, ":", port);
		if (!openTCPConnection(url, port, retries, verbose)) {
			return false;
		}
	} else {
		nlog("Logging messages to UDP server: ", url, ":", port);
		if (!wifi_udp_client.begin(port)) {
			return false;
		}
	}

	remote_host = url;
	remote_port = port;
	remote_connected = true;
	return true;
}

bool NetLog::openTCPConnection(char* url, int port, int retries, bool verbose) {
	while (!wifi_tcp_client.connect(url, port)) {
		if (verbose) {
			nlog("Could not connect to TCP server");
			nlog("  URL:  ", url);
			nlog("  Port: ", port);
		}
		if (retries < 1) {
			return false;
		}

		delay(1);
		if (verbose) {
			nlog("Retries left: ", retries);
		}
		retries--;
	}

	if (verbose && wifi_tcp_client.connected()) {
		nlog("Connected to TCP server");
	}
	return true;
}

void NetLog::closeConnection() {
	if (use_tcp) {
		wifi_tcp_client.stop();
	} else {
		wifi_udp_client.stop();
	}
	remote_connected = false;
}

void NetLog::sendMessage(const char* message) {
	int bytes_sent;
	if (use_tcp) {
		int bytes_sent = wifi_tcp_client.write(message, strlen(message));
		if (bytes_sent < 0) {
			nlog("Could not send tcp message");
		}
	} else {
		wifi_udp_client.beginPacket(remote_host, remote_port);
		int bytes_sent = wifi_udp_client.write(message, strlen(message));
		wifi_udp_client.endPacket();
		if (bytes_sent < 0) {
			nlog("Could not send datagram");
		}
	}
}

bool NetLog::connectSSL(char* url, int port) {
	if (!use_tcp) {
		nlog("Enabling TCP protocol");
		this->use_tcp = true;
	}
	nlog("Establishing SSL connection to ", url, ":", port);
	while (!wifi_tcp_client.connectSSL(url, port)) {
		nlog("Could not connect SSL to ", url, ":", port);
		delay(2000);
	}
	nlog("Connected SSL to ", url, ":", port);

	remote_host = url;
	remote_port = port;
	remote_connected = true;
	return true;
}
