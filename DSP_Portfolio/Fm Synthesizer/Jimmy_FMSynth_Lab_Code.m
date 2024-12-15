% %% Lab MATLAB Demo
% %% Bells
% clear all
% close all
% % Sample Rate
% Fs = 48000;
% % Sample Period
% T = 1/Fs;
% % time vector
% t = 0:T:3;
% 
% % Fundamental
% f0 = 100;
% % Carrier Frequency
% fc = 5*f0;
% % Modulator Frequency
% fm = 7*f0; % 5*sqrt(2)*f0
% % decay constant
% Tau = 1;
% % Modulator Amplitude Constant
% I0 = 5;
% 
% A = exp(-t/Tau);
% I = I0*A;
% 
% y = A.*sin(2*pi*fc*t + I.*sin(2*pi*fm*t));
% 
% 
% figure
% plot(t,A)
% figure
% plot(t,I)
% 
% freq = 0:Fs/length(y):Fs-Fs/length(y);
% figure
% semilogx(freq,20*log10(abs(fft(y))),'Linewidth',2)
% 
% figure
% spectrogram(y,hamming(1024),512,10*1024,Fs,'yaxis')
% 
% 
% sound(y,Fs)
% 
% pause;

% %% Brass
% clear 
% close all
% 
% % Define ADSR parameters
% attackTime = 0.1; % seconds
% decayTime = 0.1; % seconds
% sustainLevel = 0.75; % sustain level
% % duration
% d = 3;
% % Sample Rate
% Fs = 48000;
% % Sample Period
% T = 1/Fs;
% % time vector
% t = 0:T:(d - T);
% 
% % Fundamental
% f0 = 100;
% % Carrier Frequency
% fc = f0;
% % Modulator Frequency
% fm = f0; 
% 
% A = zeros(1,Fs * d);
% 
% attackSlope = 1/ (Fs * attackTime);
% decaySlope = - (1 - sustainLevel) / (Fs * decayTime);
% 
% for samp = 1:(d * Fs)
%     if samp <= attackTime * Fs
%            A(samp) = attackSlope * samp;
%     elseif samp <= (attackTime + decayTime) * Fs
%            A(samp) = 1 + decaySlope * (samp - attackTime * Fs);
%     else
%            A(samp) = sustainLevel;
%     end
% end
% 
% % Modulator Amplitude Constant
% I = 5 * A;
% 
% y = A.*sin(2*pi*fc*t + I.*sin(2*pi*fm*t));
% 
% 
% figure
% plot(t,A)
% figure
% plot(t,I)
% 
% freq = 0:Fs/length(y):Fs-Fs/length(y);
% figure
% semilogx(freq,20*log10(abs(fft(y))),'Linewidth',2)
% 
% figure
% spectrogram(y,hamming(1024),512,10*1024,Fs,'yaxis')
% 
% 
% sound(y,Fs)



%% Basson
clear all
close all

% Define ADSR parameters
attackTime = 0.1; % seconds
sustainLevel = 1; % sustain level
% duration
d = 3;
% Sample Rate
Fs = 48000;
% Sample Period
T = 1/Fs;
% time vector
t = 0:T:(d - T);

% Fundamental
f0 = 100;
% Carrier Frequency
fc = 5 * f0;
% Modulator Frequency
fm = f0; 

A = zeros(1,Fs * d);

for samp = 1:1: (d * Fs)
    if samp <= attackTime * Fs
           A(samp) = exp(0.683*(samp/Fs)/0.1) - 1;
    else
           A(samp) = sustainLevel;
    end
end

% Modulator Amplitude Constant
I = 1.5 * A;

y = A.*sin(2*pi*fc*t + I.*sin(2*pi*fm*t));


figure
plot(t,A)
figure
plot(t,I)

freq = 0:Fs/length(y):Fs-Fs/length(y);
figure
semilogx(freq,20*log10(abs(fft(y))),'Linewidth',2)

figure
spectrogram(y,hamming(1024),512,10*1024,Fs,'yaxis')


sound(y,Fs)

% %% Clarinet
% clear all
% close all
% 
% % Define ADSR parameters
% attackTime = 0.1; % seconds
% sustainLevel = 1; % sustain level
% % duration
% d = 3;
% % Sample Rate
% Fs = 48000;
% % Sample Period
% T = 1/Fs;
% % time vector
% t = 0:T:(d - T);
% 
% % Fundamental
% f0 = 300;
% % Carrier Frequency
% fc = 3 * f0;
% % Modulator Frequency
% fm = 2 * f0; 
% 
% A = zeros(1,Fs * d);
% 
% for samp = 1:1: (d * Fs)
%     if samp <= attackTime * Fs
%            A(samp) = exp(0.683*(samp/Fs)/0.1) - 1;
%     else
%            A(samp) = sustainLevel;
%     end
% end
% 
% % Modulator Amplitude Constant
% I = 4 - 2 * A;
% 
% y = A.*sin(2*pi*fc*t + I.*sin(2*pi*fm*t));
% 
% 
% figure
% plot(t,A)
% figure
% plot(t,I)
% 
% freq = 0:Fs/length(y):Fs-Fs/length(y);
% figure
% semilogx(freq,20*log10(abs(fft(y))),'Linewidth',2)
% 
% figure
% spectrogram(y,hamming(1024),512,10*1024,Fs,'yaxis')
% 
% 
% sound(y,Fs)
% 
% 
% 
% 
% 
% 
% 
% 
