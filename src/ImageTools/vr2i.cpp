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

#include "vr2i.h"


Vr2i::Vr2i ()
{
  xv = 1;
  yv = 0;
}


Vr2i::Vr2i (int x, int y)
{
  xv = x;
  yv = y;
}


Vr2i::Vr2i (const Vr2i &v)
{
  xv = v.xv;
  yv = v.yv;
}


Vr2i Vr2i::orthog () const
{
  return (Vr2i (-yv, xv));
}


bool Vr2i::orientedAs (const Vr2i &ref) const
{
  int ps = xv * ref.xv + yv * ref.yv;
  return (4 * ps * ps > 3 * (xv * xv + yv * yv)
                          * (ref.xv * ref.xv + ref.yv * ref.yv));
}


bool *Vr2i::steps (int *n) const
{
  int x2 = (xv > 0 ? xv : - xv);
  int y2 = (yv > 0 ? yv : - yv);
  int dx = x2, dy = y2;
  if (y2 > x2)
  {
    dx = y2;
    dy = x2;
    x2 = y2;
  }
  int e, x = 0, i = 0;
  *n = x2;
  bool *vecsteps = new bool[x2];

  e = dx;
  dx *= 2;
  dy *= 2;

  while (x < x2)
  {
    x ++;
    e -= dy;
    if (e < 0)
    {
      e += dx;
      vecsteps[i++] = true;
    }
    else vecsteps[i++] = false;
  }
  return (vecsteps); 
}
