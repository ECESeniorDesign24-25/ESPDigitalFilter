fs = 4500;       
f1 = 425;             
f2 = 435;             
Wp = [f1, f2] / (fs / 2); 
n = 2;                 
ripple = 1;           

[num, den] = cheby1(n, ripple, Wp, 'bandpass');

disp('num:');
disp(num);
disp('den:');
disp(den);
