% Script for creating input data for real time distribution of vaccines
clear; close all
% From digitizing - day - fraction fully vaccinated population pairs
% Starts with 01/01/2021
data_raw = load('boostersUS.txt');
% Scale to Westchester and turn into fractions
data_raw(:,2) = data_raw(:,2)*45.2/max(data_raw(:,2))/100;
% Time offset - max to consider, #day the simulation start from 01/01/2021
del_t = 473;
% Time, days
time_raw = data_raw(:,1);
% Size of New Rochelle
N_NR = 79205;
% Total number of full dose vaccines
data_total_raw = data_raw(:,2)*N_NR;

% Interpolate and generate the data in 1 day intervals
% Trim so it goes 01/01/2021 until 04/19/2022 - inclusive, 04/19 is the
% start of the simulation and it is included in the offsetting
time = 1:del_t;
data_total = round(pchip(time_raw, data_total_raw, time));

% Plots of totals
plot(time, data_total)
plot(time, data_total/N_NR)

% New full doses in a day
data_day = [data_total(1), data_total(2:end)-data_total(1:end-1)];
figure(1), plot(time, data_day)

% Generate an array of times x #vaccines per day
ind = 1;
for id = 1:length(data_day)
    for jd = 1:data_day(id)
        output(ind) = time(id);
        ind = ind + 1;
    end
end

figure(2), plot(output)

offsets = (output - del_t)';
min(offsets)
max(offsets)

figure(3), plot(offsets)

save('booster_times_NR.txt', 'offsets', '-ascii')
