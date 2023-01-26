#include "wifi_client.h"
// #include <Arduino.h>
#include <stdlib.h>

WifiClient::WifiClient(NetLog* netlog) : netlog{netlog} { wifininaFirmwareCheck(); }

bool WifiClient::wifininaFirmwareCheck() {
	String wifinina_fv = WiFi.firmwareVersion();
	String wifinina_latest_fv = WIFI_FIRMWARE_LATEST_VERSION;

	if (wifinina_fv < wifinina_latest_fv) {
		nlog("WifiNINA installed firmware version: ", wifinina_fv);
		nlog("WifiNINA latest firmware version available: ", wifinina_latest_fv);
		nlog("Please update WifiNINA firmware version.");
		return false;
	} else {
		return true;
	}
}

bool WifiClient::connectToWiFi(const char* wifi_ssid, const char* wifi_pass, const int wifi_timeout, bool verbose) {
	// check for the WiFi module:
	if (WiFi.status() == WL_NO_MODULE) {
		nlog("Communication with WiFi module failed!");
		// don't continue
		while (true)
			;
	}

	wifininaFirmwareCheck();

	// List WiFi networks
	// listNetworks();

	// Attempt to connect to WiFi network:
	WiFi.setTimeout(wifi_timeout);

	nlog("Connecting to WiFi network with SSID: ", wifi_ssid);
	// Connect to WPA2 network
	WiFi.begin(wifi_ssid, wifi_pass);
	// Connect to WPA2 enterprise network (PEAP/MSCHAPv2)
	// status = WiFi.beginEnterprise(wifi_ssid, wifi_user, wifi_pass);
	// status = WiFi.beginEnterprise(wifi_ssid, wifi_user, wifi_pass, wifi_identity, wifi_root_cert);

	wifi_status = WiFi.status();

	if (wifi_status != WL_CONNECTED) {
		nlog("Could not connect to WiFi network with SSID: ", wifi_ssid);
		nlog("Reason code: ", (int)WiFi.reasonCode(), " WiFi status: ", wifiStatusToString(wifi_status));
		return false;
	} else {
		nlog("Connected to WiFi network");
		if (verbose) {
			printCurrentNet();
			printWifiData();
		}
		// Ping server
		// ping("smartmirror7.ks.techfak.uni-bielefeld.de");

		return true;
	}

	// Connect to socket server
	// connectToServer(server_url, server_port);

	// Connect to SSL server
	// connectToSSLServer(server_url, server_port);
}

void WifiClient::printCurrentNet() {
	// Print the SSID of the network
	nlog("SSID: ", WiFi.SSID());

	// Print the MAC address of the router
	byte bssid[6];
	WiFi.BSSID(bssid);
	nlog("BSSID: ", macAddressToString(bssid));

	// Print the received signal strength
	long rssi = WiFi.RSSI();
	nlog("Signal strength (RSSI): ", (int)rssi, " dBm");

	// Print the encryption type
	byte encryption = WiFi.encryptionType();
	nlog("Encryption Type: ", encryptionTypeToString((int)encryption));
}

void WifiClient::printWifiData() {
	// Print the local IP address
	IPAddress ip = WiFi.localIP();
	nlog("IP Address: ", (int)ip[0], ".", (int)ip[1], ".", (int)ip[2], ".", (int)ip[3]);

	// Print the ocal MAC address:
	byte mac[6];
	WiFi.macAddress(mac);
	nlog("MAC address: ", macAddressToString(mac));
}

std::string WifiClient::macAddressToString(byte mac[]) {
	std::string mac_str = "";
	for (int i = 5; i >= 0; i--) {
		char hex_val[2];
		sprintf(hex_val, "%02X", hex_val[i]);
		mac_str += hex_val;
		if (i > 0) {
			mac_str += ":";
		}
	}
	return mac_str;
}

void WifiClient::listNetworks() {
	// Scan for WiFi networks:
	nlog("Scan Networks");
	int num_ssid = WiFi.scanNetworks();
	if (num_ssid == -1) {
		nlog("Couldn't get a WiFi connection");
		while (true)
			;
	}

	// Print the list of WiFi networks
	nlog("Number of available networks: ", num_ssid);

	// Print the network number and name for each network found
	for (int wifi_net = 0; wifi_net < num_ssid; wifi_net++) {
		nlog(wifi_net, ") ", WiFi.SSID(wifi_net), "\tSignal: ", WiFi.RSSI(wifi_net),
		     " dBm\tEncryption: ", encryptionTypeToString(WiFi.encryptionType(wifi_net)));
	}
}

const char* WifiClient::encryptionTypeToString(int enc_type) {
	// Read the encryption type and print out the name
	std::string enc_str = "";
	switch (enc_type) {
		case ENC_TYPE_WEP:
			return "WEP";
		case ENC_TYPE_TKIP:
			return "WPA";
		case ENC_TYPE_CCMP:
			return "WPA2";
		case ENC_TYPE_NONE:
			return "None";
		case ENC_TYPE_AUTO:
			return "Auto";
		case ENC_TYPE_UNKNOWN:
		default:
			return "Unknown";
	}
}

