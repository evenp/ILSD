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

#include "ctracksection.h"


CTrackSection::CTrackSection ()
{
  last = -1;
  holes = 0;
}


CTrackSection::~CTrackSection ()
{
  std::vector<Plateau *>::iterator it = plateaux.begin ();
  while (it != plateaux.end ())
  {
    if (*it != NULL) delete *it;
    it ++;
  }
}


void CTrackSection::clearDetectionData ()
{
  points.clear ();
}


void CTrackSection::clearDisplayData () {
  discans.clear ();
}


void CTrackSection::add (Plateau *pl, const std::vector<Pt2i> &dispix)
{
  plateaux.push_back (pl);
  discans.push_back (dispix);
}


void CTrackSection::add (Plateau *pl, const std::vector<Pt2i> &dispix,
                                      const std::vector<Pt2f> &pts)
{
  plateaux.push_back (pl);
  discans.push_back (dispix);
  points.push_back (pts);
}


bool CTrackSection::getScanBounds (int ind, Pt2i &p1, Pt2i &p2)
{
  if (discans.empty ()) return false;
  if (ind < 0 || ind >= (int) (discans.size ())) return false;
  p1.set (discans[ind].front ());
  p2.set (discans[ind].back ());
  return true;
}


void CTrackSection::accept (int num)
{
  if (! plateaux[num]->isAccepted ())
  {
    plateaux[num]->accept ();
    if (last < num)
    {
      holes += num - last - 1;
      last = num;
    }
    else holes --;
  }
}


Plateau *CTrackSection::plateau (int num) const
{
  if ((int) (plateaux.size ()) <= num) return NULL;
  return plateaux[num];
}


float CTrackSection::getHeightReference (int num) const
{
  return (plateaux.empty ()
          || plateaux[num]->getStatus () != Plateau::PLATEAU_RES_OK ?
          0.0f : plateaux[num]->getMinHeight ());
}


int CTrackSection::getAcceptedCount () const
{
  int count = 0;
  std::vector<Plateau *>::const_iterator pl = plateaux.begin ();
  while (pl != plateaux.end ())
    if ((*pl++)->isAccepted ()) count ++;
  return count;
}


bool CTrackSection::pruneDoubtfulTail (int tailMinSize)
{
  if (plateaux.empty ()) return true;
  std::vector<Plateau *>::iterator pl = plateaux.end ();
  int num = (int) (plateaux.size ());
  bool tail_found = false;
  int nb = 0;
  do
  {
    pl --;
    num --;
    if ((*pl)->isAccepted ())
    {
      if (nb == 0 && last > num) last = num;
      tail_found = true;
      if (++nb == tailMinSize) return false;
    }
    else if (tail_found)
    {
      if (nb != 0)
      {
        std::vector<Plateau *>::iterator pl2 = pl;
        while (nb != 0)
        {
          pl2 ++;
          (*pl2)->prune ();
          nb --;
        }
      }
      holes --;
    }
  }
  while (pl != plateaux.begin ());
  if (nb == 0) last = -1;
  return true;
}


float CTrackSection::shiftLength (int &cumlength) const
{
  float shift = 0.0f;
  bool onTail = true;
  int nbr = 0;
  float oldpos = 0.0f;
  if (plateaux.empty ()) return (0.0f);
  std::vector<Plateau *>::const_iterator pl = plateaux.end ();
  do
  {
    pl --;
    if (onTail)
    {
      if ((*pl)->isAccepted ())
      {
        onTail = false;
        oldpos = (*pl)->estimatedCenter ();
      }
    }
    else
    {
      cumlength ++;
      if ((*pl)->isAccepted ())
      {
        nbr = 0;
        float pos = (*pl)->estimatedCenter ();
        shift += (oldpos < pos ? pos - oldpos : oldpos - pos);
        oldpos = pos;
      }
      else
      {
        nbr ++;
      }
    }
  }
  while (pl != plateaux.begin ());
  cumlength -= nbr;
  return (shift);
}
