/*  Copyright 2021 Philippe Even, Phuc Ngo and Pierre Even,
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

#include "pt3i.h"


Pt3i::Pt3i ()
{
  xp = 0;
  yp = 0;
  zp = 0;
}


Pt3i::Pt3i (int x, int y, int z)
{
  xp = x;
  yp = y;
  zp = z;
}


Pt3i::Pt3i (const Pt3i &p)
{
  xp = p.xp;
  yp = p.yp;
  zp = p.zp;
}


bool Pt3i::greaterThan (const Pt3i &p) const
{
  if (xp > p.xp) return true;
  if (xp < p.xp) return false;
  if (yp > p.yp) return true;
  if (yp < p.yp) return false;
  return (zp > p.zp);
}
