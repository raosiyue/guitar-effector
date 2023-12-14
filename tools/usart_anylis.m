clear all
close all
clear sound

s1=serial('COM14','BaudRate',115200); %设置串口波特率
s1.DataTerminalReady='on';   %matlab串口打开方式会控制DTR/RTS，导致复位，加上这两句
s1.RequestToSend='off';
s1.BytesAvailableFcnMode = 'byte';%指定的字节数可用时，会发生字节可用事件
s1.InputBufferSize = 10000;        %输入缓冲区长度8000字节
s1.BytesAvailableFcnCount=4;    %接收8000个字节触发一次
display('serial port define done')
%尝试代开串口
try
  fopen(s1);                       %打开串口对象
  display('serial port open done')
catch err
  fclose(instrfind);               %关闭被占用的串口
  error('请确认选择了正确的串口');   %输出错误提示
end
figure();
cnt = 0;
t = [0:(1/8192):1000];
out = [];
out = [out, [0.1 * sin(82 * 2*3.14*t(1:81920)), 0.1 * sin(110 * 2*3.14*t(1:81920)), 0.1 * sin(146 * 2*3.14*t(1:81920)), 0.1 * sin(195 * 2*3.14*t(1:81920)), 0.1 * sin(246 * 2*3.14*t(1:81920)), 0.1 * sin(330 * 2*3.14*t(1:81920))]];
sound(out);
while 1
    data = fread(s1,1,'uint32');
    %display('byte get')
    if data == 305419896
        display('junk get ')
        cnt
        cnt = 0;
        datatype = fread(s1,1,'uint32');
        datalen = fread(s1,1,'uint32');
        display('datalen get')
        this_frame = [];
        for i = 1 : datalen / 4
            this_frame = [this_frame, fread(s1, 1, 'float32')];
        end
        display('whole get')
        hold off;
        plot(this_frame);
        hold on;
        display(tuner(this_frame));
        pause(0);
        
    else
        cnt = cnt + 1;

    end
end
 
fclose(s1);  %关闭串口
