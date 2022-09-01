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

#include "digitalstraightline.h"

const int DigitalStraightLine::DSL_THIN = 1;
const int DigitalStraightLine::DSL_NAIVE = 2;
const int DigitalStraightLine::DSL_STANDARD = 3;


DigitalStraightLine::DigitalStraightLine (int a, int b, int c, int nu)
{
  this->a = a;
  this->b = b;
  if (nu < 0)
  {
    this->c = c + 1 + nu;
    this->nu = - nu;
  }
  else
  {
    this->c = c;
    this->nu = nu;
  }
  if (a < 0)
  {
    this->a = - this->a;
    this->b = - this->b;
    this->c = 1 - c - nu;
  }
  else if (a == 0 && b < 0)
  {
    this->b = - this->b;
    this->c = 1 - c - nu;
  }
  int pg = pgcd (a, (b < 0 ?  - b : b));
  if (pg != 1)
  {
    a /= pg;
    b /= pg;
    c /= pg;
    nu /= pg;
  }
}


DigitalStraightLine::DigitalStraightLine (Pt2i p1, Pt2i p2, int type)
{
  if (p1.y () < p2.y ())
  {
    a = p2.y () - p1.y ();
    b = p1.x () - p2.x ();
  }
  else
  {
    a = p1.y () - p2.y ();
    b = p2.x () - p1.x ();
    if (a == 0 && b < 0) b = -b;
  }
  int pg = pgcd (a, b < 0 ? -b : b);
  a /= pg;
  b /= pg;
  c = a * p1.x () + b * p1.y ();

  if (type == DSL_NAIVE)
  {
    nu = b;
    if (nu < 0) nu = -nu;
    if (nu < a) nu = a;
    // To match Pt2i::stepsTo method ...
    c -= nu / 2;
  }

  else if (type == DSL_STANDARD)
  {
    nu = a + (b < 0 ? -b : b);
    if ((b > 0 && a > b) || (b < 0 && a < -b))
      c -= (nu - 1) / 2;
    else
      c -= nu / 2;
  }

  else // type == DSL_THIN
    nu = 1;
}


DigitalStraightLine::DigitalStraightLine (Pt2i p1, Pt2i p2, Pt2i p3)
{
  if (p1.y () < p2.y ())
  {
    a = p2.y () - p1.y ();
    b = p1.x () - p2.x ();
  }
  else
  {
    a = p1.y () - p2.y ();
    b = p2.x () - p1.x ();
    if (a == 0 && b < 0) b = -b;
  }
  int pg = pgcd (a, b < 0 ? -b : b);
  a /= pg;
  b /= pg;
  c = a * p1.x () + b * p1.y ();
  int d = a * p3.x () + b * p3.y ();
  if (d < c)
  {
    nu = c - d + 1;
    c = d;
  }
  else nu = d - c + 1;
}


DigitalStraightLine::DigitalStraightLine (Pt2i p1, Pt2i p2,
                                          int type, int atRight)
{
  if (p1.y () < p2.y ())
  {
    a = p2.y () - p1.y ();
    b = p1.x () - p2.x ();
  }
  else
  {
    a = p1.y () - p2.y ();
    b = p2.x () - p1.x ();
    if (a == 0 && b < 0) b = -b;
  }
  int pg = pgcd (a, b < 0 ? -b : b);
  a /= pg;
  b /= pg;
  c = a * p1.x () + b * p1.y ();

  int bb = (b < 0 ? -b : b);
  c += atRight * (a < bb ? bb : a);

  if (type == DSL_NAIVE)
  {
    nu = b;
    if (nu < 0) nu = -nu;
    if (nu < a) nu = a;
    // To match Pt2i::stepsTo method ...
    if ((b > 0 && a > b) || (b < 0 && a < -b))
      c -= (nu - 1) / 2;
    else
      c -= nu / 2;
  }

  else if (type == DSL_STANDARD)
  {
    nu = a + (b < 0 ? -b : b);
    if ((b > 0 && a > b) || (b < 0 && a < -b))
      c -= (nu - 1) / 2;
    else
      c -= nu / 2;
  }

  else // type == DSL_THIN
    nu = 1;
}


DigitalStraightLine::DigitalStraightLine (const DigitalStraightLine &l)
{
  a = l.a;
  b = l.b;
  c = l.c;
  nu = l.nu;
}


