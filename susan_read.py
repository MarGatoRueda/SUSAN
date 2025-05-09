import os
import serial
import numpy as np
from datetime import datetime
import plotext as pltx
import csv
import time

import serial.tools.list_ports

ports = list(serial.tools.list_ports.comports())
print("Ports detected:")
for i, port in enumerate(ports):
    print(f"{i+1}: {port.device}")

choice = input("Please choose port number connected to SUSAN: ")
com = ports[int(choice)-1].device  # Get the chosen device port


calibration_data = None
data = None

print('SUSAN User Terminal v1')
print('Made at MWL\n')

#com = str(input('Choose your Arduino Port: '))
ser = serial.Serial(com, 115200)
time.sleep(2)
ser.flushInput()
ser.flushOutput()

def menu():
    print('1. Start SUSAN routine')
    print('2. Save Data')
    print('3. Exit')
    print('4. PWM Test')
    choice = int(input('Choose an option: '))
    if choice == 1:
        read_data()
    elif choice == 2:
        save_data()
    elif choice == 3:
        exit()
    elif choice == 4:
        pwm_test()
    else:
        print('Invalid Option')
        menu()

def read_data():
    global calibration_data
    global data
    calibration_data = np.array([["time", "415nm", "445nm", "480nm", "515nm", "555nm", "590nm", "630nm", "680nm", "Clear", "NIR"]])
    data = np.array([["time", "415nm", "445nm", "480nm", "515nm", "555nm", "590nm", "630nm", "680nm", "Clear", "NIR"]])
    print('Wake-up command sent. Waiting for data...')
    ser.write('w'.encode('utf-8'))
    reading_calibration = True
    reading_data = False
    
    while True:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8').rstrip('\r\n')

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
                if 'All measurements done.' in line:
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
    global calibration_data
    global data

    if calibration_data is None and data is None:
        print("No data to save. Please run the SUSAN routine first.")
        menu()
        return
    
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
    print('Data saved successfully.')
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

def pwm_test():
    # Test PWM, sends the command to the Arduino to test the PWM. Plot the received data using plot_data, and save it as numpy array, where
    # the first column is the time, the second column is the PWM value and the next columns are the color values.
    global data
    global calibration_data
    calibration_data = np.array([["time", "415nm", "445nm", "480nm", "515nm", "555nm", "590nm", "630nm", "680nm", "Clear", "NIR"]])
    # Leave the calibration data empty, as it is not needed for the PWM test.
    data = np.array([["time", "415nm", "445nm", "480nm", "515nm", "555nm", "590nm", "630nm", "680nm", "Clear", "NIR"]])

    ser.write('p'.encode('utf-8'))
    print('PWM Test command sent. Waiting for data...')
    reading_data = True

    while True:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8').rstrip('\r\n')

            if 'Done' in line:
                print('PWM Test finished, remember to save your data.')
                menu()
                break
            else:
                if ',' in line:
                    rawdata = timeis()
                    spectnum = [int(i) for i in line.split(',')]
                    plot_data(spectnum)
                    rawdata = rawdata + ',' + line
                    data = np.append(data, np.array([rawdata.split(',')]), axis=0)
menu()