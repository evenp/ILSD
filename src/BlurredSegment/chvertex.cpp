/*  Copyright 2021 Philippe Even and Phuc Ngo,
      co-authors of paper:
      Even, P., Grzesznik, A., Gebhardt, A., Chenal, T., Even, P. and Ngo, P.,
      2021,
      Fast extraction of linear structures fromLiDAR raw data
      for archaeomorphological structure prospection.
      In the International Archives of the Photogrammetry, Remote Sensing
      and Spatial Information Sciences (proceedings of the 2021 edition
      of the XXIVth ISPRS Congress).

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <cstddef>
#include "chvertex.h"


CHVertex::CHVertex () : Pt2i ()
{
  lv = NULL;
  rv = NULL;
}


CHVertex::CHVertex (int x, int y) : Pt2i (x, y)
{
  lv = NULL;
  rv = NULL;
}


CHVertex::CHVertex (const Pt2i &p) : Pt2i (p)
{
  lv = NULL;
  rv = NULL;
}


CHVertex::~CHVertex ()
{
}


/*
ostream& operator<< (ostream &os, const CHVertex &v)
{
  os << "(" << v.xp << ", " << v.yp << ")";
  return os;
}
*/
