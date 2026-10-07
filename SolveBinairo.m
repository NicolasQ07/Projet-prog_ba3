% SolveBinaro  solve a Binairo/Binoxxo/BinarySudoku/Takuzu/etc.
%
%   [SOLUTION, SOLVED] = SolveBinaro(PUZZLE) solve a Binairo PUZZLE reccursively
%    by 1) checking all constrains and when not sufficient
%       2) make guess [0,1] in the first empty cell and
%           continue while valid until solution is found 
%           or backtrack to previous state
%
%   Rules
%     1. each cell is '0' or '1'.
%     2. never 3x '0' nor 3x '1' consecutive (rows or col)
%     3. each row/col contains the same numner of '0', '1'
%     4. each row/col are unique

%   Input
%     PUZZLE  n x n Grid of floats (n MUST be odd), NaN defines empty case
%            
%   Output
%     SOLUTION  grid solved or last partialy solved grid
%     SOLVED    true is a full and valid solution is found
%
%   v. 8.9.2026/ca
%   
%   Renommer ce fichier!
%

%% ------------------------------------------------------------------------------------------------------
function [grid, valid] = SolveBinairo(grid)

n = size(grid, 1);

% First solve everything possible without guessing
[grid, valid] = DirectValues(grid, n);

% An error was found
if ~valid
    return;
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
     % print the current grid for debugging
end