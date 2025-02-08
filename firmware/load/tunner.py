import time
import serial
import threading
import numpy as np
from enum import Enum


DATA_FOLDER = "data"
reading_data = False


def read_serial(serial_port):

	# Read a line from the serial port
	line = serial_port.readline().decode('utf-8').strip()

	try:
		# Parse the line
		parts = line.split(',')
		setpoint = float(parts[0].split(':')[1])
		adc = float(parts[1].split(':')[1])
		control = float(parts[2].split(':')[1])

		# Print the parsed values
		return {
			"setpoint": setpoint,
			"adc": adc,
			"control": control
		}
	except:
		return None
	
def read_serial_to_file(serial_port, file):
	global reading_data
	file.write("time,setpoint,adc,control\n")
	initial_time = time.time()
	while reading_data:
		data = read_serial(serial_port)

		if data is not None:
			# Get the time
			current_time = time.time() - initial_time
			file.write(f"{current_time},{data['setpoint']},{data['adc']},{data['control']}\n")


ku = 0.33
tu = 1 / 113.0

teoretical_kp = 0.6 * ku
teoretical_ki = 2 * ku / tu
teoretical_kd = ku * tu / 8

kps = np.linspace(0.01, 0.1, 5)
kis = np.linspace(0.0, 0.80, 2)
kds = np.linspace(0.0, 0.5, 2)

# # add teoretical values to the list
# kps = np.append(kps, teoretical_kp)
# kis = np.append(kis, teoretical_ki)
# kds = np.append(kds, teoretical_kd)

# Sort the values
kps = np.sort(kps)
kis = np.sort(kis)
kds = np.sort(kds)

# osciloscope = Ds1054z('192.168.1.31')
# print(osciloscope.idn)

# configure_oscilloscope(osciloscope)

# print(read_oscilloscope(osciloscope, SlopeEdge.RISING))

total_steps = len(kps) * len(kis) * len(kds)
current_step = 0
start_time = time.time()

for kp in kps:
	for ki in kis:
		for kd in kds:
			current_step += 1
			elapsed_time = time.time() - start_time
			avg_time_per_step = elapsed_time / current_step
			remaining_steps = total_steps - current_step
			estimated_time_remaining = avg_time_per_step * remaining_steps
			eta = time.strftime("%H:%M:%S", time.localtime(time.time() + estimated_time_remaining))
			print(f"Step {current_step}/{total_steps} | kp={kp:.5f}, ki={ki:.6f}, kd={kd:.5f}")
			print(f"Elapsed Time: {elapsed_time:.2f}s | Estimated Time Remaining: {estimated_time_remaining:.2f}s | ETA: {eta}")
			# Open file for writing
			with open(f"{DATA_FOLDER}/kp{round(kp,5)}-ki{round(ki,6)}-kd{round(kd,6)}.csv", "w") as f:
				with serial.Serial('/dev/ttyACM0', 115200) as serial_port:
					# Set the setpoint to 0
					print("Setting setpoint to 0")
					serial_port.write("WRTE:CTRL:CP__:0000.0000\n".encode('utf-8'))
					time.sleep(0.1)
					serial_port.write("WRTE:CTRL:CP__:0000.0000\n".encode('utf-8'))
					time.sleep(0.1)
					serial_port.write("WRTE:CTRL:CP__:0000.0000\n".encode('utf-8'))
					# Set the PID parameters
					serial_port.write(f"WRTE:CTRL:KP__:{kp:09.4f}\n".encode('utf-8'))
					time.sleep(0.05)
					serial_port.write(f"WRTE:CTRL:KI__:{ki:09.4f}\n".encode('utf-8'))
					time.sleep(0.05)
					serial_port.write(f"WRTE:CTRL:KD__:{kd:09.4f}\n".encode('utf-8'))
					time.sleep(0.05)
					serial_port.write(f"WRTE:CTRL:KP__:{kp:09.4f}\n".encode('utf-8'))
					time.sleep(0.05)
					serial_port.write(f"WRTE:CTRL:KI__:{ki:09.4f}\n".encode('utf-8'))
					time.sleep(0.05)
					serial_port.write(f"WRTE:CTRL:KD__:{kd:09.4f}\n".encode('utf-8'))
					time.sleep(0.05)
					# Start the process
					reading_data = True
					# Start the thread
					thread = threading.Thread(target=read_serial_to_file, args=(serial_port, f))
					thread.start()
					# Wait for 2 seconds
					time.sleep(0.2)
					# Change the setpoint
					print("Setting setpoint to 1.5")
					serial_port.write("WRTE:CTRL:CP__:0010.0000\n".encode('utf-8'))
					time.sleep(0.1)
					serial_port.write("WRTE:CTRL:CP__:0010.0000\n".encode('utf-8'))
					time.sleep(0.1)
					serial_port.write("WRTE:CTRL:CP__:0010.0000\n".encode('utf-8'))
					time.sleep(0.1)
					# Wait for 2 seconds
					time.sleep(1.0)
					# Change the setpoint
					serial_port.write("WRTE:CTRL:CP__:0000.0000\n".encode('utf-8'))
					time.sleep(0.1)
					serial_port.write("WRTE:CTRL:CP__:0000.0000\n".encode('utf-8'))
					time.sleep(0.1)
					serial_port.write("WRTE:CTRL:CP__:0000.0000\n".encode('utf-8'))
					time.sleep(0.1)
					serial_port.write("WRTE:CTRL:CP__:0000.0000\n".encode('utf-8'))
					time.sleep(0.1)
					time.sleep(0.3)
					# Wait for 2 seconds
					print("Stopping thread")
					# Stop the process
					reading_data = False
					# Wait for 0.1 seconds
					time.sleep(0.1)

