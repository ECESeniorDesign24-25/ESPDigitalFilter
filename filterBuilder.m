fs = 6000;          % Sampling frequency in Hz
f1 = 430;           % Lower cutoff frequency in Hz
f2 = 440;           % Upper cutoff frequency in Hz
Wp = [f1, f2] / (fs / 2); % normalize
n = 2;
[den, num] = butter(n, Wp, 'bandpass');

disp('num:');
disp(num);
disp('den:');
disp(den);