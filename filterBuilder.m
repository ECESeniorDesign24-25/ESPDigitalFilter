fs = 4500; % Sampling frequency (Hz)
f0 = 430; % Target center frequency (Hz)
bandwidth = 20;
filter_order = 2; 

Wn = [(f0 - bandwidth/2) (f0 + bandwidth/2)] / (fs/2);

[x, y] = butter(filter_order, Wn, 'bandpass');

disp('denom coeffs.:');
disp(x);
disp('num coeffs.:');
disp(y);
