#!/usr/bin/python
# -*-coding: utf-8 -*-

import serial
import threading
import binascii
from datetime import datetime
import struct


class SerialPort:
    def __init__(self, port, buand):
        self.port = serial.Serial(port, buand)
        self.port.xonxoff = False;
        self.port.rtscts = False;
        self.port.dsrdtr = True;
        #self.port.stopbits = STOPBITS_ONE;
        
        self.port.timeout = 0.1;
        self.port.close()

        if not self.port.isOpen():
            self.port.open()
            print("serial open done")
        self.send_data()
        self.read_data()

    def port_open(self):
        if not self.port.isOpen():
            self.port.open()
            print("serial open done")

    def port_close(self):
        self.port.close()
        print("serial close done")

    def send_data(self):
        self.port.write(0x00)

    def read_data(self):
        global is_exit
        global data_bytes
        # self.data = self.data.decode('utf-8')
        print("serial start read")
        while not is_exit:
            count = self.port.inWaiting()
            print("in waiting")
            self.data = self.port.readline()
            print(self.data)
            self.data.strip()

            self.data = self.data.decode('GB2312','ignore')
            print(self.data)
            if count > 0:
                rec_str = self.port.read(count)
                data_bytes = data_bytes + rec_str
                print('当前数据接收总字节数：'+str(len(data_bytes))+' 本次接收字节数：'+str(len(rec_str)))
                # print(str(datetime.now()),':',binascii.b2a_hex(rec_str))

if __name__ == '__main__':
    serialPort = 'COM14'  
    baudRate = 115200  
    is_exit = False
    data_bytes = bytearray()

    mSerial = SerialPort(serialPort, baudRate)
