fs = 4500; % Sampling frequency (Hz)
f0 = 430; % Target center frequency (Hz)
bandwidth = 50;
filter_order = 4; 

Wn = [(f0 - bandwidth/2) (f0 + bandwidth/2)] / (fs/2);

[x, y] = butter(filter_order, Wn, 'bandpass');

disp('Numerator coeffs.:');
disp(x);
disp('Denominator coeffs.:');
disp(y);
