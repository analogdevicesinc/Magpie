clear all;
close all;
fs = 48e3;
filename = 'SPI-log34.txt';

FID = fopen(filename);
dataFromfile = textscan(FID, '%s');
dataFromfile = string(dataFromfile{1});
dataFromfile = split(dataFromfile,' ');
numWords = length(dataFromfile);
if mod(numWords,2)==1 % off number, make even
    dataFromfile = dataFromfile(1:end-1);
end
numWords = length(dataFromfile);


data = hex2dec(dataFromfile);
data = data - 32768; % make center around 0
data = data./32768; % scale to +/- 1.0 for audio player
data = flip(data); % the data was time-reversed for some reason

% test if data is in 2's comp form; try converting to signed int
data_2scomp = data.*(data < 32768) +  (data >= 32778).*(data - 65536);
data_2scomp = data_2scomp./32768;% scale to +/- 1.0 for audio player



figure;
plot(data);
title('Data from ADC');
xlabel('samples');
ylabel('ADC output');


% win = hann(length(data));
H = abs(fft(data_2scomp));
H = smooth(H,20);
freq = linspace(0,fs,numWords);
lowbin = find(freq > 20,1);
highbin = find(freq > 20e3,1);
figure;
semilogx(freq(lowbin:highbin),20*log10(abs(H(lowbin:highbin))));
title('un-windowed FFT of data buffer');
xlabel('Freq (hz)');
ylabel('dB ampl');



% figure;
% plot(data_2scomp);
% title('spi data assuming input is 2s comp');



player = audioplayer(data,fs,16);
play(player)

