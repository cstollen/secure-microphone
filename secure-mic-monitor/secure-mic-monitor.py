import socket
import ssl
import array
import wave
import time
import threading
import signal
import datetime
import struct

HOST= "0.0.0.0"
PORT = 55555
CERT_FILE = "cert/tls-cert-hoegarden.pem"
KEY_FILE = "cert/tls-key-hoegarden.pem"
SOCKET_BUFFER_SIZE = 255

# https://docs.python.org/3/library/socket.html#socket.socket.listen

# Generate SSL certificate and key file
# cd secure-microphone/secure-mic/cert
# openssl req -nodes -new -x509 -days 3650 -extensions v3_ca -keyout ssl-key.pem -out ssl-cert.pem -subj /CN=smartmirror7.ks.techfak.uni-bielefeld.de

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

## LED Status Information
# | LED           | Status                         |
# |---------------|--------------------------------|
# | off           | flashing or error state        |
# | fast blink    | waiting for WiFi connection    |
# | double blink  | waiting for SSL/TLS connection |
# | blink         | hotword recognition mode       |
# | on            | transmitting audio stream      |

# Tunnel audio data stream from hoegarden to smartmirror7
# sshpass -p <password> ssh -N -L hoegarden.techfak.uni-bielefeld.de:55555:localhost:55555 jasmin@smartmirror7.ks.techfak.uni-bielefeld.de

# Secure microphone serial connection
# Device: /dev/ttyACM0, baud rate: 115200

exit_signal = threading.Event()

timestamp_audio_received = time.time()
receive_mode = True
max_receive_time_s = 0.5
rec_idx = 0
server_socket = None
rec_frequency = 16000 # Samplerate

# rec_time_s = 30
# rec_buffer_size = rec_frequency * rec_time_s * 2

# int16_t sample_buffer[SOCKET_BUFFER_SIZE] = {0};
# int16_t rec_buffer[rec_buffer_size] = {0};
# sample_buffer = [0] * SOCKET_BUFFER_SIZE
# rec_buffer = [0] * rec_buffer_size
# sample_buffer = array.array('H', [0] * SOCKET_BUFFER_SIZE)
# rec_buffer = array.array('H', [0] * rec_buffer_size)
# sample_buffer = bytearray([0] * SOCKET_BUFFER_SIZE)
# rec_buffer = bytearray([0] * rec_buffer_size)

# Unsigned short ('h') record buffer
rec_buffer = array.array('h')
# Binary data buffer
bin_buffer = array.array('B')

def signalHandler(signum, frame):
	print("Caught signal %d" % signum)
	exit_signal.set()

class ServiceExit(Exception):
	pass

def createServerSocket(host, port):
	backlog = 0
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
	sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	sock.bind((host, port))
	sock.listen(backlog)
	print("Waiting for SSL connection on port:", port)
	context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
	context.load_cert_chain(CERT_FILE, KEY_FILE)
	wrap_sock = context.wrap_socket(sock, server_side=True)
	return wrap_sock

def saveAsBinary(filename, audio_data):
	with open(filename, 'wb') as binary_file:
		binary_file.write(audio_data)

def saveAsAscii(filename, audio_data):
	with open(filename, 'w') as raw_file:
		for sample in audio_data:
			raw_file.write(f"{sample}\n")

def saveAsWav(filename, audio_data):
	with wave.open(filename, 'wb') as wave_file:
		wave_file.setnchannels(1)  # Mono audio
		wave_file.setsampwidth(2)  # 16-bit audio
		# wave_file.setframerate(44100)  # Sample rate
		wave_file.setframerate(rec_frequency)  # Sample rate
		wave_file.writeframes(audio_data)

def saveAudio():
	global exit_signal, rec_buffer, rec_idx, receive_mode, bin_buffer
	while not exit_signal.is_set():
		if not receive_mode and rec_idx > 0:
			# Save audio to wav file
			timestamp = time.time()
			file_timestamp = datetime.datetime.fromtimestamp(timestamp).strftime('%Y%m%d_%H%M%S')

			filename_bin = "audio_stream_" + file_timestamp + ".bin"
			print("Write audio to bin file:", filename_bin)
			saveAsBinary(filename_bin, bin_buffer)

			filename_wav = "audio_stream_" + file_timestamp + ".wav"
			print("Write audio to wav file:", filename_wav)
			saveAsWav(filename_wav, bin_buffer)

			filename_ascii = "audio_stream_" + file_timestamp + ".dat"
			print("Write audio to ascii file:", filename_ascii)
			saveAsAscii(filename_ascii, rec_buffer)

			receive_mode = True
			rec_idx = 0
			# rec_buffer = array.array('H')
			rec_buffer = array.array('h')
			bin_buffer = array.array('B')
		else:
			time.sleep(1)

def modeWatchdog():
	global exit_signal, receive_mode
	while not exit_signal.is_set():
		receive_time_s = time.time() - timestamp_audio_received
		if receive_mode and (receive_time_s > max_receive_time_s):
			receive_mode = False
			# print("Receive mode off")
			# print("Receive mode")
			print("Waiting for audio data")
			time.sleep(0.01)
		else:
			time.sleep(0.1)

