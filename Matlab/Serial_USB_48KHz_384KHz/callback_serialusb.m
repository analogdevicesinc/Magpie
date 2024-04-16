function callback_serialusb(src,fid,~)
global fs;
dataread = readline(src);
%disp(dataread);

analyze_spi(dataread);
 end