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

#include "directionalscannero1.h"


DirectionalScannerO1::DirectionalScannerO1 (
                          int xmin, int ymin, int xmax, int ymax,
                          int a, int b, int c,
                          int nbs, bool *steps, int sx, int sy)
                    : DirectionalScanner (xmin, ymin, xmax, ymax,
                                          nbs, steps, sx, sy)
{
  this->dla = a;
  this->dlb = b;
  this->dlc2 = c;

  lst1 = steps;
  rst1 = steps;
  lst2 = steps;
  rst2 = steps;
  fs = steps + nbs;
  lstop = false; 
  rstop = false;
}


DirectionalScannerO1::DirectionalScannerO1 (
                          int xmin, int ymin, int xmax, int ymax,
                          int a, int b, int c1, int c2,
                          int nbs, bool *steps, int cx, int cy)
                    : DirectionalScanner (xmin, ymin, xmax, ymax,
                                          nbs, steps, cx, cy)
{
  this->dla = a;
  this->dlb = b;
  if (c2 > c1)
  {
    this->dlc2 = c1;
    c1 = c2;
  }
  else this->dlc2 = c2;

  // Looking for the central scan start position
  bool *st = steps + nbs;
  do
  {
    if (--st < steps) st = steps + nbs - 1;
    if (*st) lcx ++;
    lcy --;
  }
  while (dla * lcx + dlb * lcy < c1);
  lst2 = st;
  rst2 = st;

  rcx = lcx;
  rcy = lcy;
/** ZZZ */
ccx = lcx;
ccy = lcy;
/** ZZZ */
  lst1 = steps;
  rst1 = steps;
  fs = steps + nbs;
  lstop = false; 
  rstop = false;
}


DirectionalScannerO1::DirectionalScannerO1 (
                          int xmin, int ymin, int xmax, int ymax,
                          int a, int b,
                          int nbs, bool *steps, int cx, int cy, int length)
                    : DirectionalScanner (xmin, ymin, xmax, ymax,
                                          nbs, steps, cx, cy)
{
  this->dla = a;
  this->dlb = b;
  fs = steps + nbs;
  int w_2 = (length + 1) / 2;

  // Looking for the central scan start position
  bool *st = steps + nbs;
  for (int i = 0; i < w_2; i++)
  {
    if (--st < steps) st = steps + nbs - 1;
    if (*st) lcx ++;
    lcy --;
  }
  lst2 = st;
  rst2 = st;

  // Looking for the upper leaning line
  st = steps;
  while (w_2-- > 0)
  {
    if (*st) cx--;
    cy++;
    if (++st >= fs) st = steps;
  }
  dlc2 = dla * cx + dlb * cy;

  rcx = lcx;
  rcy = lcy;
/** ZZZ */
ccx = lcx;
ccy = lcy;
/** ZZZ */
  lst1 = steps;
  rst1 = steps;
  lstop = false; 
  rstop = false;
}


DirectionalScannerO1::DirectionalScannerO1 (DirectionalScannerO1 *ds)
                    : DirectionalScanner (ds)
{
  lst1 = ds->lst1;
  rst1 = ds->rst1;
  lstop = ds->lstop;
  rstop = ds->rstop;
}


DirectionalScanner *DirectionalScannerO1::getCopy ()
{
  return (new DirectionalScannerO1 (this));
}


int DirectionalScannerO1::first (std::vector<Pt2i> &scan) const
{
  int x = lcx, y = lcy;      // Current position coordinates
  bool *nst = lst2;          // Current step in scan direction (jpts)

  while ((x >= xmax || y < ymin) && dla * x + dlb * y >= dlc2)
  {
    if (*nst) x--;
    y++;
    if (++nst >= fs) nst = steps;
  }
  while (dla * x + dlb * y >= dlc2 && x >= xmin && y < ymax)
  {
    scan.push_back (Pt2i (x, y));
    if (*nst) x--;
    y++;
    if (++nst >= fs) nst = steps;
  }
  return ((int) (scan.size ()));
}


int DirectionalScannerO1::nextOnLeft (std::vector<Pt2i> &scan)
{
  // Prepares the next scan
  if (clearance) scan.clear ();
  if (lstop)
  {
    lcy --;
    if (--lst2 < steps) lst2 = fs - 1;
    lstop = false;
  }
  else
  {
    if (--lst1 < steps) lst1 = fs - 1;
    lcx --;
    if (*lst1)
    {
      lcy --;
      if (--lst2 < steps) lst2 = fs - 1;
      if (*lst2)
      {
        if (++lst2 >= fs) lst2 = steps;
        lcy ++;
        lstop = true;
      }
    }
  }

  // Computes the next scan
  int x = lcx;
  int y = lcy;
  bool *nst = lst2;
  while ((x >= xmax || y < ymin) && dla * x + dlb * y >= dlc2)
  {
    if (*nst) x--;
    y++;
    if (++nst >= fs) nst = steps;
  }
  while (dla * x + dlb * y >= dlc2 && x >= xmin && y < ymax)
  {
    scan.push_back (Pt2i (x, y));
    if (*nst) x--;
    y++;
    if (++nst >= fs) nst = steps;
  }
  return ((int) (scan.size ()));
}


