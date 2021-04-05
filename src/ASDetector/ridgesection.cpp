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

#include "ridgesection.h"


RidgeSection::RidgeSection ()
{
}


RidgeSection::~RidgeSection ()
{
  std::vector<Bump *>::iterator it = bumps.begin ();
  while (it != bumps.end ())
  {
    if (*it != NULL) delete *it;
    it ++;
  }
}


void RidgeSection::clearDetectionData ()
{
  points.clear ();
}


void RidgeSection::clearDisplayData ()
{
  discans.clear ();
}


void RidgeSection::add (Bump *bump, const std::vector<Pt2i> &dispix)
{
  bumps.push_back (bump);
  discans.push_back (dispix);
}


void RidgeSection::add (Bump *bump, const std::vector<Pt2i> &dispix,
                                    const std::vector<Pt2f> &pts)
{
  bumps.push_back (bump);
  discans.push_back (dispix);
  points.push_back (pts);
}


Bump *RidgeSection::bump (int num) const
{
  if ((int) (bumps.size ()) <= num) return NULL;
  return bumps[num];
}


float RidgeSection::getHeightReference (int num) const
{
  return (bumps.empty () || bumps[num]->getStatus () != Bump::RES_OK ?
             0.0f : bumps[num]->estimatedCenter().y ());
}


bool RidgeSection::getScanBounds (int ind, Pt2i &p1, Pt2i &p2)
{
  if (discans.empty ()) return false;
  if (ind < 0 || ind >= (int) (discans.size ())) return false;
  p1.set (discans[ind].front ());
  p2.set (discans[ind].back ());
  return true;
}
