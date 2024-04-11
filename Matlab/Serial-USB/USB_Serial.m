clear all;
close all;
global fs;

fs = 48e3;
count = 1;
dataread = char(10000,1);
fprintf('found serial port, %s\n',serialportlist);
fprintf('make sure port is set to one of the above in Line 10 of USB_serial.m')
port = 'COM7'; %Please check using matlab "serialportlist"
baudrate = 115200;
%Creatingating Serial object
serial_obj = serialport(port, baudrate);
  %Now open serial port
configureCallback(serial_obj,"terminator",@callback_serialusb);

%figure;

% flush(serial_obj);






