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

#include "directionalscannero7.h"


DirectionalScannerO7::DirectionalScannerO7 (
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


DirectionalScannerO7::DirectionalScannerO7 (
                          int xmin, int ymin, int xmax, int ymax,
                          int a, int b, int c1, int c2,
                          int nbs, bool *steps, int cx, int cy)
                    : DirectionalScanner (xmin, ymin, xmax, ymax,
                                          nbs, steps, cx, cy)
{
  this->dla = a;
  this->dlb = b;
  if (c2 < c1)
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
    if (*st) lcy --;
    lcx --;
  }
  while (dla * lcx + dlb * lcy > c1);
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


DirectionalScannerO7::DirectionalScannerO7 (
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
    if (*st) lcy --;
    lcx --;
  }
  lst2 = st;
  rst2 = st;

  // Looking for the upper leaning line
  st = steps;
  while (w_2-- > 0)
  {
    if (*st) cy++;
    cx++;
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


DirectionalScannerO7::DirectionalScannerO7 (DirectionalScannerO7 *ds)
                    : DirectionalScanner (ds)
{
  lst1 = ds->lst1;
  rst1 = ds->rst1;
  lstop = ds->lstop;
  rstop = ds->rstop;
}


DirectionalScanner *DirectionalScannerO7::getCopy ()
{
  return (new DirectionalScannerO7 (this));
}


int DirectionalScannerO7::first (std::vector<Pt2i> &scan) const
{
  int x = lcx, y = lcy;      // Current position coordinates
  bool *nst = lst2;          // Current step in scan direction (jpts)

  while ((y < ymin || x < xmin) && dla * x + dlb * y <= dlc2)
  {
    if (*nst) y++;
    x++;
    if (++nst >= fs) nst = steps;
  }
  while (dla * x + dlb * y <= dlc2 && y < ymax && x < xmax)
  {
    scan.push_back (Pt2i (x, y));
    if (*nst) y++;
    x++;
    if (++nst >= fs) nst = steps;
  }
  return ((int) (scan.size ()));
}


int DirectionalScannerO7::nextOnLeft (std::vector<Pt2i> &scan)
{
  // Prepares the next scan
  if (clearance) scan.clear ();
  if (lstop)
  {
    lcx --;
    if (--lst2 < steps) lst2 = fs - 1;
    lstop = false;
  }
  else
  {
    if (--lst1 < steps) lst1 = fs - 1;
    lcy ++;
    if (*lst1)
    {
      if (--lst2 < steps) lst2 = fs - 1;
      if (*lst2)
      {
        if (++lst2 >= fs) lst2 = steps;
        lstop = true;
      }
      else lcx --;
    }
  }

  // Computes the next scan
  int x = lcx;
  int y = lcy;
  bool *nst = lst2;
  while ((y < ymin || x < xmin) && dla * x + dlb * y <= dlc2)
  {
    if (*nst) y++;
    x++;
    if (++nst >= fs) nst = steps;
  }
  while (dla * x + dlb * y <= dlc2 && y < ymax && x < xmax)
  {
    scan.push_back (Pt2i (x, y));
    if (*nst) y++;
    x++;
    if (++nst >= fs) nst = steps;
  }
  return ((int) (scan.size ()));
}


int DirectionalScannerO7::nextOnRight (std::vector<Pt2i> &scan)
{
  // Prepares the next scan
  if (clearance) scan.clear ();
  if (rstop)
  {
    rcy --;
    rstop = false;
  }
  else
  {
    rcy --;
    if (*rst1)
    {
      rcx ++;
      if (*rst2)
      {
        rcy ++;
        rstop = true;
      }
      if (++rst2 >= fs) rst2 = steps;
    }
    if (++rst1 >= fs) rst1 = steps;
  }

  // Computes the next scan
  int x = rcx;
  int y = rcy;
  bool *nst = rst2;
  while ((y < ymin || x < xmin) && dla * x + dlb * y <= dlc2)
  {
    if (*nst) y++;
    x++;
    if (++nst == fs) nst = steps;
  }
  while (dla * x + dlb * y <= dlc2 && y < ymax && x < xmax)
  {
    scan.push_back (Pt2i (x, y));
    if (*nst) y++;
    x++;
    if (++nst == fs) nst = steps;
  }
  return ((int) (scan.size ()));
}


Pt2i DirectionalScannerO7::locate (const Pt2i &pt) const
{
  int x = ccx, y = ccy;      // Current position coordinates
  bool *nst = steps;         // Current step in scan direction (jpts)
  int cx = pt.x () - x, cy = 0;
  bool *st1 = steps;
  bool *st2 = steps;
  
  if (cx >= 0)
  {
    // Climbs the first scan up
    while (x < pt.x ())
    {
      if (*nst) y++;
      x++;
      if (++nst >= fs) nst = steps;
    }
  }
  else
  {
    // Climbs the first scan down
    while (x < pt.x ())
    {
      x--;
      if (--nst < steps) nst = fs - 1;
      if (*nst) y--;
    }
  }
  cy = y - pt.y ();

  // Comes back to scan origin
  x = ccx;
  y = ccy;
  int ny = cy;
  bool trans = false;
  while (ny != 0)
  {
    // Jumps leftwards along scan bound
    if (cy < 0)
    {
      if (trans)
      {
        y --;
        trans = false;
      }
      else
      {
        if (--st1 < steps) st1 = fs - 1;
        y --;
        if (*st1)
        {
          x --;
          if (*st2)
          {
            y ++;
            trans = true;
          }
          if (++st2 >= fs) st2 = steps;
        }
      }
      ny ++;
    }
    else
    // Jumps rightwards along scan bound
    {
      if (trans)
      {
        x ++;
        if (--st2 < steps) st2 = fs - 1;
        trans = false;
      }
      else
      {
        y ++;
        if (*st1)
        {
          if (--st2 < steps) st2 = fs - 1;
          if (*st2)
          {
            if (++st2 >= fs) st2 = steps;
            trans = true;
          }
          else x ++;
        }
        if (++st1 >= fs) st1 = steps;
      }
      ny --;
    }
  }

  return (Pt2i (cy, pt.x () - x));
}
