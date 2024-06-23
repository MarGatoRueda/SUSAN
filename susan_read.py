import os
import serial
import numpy as np
from datetime import datetime
import plotext as pltx
import csv

print('Spectrometer Logger v0.1.0')
print('For Sample Spectrum Analysis\n')

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
    global calibration_data
    global data
    calibration_data = np.array([["time", "415nm", "445nm", "480nm", "515nm", "555nm", "590nm", "630nm", "680nm", "Clear", "NIR"]])
    data = np.array([["time", "415nm", "445nm", "480nm", "515nm", "555nm", "590nm", "630nm", "680nm", "Clear", "NIR"]])
    
    ser.write('w'.encode('utf-8'))
    print('Wake-up command sent. Waiting for data...')

    reading_calibration = True
    reading_data = False
    
    while True:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8')
            print(f"Received line: {line}")  # Debug print

            if reading_calibration:
                if 'Refractive LED measurements done.' in line:
                    reading_calibration = False
                    reading_data = True
                    print('Calibration done. Moving to data readings...')
                else:
                    if ',' in line:
                        rawdata = timeis()
                        spectnum = [int(i) for i in line.split(',')]
                        plot_data(spectnum)
                        rawdata = rawdata + ',' + line
                        calibration_data = np.append(calibration_data, np.array([rawdata.split(',')]), axis=0)
            elif reading_data:
                if 'Done.' in line:
                    print('Routine finished, remember to save your data.')
                    menu()
                    break
                else:
                    if ',' in line:
                        rawdata = timeis()
                        spectnum = [int(i) for i in line.split(',')]
                        plot_data(spectnum)
                        rawdata = rawdata + ',' + line
                        data = np.append(data, np.array([rawdata.split(',')]), axis=0)

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
    f = open(path+'/'+filename, 'w', newline='')
    writer = csv.writer(f)
    writer.writerows(calibration_data)
    writer.writerows(data)
    f.close()
    menu()

def plot_data(inputdata):
    pltx.clt()
    colors = ["415nm", "445nm", "480nm", "515nm", "555nm",
              "590nm", "630nm", "680nm", "Clear", "NIR"]
    col = [93, 'blue+', 'cyan+', 46, 118, 220, 208, 196, 255, 1]
    ploting = inputdata
    pltx.simple_bar(colors, ploting, width=100, title="Color Spectrum From 415nm to 680nm", color=col)
    pltx.show()
    pltx.sleep(0.01)

menu()
