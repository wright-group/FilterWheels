import serial
import time

port = serial.Serial("COM12", baudrate=57600)
#port.write(b"U 1\n")
#time.sleep(5.5)
port.write(b"M 0 -10\n")
time.sleep(1.5)
#port.write(b"H\n")
#port.close()


#end