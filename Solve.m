
% Solve.m
% Main script for the Binairo project

clear;
clc;

% Original Binairo grid
B = [
    NaN 0   NaN NaN NaN NaN;
    NaN NaN 0   NaN 0   NaN;
    NaN NaN NaN NaN 1   0;
    1   1   NaN NaN NaN NaN;
    NaN 0   NaN 0   NaN NaN;
    NaN NaN NaN NaN 0   NaN
    ];

% Solve the Binairo
[solution, solved] = SolveBinairo(B);

% Display result
disp('Original grid:');
disp(B);

disp('Solved grid:');
disp(solution);

if solved
    disp('Binairo solved successfully!');
else
    disp('No valid solution found.');
end

% Generate PDF
DisplayBinairo(B, solution, 'Binairo_6x6.pdf', solved);

disp('PDF generated.');
