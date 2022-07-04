%% Run a number of COVID simulations and store results

clear
close all

num_sim = 100;
num_steps = 720;
dt = 0.25;
time=0:dt:dt*num_steps;

% All the collected data
cur_infected = zeros(num_sim, num_steps+1);
tot_infected = zeros(num_sim, num_steps+1);
tot_deaths = zeros(num_sim, num_steps+1);

for i=1:num_sim
   !./covid_exe >> output/simulation.log
   % Collect 
   cur_infected(i,:) = load('output/infected_with_time.txt');
   tot_infected(i,:) = load('output/total_infected.txt');
   tot_deaths(i,:) = load('output/dead_with_time.txt');
end

save('sim_results.mat')
