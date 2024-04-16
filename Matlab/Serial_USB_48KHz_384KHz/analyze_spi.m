function analyze_spi(dataread)
global fs;

dataArray = split(dataread,' ');
%disp(dataArray);

numWords = length(dataArray);
if mod(numWords,2)==1 % off number, make even
dataArray = dataArray(1:end-1);
end
numWords = length(dataArray);


data16 = hex2dec(dataArray) - 32768;
%data16 = 256*data8(1:2:end) + data8(2:2:end);
%data16 = data16.*(data16 < 32768) +  (data16 >= 32778).*(data16 - 65536);
dataFloat = data16./32768; % scale to +/- 1.0 for audio player
dataFloat = flip(dataFloat); % the data was time-reversed for some reason


win = hann(length(dataFloat));
% H = abs(fft(win.*dataFloat)); % use this line for sine wave distortion analysis
H = abs(fft(dataFloat)); % use this line for random audio recordings

H = smooth(H,4);
freq = linspace(0,fs,numWords);
lowbin = find(freq > 20,1);
if fs==48e3
    highbin = find(freq > 20e3,1);
else
    highbin = find(freq > 160e3,1);
end





close all;
p = figure;
p.Position = [100 100 1200 700];
subplot(2,1,1), plot(dataFloat);
title('Data from ADC');
xlabel('samples');
ylabel('ADC output');

subplot(2,1,2),semilogx(freq(lowbin:highbin),20*log10(H(lowbin:highbin)));
title('windowed FFT of data buffer');
xlabel('Freq (hz)');
ylabel('dB ampl');

figure(p); % pop to front of all windows

%timeToPause = 0.5 + length(dataFloat)/fs; % 1/2 second more than the time duration of the audio clip

if fs == 48e3
    player = audioplayer(dataFloat,fs,16);
    playblocking(player);
elseif fs == 384e3
    dataFloatDeci = decimate(dataFloat,8);
    player = audioplayer(dataFloatDeci,48e3,16);
    playblocking(player);
else
    fprintf('ERROR, sample rate must be set to either 48e3 of 384e');
    exit();
end

       