int DigitalStraightLine::manhattan (Pt2i pix) const
{
  int absb = b < 0 ? -b : b;
  int per = (a < absb ? absb : a);
  int pos = a * pix.x () + b * pix.y () - c;
  if (pos < 0) return ((pos + 1 - per) / per);
  else if (pos >= nu) return ((pos + per - nu) / per);
  else return 0;
}


Pt2i DigitalStraightLine::getABoundingPoint (bool upper) const
{
  int sa = a, sb = b, u1 = 1, v1 = 0, u2 = 0, v2 = 1;
  while (sb != 0)
  {
    int r = sa % sb;
    int q = sa / sb;
    int u3 = u1 - q * u2;
    int v3 = v1 - q * v2;
    u1 = u2;
    v1 = v2;
    u2 = u3;
    v2 = v3;
    sa = sb;
    sb = r;
  }
  if (sa < 0)  // should be 1 or -1 if a and b are primal
  {
    u1 = - u1;
    v1 = - v1;
  }
  return (upper ? Pt2i (u1 * (c + nu - 1), v1 * (c + nu - 1))
                : Pt2i (u1 * c, v1 * c));
}


void DigitalStraightLine::adjustWorkArea (int &xmin, int &ymin,
                                          int &width, int &height) const
{
  (void) xmin;
  (void) ymin;
  (void) width;
  (void) height;
}


void DigitalStraightLine::getBounds (std::vector<Pt2i> &bound,
                             int xmin, int ymin, int width, int height) const
{
  getBoundPoints (bound, false, xmin, ymin, width, height);
  if (nu > period ()) getBoundPoints (bound, true, xmin, ymin, width, height);
}


void DigitalStraightLine::getBoundPoints (std::vector<Pt2i> &points,
            bool opposite, int xmin, int ymin, int width, int height) const
{
  if (opposite && nu < period ()) return;

  int x, y, dec, r;
  Pt2i pb = getABoundingPoint (opposite);
  adjustWorkArea (xmin, ymin, width, height);

  if (b > 0)   // downwards
    if (b >= a) // rather horizontal (8th octant)
    {
      x = pb.x ();
      y = pb.y ();
      dec = x <= xmin ? (xmin - x) / b : (xmin - x) / b - 1;
      x += dec * b;
      y -= dec * a;
      r = (opposite ? b - 1 : 0);

      while (x < xmin)
      {
        x++;
        r += a; if (r >= b)
        {
          y --;
          r -= b;
        }
      }
      if ((opposite || r < nu ) && y >= ymin && y < ymin + height)
        points.push_back (Pt2i (x, y));
      while (++x < xmin + width)
      {
        r += a;
        if (r >= b)
        {
          y --;
          r -= b;
        }
        if ((opposite || r < nu) && y >= ymin && y < ymin + height)
          points.push_back (Pt2i (x, y));
      }
    }
    else // rather vertical (7th octant)
    {
      x = pb.x ();
      y = pb.y ();
      dec = y >= ymin + height ? (y - ymin - height) / a
                                 : (y - ymin - height) / a - 1;
      x += dec * b;
      y -= dec * a;
      r = (opposite ? 0 : a - 1);

      while (y >= ymin + height)
      {
        y--;
        r += b;
        if (r >= a)
        {
          x ++;
          r -= a;
        }
      }

      if ((opposite || r >= a - nu) && x >= xmin && x < xmin + width)
        points.push_back (Pt2i (x, y));
      while (y-- > ymin)
      {
        r += b;
        if (r >= a)
        {
          x ++;
          r -= a;
        }
        if ((opposite || r >= a - nu) && x >= xmin && x < xmin + width)
          points.push_back (Pt2i (x, y));
      }
    }
  else   // upwards
    if (-b >= a) // rather horizontal (1st octant)
    {
      x = pb.x ();
      y = pb.y ();
      dec = x <= xmin ? (x - xmin) / b : (x - xmin) / b - 1;
      x -= dec * b;
      y += dec * a;
      r = (opposite ? b + 1 : 0);

      while (x < xmin)
      {
        x++;
        r -= a;
        if (r <= b)
        {
          y ++;
          r -= b;
        }
      }

      if ((opposite || r > -nu) && y >= ymin && y < ymin + height)
        points.push_back (Pt2i (x, y));
      while (++x < xmin + width)
      {
        r -= a;
        if (r <= b)
        {
          y ++;
          r -= b;
        }
        if ((opposite || r > -nu) && y >= ymin && y < ymin + height)
          points.push_back (Pt2i (x, y));
      }
    }
    else // rather vertical (2nd octant)
    {
      x = pb.x ();
      y = pb.y ();
      dec = y > ymin ? (ymin - y) / a - 1 : (ymin - y) / a;
      x -= dec * b;
      y += dec * a;
      r = (opposite ? 0 : a - 1);

      while (y < ymin)
      {
        y++;
        r -= b;
        if (r >= a)
        {
          x ++;
          r -= a;
        }
      }

      if ((opposite || r >= a - nu) && x >= xmin && x < xmin + width)
        points.push_back (Pt2i (x, y));
      while (++y < ymin + height)
      {
        r -= b;
        if (r >= a)
        {
          x ++;
          r -= a;
        }
        if ((opposite || r >= a - nu) && x >= xmin && x < xmin + width)
          points.push_back (Pt2i (x, y));
      }
    }
}


