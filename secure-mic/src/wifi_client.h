#pragma once

#include <SPI.h>
#include <WiFiNINA.h>
#include <initializer_list>
#include "netlog.h"

/*
struct Connection {
  char* url = "";
  int port = -1;
  bool udp = false;
  bool ssl = false;
};
*/

class WifiClient {
 public:
	WifiClient(NetLog* netlog = nullptr);
	// void setVerbosity(bool verbose) { this->verbose = verbose; }
	bool connectToWiFi(const char* wifi_ssid, const char* wifi_pass, const int wifi_timeout = 50000,
	                   bool verbose = false);
	void ping(const char* host_name);
	bool connectTCP(char* url, int port, int retries = 10, bool verbose = false);
	bool connectSSL(const char* url, const int port);
	bool connected() { return wifi_tcp_client.connected(); }
	int send(uint8_t* payload, size_t payload_size);
	int send(char* payload) { send(reinterpret_cast<uint8_t*>(payload), strlen(payload)); }
	int ping(char* url);
	void disconnectSSL() { wifi_tcp_client.stop(); }

 private:
	// bool verbose = false;
	NetLog* netlog = nullptr;
	WiFiClient wifi_tcp_client;
	// WiFiUDP wifi_udp_client;
	char* remote_host = "";
	int remote_port = 4444;
	uint8_t wifi_status = WL_IDLE_STATUS;

	bool wifininaFirmwareCheck();
	void connect(const char* wifi_ssid, const char* wifi_pass, const int wifi_timeout = 50000, bool verbose = false);
	void printCurrentNet();
	void printWifiData();
	void listNetworks();
	std::string macAddressToString(byte mac[]);
	const char* encryptionTypeToString(int enc_type);
	const char* wifiStatusToString(uint8_t status);
	uint8_t beginEnterprise(const char* ssid, const char* username, const char* password, const char* identity,
	                        const char* ca, const int wifi_timeout = 50000);

	std::string concatLog() { return std::string(); };
	template <typename T, typename... Args>
	std::string concatLog(T first, Args... args) {
		std::stringstream ss;
		ss << first << concatLog(args...);
		return ss.str();
	}

	template <typename... Args>
	void nlog(Args... args) {
		if (netlog != nullptr) {
			netlog->nlog(args...);
		} else {
			std::string message = concatLog(args...) + "\n";
			Serial.print(message.c_str());
		}
	}
};
