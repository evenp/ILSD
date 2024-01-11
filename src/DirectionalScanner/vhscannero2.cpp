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

#include "vhscannero2.h"


VHScannerO2::VHScannerO2 (int xmin, int ymin, int xmax, int ymax,
                          int a, int b, int c,
                          int nbs, bool *steps, int sx, int sy)
                    : AdaptiveScannerO2 (xmin, ymin, xmax, ymax,
                                         a, b, c, nbs, steps, sx, sy)
{
}


VHScannerO2::VHScannerO2 (int xmin, int ymin, int xmax, int ymax,
                          int a, int b, int c1, int c2,
                          int nbs, bool *steps, int cx, int cy)
{
  this->xmin = xmin;
  this->xmax = xmax;
  this->ymin = ymin;
  this->ymax = ymax;
  this->nbs = nbs;
  this->steps = steps;
  lcx = cx;
  lcy = cy;
  rcx = cx;
  rcy = cy;
  this->dla = a;
  this->dlb = b;
  if (c2 > c1)
  {
    this->dlc1 = c2;
    this->dlc2 = c1;
    c1 = c2;
  }
  else
  {
    this->dlc1 = c1;
    this->dlc2 = c2;
  }

  this->templ_a = a;
  this->templ_b = b;
  this->templ_nu = this->dlc1 - this->dlc2;

  // Looking for the central scan start position
  do
  {
    lcx ++;
  }
  while (dla * lcx + dlb * lcy < c1);

  rcx = lcx;
  rcy = lcy;
  lst2 = steps;
  rst2 = steps;
  fs = steps + nbs;
}


VHScannerO2::VHScannerO2 (int xmin, int ymin, int xmax, int ymax,
                          int a, int b, int nbs, bool *steps,
                          int cx, int cy, int length)
{
  this->xmin = xmin;
  this->xmax = xmax;
  this->ymin = ymin;
  this->ymax = ymax;
  this->nbs = nbs;
  this->steps = steps;
  lcx = cx;
  lcy = cy;
  rcx = cx;
  rcy = cy;
  this->dla = a;
  this->dlb = b;
  fs = steps + nbs;
  int w_2 = (length + 1) / 2;

  // Looking for the central scan start position
  for (int i = 0; i < w_2; i++)
  {
    lcx ++;
  }
  dlc1 = dla * lcx + dlb * lcy;

  // Looking for the upper leaning line
  while (w_2-- > 0)
  {
    cx--;
  }
  dlc2 = dla * cx + dlb * cy;

  this->templ_a = a;
  this->templ_b = b;
  this->templ_nu = this->dlc1 - this->dlc2;

  rcx = lcx;
  rcy = lcy;
  lst2 = steps;
  rst2 = steps;
}


VHScannerO2::VHScannerO2 (VHScannerO2 *ds) : AdaptiveScannerO2 (ds)
{
}


DirectionalScanner *VHScannerO2::getCopy ()
{
  return (new VHScannerO2 (this));
}


int VHScannerO2::first (std::vector<Pt2i> &scan) const
{
  int x = lcx, y = lcy;      // Current position coordinates

  while (x >= xmax && dla * x + dlb * y >= dlc2)
  {
    x--;
  }
  while (dla * x + dlb * y >= dlc2 && x >= xmin)
  {
    scan.push_back (Pt2i (x, y));
    x--;
  }
  return ((int) (scan.size ()));
}


int VHScannerO2::nextOnLeft (std::vector<Pt2i> &scan)
{
  // Prepares the next scan
  if (clearance) scan.clear ();
  lcy --;
  if (lcy < ymin) return 0;

  // Whenever the control line changed
  while (lcx > xmin && dla * lcx + dlb * lcy > dlc1)
  {
    lcx --;
  }
  while (lcx < xmax - 1 && dla * lcx + dlb * lcy < dlc1)
  {
    lcx ++;
  }

  // Computes the next scan
  int x = lcx;
  int y = lcy;
  while (x >= xmax && dla * x + dlb * y >= dlc2)
  {
    x--;
  }
  while (dla * x + dlb * y >= dlc2 && x >= xmin)
  {
    scan.push_back (Pt2i (x, y));
    x--;
  }
  return ((int) (scan.size ()));
}


int VHScannerO2::nextOnRight (std::vector<Pt2i> &scan)
{
  // Prepares the next scan
  if (clearance) scan.clear ();
  rcy ++;
  if (rcy >= ymax) return 0;

  while (rcx > xmin && dla * rcx + dlb * rcy > dlc1)
  {
    rcx --;
  }
  while (rcx < xmax - 1 && dla * rcx + dlb * rcy < dlc1)
  {
    rcx ++;
  }

  // Computes the next scan
  int x = rcx;
  int y = rcy;
  while ((y < ymin || x >= xmax) && dla * x + dlb * y >= dlc2)
  {
    x--;
  }
  while (dla * x + dlb * y >= dlc2 && y < ymax && x >= xmin)
  {
    scan.push_back (Pt2i (x, y));
    x--;
  }
  return ((int) (scan.size ()));
}


int VHScannerO2::skipLeft (std::vector<Pt2i> &scan, int skip)
{
  // Prepares the next scan
  if (clearance) scan.clear ();
  lcy -= skip;
  if (lcy < ymin) return 0;

  // Whenever the control line changed
  while (lcx > xmin && dla * lcx + dlb * lcy > dlc1)
  {
    lcx --;
  }
  while (lcx < xmax - 1 && dla * lcx + dlb * lcy < dlc1)
  {
    lcx ++;
  }

  // Computes the next scan
  int x = lcx;
  int y = lcy;
  while (x >= xmax && dla * x + dlb * y >= dlc2)
  {
    x--;
  }
  while (dla * x + dlb * y >= dlc2 && x >= xmin)
  {
    scan.push_back (Pt2i (x, y));
    x--;
  }
  return ((int) (scan.size ()));
}


int VHScannerO2::skipRight (std::vector<Pt2i> &scan, int skip)
{
  // Prepares the next scan
  if (clearance) scan.clear ();
  rcy += skip;
  if (rcy >= ymax) return 0;

  while (rcx > xmin && dla * rcx + dlb * rcy > dlc1)
  {
    rcx --;
  }
  while (rcx < xmax - 1 && dla * rcx + dlb * rcy < dlc1)
  {
    rcx ++;
  }

  // Computes the next scan
  int x = rcx;
  int y = rcy;
  while ((y < ymin || x >= xmax) && dla * x + dlb * y >= dlc2)
  {
    x--;
  }
  while (dla * x + dlb * y >= dlc2 && y < ymax && x >= xmin)
  {
    scan.push_back (Pt2i (x, y));
    x--;
  }
  return ((int) (scan.size ()));
}


void VHScannerO2::skipLeft (int skip)
{
  lcy -= skip - 1;
}


void VHScannerO2::skipRight (int skip)
{
  rcy += skip - 1;
}
