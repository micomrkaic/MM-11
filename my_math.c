/*
 * This file is part of MM-11.
 *
 * MM-11 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MM-11 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MM-11 If not, see <https://www.gnu.org/licenses/>.
 */
#include <math.h>
#include <stdio.h>
#include "my_math.h"

// **** Numerical functions **** 
double normal_pdf(double x) {
  return exp(-0.5 * x * x) / sqrt(2.0 * M_PI);
}

double normal_cdf(double x) {
  return 0.5 * (1.0 + erf(x / sqrt(2.0)));
}

double inverse_normal_cdf(double p, double tol) {
  if (p <= 0.0 || p >= 1.0) {
    fprintf(stderr, "p must be in (0,1)\n");
    return NAN;
  }

  double lo = -10, hi = 10, mid;

  while ((hi - lo) > tol) {
    mid = 0.5 * (lo + hi);
    double cdf_mid = normal_cdf(mid);
    if (cdf_mid < p) {
      lo = mid;
    } else {
      hi = mid;
    }
  }
  return 0.5 * (lo + hi);
}