int DirectionalScannerO1::nextOnRight (std::vector<Pt2i> &scan)
{
  // Prepares the next scan
  if (clearance) scan.clear ();
  if (rstop)
  {
    rcx ++;
    rstop = false;
  }
  else
  {
    rcx ++;
    if (*rst1)
    {
      if (*rst2)
      {
        rcx --;
        rstop = true;
      }
      rcy ++;
      if (++rst2 >= fs) rst2 = steps;
    }
    if (++rst1 >= fs) rst1 = steps;
  }

  // Computes the next scan
  int x = rcx;
  int y = rcy;
  bool *nst = rst2;
  while ((x >= xmax || y < ymin) && dla * x + dlb * y >= dlc2)
  {
    if (*nst) x--;
    y++;
    if (++nst >= fs) nst = steps;
  }
  while (dla * x + dlb * y >= dlc2 && x >= xmin && y < ymax)
  {
    scan.push_back (Pt2i (x, y));
    if (*nst) x--;
    y++;
    if (++nst >= fs) nst = steps;
  }
  return ((int) (scan.size ()));
}


int DirectionalScannerO1::skipLeft (std::vector<Pt2i> &scan, int skip)
{
  // Prepares the next scan
  if (clearance) scan.clear ();
  while (skip-- != 0)
  {
    if (lstop)
    {
      lcy --;
      if (--lst2 < steps) lst2 = fs - 1;
      lstop = false;
    }
    else
    {
      if (--lst1 < steps) lst1 = fs - 1;
      lcx --;
      if (*lst1)
      {
        lcy --;
        if (--lst2 < steps) lst2 = fs - 1;
        if (*lst2)
        {
          if (++lst2 >= fs) lst2 = steps;
          lcy ++;
          lstop = true;
        }
      }
    }
  }

  // Computes the next scan
  int x = lcx;
  int y = lcy;
  bool *nst = lst2;
  while ((x >= xmax || y < ymin) && dla * x + dlb * y >= dlc2)
  {
    if (*nst) x--;
    y++;
    if (++nst >= fs) nst = steps;
  }
  while (dla * x + dlb * y >= dlc2 && x >= xmin && y < ymax)
  {
    scan.push_back (Pt2i (x, y));
    if (*nst) x--;
    y++;
    if (++nst >= fs) nst = steps;
  }
  return ((int) (scan.size ()));
}


int DirectionalScannerO1::skipRight (std::vector<Pt2i> &scan, int skip)
{
  // Prepares the next scan
  if (clearance) scan.clear ();
  while (skip-- != 0)
  {
    if (rstop)
    {
      rcx ++;
      rstop = false;
    }
    else
    {
      rcx ++;
      if (*rst1)
      {
        if (*rst2)
        {
          rcx --;
          rstop = true;
        }
        rcy ++;
        if (++rst2 >= fs) rst2 = steps;
      }
      if (++rst1 >= fs) rst1 = steps;
    }
  }

  // Computes the next scan
  int x = rcx;
  int y = rcy;
  bool *nst = rst2;
  while ((x >= xmax || y < ymin) && dla * x + dlb * y >= dlc2)
  {
    if (*nst) x--;
    y++;
    if (++nst >= fs) nst = steps;
  }
  while (dla * x + dlb * y >= dlc2 && x >= xmin && y < ymax)
  {
    scan.push_back (Pt2i (x, y));
    if (*nst) x--;
    y++;
    if (++nst >= fs) nst = steps;
  }
  return ((int) (scan.size ()));
}


Pt2i DirectionalScannerO1::locate (const Pt2i &pt) const
{
  int x = ccx, y = ccy;      // Current position coordinates
  bool *nst = steps;         // Current step in scan direction (jpts)
  int cx = 0, cy = pt.y () - y;
  bool *st1 = steps;
  bool *st2 = steps;
  
  if (cy >= 0)
  {
    // Climbs the first scan up
    while (y < pt.y ())
    {
      if (*nst) x--;
      y++;
      if (++nst >= fs) nst = steps;
    }
  }
  else
  {
    // Climbs the first scan down
    while (y > pt.y ())
    {
      y--;
      if (--nst < steps) nst = fs -1;
      if (*nst) x++;
    }
  }
  cx = pt.x () - x;

  // Comes back to scan origin
  x = ccx;
  y = ccy;
  int nx = cx;
  bool trans = false;
  while (nx != 0)
  {
    // Jumps leftwards along scan bound
    if (cx < 0)
    {
      if (trans)
      {
        y --;
        if (--st2 < steps) st2 = fs - 1;
        trans = false;
      }
      else
      {
        if (--st1 < steps) st1 = fs - 1;
        x --;
        if (*st1)
        {
          y --;
          if (--st2 < steps) st2 = fs - 1;
          if (*st2)
          {
            if (++st2 >= fs) st2 = steps;
            y ++;
            trans = true;
          }
        }
      }
      nx ++;
    }
    else
    // Jumps rightwards along scan bound
    {
      if (trans)
      {
        x ++;
        trans = false;
      }
      else
      {
        x ++;
        if (*st1)
        {
          if (*st2)
          { 
            x --;
            trans = true;
          }
          y ++;
          if (++st2 >= fs) st2 = steps;
        }
        if (++st1 >= fs) st1 = steps;
      }
      nx --;
    }
  }

  return (Pt2i (cx, pt.y () - y));
}