const char* WifiClient::wifiStatusToString(uint8_t status) {
	// WL_CONNECTED: assigned when connected to a WiFi network;
	// WL_AP_CONNECTED : assigned when a device is connected in Access Point mode;
	// WL_AP_LISTENING : assigned when the listening for connections in Access Point mode;
	// WL_NO_SHIELD: assigned when no WiFi shield is present;
	// WL_NO_MODULE: assigned when the communication with an integrated WiFi module fails;
	// WL_IDLE_STATUS: it is a temporary status assigned when WiFi.begin() is called and remains active until the number
	// of attempts expires (resulting in WL_CONNECT_FAILED) or a connection is established (resulting in WL_CONNECTED);
	// WL_NO_SSID_AVAIL: assigned when no SSID are available;
	// WL_SCAN_COMPLETED: assigned when the scan networks is completed;
	// WL_CONNECT_FAILED: assigned when the connection fails for all the attempts;
	// WL_CONNECTION_LOST: assigned when the connection is lost;
	// WL_DISCONNECTED: assigned when disconnected from a network;
	switch (status) {
		case WL_IDLE_STATUS:
			return "WL_IDLE_STATUS";
		case WL_NO_SSID_AVAIL:
			return "WL_NO_SSID_AVAIL";
		case WL_SCAN_COMPLETED:
			return "WL_SCAN_COMPLETED";
		case WL_CONNECTED:
			return "WL_CONNECTED";
		case WL_CONNECT_FAILED:
			return "WL_CONNECT_FAILED";
		case WL_CONNECTION_LOST:
			return "WL_CONNECTION_LOST";
		case WL_DISCONNECTED:
			return "WL_DISCONNECTED";
		case WL_AP_LISTENING:
			return "WL_AP_LISTENING";
		case WL_AP_CONNECTED:
			return "WL_AP_CONNECTED";
		case WL_NO_MODULE:
			return "WL_NO_MODULE";
		default:
			return "UNKNOWN";
	}
}

uint8_t WifiClient::beginEnterprise(const char* ssid, const char* username, const char* password, const char* identity,
                                    const char* ca, const int wifi_timeout) {
	uint8_t status = WL_IDLE_STATUS;

	// Set passphrase
	if (WiFiDrv::wifiSetEnterprise(0 /*PEAP/MSCHAPv2*/, ssid, strlen(ssid), username, strlen(username), password,
	                               strlen(password), identity, strlen(identity), ca, strlen(ca) + 1) != WL_FAILURE) {
		for (unsigned long start = millis(); (millis() - start) < wifi_timeout;) {
			delay(WL_DELAY_START_CONNECTION);
			status = WiFiDrv::getConnectionStatus();
			if ((status != WL_IDLE_STATUS) && (status != WL_NO_SSID_AVAIL) && (status != WL_SCAN_COMPLETED)) {
				break;
			}
		}
	} else {
		status = WL_CONNECT_FAILED;
	}
	return status;
}

void WifiClient::ping(const char* url) {
	IPAddress ip;
	int err = WiFi.hostByName(url, ip);
	if (err == 1) {
		// nlog("URL: ", url);
		// nlog("IP: ", (int)ip[0], ".", (int)ip[1], ".", (int)ip[2], ".", (int)ip[3]);
	} else {
		nlog("Could not resolve ip for: ", url);
	}

	int ping_result = WiFi.ping(url);
	if (ping_result >= 0) {
		nlog("Ping ", url, " ", (int)ip[0], ".", (int)ip[1], ".", (int)ip[2], ".", (int)ip[3],
		     ": SUCCESS RTT = ", ping_result, " ms");
	} else {
		nlog("Ping ", url, " ", (int)ip[0], ".", (int)ip[1], ".", (int)ip[2], ".", (int)ip[3],
		     ": FAILED Error code: ", ping_result);
	}
	delay(1000);
}

bool WifiClient::connectTCP(char* url, int port, int retries, bool verbose) {
	nlog("Connecting to TCP server: ", url, ":", port);
	while (!wifi_tcp_client.connect(url, port)) {
		if (verbose) {
			nlog("Could not connect to TCP server: ", url, ":", port);
		}
		if (retries < 1) {
			return false;
		}

		delay(1000);
		if (verbose) {
			nlog("Retries left: ", retries);
		}
		retries--;
	}

	if (verbose && wifi_tcp_client.connected()) {
		nlog("Connected to TCP server", url, ":", port);
	}
	return true;
}

bool WifiClient::connectSSL(const char* url, const int port) {
	nlog("Connecting to SSL server: ", url, ":", port);
	while (!wifi_tcp_client.connectSSL(url, port)) {
		nlog("Could not connect to SSL server: ", url, ":", port);
		delay(1000);
	}
	nlog("Connected to SSL server: ", url, ":", port);
	return true;
}

int WifiClient::send(uint8_t* payload, size_t payload_size) {
	int bytes_sent = wifi_tcp_client.write(payload, payload_size);
	if (bytes_sent < 0) {
		nlog("Could not send TCP payload");
	}
	return bytes_sent;
}

int ping(char* url) {}
