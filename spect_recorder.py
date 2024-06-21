# Spectrometer Logger v0.1.0
# By Vicente Aitken at MWL, Universidad De Chile

import os
import serial
import numpy as np
from datetime import datetime
import plotext as pltx
import csv

print('Spectrometer Logger v0.1.0')
print('For Sample Spectrum Analisis\n')

com = str(input('Choose your Arduino Port: '))
ser = serial.Serial(com, 115200)

def menu():
	print('1. Read Data')
	print('2. Save Data')
	print('3. Exit')
	choice = int(input('Choose an option: '))
	if choice == 1:
		read_data()
	elif choice == 2:
		save_data()
	elif choice == 3:
		exit()
	else:
		print('Invalid Option')
		menu()

def read_data():
	global data 
	data = np.array([["time","415nm","445nm","480nm","515nm","555nm","590nm","630nm","680nm","Clear","NIR"]])
	try:
		ser.write('m'.encode('utf'))
		while True:
			if ser.in_waiting:
				rawdata = timeis()
				packet = ser.readline()
				raw = packet.decode('utf').rstrip('\r\n')
				spectnum = [int(i) for i in raw.split(',')]
				plot_data(spectnum)
				rawdata = rawdata + ',' + raw
				data = np.append(data, np.array([rawdata.split(',')]), axis=0)
	except KeyboardInterrupt: # ctrl + c
		ser.write('s'.encode('utf'))
		pltx.clt()
		print('Recorded Data for saving')
		print(data)
		print('\n\nREMEMBER TO SAVE YOUR DATA\n\n')
		menu()


def timeis():
	now = datetime.now()
	dt_string = now.strftime("%Y-%m-%d %H:%M:%S")
	return dt_string

def save_data():
	path = os.path.abspath(os.getcwd()) + '/Spectrogram Data'
	if not os.path.exists(path):
		os.makedirs(path)
	print('Data will be saved at: ' + path)
	filename = str(input('Choose a name for your file: '))
	if len(filename) < 1:
		filename = datetime.now().strftime("%Y%m%dT%H%M%S")+'.csv'
	elif '.csv' not in filename:
		filename = filename + '.csv'
	f = open(path+'/'+filename, 'w')
	writer = csv.writer(f)
	writer.writerows(data)
	f.close()
	menu()

def plot_data(inputdata):
	pltx.clt()
	colors = ["415nm","445nm","480nm","515nm","555nm",
			"590nm","630nm","680nm","Clear","NIR"]
	col = [93,'blue+','cyan+',46,118,220,208,196,255,1]
	ploting = inputdata
	#clear = max(ploting)
	#ploting[:] = [i / clear for i in ploting]
	pltx.simple_bar(colors, ploting, width = 100, title = "Color Spectrum From 415nm to 680nm", color=col)
	pltx.show()
	pltx.sleep(0.01)

menu()