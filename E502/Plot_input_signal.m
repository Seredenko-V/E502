clc, close all, clear;
data = dlmread('input.txt');
data = data';
figure(1);
plot(data);
N = length(data);
fd = 16000;
fn = 1800;
dt = 1 / fd;
w0=2*pi*fn;
delta = sqrt(1 / N * sum(data.^2)) / 10;
err = [];
a=2*cos(w0*dt);
theoretical_sample(1) = data(1);
theoretical_sample(2) = data(2);
for k=3:length(data)
   theoretical_sample(k)=a*data(k-1)-data(k-2);
    if abs(theoretical_sample(k)-data(k))>=delta
        err = [err k];
    end
end
real_err = []; % позиции разрывов фазы
for k=2:length(err) - 1
    if err(k) - err(k-1) > 1
        real_err = [real_err err(k)];
    end
end
disp(real_err); 