import socket
import ssl

#SERVER = "hoegarden.techfak.uni-bielefeld.de"
SERVER= "0.0.0.0"
PORT = 55555
CERT_FILE = "../secure-mic-monitor/cert/ssl-cert.pem"
KEY_FILE = "../secure-mic-monitor/cert/ssl-key.pem"
BUFFER_SIZE = 255

context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
#context.load_cert_chain('/path/to/certchain.pem', '/path/to/private.key')
context.load_cert_chain(CERT_FILE, KEY_FILE)

# https://docs.python.org/3/library/socket.html#socket.socket.listen

# Flash existing certificate to rp2040
# cd secure-microphone/secure-mic
# tools/arduino-fwuploader/arduino-fwuploader certificates flash --file cert/ssl-cert.pem -b arduino:mbed_nano:nanorp2040connect -a /dev/ttyACM0

# Open serial console
# cat /dev/ttyACM0

# Start netlog monitor
# cd secure-microphone/netlog-monitor/build
# ./netlog-monitor

# Start python microphone monitor
# python3 ./secure-mic-monitor.py

# Green LED on: power
# Orange LED on: Waiting for serial connection (cat /dev/ttyACM0)
# Orange LED blinking: Heartbeat / Microphone started

print("Secure microphone monitor started")

#uint16 data

#backlog = 5
backlog = 0
with socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) as sock:
#with socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0) as sock:
#	sock.bind(('127.0.0.1', PORT))
#	sock.close()
	sock.bind((SERVER, PORT))
	sock.listen(backlog)
	with context.wrap_socket(sock, server_side=True) as ssock:
		conn, addr = ssock.accept()
		print("Accepted socket connection from", addr[0] + ":" + str(addr[1]))

#httpd.socket = ssl.wrap_socket(httpd.socket,
#                               server_side=True,
#                               certfile="server.pem",
#                               keyfile="key.pem",
#                               ssl_version=ssl.PROTOCOL_TLS)

		try:
			while True:
				message = conn.recv(BUFFER_SIZE)
				#data = uint16.from_bytes(message)
				data = int.from_bytes(message, byteorder='little', signed=False)
				print("raw:")
				print(message)
				print("uint16:")
				print(data)
		except KeyboardInterrupt:
			sock.close()

