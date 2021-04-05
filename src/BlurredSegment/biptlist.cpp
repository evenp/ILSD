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

#include "biptlist.h"


BiPtList::BiPtList (Pt2i pt)
{
  pts.push_back (pt);
  start = 0;
  cpt = 1;
}


BiPtList::~BiPtList ()
{
}


void BiPtList::addFront (Pt2i pt)
{
  pts.push_front (pt);
  start++;
  cpt++;
}


void BiPtList::addBack (Pt2i pt)
{
  pts.push_back (pt);
  cpt++;
}


void BiPtList::removeFront (int n)
{
  if (n >= frontSize ()) n = frontSize () - 1; // We keep at least one point
  for (int i = 0; i < n; i++) pts.pop_front ();
  cpt -= n;
  start -= n;
  if (start < 0) start = 0; // Theoretically impossible
}


void BiPtList::removeBack (int n)
{
  if (n >= backSize ()) n = backSize () - 1;  // We keep at least one point
  for (int i = 0; i < n; i++) pts.pop_back ();
  cpt -= n;
  if (start >= cpt) start = cpt - 1;  // Theoretically impossible
}


void BiPtList::findExtrema (int &xmin, int &ymin, int &xmax, int &ymax) const
{
  std::deque<Pt2i>::const_iterator it = pts.begin ();
  xmin = it->x ();
  ymin = it->y ();
  xmax = it->x ();
  ymax = it->y ();
  while (it != pts.end ())
  {
    if (xmin > it->x ()) xmin = it->x ();
    if (xmax < it->x ()) xmax = it->x ();
    if (ymin > it->y ()) ymin = it->y ();
    if (ymax < it->y ()) ymax = it->y ();
    it++;
  }
}


std::vector<Pt2i> BiPtList::frontToBackPoints () const
{
  std::vector<Pt2i> res;
  for (std::deque<Pt2i>::const_iterator it = pts.begin ();
       it != pts.end (); it++)
    res.push_back (*it);
  return (res);
}


std::vector<Pt2i> *BiPtList::emptyVector () const
{
  return (new std::vector<Pt2i> ());
}


std::vector<Pt2i> *BiPtList::frontPoints () const
{
  // Entered from extremity to center : relevant ?
  std::vector<Pt2i> *res = new std::vector<Pt2i> ();
  std::deque<Pt2i>::const_iterator it = pts.begin ();
  for (int i = 0; i < start; i++) res->push_back (*it++);
  return res;
}


std::vector<Pt2i> *BiPtList::backPoints () const
{
  std::vector<Pt2i> *res = new std::vector<Pt2i> ();
  std::deque<Pt2i>::const_iterator it = pts.begin ();
  it += start + 1;
  for (int i = 0; i < cpt - start - 1; i++) res->push_back (*it++);
  return res;
}


EDist BiPtList::heightToEnds (const Pt2i &pt) const
{
  EDist xh = xHeightToEnds (pt);
  EDist yh = yHeightToEnds (pt);
  return (xh.lessThan (yh) ? xh : yh);
}


EDist BiPtList::xHeightToEnds (const Pt2i &pt) const
{
  int xp = pt.x (), yp = pt.y ();
  int p1x = pts.front().x (), p1y = pts.front().y ();
  int p2x = pts.back().x (), p2y = pts.back().y ();
  int ax, ay, bx, by, cx, cy;

  if (xp < p1x)
  {
    if (xp < p2x)
    {
      ax = xp;
      ay = yp;
      if (p1x < p2x)
      {
        bx = p1x;
        by = p1y;
        cx = p2x;
        cy = p2y;
      }
      else
      {
        bx = p2x;
        by = p2y;
        cx = p1x;
        cy = p1y;
      }
    }
    else
    {
      ax = p2x;
      ay = p2y;
      bx = xp;
      by = yp;
      cx = p1x;
      cy = p1y;
    }
  }
  else
  { 
    if (xp < p2x) 
    { 
      ax = p1x; 
      ay = p1y; 
      bx = xp; 
      by = yp; 
      cx = p2x; 
      cy = p2y; 
    } 
    else
    { 
      cx = xp; 
      cy = yp; 
      if (p1x < p2x) 
      { 
        ax = p1x; 
        ay = p1y; 
        bx = p2x; 
        by = p2y; 
      } 
      else 
      { 
        ax = p2x; 
        ay = p2y; 
        bx = p1x; 
        by = p1y; 
      } 
    } 
  } 
  return (EDist ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax), cx - ax));
}


EDist BiPtList::yHeightToEnds (const Pt2i &pt) const
{
  int xp = pt.x (), yp = pt.y ();
  int p1x = pts.front().x (), p1y = pts.front().y ();
  int p2x = pts.back().x (), p2y = pts.back().y ();
  int ax, ay, bx, by, cx, cy;

  if (yp < p1y)
  {
    if (yp < p2y)
    {
      ax = xp;
      ay = yp;
      if (p1y < p2y)
      {
        bx = p1x;
        by = p1y;
        cx = p2x;
        cy = p2y;
      }
      else
      {
        bx = p2x;
        by = p2y;
        cx = p1x;
        cy = p1y;
      }
    }
    else
    {
      ax = p2x;
      ay = p2y;
      bx = xp;
      by = yp;
      cx = p1x;
      cy = p1y;
    }
  }
  else
  {
    if (yp < p2y)
    {
      ax = p1x;
      ay = p1y;
      bx = xp;
      by = yp;
      cx = p2x;
      cy = p2y;
    }
    else
    {
      cx = xp;
      cy = yp;
      if (p1y < p2y)
      {
        ax = p1x;
        ay = p1y;
        bx = p2x;
        by = p2y;
      }
      else
      {
        ax = p2x;
        ay = p2y;
        bx = p1x;
        by = p1y;
      }
    }
  }
  return (EDist ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax), cy - ay));
}
