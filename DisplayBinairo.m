
function DisplayBinairo(Original, Solution, file, feasible)

n = size(Original, 1);

% Create a figure
fig = figure('Visible', 'off', 'Color', 'white');
hold on;

% Draw the grid
for k = 0:n
    plot([0 n], [k k], 'k-', 'LineWidth', 1);
    plot([k k], [0 n], 'k-', 'LineWidth', 1);
end

% Display the numbers
for r = 1:n
    for c = 1:n

        x = c - 0.5;
        y = n - r + 0.5;

        if ~isnan(Original(r,c))

            % Original numbers in black
            text(x, y, num2str(Original(r,c)), ...
                'Color', 'black', ...
                'FontWeight', 'bold', ...
                'FontSize', 18, ...
                'HorizontalAlignment', 'center', ...
                'VerticalAlignment', 'middle');

        elseif feasible && ~isnan(Solution(r,c))

            % Solved numbers in blue
            text(x, y, num2str(Solution(r,c)), ...
                'Color', 'blue', ...
                'FontWeight', 'bold', ...
                'FontSize', 18, ...
                'HorizontalAlignment', 'center', ...
                'VerticalAlignment', 'middle');
        end
    end
end

% Display error message if no solution
if ~feasible
    text(n/2, n/2, '== Error ==', ...
        'Color', 'red', ...
        'FontSize', 22, ...
        'FontWeight', 'bold', ...
        'HorizontalAlignment', 'center', ...
        'BackgroundColor', 'white');
end

% Figure formatting
axis equal;
axis([0 n 0 n]);
axis off;

% Filename at top left
text(0, n + 0.25, file, ...
    'FontSize', 10, ...
    'Interpreter', 'none', ...
    'HorizontalAlignment', 'left');

% Date and time at top right
text(n, n + 0.4, datestr(now), ...
    'FontSize', 10, ...
    'HorizontalAlignment', 'right');

% Export figure to PDF
exportgraphics(fig, file, 'ContentType', 'vector');

% Close figure
close(fig);

end
