/* 
# Copyright (C) 2015-2018 Johannes Rueckert
#
# This file is part of darch.
#
# darch is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# darch is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with darch. If not, see <http://www.gnu.org/licenses/>.
*/

#include <Rcpp.h>
#include <RcppParallel.h>
#include "helpers.h"

using namespace RcppParallel;
using namespace Rcpp;

struct Dither : public Worker
{
  RMatrix<double> data;
  
  const RVector<double> columnMask;
  
  Dither(NumericMatrix data, const NumericVector columnMask) :
  data(data), columnMask(columnMask)
  {}
  
  void operator()(std::size_t begin_col, std::size_t end_col)
  {
    int nrow = data.nrow();
    double sdColumn, variance;
    
    for (int i = begin_col; i < end_col; i++)
    {
      if (columnMask[i] == 1)
      {
        sdColumn = cppSD(data.column(i));
        variance = sdColumn * sdColumn;
        
        for (int j = 0; j < nrow; j++)
        {
          data(j, i) += R::runif(-variance, variance);
        }
      }
    }
  }
};

// edits in-place
// [[Rcpp::export]]
NumericMatrix ditherCpp(NumericMatrix data, NumericVector columnMask)
{
  NumericMatrix output = clone(data);
  int ncols = data.ncol();
  Dither worker(output, columnMask);
  
  parallelFor(0, ncols, worker);
  
  return output;
}
