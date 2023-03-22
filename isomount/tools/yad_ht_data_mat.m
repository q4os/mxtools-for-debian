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
##
##----------------------------------------------------------------------------
## Note: you can easily modify this script by altering the "retval =" line
## in mat_row() and by changing the help and perhaps the "rows" name in the
## input data.
##----------------------------------------------------------------------------

## -*- texinfo -*-
## @deftypefn {} matrix = yad_ht_data_mat ( [dpi_1, rows_1; dpi_2, rows_2; ...] )
##
## Input is a matrix with rows consisting of [dpi, rows] for the test conditions.
##
## Produces a matrix, to be inverted with columns:
##  YAD_HT_DATA_OFF  YAD_HT_DATA_PAD  YAD_HT_DATA_PPC   YAD_HT_DATA_PPR
## for each data point provided.
##
## If the input has 3 or more columns them we treat these extra column as
## measuremnts of the desired height of the table in pixels give the number
## of rows and the dpi.
##
##  @seealso{yad_any_mat}
## @end deftypefn

## Author:  BitJam
## Created: 2018-04-03

function [retval] = yad_ht_data_mat (dpi_rows)
    retval = yad_any_mat (2, @mat_row , dpi_rows);
endfunction

function [retval] = mat_row (dpi_rows)
    dpi    = dpi_rows(:,1);
    rows   = dpi_rows(:,2);
    retval = [1, dpi / 96, dpi * rows / 96, rows / 1];
    retval = [1, dpi, dpi * rows, rows ];
endfunction