const Pt2i DigitalStraightLine::centerOfIntersection (
                                     DigitalStraightLine *l) const
{
  int den = a * l->b - b * l->a;
  if (den == 0) return (Pt2i (0, 0));
  return (Pt2i (
    ((c + nu / 2) * l->b - b * (l->c + l->nu / 2) + den / 2) / den,
    (a * (l->c + l->nu / 2) - (c + nu / 2) * l->a + den / 2) / den));
}


const Pt2i DigitalStraightLine::centerOfIntersection (
                                     Pt2i p1, Pt2i p2) const
{
  int sa = p2.y () - p1.y ();
  int sb = p1.x () - p2.x ();
  if (sa == 0)
  {
    if (sb == 0) return (Pt2i (0, 0));
    if (sb < 0) sb = -sb;
  }
  if (sa < 0)
  {
    sa = -sa;
    sb = -sb;
  }
  int pg = pgcd (sa, sb < 0 ? -sb : sb);
  sa /= pg;
  sb /= pg;

  int den = a * sb - b * sa;
  if (den == 0) return (Pt2i (0, 0));
  int sc = sa * p1.x () + sb * p1.y ();
  return (Pt2i (((c + nu / 2) * sb - b * sc + den / 2) / den,
                 (a * sc - (c + nu / 2) * sa + den / 2) / den));
}


bool DigitalStraightLine::owns (const Pt2i &p) const
{
  int val = a * p.x () + b * p.y () - c;
  return (val >= 0 && val < nu);
}


bool DigitalStraightLine::owns (const Pt2i &p1, const Pt2i &p2) const
{
  int val1 = a * p1.x () + b * p1.y () - c;
  int val2 = a * p2.x () + b * p2.y () - c;
  return (val1 < val2 ? val1 <= 0 && val2 < nu
                      : val2 <= 0 && val1 < nu);
}


bool DigitalStraightLine::crosses (const Pt2i &p1, const Pt2i &p2) const
{
  int val1 = a * p1.x () + b * p1.y () - c;
  int val2 = a * p2.x () + b * p2.y () - c;
  return (val1 < val2 ? val2 >= 0 && val1 < nu
                      : val1 >= 0 && val2 < nu);
}


int DigitalStraightLine::sideShift (Pt2i pt) const
{
  int rmd = a * pt.x () + b * pt.y () - c;
  if (a == 0) rmd = -rmd;
  return (rmd < 0 ? (rmd + 1) / nu - 1 : rmd / nu);
}


int DigitalStraightLine::sideShiftSafe (Pt2i pt) const
{
  int64_t rmd = a * ((int64_t) pt.x ()) + b * ((int64_t) pt.y ()) - c;
  if (a == 0) rmd = -rmd;
  return (rmd < 0 ? (int) ((rmd + 1) / nu) - 1 : (int) (rmd / nu));
}


int DigitalStraightLine::rmd (Pt2i pt) const
{
  return (a * pt.x () + b * pt.y () - c);
}


int DigitalStraightLine::pgcd (int a, int b)
{
  int r;
  while (b != 0)
  {
    r = a % b;
    a = b;
    b = r;
  }
  return (a);
}
