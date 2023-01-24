#pragma once

#include <initializer_list>
#include <string>
#include <bitset>
#include <sstream>
#include <stdio.h>
#include <SPI.h>
#include <WiFiNINA.h>

class NetLog {
 public:
	~NetLog();
	void setSerialLogging(bool serial_logging) { this->serial_logging = serial_logging; }
	void setRemoteLogging(bool remote_logging) { this->remote_logging = remote_logging; }
	void setPrintFuntion(int (*print)(const char* __restrict__ __format, ...)) { this->print = print; }
	void setEnableTcp(bool enable_tcp) { this->use_tcp = enable_tcp; };
	bool openConnection(char* url, int port, bool use_tcp = false, bool enable_ssl = false, bool verbose = false,
	                    int retries = 10);
	void closeConnection();
	void connectToWiFi(const char* wifi_ssid, const char* wifi_pass, const int wifi_timeout = 50000,
	                   bool verbose = false);
	bool connectSSL(char* url, int port);

	template <typename... Args>
	void nlog(Args... args) {
#ifdef DEBUG
		// Concatenate log arguments
		std::string message = concatLog(args...) + "\n";

		// Log to serial connection
		if (serial_logging) {
			Serial.print(message.c_str());
		}

		// Log to remote connection
		if (remote_logging && remote_connected) {
			sendMessage(message.c_str());
		}
#endif
	}

	std::string concatLog() { return std::string(); };
	template <typename T, typename... Args>
	std::string concatLog(T first, Args... args) {
		std::stringstream ss;
		ss << first << concatLog(args...);
		return ss.str();
	}

 private:
	bool serial_logging = true;
	bool remote_logging = true;
	bool use_tcp = false;
	char* server_url;
	int server_port;
	int tcp_socket;
	int udp_socket;
	bool remote_connected = false;
	int (*print)(const char* __restrict__ __format, ...) = *printf;

	WiFiClient wifi_tcp_client;
	WiFiUDP wifi_udp_client;
	char* remote_host = "";
	int remote_port = 4444;
	uint8_t wifi_status = WL_IDLE_STATUS;

	WiFiSSLClient wifi_ssl_client;
	bool openTCPConnection(char* url, int port, int retries = 10, bool verbose = false);
	void sendMessage(const char* message);
};