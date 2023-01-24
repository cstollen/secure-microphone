#include "netlog_monitor.h"
#include <iostream>
#include <unistd.h>

int main(int argc, char const* argv[]) {
	std::cout << "Netlog monitor" << std::endl;
	Protocol protocol = TCP;
#ifdef NETLOG_PROTOCOL
	if (NETLOG_PROTOCOL == TCP) {
		protocol = TCP;
	} else if (NETLOG_PROTOCOL == UDP) {
		protocol = UDP;
	}
#endif
	unsigned port = 4444;
#ifdef NETLOG_PORT
	port = NETLOG_PORT;
#endif
	bool enable_ssl = false;
#ifdef NETLOG_SSL
	if (NETLOG_SSL == 1) {
		enable_ssl = true;
	} else {
		enable_ssl = false;
	}
#endif

	NetlogMonitor netlog_monitor(port, protocol, enable_ssl);

	char message[BUFFER_SIZE] = {0};
	while (true) {
		if (!netlog_monitor.hasConnection()) {
			sleep(1);
			netlog_monitor.openConnection(port, protocol, enable_ssl);
		}

		int bytes_read = netlog_monitor.readMessage(message);
		if (bytes_read < 1) {
			continue;
		}

		std::cout << message;
	}

	exit(EXIT_SUCCESS);
}
