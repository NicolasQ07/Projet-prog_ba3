% SolveBinairo  solve a Binairo/Binoxxo/BinarySudoku/Takuzu/etc.
%
%   [SOLUTION, SOLVED] = SolveBinairo(PUZZLE) solves a Binairo PUZZLE recursively
%    by 1) checking all constraints and, when not sufficient,
%       2) making a guess [0,1] in the first empty cell and
%           continuing while valid until a solution is found
%           or backtracking to the previous state
%
%   Rules
%     1. each cell is '0' or '1'.
%     2. never 3x '0' nor 3x '1' consecutive (rows or col)
%     3. each row/col contains the same number of '0', '1'
%     4. each row/col are unique
%
%   Input
%     PUZZLE  n x n grid of floats (n MUST be even), NaN defines an empty cell
%
%   Output
%     SOLUTION  grid solved or last partially solved grid
%     SOLVED    true if a full and valid solution is found, false if the grid
%               has no solution or is not a Binairo (not square, odd size,
%               value other than 0, 1 or NaN)
%
%   Mya Bennani, v. 24.11.2026
%     07.10.2026, MB, solver and checks
%     23.11.2026, NQ, every row and column checked, also the full ones
%     24.11.2026, NQ, check of the input grid, printGrid, header
%

%% ------------------------------------------------------------------------------------------------------
function [grid, valid] = SolveBinairo(grid)

n = size(grid, 1);

% A Binairo must be square, of even size, and only contain 0, 1 or NaN
if n == 0 || size(grid, 2) ~= n || mod(n, 2) ~= 0 || ...
        any(~isnan(grid(:)) & grid(:) ~= 0 & grid(:) ~= 1)
    valid = false;
    return;
end

% First solve everything possible without guessing
[grid, valid] = DirectValues(grid, n);

% An error was found
if ~valid
    return;
end

% Check every row and column, also the ones that were already full
for k = 1:n
    if ~CheckVectorOk(grid(k,:), n) || ~CheckVectorOk(grid(:,k), n) || ~CheckVectorUniqueOk(grid, k, k, n)
        valid = false;
        return;
    end
end

% No empty cells -> solved
if ~any(isnan(grid(:)))
    valid = true;
    return;
end

% Find the first empty cell
index = find(isnan(grid), 1);
[r,c] = ind2sub(size(grid), index);

% ----- Guess 0 -----
if CheckValidMove(grid, r, c, 0, n)

    newGrid = grid;
    newGrid(r,c) = 0;

    [newGrid, solved] = SolveBinairo(newGrid);

    if solved
        grid = newGrid;
        valid = true;
        return;
    end
end

% ----- Guess 1 -----
if CheckValidMove(grid, r, c, 1, n)

    newGrid = grid;
    newGrid(r,c) = 1;

    [newGrid, solved] = SolveBinairo(newGrid);

    if solved
        grid = newGrid;
        valid = true;
        return;
    end
end

% Neither guess gives a solution
valid = false;

end

%% ------------------------------------------------------------------------------------------------------
function [grid, ok] = DirectValues(grid, n)

ok = true;
changed = true;

% Continue while the grid is changing
while changed

    changed = false;

    % Go through all cells
    for r = 1:n
        for c = 1:n

            % Only look at empty cells
            if isnan(grid(r,c))

                % Try 0 and 1
                zeroOk = CheckValidMove(grid, r, c, 0, n);
                oneOk  = CheckValidMove(grid, r, c, 1, n);

                % Neither 0 nor 1 is possible -> error
                if ~zeroOk && ~oneOk
                    ok = false;
                    return;

                    % Only 0 is possible
                elseif zeroOk && ~oneOk
                    grid(r,c) = 0;
                    changed = true;

                    % Only 1 is possible
                elseif ~zeroOk && oneOk
                    grid(r,c) = 1;
                    changed = true;

                end
            end
        end
    end
end

end


%% ------------------------------------------------------------------------------------------------------

function ok = CheckValidMove(grid, r, c, v, n)

    % Put the value v in the chosen cell
    grid(r,c) = v;

    % Check the row
    rowOk = CheckVectorOk(grid(r,:), n);

    % Check the column
    colOk = CheckVectorOk(grid(:,c), n);

    % Check that full rows and columns are unique
    uniqueOk = CheckVectorUniqueOk(grid, r, c, n);

    % The move is valid only if all conditions are valid
    ok = rowOk && colOk && uniqueOk;

end


%% ------------------------------------------------------------------------------------------------------
function ok = CheckVectorOk(vector, n)
ok = true;

    % Too many 0 or 1
    if sum(vector == 0) > n/2 || sum(vector == 1) > n/2
        ok = false;
        return;
    end

    % Three identical consecutive values
    if any((vector(1:end-2) == vector(2:end-1)) & ...
           (vector(2:end-1) == vector(3:end)))
        ok = false;
        return;
    end
end


%% ------------------------------------------------------------------------------------------------------
function ok = CheckVectorUniqueOk(grid, r, c, n)

    ok = true;

    % Check current row against all other rows
    if ~any(isnan(grid(r,:)))
        for i = 1:n
            if i ~= r && ~any(isnan(grid(i,:)))
                if all(grid(r,:) == grid(i,:))
                    ok = false;
                    return;
                end
            end
        end
    end

    % Check current column against all other columns
    if ~any(isnan(grid(:,c)))
        for i = 1:n
            if i ~= c && ~any(isnan(grid(:,i)))
                if all(grid(:,c) == grid(:,i))
                    ok = false;
                    return;
                end
            end
        end
    end

end

%% ------------------------------------------------------------------------------------------------------
function printGrid(grid)
     % print the current grid for debugging, '.' for an empty cell
     % (add printGrid(grid); anywhere in this file to follow the solver)
     for r = 1:size(grid, 1)
         for c = 1:size(grid, 2)
             if isnan(grid(r,c))
                 fprintf(' .');
             else
                 fprintf(' %d', grid(r,c));
             end
         end
         fprintf('\n');
     end
     fprintf('\n');
end