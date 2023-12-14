clear all
close all
clear sound

s1=serial('COM14','BaudRate',115200); %���ô��ڲ�����
s1.DataTerminalReady='on';   %matlab���ڴ򿪷�ʽ�����DTR/RTS�����¸�λ������������
s1.RequestToSend='off';
s1.BytesAvailableFcnMode = 'byte';%ָ�����ֽ�������ʱ���ᷢ���ֽڿ����¼�
s1.InputBufferSize = 10000;        %���뻺��������8000�ֽ�
s1.BytesAvailableFcnCount=4;    %����8000���ֽڴ���һ��
display('serial port define done')
%���Դ�������
try
  fopen(s1);                       %�򿪴��ڶ���
  display('serial port open done')
catch err
  fclose(instrfind);               %�رձ�ռ�õĴ���
  error('��ȷ��ѡ������ȷ�Ĵ���');   %���������ʾ
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
 
fclose(s1);  %�رմ���
