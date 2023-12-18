import array
import wave
import time
import datetime
import struct

def saveAsBinary(filename, audio_data):
	with open(filename, 'wb') as binary_file:
		binary_file.write(audio_data)

def saveAsAscii(filename, audio_data):
	with open(filename, 'w') as raw_file:
		for sample in audio_data:
			raw_file.write(f"{sample}\n")

def saveAsWav(filename, audio_data):
	rec_frequency = 16000
	with wave.open(filename, 'wb') as wave_file:
		wave_file.setnchannels(1)  # Mono audio
		wave_file.setsampwidth(2)  # 16-bit audio
		# wave_file.setframerate(44100)  # Sample rate
		wave_file.setframerate(rec_frequency)  # Sample rate
		# audio_byte_data = struct.pack('<h', audio_data)
		wave_file.writeframes(audio_data)
		# audio_byte_data = array.array('h', audio_data).tobytes()
		# wave_file.writeframes(audio_byte_data)

def main():
	bin_filename = "audio_stream_test.bin"
	bin_buffer = array.array('B')

	# with open(bin_filename, 'rb') as bin_file:
	# 	chunk = bin_file.read(2)
	# 	while chunk:
	# 		bin_buffer.extend(chunk)
	# 		chunk = bin_file.read(2)

	with open(bin_filename, 'rb') as bin_file:
		bin_buffer.extend(bin_file.read())

	saveAsBinary("output.bin", bin_buffer)

	print("Number of bytes:", len(bin_buffer))

	n_samples = len(bin_buffer) // 2
	rec_buffer = struct.unpack(f"<{n_samples}h", bin_buffer)

	print("len(rec_buffer):", len(rec_buffer))

	# for i in range(10):
	# 	print(rec_buffer[i])
	print(rec_buffer[0:10])

	saveAsAscii("output.dat", rec_buffer)

	saveAsWav("output.wav", bin_buffer)

	# bin_buffer_output = struct.pack(f"<{len(bin_buffer)}B", list(rec_buffer))
	# saveAsWav("output.wav", array.array('B', rec_buffer))
	# saveAsWav("output.wav", rec_buffer)

if __name__ == '__main__':
	main()
