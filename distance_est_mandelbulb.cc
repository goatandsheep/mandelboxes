/*
   This file is part of the Mandelbox program developed for the course
    CS/SE  Distributed Computer Systems taught by N. Nedialkov in the
    Winter of 2015-2016 at McMaster University.

    Copyright (C) 2015-2016 T. Gwosdz and N. Nedialkov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "vector3d.h"
#include "mandelbox.h"

extern MandelBoxParams mandelBox_params;
extern double MandelBulbDistanceEstimator(const vec3 &pos, const MandelBoxParams &mPar);

//Distance Estimator Field Selector
double DE(const vec3 &p)
{
  double c1 = fabs(mandelBox_params.scale - 1.0);
  double c2 = pow( fabs(mandelBox_params.scale), 1 - mandelBox_params.num_iter);
  double d = MandelBulbDistanceEstimator(p, mandelBox_params);
  return d;
}
