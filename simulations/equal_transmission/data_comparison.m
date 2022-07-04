%% Post-processing 
clear
close all

% Data directory and the mat file
mfile = 'sim_results.mat';

% Plot settings
clrf = 1-(1-[7, 125, 5]/255)/4;
clrm = [7, 125, 5]/255;

% Plot all realizations and the mean
load(mfile)

% Data collection starts immediately
cst = 1;

% Active
ylab = 'Prevalence';
plot_title = '$\mathrm{E+S_y}$';
temp = cur_infected(:,cst:end);
plot_all_and_mean(time, temp, 1, clrm, clrf, plot_title, ylab, false)

ylab = 'Prevalence - Strain 1';
plot_title = '$\mathrm{E+S_y}$';
temp = cur_infected_strain_1(:,cst:end);
plot_all_and_mean(time, temp, 2, clrm, clrf, plot_title, ylab, false)

ylab = 'Prevalence - Strain 2';
plot_title = '$\mathrm{E+S_y}$';
temp = cur_infected_strain_2(:,cst:end);
plot_all_and_mean(time, temp, 3, clrm, clrf, plot_title, ylab, false)

% Total
ylab = 'Number of infections';
plot_title = '$\mathrm{E+S_y}$';
temp = tot_infected(:,cst:end);
plot_all_and_mean(time, temp, 4, clrm, clrf, plot_title, ylab, false)

ylab = 'Number of infections - Strain 1';
plot_title = '$\mathrm{E+S_y}$';
temp = tot_infected_strain_1(:,cst:end);
plot_all_and_mean(time, temp, 5, clrm, clrf, plot_title, ylab, false)

ylab = 'Number of infections - Strain 2';
plot_title = '$\mathrm{E+S_y}$';
temp = tot_infected_strain_2(:,cst:end);
plot_all_and_mean(time, temp, 6, clrm, clrf, plot_title, ylab, false)

% Total deaths
ylab = 'Number of deaths';
plot_title = '$\mathrm{R_D}$';
temp = tot_deaths(:,cst:end);
plot_all_and_mean(time, temp, 7, clrm, clrf, plot_title, ylab, false)

function plot_all_and_mean(time, y, i, clrm, clrf, plot_title, ylab, noMarkers)

    % Create figure
    figure1 = figure(i);

    % Create axes
    axes1 = axes('Parent',figure1);

    for i=1:size(y,1)
        plot(time, y(i,:), 'LineWidth', 2, 'Color', clrf)
        hold on
    end
    plot(time, mean(y,1), 'LineWidth', 2, 'Color', clrm)

    % Create ylabel
%     ylabel(ylab,'Interpreter','latex');
    ylabel(ylab);

    % Create xlabel
%     xlabel('Time (days)','Interpreter','latex');
    xlabel('Time, days');

    % Create title
%     title(plot_title,'Interpreter','latex');

    % Ticks
    %xticks([0 60 120 180])
    %xticklabels({'Sept 7','Nov 6','Jan 5','Mar 6'})
    %xtickangle(45)

    % Uncomment the following line to preserve the Y-limits of the axes
    % ylim(axes1,[0 5]);
    box(axes1,'on');
    % Set the remaining axes properties
%     set(axes1,'FontSize',28,'TickLabelInterpreter','latex','XGrid','on','YGrid',...
%         'on'); 
    set(axes1,'FontSize',28,'FontName', 'Arial','XGrid','on','YGrid',...
        'on'); 
    xlim([0,90])
    

    % Size
    set(gcf,'Position',[200 500 950 750])
    
    saveas(gcf,[ylab,'.png'])
    
    % Add a vertical line signifying end of vaccination campaign
    % (hardcoded to this vaccination rate)
    %ymm = ylim;
    %plot([28.5, 28.5], [0, ymm(2)], 'k', 'LineWidth', 1.52)
end

function plot_one(time, y, i, clrm, clrf, plot_title, ylab, noMarkers)

    % Create figure
    figure1 = figure(i);

    % Create axes
    axes1 = axes('Parent',figure1);

    for i=1:size(y,1)
        plot(time, y(i,:), 'LineWidth', 2, 'Color', clrf)
        hold on
    end
    plot(time, mean(y,1), 'v-', 'LineWidth', 2, 'Color', clrm)
 
    % Create ylabel
    ylabel(ylab,'Interpreter','latex');

    % Create xlabel
    xlabel('Time (days)','Interpreter','latex');

    % Create title
    title(plot_title,'Interpreter','latex');

    % Uncomment the following line to preserve the Y-limits of the axes
    % ylim(axes1,[0 5]);
    box(axes1,'on');
    % Set the remaining axes properties
    set(axes1,'FontSize',24,'TickLabelInterpreter','latex','XGrid','on','YGrid',...
        'on'); 
    
    % Ticks
    xticks([1 50 100 140])
    xticklabels({'March 3','April 22','June 11','July 21'})
    
    % Add events
    plot([10,10],ylim, '--', 'LineWidth', 2, 'Color', [188/255, 19/255, 30/255])
    plot([19,19],ylim, '-.', 'LineWidth', 2, 'Color', [188/255, 19/255, 30/255])
    plot([84,84],ylim, '--', 'LineWidth', 2, 'Color', [123/255, 33/255, 157/255])
    plot([98, 98],ylim, '-.', 'LineWidth', 2, 'Color', [123/255, 33/255, 157/255])
    plot([112, 112],ylim, ':', 'LineWidth', 2, 'Color', [123/255, 33/255, 157/255])    
    
end