# def receiveAudioData(client_socket):
# 	global exit_signal, rec_idx, receive_mode, timestamp_audio_received
# 	while not exit_signal.is_set():
# 		while rec_idx < rec_buffer_size:
# 			if exit_signal.is_set():
# 				break
# 			data_size = struct.unpack('!I', client_socket.recv(4))[0]
# 			message_bytes = client_socket.recv(SOCKET_BUFFER_SIZE)
# 			bytes_read = len(message_bytes)
# 			if bytes_read == 0:
# 				continue
# 			# data = array.array('H', message_bytes)
# 			# samples_read = len(data)
# 			timestamp_audio_received = time.time()
# 			receive_mode = True
# 			# print("Receive mode on")
# 			if rec_idx + bytes_read < rec_buffer_size:
# 				# Fill recording buffer
# 				# std::memcpy(rec_buffer + rec_idx, &sample_buffer, samples_read * sizeof(int16_t));
# 				for i in range(bytes_read):
# 					rec_buffer[rec_idx] = message_bytes[i]
# 				rec_idx += bytes_read
# 				if rec_idx % rec_frequency == 0:
# 					print("recorded samples:", rec_idx)
# 			else:
# 				# Fill remainder of recording buffer
# 				# std::memcpy(rec_buffer + rec_idx, &sample_buffer, (rec_buffer_size - rec_idx) * sizeof(int16_t));
# 				for i in range(rec_buffer_size - rec_idx):
# 					rec_buffer[rec_idx] = message_bytes[i]
# 				rec_idx += rec_buffer_size - rec_idx
# 				print("recorded samples:", rec_idx)
# 				break
# 		print("Audio sample buffer overflow: Maximum record time:", rec_time_s, "seconds")

def receiveAudioData(client_socket):
	global exit_signal, receive_mode, timestamp_audio_received, rec_buffer, rec_idx
	audio_data = b''
	while not exit_signal.is_set():
		# data_size = struct.unpack('!I', client_socket.recv(4))[0]
		chunk = client_socket.recv(SOCKET_BUFFER_SIZE)
		audio_data += chunk
		bytes_read = len(audio_data)
		samples_read = bytes_read//2
		if bytes_read == 0:
			print("Received zero bytes")
			continue
		timestamp_audio_received = time.time()
		if not receive_mode:
			# First data chunk
			receive_mode = True
			print("Receiving audio data")
		# Struct unpack format string: little-endian (<), unsigned short (h)
		rec_buffer.extend(struct.unpack(f"<{samples_read}h", audio_data))
		bin_buffer.extend(audio_data)
		rec_idx += samples_read
		audio_data = b''
		# if bytes_read < SOCKET_BUFFER_SIZE:
		# 	# Last data chunk
		# 	receive_mode = False
		# 	print("Waiting for audio data")

def main(args=None):
	# signal.signal(signal.SIGTERM, signalHandler)
	# signal.signal(signal.SIGINT, signalHandler)

	print("Secure microphone monitor started")

	server_socket = createServerSocket(HOST, PORT)
	client_socket, client_address = server_socket.accept()
	print("Accepted SSL connection from", client_address[0] + ":" + str(client_address[1]))

	mode_watchdog_thread = threading.Thread(target=modeWatchdog, args=(), daemon=True)
	receive_audio_data_thread = threading.Thread(target=receiveAudioData, args=(client_socket, ), daemon=True)
	save_audio_thread = threading.Thread(target=saveAudio, args=(), daemon=True)

	try:
		mode_watchdog_thread.start()
		receive_audio_data_thread.start()
		save_audio_thread.start()

		while True:
			time.sleep(0.1)
	# except ServiceExit:
	# 	time_to_exit_s = 1
	# 	exit_signal.set()
	# 	exit_timer = threading.Timer(time_to_exit_s, exit, args=(0, ))
	# 	exit_timer.start()
	# 	mode_watchdog_thread.join()
	# 	receive_audio_data_thread.join()
	# 	save_audio_thread.join()
	# 	exit_timer.cancel()
	except KeyboardInterrupt:
		exit_signal.set()
		time.sleep(1)

if __name__ == '__main__':
	main()

# context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
# #context.load_cert_chain('/path/to/certchain.pem', '/path/to/private.key')
# context.load_cert_chain(CERT_FILE, KEY_FILE)

# #backlog = 5
# backlog = 0
# with socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) as sock:
# 	sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# 	sock.bind((SERVER, PORT))
# 	sock.listen(backlog)
# 	print("Waiting for SSL connection on port:", PORT)
# 	with context.wrap_socket(sock, server_side=True) as ssock:
# 		conn, addr = ssock.accept()
# 		print("Accepted SSL connection from", addr[0] + ":" + str(addr[1]))
# 		try:
# 			while True:
# 				message_bytes = conn.recv(SOCKET_BUFFER_SIZE)
# 				# data_size = struct.unpack('!I', client_socket.recv(4))[0]
# 				#data = uint16.from_bytes(message)
# 				#data = int.from_bytes(message, byteorder='little', signed=False)
# 				# data = []
# 				# for i in range(0, len(message), 2):
# 				# 	data.append(int.from_bytes(message[i:i+1]))
# 				if len(message_bytes) == 0:
# 					continue
# 				data = array.array('H', message_bytes)
# 				print("bytes:")
# 				print(message_bytes)
# 				print("uint16:")
# 				print(data)
# 				saveAsWav(data)
# 		except KeyboardInterrupt:
# 			sock.close()

