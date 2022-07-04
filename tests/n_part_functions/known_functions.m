% Generates solutions for n-part tests

% Three part function without offset
x = linspace(0,100);
pt = 50 + 5e-1*x(50);
y = [50 + 5e-1*x(1:50), pt*ones(size(x(51:74))), 79.7203 - 0.0672*x(75:end)];
figure(1), plot(x,y)
temp = [x; y]';
save('tpf_results.txt', 'temp', '-ascii')

% Four part function without offset
x = linspace(0,100);
pt1 = 70 + 3e-1*x(30);
pt2 = 0.5550*x(50)+62.5304;
y = [70 + 3e-1*x(1:30), 0.5550*x(31:50)+62.5304, ...
    pt2*ones(size(x(51:81))),-3.1263*x(82:end)+342.6304];
figure(2), plot(x,y)
temp = [x; y]';
save('fpf_results.txt', 'temp', '-ascii')

