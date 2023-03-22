## Copyright (C) 2018 
## 
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

## -*- texinfo -*- 
## @deftypefn {} {@var{retval} =} yad_any_mat (data_cols, mat_row_func, [dpi_1, rows_1; dpi_2, rows_2; ...] )
## 
## The first parameter is the number of data columns (typically 2 or 1) in the input
## data (third parameter).
##
## The second parameter is a function handle that will return one row of the matrix
## for each row in the second parameter using 1:data_cols as in index.
## If there are no extra columns then we return a maxtrix, ripe for inversion.
## If there is at least one more column then instead we treat that as a vector
## measurements and apply the inverted matrix to it.
##
## @end deftypefn

## Author:  <jbowlin@chi>
## Created: 2018-04-03

function [retval] = yad_any_mat (data_cols, func, data)

    for r = 1:rows(data)
       mat(r, :) = feval (func, data(r, 1:data_cols));
    endfor

# If there are extra columns then apply the inverted matrix to each column
# and return a matrix of the results

    cols = columns(data);
    r=[];
    if ( cols > data_cols)
        for col = data_cols+1:cols
            r = [r; data(:, col) \ mat];
        endfor
        retval = r;

# otherwise just return the matrix
    else
        retval = mat;
    endif
endfunction
